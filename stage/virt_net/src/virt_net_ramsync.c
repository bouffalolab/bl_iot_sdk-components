#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>

#include <lwip/api.h>
#include <lwip/opt.h>
#include <lwip/def.h>
#include <lwip/mem.h>
#include <lwip/pbuf.h>
#include <lwip/tcpip.h>
#include <lwip/dns.h>
#include <lwip/etharp.h>
#if LWIP_IPV6
#include <lwip/ethip6.h>
#include <lwip/dhcp6.h>
#endif
#include <ethernetif.h>

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <timers.h>
#include <blog.h>

#include "bitset.h"

#include "virt_net.h"
#include "pkg_protocol.h"
#include <ramsync_upper.h>
#include "blog.h"

#ifndef CFG_VIRT_NET_STACK_DEPTH
#define CFG_VIRT_NET_STACK_DEPTH 1024
#endif /* CFG_VIRT_NET_STACK_DEPTH */

struct virt_net_ramsync;
struct virt_net_custom_pbuf {
  struct pbuf_custom p;
  struct virt_net_ramsync *sobj;
};

#define VIRT_NET_RXBUFF_OFFSET (sizeof(struct virt_net_custom_pbuf))

/* 头部的假数据，绕过DMA/SPI FIFO缓存 */
#if defined(CFG_USE_PSRAM)
#define VIRT_NET_RXBUFF_CNT (16)
#else
#define VIRT_NET_RXBUFF_CNT (4)
#endif
#define VIRT_NET_BUFF_SIZE (TP_PAYLOAD_LEN + VIRT_NET_RXBUFF_OFFSET) /* + sizeof(pbuf header) */
#define VIRT_NET_MAX_PENDING_CMD (32)

BITSET_DEFINE(rx_buf_ind, VIRT_NET_RXBUFF_CNT);

typedef uint8_t (*rx_buff_t)[VIRT_NET_BUFF_SIZE];

static int inited = 0;

/* semaphore slot */
static SemaphoreHandle_t sem_slot[VIRT_NET_MAX_PENDING_CMD];
static void *sem_context[VIRT_NET_MAX_PENDING_CMD];
static SemaphoreHandle_t slot_mutex;
static StaticSemaphore_t xSlotMutexBuffer;

static uint8_t* tx_buffer;

struct virt_net_ramsync {
  struct virt_net vnet;

  tp_uramsync_t ramsync_ctx;

  volatile int running;
  TaskHandle_t task_virt_net;

  rx_buff_t rx_buff;

  SemaphoreHandle_t rx_buf_ind_mutex;
  uint8_t dhcp_start;
  uint8_t sta_linkup;
  TimerHandle_t dhcp_timer;
  struct rx_buf_ind rx_buf_ind;
#if LWIP_IPV6
  struct dhcp6 vnet_dhcp6;
#endif
};

static int __virt_net_ramsync_init(virt_net_t obj);
static int __virt_net_ramsync_deinit(virt_net_t obj);
static void __virt_net_task(void *pvParameters);

static inline int acquire_semaphore_slot(SemaphoreHandle_t sem) {
  int idx;
  assert(sem != NULL);

  xSemaphoreTake(slot_mutex, portMAX_DELAY);
  for (idx = 0; idx < (sizeof(sem_slot) / sizeof(sem_slot[0])); idx++) {
    if (sem_slot[idx] != NULL) {
      continue;
    }

    sem_slot[idx] = sem;
    xSemaphoreGive(slot_mutex);
    return idx;
  }
  xSemaphoreGive(slot_mutex);

  return -1;
}

static inline SemaphoreHandle_t release_semaphore_slot(int idx) {
  assert(idx >= 0 && idx < (sizeof(sem_slot) / sizeof(sem_slot[0])));
  SemaphoreHandle_t ret = NULL;

  xSemaphoreTake(slot_mutex, portMAX_DELAY);
  if (sem_slot[idx]) {
    ret = sem_slot[idx];
    sem_slot[idx] = NULL;
  }
  xSemaphoreGive(slot_mutex);

  return ret;
}

