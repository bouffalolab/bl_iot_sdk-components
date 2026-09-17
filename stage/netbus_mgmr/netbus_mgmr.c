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
#include <stdbool.h>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <timers.h>
#include <lwip/netif.h>
#include <blog.h>
#include <bl_wifi.h>
#include <wifi_mgmr_ext.h>

#include "netbus_wifi_mgmr_cmd_handlers.h"
#include "netbus_mgmr.h"

netbus_wifi_mgmr_ctx_t g_netbus_wifi_mgmr_env;

#define INITIAL_AVAILABLE_TX_SLOTS 1
uint8_t available_tx_slots = INITIAL_AVAILABLE_TX_SLOTS;

#define FRAME_LEN_MAX 2000

static void handle_eth_frame_to_wifi_tx(netbus_wifi_mgmr_ctx_t *env, netbus_wifi_mgmr_msg_cmd_t *cmd, bool is_sta)
{
err_t wifi_tx(struct netif *netif, struct pbuf* p);
    struct pbuf *p = cmd->data_ptr;
    struct netif *iface = NULL;

    if (is_sta) {
        iface = env->sta_netif;
    } else {
        iface = env->ap_netif;
    }
    if (!iface) {
        goto free;
    }

    struct pbuf *p_or_1 = (struct pbuf *)((uintptr_t)p | 1);

    if (wifi_tx(iface, p_or_1) != ERR_OK) {
        pbuf_free(p);
        // extern int get_tx_pbuf_free_count(netbus_wifi_mgmr_ctx_t *env);
        // bflbwrite_s_reg(&env->trcver_ctx, 0, get_tx_pbuf_free_count(env));
    }
free:
    return;
}

static void netbufwifi_frame_mgmr_task(void *pvParameters)
{
    netbus_wifi_mgmr_ctx_t *env = (netbus_wifi_mgmr_ctx_t *)pvParameters;
    netbus_wifi_mgmr_msg_t msg;

    while (1) {
        if (pdPASS != xQueueReceive(env->frame_msg_queue, &msg, portMAX_DELAY)) {
            continue;
        }
        blog_info("netbufwifi_frame_mgmr_task msg.type 0x%x.\r\n", msg.type);
        switch (msg.type) {
        case NETBUS_WIFI_MGMR_MSG_TYPE_ETH_FRAME_STA_TO_WIFI_TX:
        {
            handle_eth_frame_to_wifi_tx(env, &msg.u.cmd, true);
        }
        break;
        case NETBUS_WIFI_MGMR_MSG_TYPE_ETH_FRAME_STA_FROM_WIFI_RX:
        {
            struct pbuf *q = msg.u.eth_frame.p;
            if (q->tot_len > FRAME_LEN_MAX) {
                goto free;
            }
free:
            pbuf_free(q);
        }
        break;
        case NETBUS_WIFI_MGMR_MSG_TYPE_ETH_FRAME_AP_FROM_WIFI_RX:
        {
            struct pbuf *q = msg.u.eth_frame.p;
            if (q->tot_len > FRAME_LEN_MAX) {
                goto free_ap_wrx;
            }
free_ap_wrx:
            pbuf_free(q);
        }
        break;
        case NETBUS_WIFI_MGMR_MSG_TYPE_ETH_FRAME_AP_TO_WIFI_TX:
        {
            handle_eth_frame_to_wifi_tx(env, &msg.u.cmd, false);
        }
        break;
        case NETBUS_WIFI_MGMR_MSG_TYPE_WIFI_FRAME_SNIFFER_TO_WIFI_TX:
        {
            wifi_mgmr_raw_80211_send(msg.u.cmd.data_ptr, msg.u.cmd.data_len);
            blog_info("TX RAW Frame len %u, first 4 bytes:\r\n", msg.u.cmd.data_len);
            log_buf(msg.u.cmd.data_ptr, msg.u.cmd.data_len >= 4 ? 4 : msg.u.cmd.data_len);
            vPortFree(msg.u.cmd.data_ptr);
        }
        break;
        case NETBUS_WIFI_MGMR_MSG_TYPE_CMD:
        {
            netbus_wifi_mgmr_cmd_entry(env, &msg.u.cmd);
        }
        break;
        default:
        break;
        }
    }
}

static void netbuswifi_cmd_mgmr_task(void *pvParameters)
{
    netbus_wifi_mgmr_ctx_t *env = (netbus_wifi_mgmr_ctx_t *)pvParameters;
    netbus_wifi_mgmr_msg_t msg;

    while (1) {
        if (pdPASS != xQueueReceive(env->cmd_msg_queue, &msg, portMAX_DELAY)) {
            continue;
        }
        switch (msg.type) {
        case NETBUS_WIFI_MGMR_MSG_TYPE_CMD:
        {
            netbus_wifi_mgmr_cmd_entry(env, &msg.u.cmd);
        }
        break;
        default:
        break;
        }
    }
}

