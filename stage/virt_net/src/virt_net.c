/*
 * Copyright (c) 2016-2023 Bouffalolab.
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
