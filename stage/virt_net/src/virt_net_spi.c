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
#include <blog.h>

#include "bitset.h"

#include "virt_net_spi.h"
#include "pkg_protocol.h"

struct virt_net_spi;
struct spi_custom_pbuf {
  struct pbuf_custom p;
  struct virt_net_spi *sobj;
};

#define VIRT_NET_RXBUFF_OFFSET (sizeof(struct spi_custom_pbuf))

/* 头部的假数据，绕过DMA/SPI FIFO缓存 */
#define VIRT_NET_RXBUFF_CNT (4)
#define VIRT_NET_BUFF_SIZE (4096 - 32 - 8 - 16 + VIRT_NET_RXBUFF_OFFSET) /* + sizeof(pbuf header) */
#define VIRT_NET_MAX_PENDING_CMD (4)

BITSET_DEFINE(rx_buf_ind, VIRT_NET_RXBUFF_CNT);

typedef uint8_t (*rx_buff_t)[VIRT_NET_BUFF_SIZE];

static int inited = 0;

/* semaphore slot */
static SemaphoreHandle_t sem_slot[VIRT_NET_MAX_PENDING_CMD];
static void *sem_context[VIRT_NET_MAX_PENDING_CMD];
static SemaphoreHandle_t slot_mutex;
static StaticSemaphore_t xSlotMutexBuffer;

static uint8_t tx_buffer[VIRT_NET_BUFF_SIZE];
static SemaphoreHandle_t tx_mutex;
static StaticSemaphore_t tx_mutex_buffer;

struct virt_net_spi {
  struct virt_net vnet;
  tp_spi_master_ctx_t master_ctx;
  tp_spi_config_t spi_config;

  volatile int running;
  TaskHandle_t task_spi_rx;

  rx_buff_t rx_buff;

  SemaphoreHandle_t rx_buf_ind_mutex;
  struct rx_buf_ind rx_buf_ind;

#if LWIP_IPV6
  struct dhcp6 vnet_dhcp6;
#endif
};

static int virt_net_spi_init(virt_net_t obj);
static int virt_net_spi_control(virt_net_t obj, int cmd, ...);
static int virt_net_spi_deinit(virt_net_t *obj);

static void spi_receive_task(void *pvParameters);

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
  SemaphoreHandle_t ret;

  xSemaphoreTake(slot_mutex, portMAX_DELAY);
  assert(sem_slot[idx] != NULL);
  ret = sem_slot[idx];
  sem_slot[idx] = NULL;
  xSemaphoreGive(slot_mutex);

  return ret;
}

