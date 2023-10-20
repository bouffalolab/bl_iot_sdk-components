#include <stdbool.h>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
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

#define MAX_TX_PBUF_CNT (10 + 2)

static void init_tx_desc(netbus_wifi_mgmr_ctx_t *env)
{
    int i;
    int alloced_num = 0;
    struct tx_pbuf_desc *desc = &env->tx_desc;
    struct pbuf *p;

    memset(desc, 0, sizeof(*desc));
    for (i = 0; i < TX_PBUF_DESC_PBUF_MAX_NUM; ++i) {
        p = pbuf_alloc(PBUF_RAW_TX, 1514, PBUF_RAM);
        if (p) {
            blog_info("preallocated pbuf %p, p->next %p\r\n", p, p->next);
            desc->pbufs[i] = p;
            // backup
            memcpy(&desc->pbufs_back[i], p, sizeof(*p));
            alloced_num++;
            if (alloced_num >= MAX_TX_PBUF_CNT) {
                break;
            }
        } else {
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

struct pbuf *get_tx_pbuf(netbus_wifi_mgmr_ctx_t *env)
{
    int i;
    struct tx_pbuf_desc *desc = &env->tx_desc;
    struct pbuf *p;

    for (i = 0; i < desc->pbufs_num; ++i) {
        p = desc->pbufs[i];
        if (p->ref == 1) {
            // restore
            memcpy(p, &desc->pbufs_back[i], sizeof(*p));
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
        p = desc->pbufs[i];
        if (p->ref == 1) {
            cnt++;
        }
    }
    return cnt;
}

int update_tx_pbuf_free_cnt_to_scratch_reg(void)
{
    if (g_netbus_wifi_mgmr_env.init) {
        int free_cnt = get_tx_pbuf_free_count(&g_netbus_wifi_mgmr_env);
        bflbwrite_s_reg(&g_netbus_wifi_mgmr_env.trcver_ctx, 0, free_cnt);
    }
    return 0;
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
