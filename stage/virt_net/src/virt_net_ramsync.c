/*
 * Copyright (c) 2016-2026 Bouffalolab.
 *
 * This file is part of
 *     *** Bouffalolab Software Dev Kit ***
 *      (see www.bouffalolab.com).
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of Bouffalo Lab nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
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
#include "virt_net_pending.h"
#include "pkg_protocol.h"
#include "virt_net_protocol.h"
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
#ifndef CFG_VIRT_NET_RXBUFF_CNT
#if defined(CFG_USE_PSRAM)
#define VIRT_NET_RXBUFF_CNT (16)
#else
#define VIRT_NET_RXBUFF_CNT (4)
#endif
#else
#define VIRT_NET_RXBUFF_CNT CFG_VIRT_NET_RXBUFF_CNT
#endif
#define VIRT_NET_BUFF_SIZE (TP_PAYLOAD_LEN + VIRT_NET_RXBUFF_OFFSET) /* + sizeof(pbuf header) */
BITSET_DEFINE(rx_buf_ind, VIRT_NET_RXBUFF_CNT);

typedef uint8_t (*rx_buff_t)[VIRT_NET_BUFF_SIZE];

static int inited = 0;

struct virt_net_waiter {
  SemaphoreHandle_t sem;
  int result;
};

struct virt_net_completion_context {
  struct virt_net_ramsync *sobj;
  uint8_t *response;
};

static struct virt_net_pending_table pending_table;
static SemaphoreHandle_t slot_mutex;
static StaticSemaphore_t xSlotMutexBuffer;
static SemaphoreHandle_t tx_mutex;
static StaticSemaphore_t xTxMutexBuffer;

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
  size_t frame_length;
  int tx_result;
  assert(sobj != NULL);

  xSemaphoreTake(tx_mutex, portMAX_DELAY);
  if (TP_PAYLOAD_LEN < sizeof(*pkg_header) + sizeof(subtype) ||
      p->tot_len > TP_PAYLOAD_LEN - sizeof(*pkg_header) - sizeof(subtype)) {
    xSemaphoreGive(tx_mutex);
    return ERR_BUF;
  }

  subtype = 0x2000;
  memcpy(pkg_header->payload + offset, &subtype, sizeof(subtype));
  offset = sizeof(subtype);

  for (curr = p; curr != NULL; curr = curr->next) {
    memcpy(pkg_header->payload + offset, curr->payload, curr->len);
    offset += curr->len;
  }

  pkg_header->type = PKG_DATA_FRAME;
  /**netbus: 
   * 1. handle_eth_wifi_frame_recv passes (pkg_header->length - 2) to pbuf_take
   * 2. pbuf has 1514 tot_len limitation*/
  pkg_header->length = p->tot_len + sizeof(subtype);

  frame_length = sizeof(*pkg_header) + pkg_header->length;
  tx_result = uramsync_tx_push_toback(&sobj->ramsync_ctx, tx_buffer,
                                      frame_length, URAMSYNC_FOREVER);
  blog_info("__link_output pkg_header->len:%d type:0x%x\r\n", pkg_header->length, pkg_header->type);
  xSemaphoreGive(tx_mutex);

  return tx_result;
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
  if (pbuf == NULL) {
    return -1;
  }

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

    event_propagate(sobj, VIRT_NET_EV_ON_DHCP_TIMEOUT, NULL);

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

