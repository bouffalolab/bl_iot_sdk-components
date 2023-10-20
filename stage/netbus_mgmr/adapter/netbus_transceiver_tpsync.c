#include <utils_log.h>
#include <lwip/pbuf.h>
#include <lwip/netifapi.h>
#include <FreeRTOS.h>
#include <task.h>
#include <aos/kernel.h>

#include <bl_wifi.h>
#include "blog.h"
#include <netbus_mgmr.h>
#include "netbus_transceiver.h"
#include <ramsync_upper.h>
#include "blmalloc.h"

static tp_uramsync_t *gp_ramsync;
#define NETBUS_MSG_MAXSIZE             (TP_PAYLOAD_LEN)

static void handle_sta_ap_to_wifi_tx(netbus_wifi_mgmr_ctx_t *ctx, const uint8_t *frame, const uint16_t frame_len, bool is_sta)
{
    netbus_wifi_mgmr_msg_t swm_msg;
    struct pbuf *p;

    if (frame == NULL || frame_len > 2048) {
        blog_error("frame or frame_len error\r\n");
        return;
    }

    extern struct pbuf *get_tx_pbuf(netbus_wifi_mgmr_ctx_t *env);
    p = get_tx_pbuf(ctx);

    if (!p) {
        blog_error("no more tx pbuf, flow control not working?\r\n");
        return;
    }
    pbuf_ref(p);

    update_tx_pbuf_free_cnt_to_scratch_reg();

    if (pbuf_take(p, frame, frame_len) != ERR_OK) {
        blog_error("pbuf_take err\r\n");
        pbuf_free(p);
        return;
    }
    p->len = p->tot_len = frame_len;

    if (is_sta) {
        swm_msg.type = NETBUS_WIFI_MGMR_MSG_TYPE_ETH_FRAME_STA_TO_WIFI_TX;
    } else {
        swm_msg.type = NETBUS_WIFI_MGMR_MSG_TYPE_ETH_FRAME_AP_TO_WIFI_TX;
    }
    swm_msg.u.cmd.data_ptr = p;
    swm_msg.u.cmd.data_len = frame_len;
    if (netbus_wifi_mgmr_msg_send(ctx, &swm_msg, false, false)) {
        blog_error("mgmr msg send failed, drop frame\r\n");
        pbuf_free(p);
        return;
    }
}

static void handle_sniffer_tx(netbus_wifi_mgmr_ctx_t *ctx, const uint8_t *frame, const uint16_t frame_len)
{
    void *cp;
    netbus_wifi_mgmr_msg_t swm_msg;

    cp = pvPortMalloc(frame_len);
    if (!cp) {
        blog_error("malloc\r\n");
        return;
    }

    memcpy(cp, frame, frame_len);
    swm_msg.type = NETBUS_WIFI_MGMR_MSG_TYPE_WIFI_FRAME_SNIFFER_TO_WIFI_TX;
    swm_msg.u.cmd.data_ptr = cp;
    swm_msg.u.cmd.data_len = frame_len;
    if (netbus_wifi_mgmr_msg_send(ctx, &swm_msg, false, false)) {
        blog_error("mgmr msg send failed, drop raw tx frame\r\n");
        vPortFree(cp);
    }
}

static void handle_eth_wifi_frame_recv(netbus_wifi_mgmr_ctx_t *ctx, const uint8_t *frame, const uint16_t frame_len)
{
    uint16_t subtype;

    subtype = frame[0] + (frame[1] << 8);
    blog_info("wifi frame handle, subtype 0x%x\r\n", subtype);
    switch (subtype) {
    case BF1B_MSG_ETH_WIFI_FRAME_SUBTYPE_STA_TO_WIFI_TX:
        handle_sta_ap_to_wifi_tx(ctx, frame + 2, frame_len - 2, true);
        break;
    case BF1B_MSG_ETH_WIFI_FRAME_SUBTYPE_AP_TO_WIFI_TX:
        handle_sta_ap_to_wifi_tx(ctx, frame + 2, frame_len - 2, false);
        break;
    case BF1B_MSG_ETH_WIFI_FRAME_SUBTYPE_SNIFFER_TO_WIFI_TX:
        handle_sniffer_tx(ctx, frame + 2, frame_len - 2);
        break;
    case BF1B_MSG_ETH_WIFI_FRAME_SUBTYPE_EXT:
        blog_info("not support.\r\n");
        //sdiowifi_ext_frame_process(ctx, frame + 2, frame_len - 2);
        break;
    default:
        break;
    }
}