static uint8_t *spi_alloc_rxbuf(struct virt_net_spi *sobj) {
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

static void spi_free_rxbuf(struct virt_net_spi *sobj, uint8_t *buf) {
  int idx;
  assert(sobj != NULL);

  idx = (rx_buff_t)buf - sobj->rx_buff;

  assert(idx < VIRT_NET_RXBUFF_CNT && idx >= 0);
  assert(sobj->rx_buff[idx] == buf);

  xSemaphoreTake(sobj->rx_buf_ind_mutex, portMAX_DELAY);
  assert(!BIT_ISSET(VIRT_NET_RXBUFF_CNT, idx, &sobj->rx_buf_ind));

  BIT_SET(VIRT_NET_RXBUFF_CNT, idx, &sobj->rx_buf_ind);
  xSemaphoreGive(sobj->rx_buf_ind_mutex);
}

virt_net_t virt_net_spi_create(tp_spi_config_t *spi_config) {
  assert(spi_config != NULL);

  taskENTER_CRITICAL();
  if (inited == 0) {
    inited = 1;
    taskEXIT_CRITICAL();
    slot_mutex = xSemaphoreCreateMutexStatic(&xSlotMutexBuffer);
    tx_mutex = xSemaphoreCreateMutexStatic(&tx_mutex_buffer);
    memset(sem_slot, 0, sizeof(sem_slot));
  }

  struct virt_net_spi *sobj = pvPortMalloc(sizeof(struct virt_net_spi));
  if (sobj == NULL) {
    return NULL;
  }

  memset(sobj, 0, sizeof(struct virt_net_spi));

  sobj->vnet.init = virt_net_spi_init;
  sobj->vnet.deinit = virt_net_spi_deinit;
  sobj->vnet.ctrl = virt_net_spi_control;

  /* XXX need backup this config? */
  memcpy(&sobj->spi_config, spi_config, sizeof(tp_spi_config_t));
  tp_spi_master_init(&sobj->master_ctx, &sobj->spi_config);

  /* Init rx ind */
  assert(VIRT_NET_RXBUFF_CNT <= _BITSET_BITS);
  sobj->rx_buf_ind.__bits[0] = (1 << VIRT_NET_RXBUFF_CNT) - 1;

  sobj->rx_buf_ind_mutex = xSemaphoreCreateMutex();
  if (sobj->rx_buf_ind_mutex == NULL) {
    printf("create spi rx buffer ind semaphore failed\r\n");
    goto _errout_1;
  }

  sobj->rx_buff = (rx_buff_t)pvPortMalloc(VIRT_NET_BUFF_SIZE * VIRT_NET_RXBUFF_CNT);
  if (sobj->rx_buff == NULL) {
    printf("alloc spi rx buffer failed\r\n");
    goto _errout_2;
  }

  /* create spi receivce thread */
  if (xTaskCreate(spi_receive_task, "virt net spi rx", 1024 + 256, (void *)sobj, 30, &sobj->task_spi_rx) != pdPASS) {
    printf("create spi receive task failed\r\n");
    goto _errout_3;
  }

  sobj->running = 1;
  vTaskResume(sobj->task_spi_rx);

#if 1
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

_errout_3:
  vPortFree(sobj->rx_buff);
_errout_2:
  vSemaphoreDelete(sobj->rx_buf_ind_mutex);
_errout_1:
  vPortFree(sobj);

  return NULL;
}

void virt_net_spi_destroy(virt_net_t *obj) {
  assert(obj != NULL && *obj != NULL);
  struct virt_net_spi *sobj = (struct virt_net_spi *)*obj;
  vPortFree(sobj->rx_buff);
  *obj = NULL;
}

static void netif_status_callback(struct netif *netif) {
  uint32_t is_got_ip = 0;

  if (netif->flags & NETIF_FLAG_UP) {
    /** interface is up status */
    struct virt_net_spi *sobj = (struct virt_net_spi *)netif->state;

#if LWIP_IPV4
    is_got_ip = !ip4_addr_isany(netif_ip4_addr(netif)) ? 1 : 0;
#endif

#if LWIP_IPV6
    for (uint32_t i = 0; i < LWIP_IPV6_NUM_ADDRESSES; i ++ ) {
      if (!ip6_addr_isany(netif_ip6_addr(netif, i)) && !ip6_addr_islinklocal(netif_ip6_addr(netif, i))
            && ip6_addr_ispreferred(netif_ip6_addr_state(netif, i))
            ) {
        is_got_ip |= 2;
      }
    }
#endif

    if (is_got_ip && sobj) {
      event_propagate(sobj, VIRT_NET_EV_ON_GOT_IP, (void *)is_got_ip);
    }
  }
  else {
    printf("interface is down status.\n");
  }
}

static err_t spi_link_output(struct netif *netif, struct pbuf *p) {
  struct virt_net_spi *sobj = (struct virt_net_spi *)netif->state;
  struct pbuf *curr;
  unsigned int offset = 0;
  uint32_t temp;
  struct pkg_protocol *pkg_header = (struct pkg_protocol *)tx_buffer;
  assert(sobj != NULL);

  xSemaphoreTake(tx_mutex, portMAX_DELAY);

  temp = 0x2000;
  memcpy(pkg_header->payload + offset, &temp, 2);
  offset = 2;

  for (curr = p; curr != NULL; curr = curr->next) {
    assert(offset + curr->len <= VIRT_NET_BUFF_SIZE - VIRT_NET_RXBUFF_OFFSET);
    memcpy(pkg_header->payload + offset, curr->payload, curr->len);
    offset += curr->len;
  }

  pkg_header->type = PKG_DATA_FRAME;
  pkg_header->length = p->tot_len + 2 + sizeof(struct pkg_protocol);

  spi_send(&sobj->master_ctx, tx_buffer, VIRT_NET_BUFF_SIZE - VIRT_NET_RXBUFF_OFFSET, portMAX_DELAY);

  xSemaphoreGive(tx_mutex);

  return ERR_OK;
}

static void pbuf_free_spi_custom_fn(struct pbuf *p) {
  struct spi_custom_pbuf *spbuf = (struct spi_custom_pbuf *)p;
  spi_free_rxbuf(spbuf->sobj, (void *)p);

  vTaskResume(spbuf->sobj->task_spi_rx);
}

static int pkg_protocol_data_handler(struct virt_net_spi *sobj, struct pkg_protocol *pkg_header) {
  struct pbuf *pbuf;
  struct spi_custom_pbuf *spbuf = (struct spi_custom_pbuf *)((uintptr_t)pkg_header - VIRT_NET_RXBUFF_OFFSET);
  memset(spbuf, 0, sizeof(struct spi_custom_pbuf));
  spbuf->p.custom_free_function = pbuf_free_spi_custom_fn;
  spbuf->sobj = sobj;

  if (sobj->vnet.netif.input) {
    pbuf = pbuf_alloced_custom(PBUF_RAW, pkg_header->length, PBUF_REF, &spbuf->p,
                               (uint8_t *)pkg_header->payload, pkg_header->length);

    if (sobj->vnet.netif.input(pbuf, &sobj->vnet.netif) != ERR_OK) {
      printf("handle to tcpip stack failed\r\n");
      return -1;
    }
  }

  return 0;
}

/* call event callback */
static inline int event_propagate(struct virt_net_spi *sobj, enum virt_net_event_code code, void *param) {
  if (sobj->vnet.event_cb) {
      return sobj->vnet.event_cb(&sobj->vnet, code, param);
  }
  return 0;
}

static int pkg_protocol_cmd_handler(struct virt_net_spi *sobj, struct pkg_protocol *pkg_header) {
  struct pkg_protocol_cmd *pkg_cmd = (struct pkg_protocol_cmd *)pkg_header->payload;
  SemaphoreHandle_t sem;

  switch (pkg_cmd->cmd) {
  case VIRT_NET_CTRL_AP_CONNECTED_IND:
    do {
      printf("AP connect success!\r\n");
      netifapi_netif_set_link_up(&sobj->vnet.netif);
      printf("set netif link up\r\n");

#if LWIP_IPV6
      event_propagate(sobj, VIRT_NET_EV_ON_CONNECTED, &sobj->vnet_dhcp6);
#else
      event_propagate(sobj, VIRT_NET_EV_ON_CONNECTED, NULL);
#endif
    } while (0);
    break;
  case VIRT_NET_CTRL_AP_DISCONNECTED_IND:
    do {
      printf("AP connect failed!\r\n");
      netifapi_netif_set_link_down(&sobj->vnet.netif);

      event_propagate(sobj, VIRT_NET_EV_ON_DISCONNECT, NULL);
    } while (0);
    break;

  case VIRT_NET_CTRL_MAC_IND:
    do {
      uint8_t *mac = (uint8_t *)sem_context[pkg_cmd->msg_id];
      assert(mac != NULL);

      memcpy(mac, pkg_cmd->payload, 6);
      memcpy(sobj->vnet.mac, pkg_cmd->payload, 6);
      printf("got mac:%x:%x:%x:%x:%x:%x\r\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

      sem = release_semaphore_slot(pkg_cmd->msg_id);
      xSemaphoreGive(sem);

    } while (0);
    break;

  default:
    printf("unknow response: %d\r\n", pkg_cmd->cmd);
    break;
  }

  return 0;
}

static void spi_receive_task(void *pvParameters) {
  assert(pvParameters != NULL);
  struct virt_net_spi *sobj = (struct virt_net_spi *)pvParameters;
  uint8_t *rx_buffer;
  struct pkg_protocol *pkg_header;
  unsigned int recvd_len;
  assert(sobj->task_spi_rx == xTaskGetCurrentTaskHandle());

  /* we not run immediately */
  vTaskSuspend(NULL);

  while (sobj->running) {
    rx_buffer = spi_alloc_rxbuf(sobj);
    if (rx_buffer == NULL) {
      printf("no rx buffer, suspend..\r\n");
      vTaskSuspend(NULL);
      continue;
    }

    pkg_header = (struct pkg_protocol *)(rx_buffer + VIRT_NET_RXBUFF_OFFSET);

    recvd_len = spi_recv(&sobj->master_ctx, (uint8_t *)pkg_header, VIRT_NET_BUFF_SIZE - VIRT_NET_RXBUFF_OFFSET, portMAX_DELAY);
    /* FIXME: api should reutrn valid payload */
    if (recvd_len == 0) {
      /* we should release the buffer */
      spi_free_rxbuf(sobj, rx_buffer);
      continue;
    }

    switch (pkg_header->type) {
    case PKG_DATA_FRAME:
      if (pkg_protocol_data_handler(sobj, pkg_header)) {
        spi_free_rxbuf(sobj, rx_buffer);
      }
      break;

    case PKG_CMD_FRAME:
      pkg_protocol_cmd_handler(sobj, pkg_header);
      spi_free_rxbuf(sobj, rx_buffer);
      break;

    default:
      printf("unknow pkg type:0x%x\r\n", pkg_header->type);
      spi_free_rxbuf(sobj, rx_buffer);
      break;
    }
  }

  vTaskDelete(NULL);
}

static err_t virt_net_spi_netif_init(struct netif *netif) {
  struct virt_net_spi *sobj = (struct virt_net_spi *)netif->state;

  netif->hostname = "virt_net_spi";

  netif->name[0]    = 'v';
  netif->name[1]    = 'n';

  netif->hwaddr_len = ETHARP_HWADDR_LEN;
  memcpy(netif->hwaddr, sobj->vnet.mac, ETHARP_HWADDR_LEN);

  /* set netif maximum transfer unit */
  netif->mtu = sobj->vnet.mtu;

  /* Accept broadcast address and ARP traffic */
  netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;
  netif->output = etharp_output;
  netif->linkoutput = spi_link_output;

#if LWIP_IPV6
  netif->flags |= (NETIF_FLAG_ETHERNET | NETIF_FLAG_IGMP | NETIF_FLAG_MLD6);
  netif->output_ip6 = ethip6_output;
#endif

  /* Set callback to be called when interface is brought up/down or address is changed while up */
  netif_set_status_callback(netif, netif_status_callback);

  return ERR_OK;
}

static int virt_net_spi_init(virt_net_t obj) {
  assert(obj != NULL);
  struct virt_net_spi *sobj = (struct virt_net_spi *)obj;

#if 0
  ip4_addr_t ipaddr;
  ip4_addr_t netmask;
  ip4_addr_t gw;

  IP4_ADDR(&ipaddr, 192, 168, 11, 111);
  IP4_ADDR(&netmask, 255, 255, 255, 0);
  IP4_ADDR(&gw, 192, 168, 11, 1);
#endif
#if LWIP_IPV4
  if (netifapi_netif_add(&sobj->vnet.netif, NULL, NULL, NULL, (void *)sobj, virt_net_spi_netif_init, tcpip_input) != ERR_OK) {
#else
    if (netifapi_netif_add(&sobj->vnet.netif,  (void *)sobj, virt_net_spi_netif_init, tcpip_input) != ERR_OK) {
#endif
    printf("add spi netif failed\r\n");
    return -1;
  }

  return 0;
}

static int virt_net_spi_control(virt_net_t obj, int cmd, ...) {
  assert(obj != NULL);
  struct virt_net_spi *sobj = (struct virt_net_spi *)obj;
  struct pkg_protocol *pkg_header = (struct pkg_protocol *)tx_buffer;
  struct pkg_protocol_cmd *pkg_cmd = (struct pkg_protocol_cmd *)pkg_header->payload;

  va_list args;

  switch (cmd) {
  case VIRT_NET_CTRL_HELLO:
    do {
      xSemaphoreTake(tx_mutex, portMAX_DELAY);

      pkg_header->type = PKG_CMD_FRAME;
      pkg_header->length = sizeof(struct pkg_protocol) + sizeof(struct pkg_protocol_cmd);

      pkg_cmd->cmd = VIRT_NET_CTRL_HELLO;
      pkg_cmd->msg_id = -1;

      spi_send(&sobj->master_ctx, tx_buffer, VIRT_NET_BUFF_SIZE - VIRT_NET_RXBUFF_OFFSET, portMAX_DELAY);
      xSemaphoreGive(tx_mutex);

      printf("send hello\r\n");

      return 0;
    } while (0);
    break;
  case VIRT_NET_CTRL_CONNECT_AP:
    do {
      struct pkg_cmd_connect_ap *connect_ap_cmd = (struct pkg_cmd_connect_ap *)&pkg_cmd->payload;
      uint8_t *tmp;
      int passphr_len;

      xSemaphoreTake(tx_mutex, portMAX_DELAY);

      pkg_header->type = PKG_CMD_FRAME;
      pkg_header->length = sizeof(struct pkg_protocol) + sizeof(struct pkg_protocol_cmd) + sizeof(struct pkg_cmd_connect_ap);

      pkg_cmd->cmd = VIRT_NET_CTRL_CONNECT_AP;
      pkg_cmd->msg_id = -1;

      va_start(args, cmd);
      /* XXX avoid misaligned acces */
      connect_ap_cmd->ssid_len = (uint8_t)va_arg(args, unsigned int) + 1;
      passphr_len = (uint8_t)va_arg(args, unsigned int) + 1;
      printf("passphr_len:%d\r\n", passphr_len);
      if (connect_ap_cmd->ssid_len > sizeof(connect_ap_cmd->ssid) || passphr_len > sizeof(connect_ap_cmd->passphr)) {
        xSemaphoreGive(tx_mutex);

        printf("ssid_len or passphr_len too long \r\n");
        va_end(args);

        return -1;
      }

      /* Copy ssid */
      tmp = va_arg(args, uint8_t *);
      memcpy(connect_ap_cmd->ssid, tmp, connect_ap_cmd->ssid_len);
      connect_ap_cmd->ssid[connect_ap_cmd->ssid_len] = '\0';
      /* Copy passphr */
      tmp = va_arg(args, uint8_t *);
      memcpy(connect_ap_cmd->passphr, tmp, passphr_len);
      connect_ap_cmd->passphr[passphr_len] = '\0';

      va_end(args);

      spi_send(&sobj->master_ctx, tx_buffer, VIRT_NET_BUFF_SIZE - VIRT_NET_RXBUFF_OFFSET, portMAX_DELAY);
      xSemaphoreGive(tx_mutex);

      printf("connect ap ssid:%s, passphr:%s\r\n", connect_ap_cmd->ssid, connect_ap_cmd->passphr);

      return 0;
    } while (0);
    break;

  case VIRT_NET_CTRL_GET_MAC:
    do {
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

      xSemaphoreTake(tx_mutex, portMAX_DELAY);

      pkg_header->type = PKG_CMD_FRAME;
      pkg_header->length = sizeof(struct pkg_protocol) + sizeof(struct pkg_protocol_cmd);

      pkg_cmd->cmd = VIRT_NET_CTRL_GET_MAC;
      pkg_cmd->msg_id = msg_id;

      va_start(args, cmd);

      tmp = va_arg(args, uint8_t *);

      sem_context[msg_id] = (void *)tmp;

      va_end(args);

      spi_send(&sobj->master_ctx, tx_buffer, VIRT_NET_BUFF_SIZE - VIRT_NET_RXBUFF_OFFSET, portMAX_DELAY);
      xSemaphoreGive(tx_mutex);

      printf("get mac..\r\n");

      /* waiting response */
      xSemaphoreTake(sem, portMAX_DELAY);

      return 0;
    } while (0);
    break;

  default:
    printf("unsupport cmd:%d\r\n", cmd);
    break;
  }
  return -1;
}

static int virt_net_spi_deinit(virt_net_t *obj) {
  /* TODO */
  return -1;
}