static uint8_t *__alloc_rxbuf(struct virt_net_ramsync *sobj) {
  assert(sobj != NULL);

  xSemaphoreTake(sobj->rx_buf_ind_mutex, portMAX_DELAY);

  int idx = BIT_FFS(VIRT_NET_RXBUFF_CNT, &sobj->rx_buf_ind);

  if (idx == 0) {
    xSemaphoreGive(sobj->rx_buf_ind_mutex);

    printf("rx buff alloc failed!\r\n");
    return NULL;
  }

  BIT_CLR(VIRT_NET_RXBUFF_CNT, idx - 1, &sobj->rx_buf_ind);
  xSemaphoreGive(sobj->rx_buf_ind_mutex);

  return sobj->rx_buff[idx - 1];
}

static void __free_rxbuf(struct virt_net_ramsync *sobj, uint8_t *buf) {
  uint32_t idx;
  assert(sobj != NULL);

  idx = (rx_buff_t)buf - sobj->rx_buff;

  assert(idx < VIRT_NET_RXBUFF_CNT);
  assert(sobj->rx_buff[idx] == buf);

  xSemaphoreTake(sobj->rx_buf_ind_mutex, portMAX_DELAY);
  assert(!BIT_ISSET(VIRT_NET_RXBUFF_CNT, idx, &sobj->rx_buf_ind));

  BIT_SET(VIRT_NET_RXBUFF_CNT, idx, &sobj->rx_buf_ind);
  xSemaphoreGive(sobj->rx_buf_ind_mutex);
}

/* call event callback */
static inline int event_propagate(struct virt_net_ramsync *sobj, enum virt_net_event_code code, void *param) {
  if (sobj->vnet.event_cb) {
      return sobj->vnet.event_cb(&sobj->vnet, code, param);
  }
  return 0;
}

void virt_net_spi_destroy(virt_net_t *obj) 
{
  assert(obj != NULL && *obj != NULL);
  struct virt_net_ramsync *sobj = (struct virt_net_ramsync *)*obj;
  vPortFree(sobj);
  *obj = NULL;
}

static void netif_status_callback(struct netif *netif) {

  if (netif->flags & NETIF_FLAG_UP) {
    /** interface is up status */
    struct virt_net_ramsync *sobj = (struct virt_net_ramsync *)netif->state;

#if LWIP_IPV4
    if (!ip4_addr_isany(netif_ip4_addr(netif))){
      if (sobj->dhcp_start) {
        xTimerStop(sobj->dhcp_timer, 0);
        xTimerDelete(sobj->dhcp_timer, 0);
        sobj->dhcp_start = 0;
      }

      event_propagate(sobj, VIRT_NET_EV_ON_GOT_IP, (void *)netif);
    }
#endif
  }
  else {
    printf("Interface is down status.\r\n");
  }
}


static err_t __link_output(struct netif *netif, struct pbuf *p) 
{
  struct virt_net_ramsync *sobj = (struct virt_net_ramsync *)netif->state;
  struct pbuf *curr;
  unsigned int offset = 0;
  uint16_t subtype;
  struct pkg_protocol *pkg_header = (struct pkg_protocol *)tx_buffer;
  assert(sobj != NULL);

  subtype = 0x2000;
  memcpy(pkg_header->payload + offset, &subtype, 2);
  offset = 2;

  for (curr = p; curr != NULL; curr = curr->next) {
    memcpy(pkg_header->payload + offset, curr->payload, curr->len);
    offset += curr->len;
  }

  pkg_header->type = PKG_DATA_FRAME;
  pkg_header->length = p->tot_len + 2 + sizeof(struct pkg_protocol);

  uramsync_tx_push_toback(&sobj->ramsync_ctx, tx_buffer,
                          pkg_header->length + 4, URAMSYNC_FOREVER);
  blog_info("__link_output pkg_header->len:%d type:0x%x\r\n", pkg_header->length, pkg_header->type);

  return ERR_OK;
}

static void pbuf_free_spi_custom_fn(struct pbuf *p) {
  struct virt_net_custom_pbuf *spbuf = (struct virt_net_custom_pbuf *)p;
  __free_rxbuf(spbuf->sobj, (void *)p);

  vTaskResume(spbuf->sobj->task_virt_net);
}