static void handle_cmd_recv(netbus_wifi_mgmr_ctx_t *ctx, const uint8_t *data, const uint16_t data_len)
{
    netbus_wifi_mgmr_msg_t swm_msg;
    netbus_min_cmd_msg_t *cmd;
    void *dptr = NULL;
    int dlen;

    cmd = (netbus_min_cmd_msg_t *)data;
    memset(&swm_msg, 0, sizeof(swm_msg));
    swm_msg.type = NETBUS_WIFI_MGMR_MSG_TYPE_CMD;
    swm_msg.u.cmd.cmd = cmd->cmd;
    swm_msg.u.cmd.msg_id = cmd->msg_id;

    if (data_len == sizeof(netbus_min_cmd_msg_t)) {
        /* empty here */
    } else if (data_len > sizeof(netbus_min_cmd_msg_t)) {
        dlen = data_len - 0;
        dptr = pvPortMalloc(dlen);
        if (dptr == NULL) {
            blog_error("malloc");
            return;
        }
        memcpy(dptr, data, dlen);
        swm_msg.u.cmd.data_ptr = dptr;
        swm_msg.u.cmd.data_len = dlen;
    } else {
        blog_error("cmd of unexpected size\r\n");
        return;
    }
    if (netbus_wifi_mgmr_msg_send(ctx, &swm_msg, true, false)) {
        blog_error("mgmr msg send failed, drop cmd\r\n");
        vPortFree(dptr);
    }
}

static void netbus_recv_cb(void *arg, void *data_ptr, uint16_t data_len)
{
    netbus_msg_t *msg;

    netbus_wifi_mgmr_ctx_t *ctx;

    msg = (netbus_msg_t *)data_ptr;

    blog_info("netbus_recv len:%d type:0x%x\r\n\r\n", msg->len, msg->type);

    ctx = &g_netbus_wifi_mgmr_env;
    switch (msg->type) {
    case BF1B_MSG_TYPE_ETH_WIFI_FRAME:
        handle_eth_wifi_frame_recv(ctx, msg->payload, msg->len);
        break;
    case BF1B_MSG_TYPE_CMD:
        handle_cmd_recv(ctx, msg->payload, msg->len);
        break;
    default:
        break;
    }
}

static int bflb_net_wifi_trcver_write_mac(net_wifi_trcver_ctx_t *ctx)
{
    int i;
    uint8_t mac[6];

    bl_wifi_mac_addr_get(mac);

    for (i = 0; i < 6; ++i) {
        bflbwrite_s_reg(ctx, WIFI_MAC_ADDR_SCRATCH_OFFSET + i, mac[i]);
    }

    return 0;
}

static void rx_task_entry(void *arg)
{
	tp_uramsync_t *ramsync = (tp_uramsync_t *)arg;
    uint8_t *msg;
    uint32_t msg_len;

    msg = pvPortMalloc(NETBUS_MSG_MAXSIZE);
    if (NULL == msg) {
    	blog_error("mem err len = %d\r\n", NETBUS_MSG_MAXSIZE);
        return;
    }

    while (1) {
        uramsync_rx_pop(ramsync, msg, &msg_len, URAMSYNC_FOREVER);
        if (msg_len) {
            //tpdbg_buf("rx", msg, msg_len);
            netbus_recv_cb(NULL, msg, msg_len);
        }
    }
}

