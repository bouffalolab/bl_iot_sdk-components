/**
 * Copyright (c) 2016-2021 Bouffalolab Co., Ltd.
 *
 * Contact information:
 * web site:    https://www.bouffalolab.com/
 */

#include "ramsync_low.h"
#include <string.h>
#include "bl602.h"
#include <aos/kernel.h>
#include <cli.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

#include "blmalloc.h"

#define LRAMSYNC_TEST_BUFSIZE 2048

static lramsync_ctx_t ramsync_ctx;
//static __attribute__((section(".wifi_ram.lramsync"))) uint8_t tx_buf[2048], rx_buf[2048];
static volatile uint32_t cnt = 0;

static SemaphoreHandle_t xSemaphore;

ATTR_TCM_SECTION static void _ramsync_rx(void *arg)
{
	BaseType_t xHigherPriorityTaskWoken;

	cnt++;
	xSemaphoreGiveFromISR(xSemaphore, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

ATTR_TCM_SECTION static void _ramsync_tx(void *arg)
{
    //printf("tx handler\r\n");
}

int lramsync_test(void)
{
	uint8_t *ptxbuf;
	uint8_t *prxbuf;
    node_mem_t tx[2], rx[2];
    xSemaphore = xSemaphoreCreateBinary();

    printf("lramsync test start\r\n");

    ptxbuf = blmalloc(LRAMSYNC_TEST_BUFSIZE, OCRAM);
    if (ptxbuf == NULL) {
    	return -1;
    }
    prxbuf = blmalloc(LRAMSYNC_TEST_BUFSIZE, OCRAM);
    if (prxbuf == NULL) {
    	return -1;
    }
    tx[0].buf = ptxbuf;
    tx[0].len = LRAMSYNC_TEST_BUFSIZE / 2;
    tx[1].buf = &ptxbuf[tx[0].len];
    tx[1].len = LRAMSYNC_TEST_BUFSIZE / 2;

    rx[0].buf = prxbuf;
    rx[0].len = LRAMSYNC_TEST_BUFSIZE / 2;
    rx[1].buf = &prxbuf[rx[0].len];
    rx[1].len = LRAMSYNC_TEST_BUFSIZE / 2;

    for (int i = 0; i < LRAMSYNC_TEST_BUFSIZE; i++) {
    	ptxbuf[i] = (i & 0xff);
    }

    lramsync_init(&ramsync_ctx, tx, 2, _ramsync_tx, NULL, rx, 2, _ramsync_rx, NULL, NULL, NULL);
    //lramsync_callback_register(&ramsync_ctx, _ramsync_rx, NULL, _ramsync_tx, NULL);
    lramsync_start(&ramsync_ctx);

    uint32_t cnt_tmp = 0;

    while (1) {
    	xSemaphoreTake(xSemaphore, (uint32_t)(-1));
    	if (cnt_tmp < cnt) {
    		uint32_t offset = (cnt_tmp%2)*1024;

    		for (int i = 0; i < LRAMSYNC_TEST_BUFSIZE / 2; i++) {
    			if (prxbuf[offset + i] != (i & 0xff)) {
    				printf("rx failed!!!!!\r\n");
    				break;
    			}
    		}
    		//printf("rx ok %ld\r\n", cnt_tmp);
    		cnt_tmp++;
    	}
    }
    return 0;
}

void lrs_test_entry(void *arg)
{
    lramsync_test();
}

static void lramsync_test_cmd(char *buf, int len, int argc, char **argv)
{
    aos_task_new("lrs_test", lrs_test_entry, "/dev/ttyS0", 2048);
}

// STATIC_CLI_CMD_ATTRIBUTE makes this(these) command(s) static
const static struct cli_command cmds_user[] STATIC_CLI_CMD_ATTRIBUTE = {
        {"lrs_test", "lrs_test", lramsync_test_cmd},
};

int lramsync_test_cli_init(void)
{
    // static command(s) do NOT need to call aos_cli_register_command(s) to register.
    // However, calling aos_cli_register_command(s) here is OK but is of no effect as cmds_user are included in cmds list.
    // XXX NOTE: Calling this *empty* function is necessary to make cmds_user in this file to be kept in the final link.
    //return aos_cli_register_commands(cmds_user, sizeof(cmds_user)/sizeof(cmds_user[0]));
    return 0;
}
