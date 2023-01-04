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
#ifndef _PKG_PROTOCOL_H_
#define _PKG_PROTOCOL_H_

#define PKG_DATA_FRAME 0x7878
#define PKG_CMD_FRAME 0xef01

typedef enum {
    BF1B_WIFI_LINK_STATUS_UNKNOWN = 0,
    BF1B_WIFI_LINK_STATUS_DOWN,
    BF1B_WIFI_LINK_STATUS_UP,
} bflbwifi_link_status_t;

struct pkg_protocol {
  uint16_t type;
  uint16_t length;
  uint8_t payload[0];
};

/* 连接AP */
struct pkg_cmd_connect_ap {
  uint16_t ssid_len;
  uint8_t ssid[32];
  uint8_t ssid_tail[1];
  uint8_t passphr[64];
  uint8_t psk_tail[1];
} __attribute__((packed));

struct pkg_protocol_cmd {
  uint16_t cmd;
  uint16_t msg_id;
  uint8_t payload[0];
} __attribute__((packed));

struct bflbwifi_scan_record {
    uint8_t bssid[6];
    uint8_t ssid[32 + 1];
    uint8_t channel;
    int8_t rssi;
    uint8_t auth_mode;
    uint8_t cipher;
};

#pragma pack(push, 1)
typedef struct {
    uint16_t cmd;
    uint16_t msg_id;
} netbus_min_cmd_msg_t;
typedef struct {
    netbus_min_cmd_msg_t hdr;
    uint16_t num;
    struct bflbwifi_scan_record records[];
} netbus_fs_scan_ind_cmd_msg_t;

typedef struct {
    uint16_t cmd;
    uint16_t msg_id;
    void *data_ptr;
    uint16_t data_len;
} netbus_wifi_mgmr_msg_cmd_t;

struct bflbwifi_ap_record {
    int mgmr_state;
    uint8_t link_status;
    uint8_t bssid[6];
    uint8_t ssid[32 + 1];
    uint8_t channel;
    int8_t rssi;
    uint8_t auth_mode;
    uint8_t cipher;
};

typedef struct {
    netbus_min_cmd_msg_t hdr;
    struct bflbwifi_ap_record record;
} netbus_fs_link_status_ind_cmd_msg_t;

#pragma pack(pop)
#endif