static int pkg_protocol_data_handler(struct virt_net_ramsync *sobj, struct pkg_protocol *pkg_header) {
  struct pbuf *pbuf;
  struct virt_net_custom_pbuf *spbuf = (struct virt_net_custom_pbuf *)((uint32_t)pkg_header - VIRT_NET_RXBUFF_OFFSET);

  memset(spbuf, 0, sizeof(struct virt_net_custom_pbuf));
  spbuf->p.custom_free_function = pbuf_free_spi_custom_fn;
  spbuf->sobj = sobj;

  pbuf = pbuf_alloced_custom(PBUF_RAW, pkg_header->length, PBUF_REF, &spbuf->p,
                             (uint8_t *)pkg_header->payload, pkg_header->length);

  if (!sobj->vnet.netif.input) {
    return -1;
  }
  if (sobj->vnet.netif.input(pbuf, &sobj->vnet.netif) != ERR_OK) {
    printf("handle to tcpip stack failed\r\n");
    return -1;
  }

  return 0;
}

static void __timer_callback(TimerHandle_t handle)
{
    struct virt_net_ramsync *sobj = (struct virt_net_ramsync *)pvTimerGetTimerID(handle);

    printf("Dhcp timeout, Ip Got Failed.\r\n");
    if (sobj->vnet.ctrl) {
        sobj->vnet.ctrl(&sobj->vnet, VIRT_NET_CTRL_DISCONNECT_AP);
    }
}

static void wifi_info_gate(struct netif* netif, uint32_t *ip, uint32_t *gw, uint32_t *mask)
{
    *ip = netif_ip4_addr(netif)->addr;
    *mask = netif_ip4_netmask(netif)->addr;
    *gw = netif_ip4_gw(netif)->addr;
}

