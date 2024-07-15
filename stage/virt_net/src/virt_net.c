#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "virt_net.h"

void virt_net_setup_callback(virt_net_t obj, virt_net_event_callback cb, void *opaque) {
  assert(obj != NULL);
  struct virt_net *obj_t = (struct virt_net *)obj;

  obj_t->opaque = opaque;
  obj_t->event_cb = cb;
}

int virt_net_send_nop(virt_net_t obj) {
  assert(obj != NULL);

  return obj->ctrl(obj, VIRT_NET_CTRL_HELLO);
}

int virt_net_get_mac(virt_net_t obj, uint8_t mac[6]) {
  assert(obj != NULL);
  assert(mac != NULL);

  return obj->ctrl(obj, VIRT_NET_CTRL_GET_MAC, mac);
}

int virt_net_connect_ap(virt_net_t obj, const char *ssid, const char *passphr) {
  assert(obj != NULL);
  assert(ssid != NULL);
  unsigned int ssid_len = strlen(ssid);
  unsigned int passphr_len = passphr ? strlen(passphr) : 0;
  passphr = passphr ? passphr : "";

  return obj->ctrl(obj, VIRT_NET_CTRL_CONNECT_AP,
      ssid_len, passphr_len,
      (const uint8_t *)ssid, (const uint8_t *)passphr);
}

int virt_net_disconnect(virt_net_t obj) {
  if (obj == NULL) {
	  return -1;
  }
  return obj->ctrl(obj, VIRT_NET_CTRL_DISCONNECT_AP);
}

int virt_net_scan(virt_net_t obj) {
  assert(obj != NULL);

  return obj->ctrl(obj, VIRT_NET_CTRL_START_SCAN);
}

int virt_net_get_link_status(virt_net_t obj) {
  assert(obj != NULL);

  return obj->ctrl(obj, VIRT_NET_CTRL_GET_LINK_STATUS);
}

int virt_net_enter_hbn(virt_net_t obj) {
  assert(obj != NULL);

  return obj->ctrl(obj, VIRT_NET_CTRL_HBN);
}

int virt_net_slave_version(virt_net_t obj, uint32_t * version)
{
    assert(obj != NULL);
    assert(version != NULL);

    return obj->ctrl(obj, VIRT_NET_CTRL_GET_DEV_VERSION, (uint8_t*)version);
}