#ifndef NETBUS_TX_PBUF_CNT
/* 25 = what actually fits: pbuf_alloc(PBUF_RAM, 1600) draws from the 42 KiB
 * lwIP heap (lwipopts.h MEM_SIZE under CFG_NETBUS_WIFI_ENABLE) and only 25
 * succeed at boot — init_tx_desc stops at the first allocation failure.
 * Sizing this to 25 keeps the free-count arithmetic honest. Raise MEM_SIZE
 * (>= 56 KiB) if a larger pool is really wanted. */
#define NETBUS_TX_PBUF_CNT 25
#endif

#ifndef NETBUS_TX_PBUF_SIZE
#define NETBUS_TX_PBUF_SIZE 1600
#endif

static void init_tx_desc(netbus_wifi_mgmr_ctx_t *env)
{
    int i;
    int alloced_num = 0;
    struct tx_pbuf_desc *desc = &env->tx_desc;
    struct pbuf *p;

    memset(desc, 0, sizeof(*desc));
    for (i = 0; i < TX_PBUF_DESC_PBUF_MAX_NUM; ++i) {
        p = pbuf_alloc(PBUF_RAW_TX, NETBUS_TX_PBUF_SIZE, PBUF_RAM);
        if (p) {
            blog_info("preallocated pbuf %p, p->next %p\r\n", p, p->next);
            desc->pbufs[i] = p;
            // backup
            memcpy(&desc->pbufs_back[i], p, sizeof(*p));
            alloced_num++;
            if (alloced_num >= NETBUS_TX_PBUF_CNT) {
                break;
            }
        } else {
            blog_info("pbuf alloc failed\r\n");
            break;
        }
    }

#if 0
    blog_info("Preallocated 1514 pbufs num: %d\r\n", alloced_num);
    blog_info("Freeing 2 for local use.\r\n");
    pbuf_free(desc->pbufs[alloced_num - 1]); // last
    desc->pbufs[alloced_num - 1] = NULL;
    pbuf_free(desc->pbufs[alloced_num - 2]); // second last
    desc->pbufs[alloced_num - 2] = NULL;
    alloced_num -= 2;
#endif

    desc->pbufs_num = alloced_num;
    blog_info("Total pbufs num: %d\r\n", alloced_num);
}

/* Per-slot handout age + write-off marks for the stuck-pbuf reclaimer, plus
 * global handout activity for the TXCFM watchdog traffic gating. Marking is
 * driven by the (test-gated) watchdog; the arrays stay all-zero in production
 * builds, where the reclaimer hook below is a fast no-op table walk. */
static volatile uint32_t tx_pbuf_handout_tick[NETBUS_TX_PBUF_CNT];
static volatile uint8_t tx_pbuf_reclaimed[NETBUS_TX_PBUF_CNT];
static volatile uint32_t tx_pbuf_last_handout_tick;
static volatile uint32_t tx_pbuf_cfm_count; /* bumped once per bl_tx_cfm */
#ifndef NETBUS_TX_PBUF_RECLAIM_MS
#define NETBUS_TX_PBUF_RECLAIM_MS (120000)
#endif

struct pbuf *get_tx_pbuf(netbus_wifi_mgmr_ctx_t *env)
{
    int i;
    struct tx_pbuf_desc *desc = &env->tx_desc;
    struct pbuf *p;

    for (i = 0; i < desc->pbufs_num; ++i) {
        if (tx_pbuf_reclaimed[i]) {
            continue;
        }
        p = desc->pbufs[i];
        if (p->ref == 1) {
            // restore
            memcpy(p, &desc->pbufs_back[i], sizeof(*p));
        tx_pbuf_handout_tick[i] = xTaskGetTickCount();
        tx_pbuf_last_handout_tick = tx_pbuf_handout_tick[i];
            return p;
        }
    }
    return NULL;
}

int get_tx_pbuf_free_count(netbus_wifi_mgmr_ctx_t *env)
{
    int cnt = 0;
    int i;
    struct tx_pbuf_desc *desc = &env->tx_desc;
    struct pbuf *p;

    for (i = 0; i < desc->pbufs_num; ++i) {
        if (tx_pbuf_reclaimed[i]) {
            continue;
        }
        p = desc->pbufs[i];
        if (p->ref == 1) {
            cnt++;
        }
    }
    return cnt;
}

