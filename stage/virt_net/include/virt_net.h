#ifndef _VIRT_NET_H_
#define _VIRT_NET_H_


#ifdef __cplusplus
extern "C" {
#endif
  
#include <lwip/netifapi.h>
#include "pkg_protocol.h"

#ifndef VIRT_NET_TASK_PRI
#define VIRT_NET_TASK_PRI (19)
#endif

typedef const struct virt_net *virt_net_t;

enum virt_net_event_code {
  VIRT_NET_EV_ON_INIT_DONE = 1,
  VIRT_NET_EV_ON_MGMR_DONE,
  VIRT_NET_EV_CMD_RECONNECT,
  VIRT_NET_EV_ON_CONNECTED,
  VIRT_NET_EV_ON_DISCONNECT,
  VIRT_NET_EV_ON_PRE_GOT_IP,
  VIRT_NET_EV_ON_GOT_IP,
  VIRT_NET_EV_ON_CONNECTING,
  VIRT_NET_EV_ON_SCAN_DONE,
  VIRT_NET_EV_ON_SCAN_DONE_ONJOIN,
  VIRT_NET_EV_ON_AP_STARTED,
  VIRT_NET_EV_ON_AP_STOPPED,
  VIRT_NET_EV_ON_PROV_SSID,
  VIRT_NET_EV_ON_PROV_BSSID,
  VIRT_NET_EV_ON_PROV_PASSWD,
  VIRT_NET_EV_ON_PROV_CONNECT,
  VIRT_NET_EV_ON_PROV_DISCONNECT,
  VIRT_NET_EV_ON_PROV_SCAN_START,
  VIRT_NET_EV_ON_PROV_STATE_GET,
  VIRT_NET_EV_ON_MGMR_DENOISE,
  VIRT_NET_EV_ON_AP_STA_ADD,
  VIRT_NET_EV_ON_AP_STA_DEL,
  VIRT_NET_EV_ON_EMERGENCY_MAC,
  VIRT_NET_EV_ON_LINK_STATUS_DONE,
};

/* Init virt net */
typedef int (*virt_net_init)(virt_net_t obj);

typedef int (*virt_net_control)(virt_net_t obj, int cmd, ...);
typedef int (*virt_net_deinit)(virt_net_t obj);

/* event callback */
/* NOTE: SHOULD NOT block in event callback */
typedef int (*virt_net_event_callback)(virt_net_t obj, enum virt_net_event_code code, void *param);

struct virt_net {
  unsigned int mtu;
  unsigned char mac[6];
#if 0
  unsigned int flags;
#endif

  struct netif netif;

  virt_net_init    init;
  virt_net_deinit  deinit;
  virt_net_control ctrl;

  /* event */
  void *opaque;
  virt_net_event_callback event_cb;
};

/* Control command */
enum virt_net_cmd{
  VIRT_NET_CTRL_RESET = 0x1000,
  VIRT_NET_CTRL_HELLO,
  VIRT_NET_CTRL_GET_MAC,
  VIRT_NET_CTRL_MAC_IND,
  VIRT_NET_CTRL_CONNECT_AP,
  VIRT_NET_CTRL_START_AP,
  VIRT_NET_CTRL_AP_CONNECTED_IND,
  VIRT_NET_CTRL_AP_DISCONNECTED_IND,
  VIRT_NET_CTRL_STA_IP_UPDATE_IND,
  VIRT_NET_CTRL_STA_SET_AUTO_RECONNECT,
  VIRT_NET_CTRL_SET_LPM_MODE,
  VIRT_NET_CTRL_START_SCAN,
  VIRT_NET_CTRL_SCAN_IND,
  VIRT_NET_CTRL_GET_STA_LIST,
  VIRT_NET_CTRL_GET_STA_LIST_IND,
  VIRT_NET_CTRL_GET_LINK_STATUS,
  VIRT_NET_CTRL_GET_LINK_STATUS_IND,
  VIRT_NET_CTRL_START_MONITOR,
  VIRT_NET_CTRL_STOP_MONITOR,
  VIRT_NET_CTRL_SET_CHANNEL,
  VIRT_NET_CTRL_GET_CHANNEL,
  VIRT_NET_CTRL_GET_CHANNEL_IND,

  // OTA related
  VIRT_NET_CTRL_GET_DEV_VERSION,
  VIRT_NET_CTRL_GET_DEV_VERSION_IND,
  VIRT_NET_CTRL_OTA,

  VIRT_NET_CTRL_SHELLCODE,
  VIRT_NET_CTRL_SHELLCODE_IND,

  VIRT_NET_CTRL_DISCONNECT_AP,
  VIRT_NET_CTRL_STOP_AP,

  VIRT_NET_CTRL_PING,
  VIRT_NET_CTRL_HBN,

  VIRT_NET_CTRL_EXT,
};

typedef int (*virt_net_cmd_callback_t)(virt_net_t obj, enum virt_net_cmd cmd, void *param);

virt_net_t virt_net_create(void *ctx);
int virt_net_delete(void *ctx);
void virt_net_setup_callback(virt_net_t obj, virt_net_event_callback cb, void *opaque);
int virt_net_send_nop(virt_net_t obj);
int virt_net_connect_ap(virt_net_t obj, const char *ssid, const char *passphr);
int virt_net_disconnect(virt_net_t obj);
int virt_net_get_mac(virt_net_t obj, uint8_t mac[6]);
int virt_net_scan(virt_net_t obj);
int virt_net_get_link_status(virt_net_t obj);
int virt_net_enter_hbn(virt_net_t obj);
int virt_net_slave_version(virt_net_t obj, uint32_t * version);

#ifdef __cplusplus
}
#endif

#endif