int bflbnet_wifi_trcver_init(net_wifi_trcver_ctx_t *ctx, const void *arg)
{
    ctx->tx_lock = xSemaphoreCreateMutexStatic(&ctx->tx_lock_buf);

    if (!ctx->tx_lock) {
        blog_error("tx_lock create failed\r\n");
        return -1;
    }

    gp_ramsync = blmalloc(sizeof(tp_uramsync_t), OCRAM);
	if (gp_ramsync == NULL) {
		blog_error("mem error\r\n");
		return -1;
	}
	uramsync_init(gp_ramsync, URAMSYNC_SLAVE_DEV_TYPE);

	xTaskCreate(rx_task_entry,
	            (char*)"bflbnet_slv_task",
	            1024,
                gp_ramsync,
	            10,
	            NULL);
	bflb_net_wifi_trcver_set_present(ctx, true);
    return 0;
}

int bflb_net_wifi_trcver_set_present(net_wifi_trcver_ctx_t *ctx, bool present)
{
    if (!ctx) {
        return -1;
    }
    ctx->host_present = present;
    return 0;
}

typedef struct _netbus_struct_desc {
    uint16_t type;
    uint16_t len;
    uint8_t buf[1];
} netbus_msgtlv_t;

// type len data
int bl_netbus_write_tlv(uint16_t type, const void *data_ptr, uint16_t data_len)
{
    netbus_msgtlv_t *msg = NULL;
    int ret;

    msg = aos_malloc(sizeof(netbus_msgtlv_t) + data_len);
    if (!msg) {
        blog_error("mem err\r\n");
    }
    msg->type = type;
    msg->len = data_len;
    memcpy(msg->buf, data_ptr, data_len);

    uramsync_tx_push_toback(gp_ramsync, msg, 4 + data_len, URAMSYNC_FOREVER);
    blog_info("netbus_write->len:%d type:0x%x\r\n\r\n", msg->len, msg->type);
    ret = data_len;
    aos_free(msg);

    return ret;
}
// type1 len pbuf
// type2 len pbuf1 len buf2
int bl_netbus_write_pbuf_tlv(struct pbuf *p)
{
    struct pbuf *next;
    uint16_t type;
    uint16_t len;
    uint8_t *buf;
    int off;
    int ret;

    if (!p) {
        return -1;
    }

    buf = aos_malloc(NETBUS_MSG_MAXSIZE);
    if (!buf) {
        blog_error("mem err\r\n");
    }

    off = 0;

    type = 0x7878;// fixme enum
    memcpy(buf + off, &type, 2);
    off += 2;

    len = p->tot_len;// ??? fixme
    memcpy(buf + off, &len, 2);
    off += 2;

    memcpy(buf + off, p->payload, p->len);
    off += p->len;// ??? fixme

    next = p->next;
    while (next) {
        memcpy(buf + off, next->payload, next->len);// ??? fixme
        off += next->len;// ??? fixme
        next = next->next;
    }

    if (off > NETBUS_MSG_MAXSIZE) {
        blog_error("length %d BUG !!!\r\n", off);
        while(1);
    }

    uramsync_tx_push_toback(gp_ramsync, buf, off, URAMSYNC_FOREVER);
    blog_info("netbus_write->len:%d type:0x%x\r\n\r\n", len, type);
    ret = off;
    aos_free(buf);

    return ret;
}

int bflbmsg_send(net_wifi_trcver_ctx_t *ctx, const uint16_t type, const void *payload, const uint16_t payload_len)
{
    int ret = 0;
    if (!ctx->host_present) {
        blog_error("host_present = NULL?\r\n");
        return -1;
    }
    bl_netbus_write_tlv(type, payload, payload_len);

    return ret;
}

int bflbmsg_send_pbuf(net_wifi_trcver_ctx_t *ctx, const uint16_t type, const uint16_t subtype, struct pbuf *p, bool is_amsdu, void *cb, void *cb_arg)
{
    int ret = 0;

    if (!ctx->host_present) {
        return -1;
    }

    bl_netbus_write_pbuf_tlv(p);

    return ret;
}

void bflbwrite_s_reg(net_wifi_trcver_ctx_t *ctx, const uint32_t offset, const uint8_t val)
{
	//log_warn("bflbwrite_s_reg offset = %ld, val = %d\r\n", offset, val);
}