static int complete_mac(void *context, const void *payload, size_t payload_len)
{
  struct virt_net_completion_context *completion_context = context;
  const struct pkg_protocol *frame = payload;
  const struct pkg_protocol_cmd *pkg_cmd;
  uint8_t *mac;

  if (completion_context == NULL || completion_context->sobj == NULL ||
      completion_context->response == NULL || frame == NULL ||
      payload_len != sizeof(*frame) + sizeof(*pkg_cmd) + 6U) {
    return -1;
  }

  pkg_cmd = (const struct pkg_protocol_cmd *)frame->payload;
  mac = completion_context->response;
  memcpy(mac, pkg_cmd->payload, 6U);
  memcpy(completion_context->sobj->vnet.mac, pkg_cmd->payload, 6U);
  blog_info("got mac:%x:%x:%x:%x:%x:%x\r\n",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  return 0;
}

static int complete_scan(void *context, const void *payload,
                         size_t payload_len)
{
  struct virt_net_completion_context *completion_context = context;
  struct pkg_protocol *frame = (struct pkg_protocol *)payload;
  netbus_fs_scan_ind_cmd_msg_t *msg;
  size_t expected_len;
  uint16_t i;

  if (completion_context == NULL || completion_context->sobj == NULL ||
      frame == NULL ||
      payload_len < sizeof(*frame) +
          offsetof(netbus_fs_scan_ind_cmd_msg_t, records)) {
    return -1;
  }

  msg = (netbus_fs_scan_ind_cmd_msg_t *)frame->payload;
  expected_len = sizeof(*frame) +
      offsetof(netbus_fs_scan_ind_cmd_msg_t, records) +
      (size_t)msg->num * sizeof(msg->records[0]);
  if (payload_len != expected_len) {
    return -1;
  }

  blog_info("Scan done! num = %d \r\n", msg->num);
  for (i = 0; i < msg->num; i++) {
    blog_info("%.*s %d %d %d %d %d\r\n",
              (int)sizeof(msg->records[i].ssid),
              (const char *)msg->records[i].ssid,
              msg->records[i].channel, msg->records[i].rssi,
              msg->records[i].channel, msg->records[i].auth_mode,
              msg->records[i].cipher);
  }

  return event_propagate(completion_context->sobj,
                         VIRT_NET_EV_ON_SCAN_DONE, frame);
}

static int complete_link_status(void *context, const void *payload,
                                size_t payload_len)
{
  struct virt_net_completion_context *completion_context = context;
  struct pkg_protocol *frame = (struct pkg_protocol *)payload;
  netbus_fs_link_status_ind_cmd_msg_t *status;
  ip4_addr_t ip = {0}, gw = {0}, mask = {0};
  char ip_str[20], gw_str[20], mask_str[20];

  if (completion_context == NULL || completion_context->sobj == NULL ||
      frame == NULL ||
      payload_len != sizeof(*frame) +
          sizeof(netbus_fs_link_status_ind_cmd_msg_t)) {
    return -1;
  }

  status = (netbus_fs_link_status_ind_cmd_msg_t *)frame->payload;
  blog_info("get link status indicate!\r\n");
  wifi_info_gate(&completion_context->sobj->vnet.netif,
                 &ip.addr, &gw.addr, &mask.addr);
  strcpy(ip_str, ip4addr_ntoa(&ip));
  strcpy(gw_str, ip4addr_ntoa(&gw));
  strcpy(mask_str, ip4addr_ntoa(&mask));
  blog_info("%s,%s,%s\r\n", ip_str, gw_str, mask_str);
  if (status->record.link_status == BF1B_WIFI_LINK_STATUS_UP) {
    blog_info("link status up!\r\n");
  } else if (status->record.link_status == BF1B_WIFI_LINK_STATUS_DOWN) {
    blog_info("link status down!\r\n");
  } else {
    blog_info("link status unknown!\r\n");
  }

  if (strcmp(ip_str, "0.0.0.0") && status->record.mgmr_state == 3) {
    status->record.mgmr_state = 4;
  }
  blog_info("state:%d\r\n", status->record.mgmr_state);
  blog_info("ssid:%.*s\r\n", (int)sizeof(status->record.ssid),
            (const char *)status->record.ssid);
  blog_info("bssid: %02x%02x%02x%02x%02x%02x\r\n",
            status->record.bssid[0], status->record.bssid[1],
            status->record.bssid[2], status->record.bssid[3],
            status->record.bssid[4], status->record.bssid[5]);

  return event_propagate(completion_context->sobj,
                         VIRT_NET_EV_ON_LINK_STATUS_DONE, frame);
}

static int complete_dev_version(void *context, const void *payload,
                                size_t payload_len)
{
  struct virt_net_completion_context *completion_context = context;
  const struct pkg_protocol *frame = payload;
  const struct pkg_protocol_cmd *pkg_cmd;

  if (completion_context == NULL || completion_context->response == NULL ||
      frame == NULL ||
      payload_len != sizeof(*frame) + sizeof(*pkg_cmd) + sizeof(uint32_t)) {
    return -1;
  }

  pkg_cmd = (const struct pkg_protocol_cmd *)frame->payload;
  memcpy(completion_context->response, pkg_cmd->payload, sizeof(uint32_t));

  return 0;
}

static int complete_pending_response(struct pkg_protocol *pkg_header)
{
  struct pkg_protocol_cmd *pkg_cmd =
      (struct pkg_protocol_cmd *)pkg_header->payload;
  struct virt_net_pending_entry snapshot;
  struct virt_net_waiter *waiter;
  void *finished_waiter = NULL;
  int completion_result;
  int finish_result;

  xSemaphoreTake(slot_mutex, portMAX_DELAY);
  if (virt_net_pending_claim(&pending_table, pkg_cmd->msg_id, pkg_cmd->cmd,
                             &snapshot) != VIRT_NET_PENDING_CLAIMED) {
    xSemaphoreGive(slot_mutex);
    return 0;
  }
  xSemaphoreGive(slot_mutex);

  completion_result = snapshot.complete(
      snapshot.context, pkg_header,
      sizeof(*pkg_header) + (size_t)pkg_header->length);

  waiter = (struct virt_net_waiter *)snapshot.waiter;
  xSemaphoreTake(slot_mutex, portMAX_DELAY);
  waiter->result = completion_result;
  finish_result = virt_net_pending_finish(&pending_table, snapshot.token,
                                          &finished_waiter);
  assert(finish_result == 0);
  assert(finished_waiter == waiter);
  if (finish_result == 0 && finished_waiter == waiter) {
    xSemaphoreGive(waiter->sem);
  }
  xSemaphoreGive(slot_mutex);

  return completion_result;
}

static int wait_for_pending_response(struct virt_net_waiter *waiter,
                                     uint16_t token, uint32_t timeout)
{
  enum virt_net_pending_state state;

  if (xSemaphoreTake(waiter->sem, timeout) == pdTRUE) {
    return waiter->result;
  }

  xSemaphoreTake(slot_mutex, portMAX_DELAY);
  state = virt_net_pending_cancel(&pending_table, token);
  switch (state) {
  case VIRT_NET_PENDING_WAITING:
    xSemaphoreGive(slot_mutex);
    return -1;
  case VIRT_NET_PENDING_COMPLETING:
    xSemaphoreGive(slot_mutex);
    if (xSemaphoreTake(waiter->sem, portMAX_DELAY) != pdTRUE) {
      return -1;
    }
    return waiter->result;
  case VIRT_NET_PENDING_FREE:
    xSemaphoreGive(slot_mutex);
    if (xSemaphoreTake(waiter->sem, 0) != pdTRUE) {
      return -1;
    }
    return waiter->result;
  default:
    xSemaphoreGive(slot_mutex);
    return -1;
  }
}

static int pkg_protocol_cmd_handler(struct virt_net_ramsync *sobj, struct pkg_protocol *pkg_header) 
{
  struct pkg_protocol_cmd *pkg_cmd = (struct pkg_protocol_cmd *)pkg_header->payload;

  blog_info("pkg_protocol_cmd_handler cmd:0x%x.\r\n", pkg_cmd->cmd);
  switch (pkg_cmd->cmd) {
  case VIRT_NET_CTRL_AP_CONNECTED_IND:
    {
      netifapi_netif_set_link_up((&sobj->vnet.netif));
      sobj->sta_linkup = 1;

      blog_info("start dhcp...\r\n");
      netifapi_netif_set_up(&sobj->vnet.netif);
#if LWIP_IPV6
      LOCK_TCPIP_CORE();
      netif_create_ip6_linklocal_address(&sobj->vnet.netif, 1);
      UNLOCK_TCPIP_CORE();
#if LWIP_IPV6_AUTOCONFIG
      sobj->vnet.netif.ip6_autoconfig_enabled = 1;
#endif
      // dhcp6_set_struct(&sobj->vnet.netif, &sobj->vnet_dhcp6);
      // dhcp6_enable_stateless(&sobj->vnet.netif);
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
  case VIRT_NET_CTRL_SCAN_IND:
  case VIRT_NET_CTRL_GET_LINK_STATUS_IND:
  case VIRT_NET_CTRL_GET_DEV_VERSION_IND:
    complete_pending_response(pkg_header);
    break;
  case VIRT_NET_CTRL_SLAVE_READY_IND:
    {
        printf("slave ready ind.\r\n");
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
        event_propagate(sobj, VIRT_NET_EV_ON_SLAVE_READY, NULL);

        if (sobj->vnet.ctrl) {
            sobj->vnet.ctrl(&sobj->vnet, VIRT_NET_CTRL_CMD_CFM, VIRT_NET_CTRL_SLAVE_READY_IND);
        }
    }
    break;
  case VIRT_NET_CTRL_SLAVE_HEARTBEAT:
    {
        event_propagate(sobj, VIRT_NET_EV_ON_SLAVE_HEARTBEAT, NULL);
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
  int rx_result;
  assert(sobj->task_virt_net == xTaskGetCurrentTaskHandle());

  while (sobj->running) {

    rx_buffer = __alloc_rxbuf(sobj);
    if (rx_buffer == NULL) {
      printf("no rx buffer, suspend..\r\n");
      vTaskSuspend(NULL);
      continue;
    }
 
    pkg_header = (struct pkg_protocol *)(rx_buffer + VIRT_NET_RXBUFF_OFFSET);

    recvd_len = 0;
    rx_result = uramsync_rx_pop(&sobj->ramsync_ctx, (uint8_t *)pkg_header,
                                &recvd_len, URAMSYNC_FOREVER);
    if (rx_result != 0) {
      blog_info("Drop virt-net RX failure, result:%d.\r\n", rx_result);
      __free_rxbuf(sobj, rx_buffer);
      continue;
    }
    if (virt_net_frame_validate(pkg_header, recvd_len) != 0) {
      blog_info("Drop malformed virt-net frame, recvd_len:%d.\r\n", recvd_len);
      __free_rxbuf(sobj, rx_buffer);
      continue;
    }
    blog_info("Recv pkg_header->len:%d type:0x%x recvd_len %d.\r\n\r\n", pkg_header->length, pkg_header->type, recvd_len);

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
  struct virt_net_waiter response_waiter = {0};
  struct virt_net_completion_context completion_context = {
    .sobj = sobj,
    .response = NULL,
  };
  virt_net_pending_complete_fn completion = NULL;
  va_list args;
  const uint8_t *ssid = NULL;
  const uint8_t *passphrase = NULL;
  uint8_t *response_context = NULL;
  unsigned int ssid_len = 0;
  unsigned int passphrase_len = 0;
  uint16_t confirm_cmd = 0;
  uint16_t expected_response_cmd = 0;
  uint16_t msg_id = VIRT_NET_PENDING_UNSOLICITED_TOKEN;
  SemaphoreHandle_t response_sem = NULL;
  StaticSemaphore_t sem_buffer;
  uint32_t response_timeout = 0;
  const char *response_error = NULL;
  int acquire_result;
  int tx_result;
  size_t frame_length;

  switch (cmd) {
  case VIRT_NET_CTRL_CONNECT_AP:
    va_start(args, cmd);
    ssid_len = va_arg(args, unsigned int);
    passphrase_len = va_arg(args, unsigned int);
    ssid = va_arg(args, const uint8_t *);
    passphrase = va_arg(args, const uint8_t *);
    va_end(args);
    if (ssid_len > sizeof(((struct pkg_cmd_connect_ap *)0)->ssid) ||
        passphrase_len > sizeof(((struct pkg_cmd_connect_ap *)0)->passphr) ||
        (ssid_len != 0U && ssid == NULL) ||
        (passphrase_len != 0U && passphrase == NULL) ||
        sizeof(*pkg_header) + sizeof(*pkg_cmd) +
            sizeof(struct pkg_cmd_connect_ap) > TP_PAYLOAD_LEN) {
      printf("ssid_len or passphr_len too long \r\n");
      return -1;
    }
    break;
  case VIRT_NET_CTRL_GET_MAC:
    va_start(args, cmd);
    response_context = va_arg(args, uint8_t *);
    va_end(args);
    completion_context.response = response_context;
    expected_response_cmd = VIRT_NET_CTRL_MAC_IND;
    completion = complete_mac;
    break;
  case VIRT_NET_CTRL_START_SCAN:
    expected_response_cmd = VIRT_NET_CTRL_SCAN_IND;
    completion = complete_scan;
    break;
  case VIRT_NET_CTRL_GET_LINK_STATUS:
    expected_response_cmd = VIRT_NET_CTRL_GET_LINK_STATUS_IND;
    completion = complete_link_status;
    break;
  case VIRT_NET_CTRL_GET_DEV_VERSION:
    va_start(args, cmd);
    response_context = va_arg(args, uint8_t *);
    va_end(args);
    completion_context.response = response_context;
    expected_response_cmd = VIRT_NET_CTRL_GET_DEV_VERSION_IND;
    completion = complete_dev_version;
    break;
  case VIRT_NET_CTRL_CMD_CFM:
    va_start(args, cmd);
    confirm_cmd = (uint16_t)va_arg(args, unsigned int);
    va_end(args);
    break;
  default:
    break;
  }

  if (completion != NULL) {
    response_sem = xSemaphoreCreateBinaryStatic(&sem_buffer);
    response_waiter.sem = response_sem;
    response_waiter.result = -1;
    assert(response_waiter.sem != NULL);

    xSemaphoreTake(slot_mutex, portMAX_DELAY);
    acquire_result = virt_net_pending_acquire(
        &pending_table, expected_response_cmd, &response_waiter,
        &completion_context, completion, &msg_id);
    xSemaphoreGive(slot_mutex);
    if (acquire_result != 0) {
      printf("Bug? too many pending commands\r\n");
      return -1;
    }
  }

  xSemaphoreTake(tx_mutex, portMAX_DELAY);

  switch (cmd) {
  case VIRT_NET_CTRL_HELLO:
    pkg_header->type = PKG_CMD_FRAME;
    pkg_header->length = sizeof(struct pkg_protocol_cmd);
    pkg_cmd->cmd = VIRT_NET_CTRL_HELLO;
    pkg_cmd->msg_id = UINT16_MAX;
    blog_info("send hello\r\n");
    break;
  case VIRT_NET_CTRL_CONNECT_AP:
    {
      struct pkg_cmd_connect_ap *connect_ap_cmd = (struct pkg_cmd_connect_ap *)&pkg_cmd->payload;

      pkg_header->type = PKG_CMD_FRAME;
      pkg_header->length = sizeof(struct pkg_protocol_cmd) + sizeof(struct pkg_cmd_connect_ap);
      pkg_cmd->cmd = VIRT_NET_CTRL_CONNECT_AP;
      pkg_cmd->msg_id = UINT16_MAX;
      connect_ap_cmd->ssid_len = (uint16_t)ssid_len;

      /* Copy ssid */
      memset(connect_ap_cmd->ssid, 0, sizeof(connect_ap_cmd->ssid));
      memset(connect_ap_cmd->ssid_tail, 0, sizeof(connect_ap_cmd->ssid_tail));
      if (ssid_len != 0U) {
        memcpy(connect_ap_cmd->ssid, ssid, ssid_len);
      }

      /* Copy passphr */
      memset(connect_ap_cmd->passphr, 0, sizeof(connect_ap_cmd->passphr));
      memset(connect_ap_cmd->psk_tail, 0, sizeof(connect_ap_cmd->psk_tail));
      if (passphrase_len != 0U) {
        memcpy(connect_ap_cmd->passphr, passphrase, passphrase_len);
      }

      blog_info("connect ap ssid:%.*s, password_set:%d, password_len:%u\r\n",
                (int)ssid_len, (const char *)connect_ap_cmd->ssid,
                passphrase_len != 0U, passphrase_len);
    }
    break;
  case VIRT_NET_CTRL_DISCONNECT_AP:
    pkg_header->type = PKG_CMD_FRAME;
    pkg_header->length = sizeof(struct pkg_protocol_cmd);
    pkg_cmd->cmd = VIRT_NET_CTRL_DISCONNECT_AP;
    pkg_cmd->msg_id = UINT16_MAX;
    blog_info("send disconnect\r\n");
    break;
  case VIRT_NET_CTRL_GET_MAC:
    pkg_header->type = PKG_CMD_FRAME;
    pkg_header->length = sizeof(struct pkg_protocol_cmd);
    pkg_cmd->cmd = VIRT_NET_CTRL_GET_MAC;
    pkg_cmd->msg_id = msg_id;
    response_timeout = 3000;
    response_error = "get mac failed!!!!\r\n";
    blog_info("get mac..\r\n");
    break;
  case VIRT_NET_CTRL_START_SCAN:
    pkg_header->type = PKG_CMD_FRAME;
    pkg_header->length = sizeof(struct pkg_protocol_cmd);
    pkg_cmd->cmd = VIRT_NET_CTRL_START_SCAN;
    pkg_cmd->msg_id = msg_id;
    response_timeout = 10000;
    response_error = "scan failed!!!!\r\n";
    blog_info("scan..\r\n");
    break;
  case VIRT_NET_CTRL_GET_LINK_STATUS:
    pkg_header->type = PKG_CMD_FRAME;
    pkg_header->length = sizeof(struct pkg_protocol_cmd);
    pkg_cmd->cmd = VIRT_NET_CTRL_GET_LINK_STATUS;
    pkg_cmd->msg_id = msg_id;
    response_timeout = 3000;
    response_error = "get link status failed!!!!\r\n";
    blog_info("get link status..\r\n");
    break;
  case VIRT_NET_CTRL_HBN:
    blog_info("enter hbn\r\n");
    pkg_header->type = PKG_CMD_FRAME;
    pkg_header->length = sizeof(struct pkg_protocol_cmd);
    pkg_cmd->cmd = VIRT_NET_CTRL_HBN;
    pkg_cmd->msg_id = UINT16_MAX;
    break;
  case VIRT_NET_CTRL_GET_DEV_VERSION:
    pkg_header->type = PKG_CMD_FRAME;
    pkg_header->length = sizeof(struct pkg_protocol_cmd);
    pkg_cmd->cmd = VIRT_NET_CTRL_GET_DEV_VERSION;
    pkg_cmd->msg_id = msg_id;
    response_timeout = 3000;
    response_error = "get slave version failed!!!!\r\n";
    blog_info("get slave version..\r\n");
    break;
  case VIRT_NET_CTRL_CMD_CFM:
    {
      struct pkg_cmd_confirm *cfm = (struct pkg_cmd_confirm *)&pkg_cmd->payload;

      pkg_header->type = PKG_CMD_FRAME;
      pkg_header->length = sizeof(struct pkg_protocol_cmd) + sizeof(struct pkg_cmd_confirm);
      pkg_cmd->cmd = VIRT_NET_CTRL_CMD_CFM;
      pkg_cmd->msg_id = UINT16_MAX;
      cfm->cmdId = confirm_cmd;
      blog_info("send cfm for cmdId=%04x\r\n", cfm->cmdId);
    }
    break;
  default:
    printf("unsupport cmd:%d\r\n", cmd);
    xSemaphoreGive(tx_mutex);
    if (completion != NULL) {
      (void)wait_for_pending_response(&response_waiter, msg_id, 0);
    }
    return -1;
  }

  frame_length = sizeof(*pkg_header) + pkg_header->length;
  if (frame_length > TP_PAYLOAD_LEN) {
    xSemaphoreGive(tx_mutex);
    if (completion != NULL) {
      (void)wait_for_pending_response(&response_waiter, msg_id, 0);
    }
    return -1;
  }

  tx_result = uramsync_tx_push_toback(&sobj->ramsync_ctx, tx_buffer,
                                      frame_length, URAMSYNC_FOREVER);
  blog_info("CTRL pkg_header->len:%d type:0x%x\r\n\r\n", pkg_header->length, pkg_header->type);
  xSemaphoreGive(tx_mutex);

  if (tx_result != 0) {
    if (completion != NULL) {
      (void)wait_for_pending_response(&response_waiter, msg_id, 0);
    }
    return tx_result;
  }

  if (completion != NULL) {
    if (xSemaphoreTake(response_sem, response_timeout) == pdTRUE) {
      tx_result = response_waiter.result;
    } else {
      tx_result = wait_for_pending_response(&response_waiter, msg_id, 0);
    }
    if (tx_result != 0) {
      printf("%s", response_error);
    }
  }

  return tx_result;
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
    slot_mutex = xSemaphoreCreateMutexStatic(&xSlotMutexBuffer);
    tx_mutex = xSemaphoreCreateMutexStatic(&xTxMutexBuffer);
    if (slot_mutex == NULL || tx_mutex == NULL) {
      printf("create virt_net command mutex failed\r\n");
      if (slot_mutex != NULL) {
        vSemaphoreDelete(slot_mutex);
        slot_mutex = NULL;
      }
      if (tx_mutex != NULL) {
        vSemaphoreDelete(tx_mutex);
        tx_mutex = NULL;
      }
      return NULL;
    }
    virt_net_pending_init(&pending_table, 0U);
    inited = 1;
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
  sobj->rx_buf_ind.__bits[0] = ((uint64_t)1 << VIRT_NET_RXBUFF_CNT) - 1;

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
  free(tx_buffer);
  tx_buffer = NULL;
_errout_3:
  free(sobj->rx_buff);
_errout_2:
  vSemaphoreDelete(sobj->rx_buf_ind_mutex);
_errout_1:
  uramsync_deinit(&sobj->ramsync_ctx);
  free(sobj);

  return NULL;
}

int virt_net_delete(void *ctx) 
{
  if (!inited) {
    return 0;
  }

  struct virt_net_ramsync *sobj = ctx;

  if (sobj == NULL) {
    return -1;
  }

  vTaskDelete(sobj->task_virt_net);

  uramsync_deinit(&sobj->ramsync_ctx);

  if (sobj->rx_buf_ind_mutex){
    vSemaphoreDelete(sobj->rx_buf_ind_mutex);
    sobj->rx_buf_ind_mutex = NULL;
  }

  if (sobj->rx_buff){
    free(sobj->rx_buff);
    sobj->rx_buff = NULL;
  }

  if(tx_buffer){
    free(tx_buffer);
    tx_buffer = NULL;
  }

  free(sobj);
  inited = 0;

  return 0;
}