/* Called by bl_tx_cfm instead of pbuf_free for pool pbufs. Returns 1 when the
 * slot had been written off as lost and the confirm arrived late: the backed
 * header is restored (ref back to 1) and the slot returns to service, so the
 * caller must not pbuf_free it down to zero. */
int netbus_tx_pbuf_cfm_reclaimed(struct pbuf *p)
{
    int i;

    for (i = 0; i < NETBUS_TX_PBUF_CNT; i++) {
        if (g_netbus_wifi_mgmr_env.tx_desc.pbufs[i] == p && tx_pbuf_reclaimed[i]) {
            memcpy(g_netbus_wifi_mgmr_env.tx_desc.pbufs[i],
                   &g_netbus_wifi_mgmr_env.tx_desc.pbufs_back[i],
                   sizeof(struct pbuf));
            tx_pbuf_reclaimed[i] = 0;
            printf("[TXWD][BL602] pbuf[%d] late cfm: recovered to pool\r\n", i);
            return 1;
        }
    }
    return 0;
}

int update_tx_pbuf_free_cnt_to_scratch_reg(void)
{
    /* Called once per firmware TX confirm (bl_tx_cfm); the watchdog uses this
     * counter as the progress signal — the free LEVEL stays flat at steady
     * state and cannot distinguish stalls from equilibrium. */
    tx_pbuf_cfm_count++;
    if (g_netbus_wifi_mgmr_env.init) {
        int free_cnt = get_tx_pbuf_free_count(&g_netbus_wifi_mgmr_env);
        bflbwrite_s_reg(&g_netbus_wifi_mgmr_env.trcver_ctx, 0, free_cnt);
    }
    return 0;
}

/* TXCFM watchdog: sample the tx pbuf free count every second. Frames in
 * flight (free < pool size) whose count stops increasing mean the firmware
 * TX-confirm path is stalled; announce the stall once, then report its
 * measured duration when a confirm finally arrives. */
static TimerHandle_t txcfm_wd_timer;
static uint32_t txcfm_wd_prev_cfm;
static uint32_t txcfm_wd_stall_start; /* 0 = not stalling */
static bool txcfm_wd_announced;

static void txcfm_wd_cb(TimerHandle_t t)
{
    int free = get_tx_pbuf_free_count(&g_netbus_wifi_mgmr_env);
    uint32_t now = xTaskGetTickCount();
    uint32_t cfm = tx_pbuf_cfm_count;
    int traffic_recent;
    int progress;
    int i;

    (void)t;
    if (!g_netbus_wifi_mgmr_env.init) {
        return;
    }

    /* Stuck-pbuf reclaimer: a slot handed out long ago and still not
     * confirmed is written off (skipped by get_tx_pbuf); if its confirm
     * arrives late, netbus_tx_pbuf_cfm_reclaimed() returns it to service. */
    for (i = 0; i < NETBUS_TX_PBUF_CNT; i++) {
        struct pbuf *p = g_netbus_wifi_mgmr_env.tx_desc.pbufs[i];

        if (!tx_pbuf_reclaimed[i] && tx_pbuf_handout_tick[i] != 0 &&
            p->ref > 1 && now - tx_pbuf_handout_tick[i] >= NETBUS_TX_PBUF_RECLAIM_MS) {
            tx_pbuf_reclaimed[i] = 1;
            printf("[TXWD][BL602] pbuf[%d] stuck %lu ms, written off (free=%d)\r\n",
                   i,
                   (unsigned long)((now - tx_pbuf_handout_tick[i]) * portTICK_PERIOD_MS),
                   free);
        }
    }

    /* Stall accounting is confirm-EVENT based: the free LEVEL flatlines at
     * steady-state pipeline depth and would misreport normal flow as a
     * stall. Zero confirms while send traffic is recent is a real stall;
     * once started it keeps timing until a confirm arrives. */
    progress = (cfm != txcfm_wd_prev_cfm);
    traffic_recent = (tx_pbuf_last_handout_tick != 0) &&
                     (now - tx_pbuf_last_handout_tick < pdMS_TO_TICKS(5000));

    if (progress) {
        if (txcfm_wd_stall_start != 0) {
            printf("[TXWD][BL602] TXCFM stall END held=%lu ms\r\n",
                   (unsigned long)((now - txcfm_wd_stall_start) * portTICK_PERIOD_MS));
            txcfm_wd_stall_start = 0;
            txcfm_wd_announced = false;
        }
    } else if (free < NETBUS_TX_PBUF_CNT && (traffic_recent || txcfm_wd_stall_start != 0)) {
        if (txcfm_wd_stall_start == 0) {
            txcfm_wd_stall_start = now;
        } else if (!txcfm_wd_announced &&
                   now - txcfm_wd_stall_start >= pdMS_TO_TICKS(2000)) {
            txcfm_wd_announced = true;
            printf("[TXWD][BL602] TXCFM stall: free=%d in_flight=%d tick=%lu\r\n",
                   free, NETBUS_TX_PBUF_CNT - free, (unsigned long)now);
        }
    } else {
        txcfm_wd_stall_start = 0;
        txcfm_wd_announced = false;
    }
    txcfm_wd_prev_cfm = cfm;
}