static int pkg_protocol_cmd_handler(struct virt_net_ramsync *sobj, struct pkg_protocol *pkg_header) 
{
  struct pkg_protocol_cmd *pkg_cmd = (struct pkg_protocol_cmd *)pkg_header->payload;
  SemaphoreHandle_t sem;

  blog_info("pkg_protocol_cmd_handler cmd:0x%x.\r\n", pkg_cmd->cmd);
  switch (pkg_cmd->cmd) {
  case VIRT_NET_CTRL_AP_CONNECTED_IND:
    {
      netifapi_netif_set_link_up((&sobj->vnet.netif));
      sobj->sta_linkup = 1;

      blog_info("start dhcp...\r\n");
      netifapi_netif_set_up(&sobj->vnet.netif);
#if LWIP_IPV6
      netif_create_ip6_linklocal_address(&sobj->vnet.netif, 1);
#if LWIP_IPV6_AUTOCONFIG
      sobj->vnet.netif.ip6_autoconfig_enabled = 1;
#endif
      dhcp6_set_struct(&sobj->vnet.netif, &sobj->vnet_dhcp6);
      dhcp6_enable_stateless(&sobj->vnet.netif);
#endif
#if LWIP_IPV4
      netifapi_dhcp_start((struct netif *)&(sobj->vnet.netif));
#endif

      sobj->dhcp_timer = xTimerCreate("dhcp", 15000, pdFALSE, (void *)sobj, __timer_callback);
      if (sobj->dhcp_timer) {
          xTimerStart(sobj->dhcp_timer, 0);
      }
      sobj->dhcp_start = 1;

      event_propagate(sobj, VIRT_NET_EV_ON_CONNECTED, NULL);
    }
    break;
  case VIRT_NET_CTRL_AP_DISCONNECTED_IND:
    {
      if (sobj->sta_linkup) {
          netifapi_netif_set_link_down(&sobj->vnet.netif);
          printf("stop dhcp...\r\n");
          #if LWIP_IPV4
          netifapi_dhcp_stop((struct netif *)&sobj->vnet.netif);
          #endif
          #if LWIP_IPV6
          dhcp6_disable((struct netif *)&sobj->vnet.netif);
          #endif
          netifapi_netif_set_down((struct netif *)&sobj->vnet.netif);
          sobj->sta_linkup = 0;
      }
      
      event_propagate(sobj, VIRT_NET_EV_ON_DISCONNECT, NULL);
    }
    break;

  case VIRT_NET_CTRL_MAC_IND:
    {
      uint8_t *mac = (uint8_t *)sem_context[pkg_cmd->msg_id];
      assert(mac != NULL);

      sem = release_semaphore_slot(pkg_cmd->msg_id);
      if (sem) {

        memcpy(mac, pkg_cmd->payload, 6);
        memcpy(sobj->vnet.mac, pkg_cmd->payload, 6);
        blog_info("got mac:%x:%x:%x:%x:%x:%x\r\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

        xSemaphoreGive(sem);
      }
    }
    break;
  case VIRT_NET_CTRL_SCAN_IND:
    do {
      netbus_fs_scan_ind_cmd_msg_t* msg = (netbus_fs_scan_ind_cmd_msg_t*)pkg_header->payload;
      blog_info("Scan done! num = %d \r\n", msg->num);

      for (int i = 0; i < msg->num; i++) {
        blog_info("%s %d %d %d %d %d\r\n", msg->records[i].ssid
            , msg->records[i].channel, msg->records[i].rssi
            , msg->records[i].channel, msg->records[i].auth_mode
            , msg->records[i].cipher);
      }

      sem = release_semaphore_slot(pkg_cmd->msg_id);
      if (sem) {
        event_propagate(sobj, VIRT_NET_EV_ON_SCAN_DONE, pkg_header);
        xSemaphoreGive(sem);
      }
    } while (0);
    break;
  case VIRT_NET_CTRL_GET_LINK_STATUS_IND:
    {
    do {
      netbus_fs_link_status_ind_cmd_msg_t *status = (netbus_fs_link_status_ind_cmd_msg_t *)pkg_header->payload;
      blog_info("get link status indicate!\r\n");
      ip4_addr_t ip = {0}, gw = {0}, mask = {0};
      char ip_str[20], gw_str[20], mask_str[20];

      wifi_info_gate(&sobj->vnet.netif, &ip.addr, &gw.addr, &mask.addr);
      strcpy(ip_str, ip4addr_ntoa(&ip));
      strcpy(gw_str, ip4addr_ntoa(&gw));
      strcpy(mask_str, ip4addr_ntoa(&mask));
      blog_info("%s,%s,%s\r\n", ip_str, gw_str, mask_str);
      if (status->record.link_status == BF1B_WIFI_LINK_STATUS_UP) {
        blog_info("link status up!\r\n");
      } else if (status->record.link_status == BF1B_WIFI_LINK_STATUS_DOWN){
        blog_info("link status down!\r\n");
      } else {
        blog_info("link status unknown!\r\n");
      }

      /*get ip*/
      if (strcmp(ip_str, "0.0.0.0") && status->record.mgmr_state == 3) {
        status->record.mgmr_state = 4;
      }
      blog_info("state:%d\r\n", status->record.mgmr_state);
      blog_info("ssid:%s\r\n", status->record.ssid);
      blog_info("bssid: %02x%02x%02x%02x%02x%02x\r\n",
        status->record.bssid[0],
        status->record.bssid[1],
        status->record.bssid[2],
        status->record.bssid[3],
        status->record.bssid[4],
        status->record.bssid[5]
      );
      event_propagate(sobj, VIRT_NET_EV_ON_LINK_STATUS_DONE, pkg_header);

      sem = release_semaphore_slot(pkg_cmd->msg_id);
      if (sem) {
        xSemaphoreGive(sem);
      }

    } while (0);
    break;
    }
  case VIRT_NET_CTRL_GET_DEV_VERSION_IND:
    {
      uint8_t *version = (uint8_t *)sem_context[pkg_cmd->msg_id];
      assert(version != NULL);

      sem = release_semaphore_slot(pkg_cmd->msg_id);
      if (sem) {
        memcpy(version, pkg_cmd->payload, 4);
        xSemaphoreGive(sem);
      }
    }
    break;
  default:
    printf("unknow response: %d\r\n", pkg_cmd->cmd);
    break;
  }

  return 0;
}

static void __virt_net_task(void *pvParameters) 
{
  assert(pvParameters != NULL);
  struct virt_net_ramsync *sobj = (struct virt_net_ramsync *)pvParameters;
  uint8_t *rx_buffer;
  struct pkg_protocol *pkg_header;
  uint32_t recvd_len;
  assert(sobj->task_virt_net == xTaskGetCurrentTaskHandle());

  while (sobj->running) {

    rx_buffer = __alloc_rxbuf(sobj);
    if (rx_buffer == NULL) {
      printf("no rx buffer, suspend..\r\n");
      vTaskSuspend(NULL);
      continue;
    }
 
    pkg_header = (struct pkg_protocol *)(rx_buffer + VIRT_NET_RXBUFF_OFFSET);

    uramsync_rx_pop(&sobj->ramsync_ctx, (uint8_t *)pkg_header, &recvd_len, URAMSYNC_FOREVER);
    blog_info("Recv pkg_header->len:%d type:0x%x recvd_len %d.\r\n\r\n", pkg_header->length, pkg_header->type, recvd_len);
    
    /* FIXME: api should reutrn valid payload */
    if (recvd_len == 0) {
      /* we should release the buffer */
      __free_rxbuf(sobj, rx_buffer);
      continue;
    }

    switch (pkg_header->type) {
    case PKG_DATA_FRAME:
      if (pkg_protocol_data_handler(sobj, pkg_header)) {
        __free_rxbuf(sobj, rx_buffer);
      }
      break;

    case PKG_CMD_FRAME:
      pkg_protocol_cmd_handler(sobj, pkg_header);
      __free_rxbuf(sobj, rx_buffer);
      break;

    default:
      printf("unknow pkg type:0x%x\r\n", pkg_header->type);
      __free_rxbuf(sobj, rx_buffer);
      break;
    }
  }

  vTaskDelete(NULL);
}

static err_t __virt_net_netif_init(struct netif *netif) 
{
  struct virt_net_ramsync *sobj = (struct virt_net_ramsync *)netif->state;

  netif->hostname = "virt_net";

  netif->name[0]    = 'v';
  netif->name[1]    = 'n';

  netif->hwaddr_len = ETHARP_HWADDR_LEN;
  memcpy(netif->hwaddr, sobj->vnet.mac, ETHARP_HWADDR_LEN);

  /* set netif maximum transfer unit */
  netif->mtu = sobj->vnet.mtu;

  /* Accept broadcast address and ARP traffic */
  netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;
  netif->output = etharp_output;
  netif->linkoutput = __link_output;

#if LWIP_IPV6
  netif->flags |= (NETIF_FLAG_ETHERNET | NETIF_FLAG_IGMP | NETIF_FLAG_MLD6);
  netif->output_ip6 = ethip6_output;
#endif

  /* Set callback to be called when interface is brought up/down or address is changed while up */
  netif_set_status_callback(netif, netif_status_callback);

  return ERR_OK;
}

static int __virt_net_ramsync_control(virt_net_t obj, int cmd, ...) 
{
  assert(obj != NULL);
  struct virt_net_ramsync *sobj = (struct virt_net_ramsync *)obj;
  struct pkg_protocol *pkg_header = (struct pkg_protocol *)tx_buffer;
  struct pkg_protocol_cmd *pkg_cmd = (struct pkg_protocol_cmd *)pkg_header->payload;

  va_list args;

  switch (cmd) {
  case VIRT_NET_CTRL_HELLO:
    {
      pkg_header->type = PKG_CMD_FRAME;
      pkg_header->length = sizeof(struct pkg_protocol_cmd);

      pkg_cmd->cmd = VIRT_NET_CTRL_HELLO;
      pkg_cmd->msg_id = -1;

      blog_info("send hello\r\n");
    }
    break;
  case VIRT_NET_CTRL_CONNECT_AP:
    {
      struct pkg_cmd_connect_ap *connect_ap_cmd = (struct pkg_cmd_connect_ap *)&pkg_cmd->payload;
      uint8_t *tmp;
      int passphr_len;

      pkg_header->type = PKG_CMD_FRAME;
      pkg_header->length = sizeof(struct pkg_protocol_cmd) + sizeof(struct pkg_cmd_connect_ap);

      pkg_cmd->cmd = VIRT_NET_CTRL_CONNECT_AP;
      pkg_cmd->msg_id = -1;

      va_start(args, cmd);
      /* XXX avoid misaligned acces */
      connect_ap_cmd->ssid_len = (uint8_t)va_arg(args, unsigned int);
      passphr_len = (uint8_t)va_arg(args, unsigned int);

      if (connect_ap_cmd->ssid_len > sizeof(connect_ap_cmd->ssid) || passphr_len > sizeof(connect_ap_cmd->passphr)) {

        printf("ssid_len or passphr_len too long \r\n");
        va_end(args);

        return -1;
      }

      /* Copy ssid */
      tmp = va_arg(args, uint8_t *);
      memset(connect_ap_cmd->ssid, 0, sizeof(connect_ap_cmd->ssid));
      memset(connect_ap_cmd->ssid_tail, 0, sizeof(connect_ap_cmd->ssid_tail));
      strncpy((char *)connect_ap_cmd->ssid, (const char *)tmp, connect_ap_cmd->ssid_len);

      /* Copy passphr */
      tmp = va_arg(args, uint8_t *);
      memset(connect_ap_cmd->passphr, 0, sizeof(connect_ap_cmd->passphr));
      memset(connect_ap_cmd->psk_tail, 0, sizeof(connect_ap_cmd->psk_tail));
      strncpy((char *)connect_ap_cmd->passphr, (const char *)tmp, passphr_len);

      va_end(args);

      blog_info("connect ap ssid:%s, passphr:%s\r\n", connect_ap_cmd->ssid, connect_ap_cmd->passphr);
    }
    break;
 case VIRT_NET_CTRL_DISCONNECT_AP:
    {
      pkg_header->type = PKG_CMD_FRAME;
      pkg_header->length = sizeof(struct pkg_protocol_cmd);

      pkg_cmd->cmd = VIRT_NET_CTRL_DISCONNECT_AP;
      pkg_cmd->msg_id = -1;
  
      blog_info("send disconnect\r\n");
    }
    break; 
  case VIRT_NET_CTRL_GET_MAC:
    {
      uint8_t *tmp;

      SemaphoreHandle_t sem;
      StaticSemaphore_t sem_buff;
      int msg_id;

      sem = xSemaphoreCreateBinaryStatic(&sem_buff);
      assert(sem != NULL);

      msg_id = acquire_semaphore_slot(sem);
      if (msg_id < 0) {
        printf("Bug? too many pending commands, try to enlarge sem_slot\r\n");
        return -1;
      }
      pkg_header->type = PKG_CMD_FRAME;
      pkg_header->length = sizeof(struct pkg_protocol_cmd);

      pkg_cmd->cmd = VIRT_NET_CTRL_GET_MAC;
      pkg_cmd->msg_id = msg_id;
      
      va_start(args, cmd);

      tmp = va_arg(args, uint8_t *);

      sem_context[msg_id] = (void *)tmp;

      va_end(args);

      blog_info("get mac..\r\n");

      uramsync_tx_push_toback(&sobj->ramsync_ctx, tx_buffer,
                              pkg_header->length + 4, URAMSYNC_FOREVER);
      blog_info("GET_MAC pkg_header->len:%d type:0x%x\r\n\r\n", pkg_header->length, pkg_header->type);
      
        /* waiting response */
      if (xSemaphoreTake(sem, 3000) != pdTRUE) {
        release_semaphore_slot(msg_id);
        printf("get mac failed!!!!\r\n");
        return -1;
      }
          
      release_semaphore_slot(msg_id);
      return 0;
    }
    break;
  case VIRT_NET_CTRL_START_SCAN:
    do {
      SemaphoreHandle_t sem;
      StaticSemaphore_t sem_buff;
      int msg_id;
      static uint8_t test_buf[128];

      sem = xSemaphoreCreateBinaryStatic(&sem_buff);
      assert(sem != NULL);

      msg_id = acquire_semaphore_slot(sem);
      if (msg_id < 0) {
        blog_info("Bug? too many pending commands, try to enlarge sem_slot\r\n");
        return -1;
      }

      pkg_header->type = PKG_CMD_FRAME;
      pkg_header->length = sizeof(struct pkg_protocol) + sizeof(struct pkg_protocol_cmd);

      pkg_cmd->cmd = VIRT_NET_CTRL_START_SCAN;
      pkg_cmd->msg_id = msg_id;

      sem_context[msg_id] = (void *)test_buf;

      uramsync_tx_push_toback(&sobj->ramsync_ctx, tx_buffer,
          pkg_header->length + 4, URAMSYNC_FOREVER);
      blog_info("SCAN pkg_header->len:%d type:0x%x\r\n\r\n", pkg_header->length, pkg_header->type);

      blog_info("scan..\r\n");

      /* waiting response */
      if (xSemaphoreTake(sem, 10000) != pdTRUE) {
          release_semaphore_slot(msg_id);
          printf("scan failed!!!!\r\n");
          return -1;
      }

      release_semaphore_slot(msg_id);
      return 0;
    } while (0);
    break;
  case VIRT_NET_CTRL_GET_LINK_STATUS:
    {
      SemaphoreHandle_t sem;
      StaticSemaphore_t sem_buff;
      int msg_id;

      sem = xSemaphoreCreateBinaryStatic(&sem_buff);
      assert(sem != NULL);

      msg_id = acquire_semaphore_slot(sem);
      if (msg_id < 0) {
        printf("Bug? too many pending commands, try to enlarge sem_slot\r\n");
        return -1;
      }
      pkg_header->type = PKG_CMD_FRAME;
      pkg_header->length = sizeof(struct pkg_protocol_cmd) + sizeof(struct pkg_protocol_cmd);

      pkg_cmd->cmd = VIRT_NET_CTRL_GET_LINK_STATUS;
      pkg_cmd->msg_id = msg_id;

      blog_info("get link status..\r\n");

      uramsync_tx_push_toback(&sobj->ramsync_ctx, tx_buffer,
          pkg_header->length + 4, URAMSYNC_FOREVER);
      blog_info("LINK STATUS pkg_header->len:%d type:0x%x\r\n\r\n", pkg_header->length, pkg_header->type);

      /* waiting response */
      if (xSemaphoreTake(sem, 3000) != pdTRUE) {
        release_semaphore_slot(msg_id);
        printf("get link status failed!!!!\r\n");
        return -1;
      }

      release_semaphore_slot(msg_id);
      return 0;
    }
    break;
  case VIRT_NET_CTRL_HBN:
    {
      blog_info("enter hbn\r\n");

      pkg_header->type = PKG_CMD_FRAME;
      pkg_header->length = sizeof(struct pkg_protocol_cmd);

      pkg_cmd->cmd = VIRT_NET_CTRL_HBN;
      pkg_cmd->msg_id = -1;
    }
    break;
  case VIRT_NET_CTRL_GET_DEV_VERSION:
    {
      uint8_t *tmp;
      SemaphoreHandle_t sem;
      StaticSemaphore_t sem_buff;
      int msg_id;

      sem = xSemaphoreCreateBinaryStatic(&sem_buff);
      assert(sem != NULL);

      msg_id = acquire_semaphore_slot(sem);
      if (msg_id < 0) {
        printf("Bug? too many pending commands, try to enlarge sem_slot\r\n");
        return -1;
      }
      pkg_header->type = PKG_CMD_FRAME;
      pkg_header->length = sizeof(struct pkg_protocol_cmd);

      pkg_cmd->cmd = VIRT_NET_CTRL_GET_DEV_VERSION;
      pkg_cmd->msg_id = msg_id;

      va_start(args, cmd);

      tmp = va_arg(args, uint8_t *);

      sem_context[msg_id] = (void *)tmp;

      va_end(args);

      blog_info("get slave version..\r\n");

      uramsync_tx_push_toback(&sobj->ramsync_ctx, tx_buffer,
                              pkg_header->length + 4, URAMSYNC_FOREVER);
      blog_info("GET_DEV_VERSION pkg_header->len:%d type:0x%x\r\n\r\n", pkg_header->length, pkg_header->type);

        /* waiting response */
      if (xSemaphoreTake(sem, 3000) != pdTRUE) {
        release_semaphore_slot(msg_id);
        printf("get slave version failed!!!!\r\n");
        return -1;
      }

      release_semaphore_slot(msg_id);
      return 0;
    }
    break;
  default:
    printf("unsupport cmd:%d\r\n", cmd);
    break;
  }

  uramsync_tx_push_toback(&sobj->ramsync_ctx, tx_buffer,
                          pkg_header->length + 4, URAMSYNC_FOREVER);
  blog_info("CTRL pkg_header->len:%d type:0x%x\r\n\r\n", pkg_header->length, pkg_header->type);

  return 0;
}

static int __virt_net_ramsync_init(virt_net_t obj) 
{
  assert(obj != NULL);
  struct virt_net_ramsync *sobj = (struct virt_net_ramsync *)obj;

#if 0
  ip4_addr_t ipaddr;
  ip4_addr_t netmask;
  ip4_addr_t gw;

  IP4_ADDR(&ipaddr, 192, 168, 11, 111);
  IP4_ADDR(&netmask, 255, 255, 255, 0);
  IP4_ADDR(&gw, 192, 168, 11, 1);
#endif

  if (netifapi_netif_add(&sobj->vnet.netif, 
    #if LWIP_IPV4
    NULL, NULL, NULL, 
    #endif
    (void *)sobj, __virt_net_netif_init, tcpip_input) != ERR_OK) {
    printf("add virt_net netif failed\r\n");
    return -1;
  }

  return 0;
}

static int __virt_net_ramsync_deinit(virt_net_t obj) 
{
  struct virt_net_ramsync *sobj = (struct virt_net_ramsync *)obj;

  netif_set_status_callback(&sobj->vnet.netif, NULL);
  netifapi_netif_remove(&sobj->vnet.netif);

  return 0;
}

virt_net_t virt_net_create(void *ctx) 
{

  if (inited == 0) {
    inited = 1;
    slot_mutex = xSemaphoreCreateMutexStatic(&xSlotMutexBuffer);
    memset(sem_slot, 0, sizeof(sem_slot));
  }

  struct virt_net_ramsync *sobj = malloc(sizeof(struct virt_net_ramsync));
  if (sobj == NULL) {
    return NULL;
  }

  memset(sobj, 0, sizeof(struct virt_net_ramsync));

  sobj->vnet.init   = __virt_net_ramsync_init;
  sobj->vnet.deinit = __virt_net_ramsync_deinit;
  sobj->vnet.ctrl   = __virt_net_ramsync_control;

  uramsync_init(&sobj->ramsync_ctx, URAMSYNC_MASTER_DEV_TYPE);

  /* Init rx ind */
  assert(VIRT_NET_RXBUFF_CNT <= _BITSET_BITS);
  sobj->rx_buf_ind.__bits[0] = (1 << VIRT_NET_RXBUFF_CNT) - 1;

  sobj->rx_buf_ind_mutex = xSemaphoreCreateMutex();
  if (sobj->rx_buf_ind_mutex == NULL) {
    printf("create virt_net rx buffer ind semaphore failed\r\n");
    goto _errout_1;
  }

  sobj->rx_buff = (rx_buff_t)malloc(VIRT_NET_BUFF_SIZE * VIRT_NET_RXBUFF_CNT);
  if (sobj->rx_buff == NULL) {
    printf("alloc virt_net rx buffer failed\r\n");
    goto _errout_2;
  }

  tx_buffer = malloc(VIRT_NET_BUFF_SIZE);
  if(tx_buffer == NULL){
    printf("Malloc tx_buffer error\r\n");
    goto _errout_3;
  }

  sobj->dhcp_start = 0;
  sobj->sta_linkup = 0;
  sobj->running = 1;

  /* create spi receivce thread */
  if (xTaskCreate(__virt_net_task, "virt_net", CFG_VIRT_NET_STACK_DEPTH, (void *)sobj, VIRT_NET_TASK_PRI, &sobj->task_virt_net) != pdPASS) {
    printf("create virt_netreceive task failed\r\n");
    goto _errout_4;
  }

#if 0
  /**
   * TODO REMOVE ME
   * Workaround PhD Yang's bug
   */
  do {
    int i;
    for (i = 0; i < 1; i++) {
      virt_net_send_nop(&sobj->vnet);
      //vTaskDelay(pdMS_TO_TICKS(500));
    }
  } while (0);
#endif

  /* Query hardware mac */
  virt_net_get_mac(&sobj->vnet, sobj->vnet.mac);

  /* XXX Set MTU */
  sobj->vnet.mtu = 1500;

  return &sobj->vnet;

_errout_4:
  free(sobj->rx_buff);
_errout_3:
  free(tx_buffer);
_errout_2:
  vSemaphoreDelete(sobj->rx_buf_ind_mutex);
_errout_1:
  free(sobj);

  return NULL;
}

int virt_net_delete(void *ctx) 
{
  if (!inited) {
    return 0;
  }
  inited = 0;

  struct virt_net_ramsync *sobj = ctx;

  vTaskDelete(sobj->task_virt_net);

  uramsync_deinit(&sobj->ramsync_ctx);

  if(sobj){
    free(sobj);
  }

  if (sobj->rx_buf_ind_mutex){
    vSemaphoreDelete(sobj->rx_buf_ind_mutex);
  }

  if (sobj->rx_buff){
    free(sobj->rx_buff);
  }

  if(tx_buffer){
    free(tx_buffer);
  }

  return 0;
}

