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
#ifndef __NETBUS_TRANSCEIVER_H__
#define __NETBUS_TRANSCEIVER_H__
#include <stdint.h>
#include <stdbool.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <lwip/netif.h>
#include <lwip/pbuf.h>

#define BF1B_MSG_TYPE_ETH_WIFI_FRAME 0x7878
#define BF1B_MSG_TYPE_CMD            0xef01

/*
 * 0->tx desc cnt
 * 1..6-> mac
 * 7->boot src info
 * 1->tty0 rcv buf rem(reuse)
 * 2->tty1 rcv buf rem(reuse)
 */
#define WIFI_MAC_ADDR_SCRATCH_OFFSET   1
#define BOOT_SRC_INFO_SCRATCH_OFFSET   (1  + 6)
#define TTY_RCV_BUF_REM_OFFSET(tty_id) (1  + (tty_id))
#define BL_BOOT_SRC_PING_VALUE         0xEE
#define BL_BOOT_SRC_PONG_VALUE         0xEF
#define BOOT_SRC_PONG_TIMEOUT_MS       (10 * 1000)

enum {
    BF1B_MSG_ETH_WIFI_FRAME_SUBTYPE_STA_TO_WIFI_TX = 0x2000,
    BF1B_MSG_ETH_WIFI_FRAME_SUBTYPE_STA_FROM_WIFI_RX,
    BF1B_MSG_ETH_WIFI_FRAME_SUBTYPE_AP_TO_WIFI_TX,
    BF1B_MSG_ETH_WIFI_FRAME_SUBTYPE_AP_FROM_WIFI_RX,
    BF1B_MSG_ETH_WIFI_FRAME_SUBTYPE_SNIFFER_TO_WIFI_TX,
    BF1B_MSG_ETH_WIFI_FRAME_SUBTYPE_SNIFFER_FROM_WIFI_RX,
    BF1B_MSG_ETH_WIFI_FRAME_SUBTYPE_EXT, // UART, etc
};

typedef struct {
    uint16_t type;
    uint16_t len; // type + len + payload
    uint8_t payload[];
} __attribute__((packed)) netbus_msg_t;

typedef struct {
    void *arg;

    bool host_present;

    SemaphoreHandle_t tx_lock;
    StaticSemaphore_t tx_lock_buf;
} net_wifi_trcver_ctx_t;

int bflbnet_wifi_trcver_init(net_wifi_trcver_ctx_t *ctx, const void *arg);
int bflb_net_wifi_trcver_set_present(net_wifi_trcver_ctx_t *ctx, bool present);

int bflbmsg_send(net_wifi_trcver_ctx_t *ctx, const uint16_t type,
        const void *payload, const uint16_t payload_len);
int bflbmsg_send_pbuf(net_wifi_trcver_ctx_t *ctx, const uint16_t type, const uint16_t subtype,
        struct pbuf *p, bool is_amsdu, void *cb, void *cb_arg);

void bflbwrite_s_reg(net_wifi_trcver_ctx_t *ctx, const uint32_t offset, const uint8_t val);

#endif
