
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <utils_log.h>
#include <cli.h>
#include <aos/kernel.h>
#include <FreeRTOS.h>

#include <ramsync_upper.h>

#include <tpdbg.h>
#include "blmalloc.h"

#define RAMSYNC_TEST_TASK_PRI         (13)

typedef struct __ramsync_ctx_test {
    tp_uramsync_t *p_ramsync;
    TaskHandle_t  rx_hdr;
    StaticTask_t  rx_task;
    uint32_t      rx_stack[2028/sizeof(uint32_t)];
} ramsync_ctx_test_t;

ramsync_ctx_test_t *mst;
ramsync_ctx_test_t *slv;

static void __ramsync_dump_cmd(ramsync_ctx_test_t *rs, char *buf, int len, int argc, char **argv)
{
    uramsync_dump(rs->p_ramsync);
}

static void ramsync_loop_task(void *arg)
{
    while (1) {
        if ((NULL != mst) && (NULL != slv) ) {
            memcpy(mst->p_ramsync->p_rx, slv->p_ramsync->p_tx, sizeof(tp_txbuf_t));
            memcpy(slv->p_ramsync->p_rx, mst->p_ramsync->p_tx, sizeof(tp_txbuf_t));

            if (slv->p_ramsync->hw.tx_cb) {
                slv->p_ramsync->hw.tx_cb(slv->p_ramsync->hw.tx_arg);
            }

            if (slv->p_ramsync->hw.rx_cb) {
                slv->p_ramsync->hw.rx_cb(slv->p_ramsync->hw.rx_arg);
            }

            if (mst->p_ramsync->hw.tx_cb) {
                mst->p_ramsync->hw.tx_cb(mst->p_ramsync->hw.tx_arg);
            }

            if (mst->p_ramsync->hw.rx_cb) {
                mst->p_ramsync->hw.rx_cb(mst->p_ramsync->hw.rx_arg);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(1));// fixme, can use isr notify
    }
}

static void rx_task_entry(void *arg)
{
    uint8_t *msg;
    uint32_t msg_len;
    ramsync_ctx_test_t *rs = (ramsync_ctx_test_t *)arg;

    msg = pvPortMalloc(TP_PAYLOAD_LEN);

    if (NULL == msg) {
        tpdbg_log("mem err len = %d\r\n", TP_PAYLOAD_LEN);
        return;
    }

    while (1) {
        uramsync_rx_pop(rs->p_ramsync, msg, &msg_len, URAMSYNC_FOREVER);
        if (msg_len) {
            tpdbg_buf("rx", msg, msg_len);
        }
    }
}

static void __ramsync_push(tp_uramsync_t * ramsync, char *buf, int len, int argc, char **argv)
{
    uint8_t *msg;
    uint32_t msg_len;
    uint32_t i;
    uint32_t sendtimes;

    if ((argc != 2) && (argc != 3)) {
        tpdbg_log("usage: rs_push arg error\r\n");
        return;
    }

    if (3 == argc) {
        sendtimes = (uint32_t)atoi(argv[2]);
    } else {
        sendtimes = 1;
    }

    msg_len = (uint32_t)atoi(argv[1]);
    if (msg_len == 0) {
        tpdbg_log("usage: rs_push argh error len = %ld\r\n", msg_len);
        return;
    }

    tpdbg_log("sendtimes = %ld, sendlen = %ld\r\n", sendtimes, msg_len);

    msg = pvPortMalloc(msg_len);
    if (NULL == msg) {
        tpdbg_log("mem err len = %ld\r\n", msg_len);
    }

    for (i = 0; i < msg_len; i++) {
        msg[i] = i&0xFF;
    }

    for (i = 0; i < sendtimes; i++) {
        uramsync_tx_push_toback(ramsync, msg, msg_len, URAMSYNC_FOREVER);
    }

    vPortFree(msg);
}

static void __ramsync_pop(char *name, tp_uramsync_t * ramsync, char *buf, int len, int argc, char **argv)
{
    uint8_t *msg;
    uint32_t msg_len;

    msg = pvPortMalloc(TP_PAYLOAD_LEN);
    if (NULL == msg) {
        tpdbg_log("mem err len = %d\r\n", TP_PAYLOAD_LEN);
    }
    msg_len = TP_PAYLOAD_LEN;

    uramsync_rx_pop(ramsync, msg, &msg_len, 0);

    if (msg_len) {
        tpdbg_buf(name, msg, msg_len);
    }

    vPortFree(msg);
}

static void ramsync_loop_cmd(char *buf, int len, int argc, char **argv)
{
    aos_task_new("rs_loop", ramsync_loop_task, NULL, 2048);
}

static void ramsync_reset1_cmd(char *buf, int len, int argc, char **argv)
{
    for (int i = 0; i < TP_TXPAYLOAD_NUM; i++) {
        slv->p_ramsync->p_tx->payload[i].magic = (TP_ST_MAGIC + 1);// only for test reset
    }
}

static void ramsync_reset2_cmd(char *buf, int len, int argc, char **argv)
{
    void test_reset_signal_cb(void *arg);
    test_reset_signal_cb(slv->p_ramsync);// only for test reset
}

static void ramsync_error_cmd(char *buf, int len, int argc, char **argv)
{
}

static void mramsync_init_cmd(char *buf, int len, int argc, char **argv)
{
    mst = pvPortMalloc(sizeof(ramsync_ctx_test_t));
    if (NULL == mst) {
        tpdbg_log("mem error\r\n");
        return;
    }
    mst->p_ramsync = pvPortMalloc(sizeof(tp_uramsync_t));
    if (NULL == mst->p_ramsync) {
        tpdbg_log("mem error\r\n");
        return;
    }

    uramsync_init(mst->p_ramsync, URAMSYNC_MASTER_DEV_TYPE);

    // start sync
    mst->rx_hdr = xTaskCreateStatic(
            rx_task_entry,
            (char*)"mrx_test",
            sizeof(mst->rx_stack)/sizeof(StackType_t), mst,
            RAMSYNC_TEST_TASK_PRI,
            mst->rx_stack,
            &mst->rx_task);
}

static void sramsync_init_cmd(char *buf, int len, int argc, char **argv)
{
    slv = pvPortMalloc(sizeof(ramsync_ctx_test_t));
    if (NULL == slv) {
        tpdbg_log("mem error\r\n");
        return;
    }
    slv->p_ramsync = blmalloc(sizeof(tp_uramsync_t), OCRAM);
    if (NULL == slv->p_ramsync) {
        tpdbg_log("mem error\r\n");
        return;
    }

    uramsync_init(slv->p_ramsync, URAMSYNC_SLAVE_DEV_TYPE);

    // start sync
    slv->rx_hdr = xTaskCreateStatic(
            rx_task_entry,
            (char*)"srx_test",
            sizeof(slv->rx_stack)/sizeof(StackType_t), slv,
            RAMSYNC_TEST_TASK_PRI,
            slv->rx_stack,
            &slv->rx_task);
}

static void mramsync_push_cmd(char *buf, int len, int argc, char **argv)
{
    __ramsync_push(mst->p_ramsync, buf, len, argc, argv);
}

static void sramsync_push_cmd(char *buf, int len, int argc, char **argv)
{
    __ramsync_push(slv->p_ramsync, buf, len, argc, argv);
}

static void mramsync_pop_cmd(char *buf, int len, int argc, char **argv)
{
    __ramsync_pop("mpop", mst->p_ramsync, buf, len, argc, argv);
}

static void sramsync_pop_cmd(char *buf, int len, int argc, char **argv)
{
    __ramsync_pop("spop", slv->p_ramsync, buf, len, argc, argv);
}

static void mramsync_dump_cmd(char *buf, int len, int argc, char **argv)
{
    if (NULL == mst) {
        tpdbg_log("mst is NULL\r\n");
        return;
    }
    __ramsync_dump_cmd(mst, buf, len, argc, argv);
}

static void sramsync_dump_cmd(char *buf, int len, int argc, char **argv)
{
    if (NULL == slv) {
        tpdbg_log("mst is NULL\r\n");
        return;
    }
    __ramsync_dump_cmd(slv, buf, len, argc, argv);
}

// STATIC_CLI_CMD_ATTRIBUTE makes this(these) command(s) static
const static struct cli_command cmds_user[] STATIC_CLI_CMD_ATTRIBUTE = {
        {"rs_loop", "ramsync init", ramsync_loop_cmd},
        {"rs_reset1", "ramsync reset", ramsync_reset1_cmd},
        {"rs_reset2", "ramsync reset", ramsync_reset2_cmd},
        {"rs_error", "ramsync error", ramsync_error_cmd},
        {"rs_minit", "ramsync init", mramsync_init_cmd},
        {"rs_mpop", "ramsync mpop rx", mramsync_pop_cmd},
        {"rs_mpush", "ramsync mpush tx", mramsync_push_cmd},
        {"rs_mdump", "ramsync dump", mramsync_dump_cmd},
        {"rs_sinit", "ramsync init", sramsync_init_cmd},
        {"rs_spop", "ramsync spop rx", sramsync_pop_cmd},
        {"rs_spush", "ramsync spush tx", sramsync_push_cmd},
        {"rs_sdump", "ramsync dump", sramsync_dump_cmd}
};

int ramsync_test_cli_init(void)
{
    // static command(s) do NOT need to call aos_cli_register_command(s) to register.
    // However, calling aos_cli_register_command(s) here is OK but is of no effect as cmds_user are included in cmds list.
    // XXX NOTE: Calling this *empty* function is necessary to make cmds_user in this file to be kept in the final link.
    //return aos_cli_register_commands(cmds_user, sizeof(cmds_user)/sizeof(cmds_user[0]));
    return 0;
}