int netbus_wifi_mgmr_start(netbus_wifi_mgmr_ctx_t *env)
{
    if (!env) {
        return -1;
    }

    if (env->init) {
        return -1;
    }

    bl_wifi_mac_addr_get(env->wifi_mac);

#if 0
    if (utils_memp_init(&env->frame_pool, 2048, NETBUS_WIFI_MGMR_FRAME_MSG_QUEUE_CAP, sizeof(void *))) {
        return -1;
    }
#endif
    env->frame_msg_queue = xQueueCreateStatic(NETBUS_WIFI_MGMR_FRAME_MSG_QUEUE_CAP, NETBUS_WIFI_MGMR_FRAME_MSG_QUEUE_SIZE, env->frame_msg_queue_storage_buffer, &env->frame_msg_queue_buffer);
    if (!env->frame_msg_queue) {
        return -1;
    }
    env->cmd_msg_queue = xQueueCreateStatic(NETBUS_WIFI_MGMR_CMD_MSG_QUEUE_CAP, NETBUS_WIFI_MGMR_CMD_MSG_QUEUE_SIZE, env->cmd_msg_queue_storage_buffer, &env->cmd_msg_queue_buffer);
    if (!env->cmd_msg_queue) {
        return -1;
    }

    xTaskCreateStatic(netbufwifi_frame_mgmr_task, (char*)"sd_frm_mgmr", NETBUS_WIFI_FRAME_TASK_STACK_DEPTH, env, NETBUS_WIFI_FRAME_TASK_PRIORITY, env->frame_task_stack_buffer, &env->frame_task_buffer);
    xTaskCreateStatic(netbuswifi_cmd_mgmr_task, (char*)"sd_cmd_mgmr", NETBUS_WIFI_CMD_TASK_STACK_DEPTH, env, NETBUS_WIFI_CMD_TASK_PRIORITY, env->cmd_task_stack_buffer, &env->cmd_task_buffer);
    bflbnet_wifi_trcver_init(&env->trcver_ctx, env);
    init_tx_desc(env);

    int free_cnt = get_tx_pbuf_free_count(env);
    log_info("Initial tx pbuf free cnt %d\r\n", free_cnt);
    bflbwrite_s_reg(&env->trcver_ctx, 0, free_cnt);

    // sdiowifi_tty_init(env, 0, "/dev/ttysd0");

    env->init = true;

    txcfm_wd_timer = xTimerCreate("txcfm_wd", pdMS_TO_TICKS(1000), pdTRUE, NULL, txcfm_wd_cb);
    if (txcfm_wd_timer) {
        xTimerStart(txcfm_wd_timer, 0);
    }

    return 0;
}

int netbus_wifi_mgmr_msg_send(netbus_wifi_mgmr_ctx_t *env, const netbus_wifi_mgmr_msg_t *msg, bool is_cmd, bool is_in_isr)
{
    QueueHandle_t queue;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (!env || !msg) {
        return -1;
    }

    if (!env->init) {
        return -1;
    }

    if (is_cmd) {
        queue = env->cmd_msg_queue;
    } else {
        queue = env->frame_msg_queue;
    }

    if (is_in_isr) {
        if (pdPASS == xQueueSendToBackFromISR(queue, msg, &xHigherPriorityTaskWoken)) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            return 0;
        } else {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            return -2;
        }
    } else {
        if (pdPASS == xQueueSendToBack(queue, msg, portMAX_DELAY)) {
            return 0;
        } else {
            return -2;
        }
    }
}

int netbus_wifi_mgmr_set_ip_got(netbus_wifi_mgmr_ctx_t *env, bool ip_got)
{
    if (!env) {
        return -1;
    }
    env->ip_got = ip_got;

    return 0;
}

int netbus_wifi_mgmr_set_mode(netbus_wifi_mgmr_ctx_t *env, netbus_wifi_mode_t mode)
{
    if (!env) {
        return -1;
    }
    env->mode = mode;
    return 0;
}
