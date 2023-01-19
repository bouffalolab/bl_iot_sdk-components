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


/*
 *
 * todo
 * 1. reset 逻辑
 * 2. seq 的判断方法需要完善
 *
 * */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <event_groups.h>

#include <utils_crc.h>

#include "ramsync_upper.h"
#include <desc_buf.h>
#include <ramsync_low.h>
#include <tpdbg.h>
#include "utils_log.h"
#include <blog.h>
#if defined(CFG_USE_DTS_SPI_CONFIG)
#include "libfdt.h"
#include "hal_board.h"
#endif /* CFG_USE_DTS_SPI_CONFIG */

#define EVT_EMPTYSLOT_BIT      (1<<0)
#define EVT_TX_DONE_BIT        (1<<1)

#if RAMSYNC_FAST_COPY_ENABLE
static void *ramsync_fast_memcpy(void *dst, const void *src, uint32_t count)
{
#define UNALIGNED(X, Y)                                               \
                        (((uint32_t)X & (sizeof(uint32_t) - 1)) | \
                         ((uint32_t)Y & (sizeof(uint32_t) - 1)))
#define BIGBLOCKSIZE    (sizeof(uint32_t) << 2)
#define LITTLEBLOCKSIZE (sizeof(uint32_t))
#define TOO_SMALL(LEN)  ((LEN) < BIGBLOCKSIZE)

    char *dst_ptr = (char *)dst;
    char *src_ptr = (char *)src;
    uint32_t *aligned_dst;
    uint32_t *aligned_src;
    int len = count;

    /* If the size is small, or either SRC or DST is unaligned,
    then punt into the byte copy loop.  This should be rare. */
    if (!TOO_SMALL(len) && !UNALIGNED(src_ptr, dst_ptr))
    {
        aligned_dst = (uint32_t *)dst_ptr;
        aligned_src = (uint32_t *)src_ptr;

        /* Copy 4X long words at a time if possible. */
        while (len >= BIGBLOCKSIZE)
        {
            *aligned_dst++ = *aligned_src++;
            *aligned_dst++ = *aligned_src++;
            *aligned_dst++ = *aligned_src++;
            *aligned_dst++ = *aligned_src++;
            len -= BIGBLOCKSIZE;
        }

        /* Copy one long word at a time if possible. */
        while (len >= LITTLEBLOCKSIZE)
        {
            *aligned_dst++ = *aligned_src++;
            len -= LITTLEBLOCKSIZE;
        }

        /* Pick up any residual with a byte copier. */
        dst_ptr = (char *)aligned_dst;
        src_ptr = (char *)aligned_src;
    }

    while (len--)
        *dst_ptr++ = *src_ptr++;

    return dst;
#undef UNALIGNED
#undef BIGBLOCKSIZE
#undef LITTLEBLOCKSIZE
#undef TOO_SMALL
}
#else
#define ramsync_fast_memcpy     memcpy
#endif

/*--------------------- debug moudule ----------------------*/
void uramsync_dump(tp_uramsync_t *rs)
{
    uint32_t i;

    tpdbg_log("rs->tx.st.magic       = 0x%08lx\r\n" , rs->tx.st.magic);
    tpdbg_log("rs->tx.st.rseq        = %ld\r\n" , rs->tx.st.rseq);
    for (i = 0; i < TP_TXPAYLOAD_NUM; i++) {
        tpdbg_log("rs->tx.payload[%ld].len = %ld\r\n" , i, rs->tx.payload[i].len);
        tpdbg_log("rs->tx.payload[%ld].seq = %ld\r\n" , i, rs->tx.payload[i].seq);
        tpdbg_log("rs->tx.payload[%ld].crc = 0x%08lx\r\n" , i, rs->tx.payload[i].crc);
        tpdbg_buf("tx", &rs->tx.payload[i], 20);
    }

    tpdbg_log("rs->rx.st.magic       = 0x%08lx\r\n" , rs->rx.st.magic);
    tpdbg_log("rs->rx.st.rseq        = %ld\r\n" , rs->rx.st.rseq);
    for (i = 0; i < TP_RXPAYLOAD_NUM; i++) {
        tpdbg_log("rs->rx.payload[%ld].len = %ld\r\n" , i, rs->rx.payload[i].len);
        tpdbg_log("rs->rx.payload[%ld].seq = %ld\r\n" , i, rs->rx.payload[i].seq);
        tpdbg_log("rs->rx.payload[%ld].crc = 0x%08lx\r\n" , i, rs->rx.payload[i].crc);
        tpdbg_buf("rx", &rs->rx.payload[i], 20);
    }
}

/*--------------------- reset module ----------------------*/
#define URAMSYNC_STATUE_IDLE               (0)
#define URAMSYNC_STATUE_TXRESET_DONE       (1)
#define URAMSYNC_STATUE_RXRESET_DONE       (2)
#define URAMSYNC_STATUE_RUNNING            (3)

static void __uramsync_reset_logic(tp_uramsync_t *uramsync)
{
    /* slot */
    memset(&uramsync->tx, 0, sizeof(tp_txbuf_t));
    uramsync->tx.st.magic = TP_ST_MAGIC;

    memset(&uramsync->rx, 0, sizeof(tp_rxbuf_t));

    /* local sequnce */
    memset(&uramsync->tx_seq, 0, sizeof(uint32_t));

    /* tx/rx desc hdr */
    // need tx_desc rx_desc set empty???

    /* for rx notify */
    xEventGroupClearBits(uramsync->emptyslot_evt, EVT_EMPTYSLOT_BIT);

    /* calulate crc for rx */
    memset(&uramsync->rx_cache, 0, sizeof(tp_payload_t));
}

static void ramsynck_reset_entry(void *arg)
{
    tp_uramsync_t *uramsync = (tp_uramsync_t *)arg;
    uint32_t i;

    tpdbg_log("[%s][tp_reset_task] 1. wait tx rx enter loop\r\n", uramsync->name);

    while (1) {
        while (1) {
            if (URAMSYNC_STATUE_RXRESET_DONE == uramsync->status) {
                break;
            }
            vTaskDelay(pdMS_TO_TICKS(1));
        }

        tpdbg_log("[%s][tp_reset_task] 2. reset status seq/rseq + reset event\r\n", uramsync->name);
        __uramsync_reset_logic(uramsync);

        if (URAMSYNC_MASTER_DEV_TYPE == uramsync->devtype) {
            tpdbg_log("[%s][tp_reset_task] 3. reset hw\r\n", uramsync->name);

            while (1) {
                lramsync_reset(&uramsync->hw);
                vTaskDelay(pdMS_TO_TICKS(2));// wait for one slot sync complete
                if (TP_ST_MAGIC == uramsync->rx.st.magic) {
                    break;
                }
                for (i = 0; i < 1; i++) {
                    tpdbg_log("[%s][tp_reset_task] sync clock error, wait %ldS\r\n", uramsync->name, i); // delete it when test pass
                    vTaskDelay(pdMS_TO_TICKS(1000));                            // delete it when test pass
                    if (TP_ST_MAGIC == uramsync->rx.st.magic) {
                        break;
                    }
                }
                if (TP_ST_MAGIC == uramsync->rx.st.magic) {
                    break;
                }
            }
            tpdbg_log("[%s][tp_reset_task] 4. update status\r\n", uramsync->name);
        } else {
            tpdbg_log("[%s][tp_reset_task] 3. update status\r\n", uramsync->name);
        }

        uramsync->status = URAMSYNC_STATUE_RUNNING;

        vTaskSuspend(NULL);
    }
}

static void __reset_signal(tp_uramsync_t *uramsync)
{
    tpdbg_log("[%s]rs_reset task start\r\n", uramsync->name);

    /* start reset task */
    if (uramsync->reset_thdr) {
        if (xPortIsInsideInterrupt()) {
            xTaskResumeFromISR(uramsync->reset_thdr);
        } else {
            vTaskResume(uramsync->reset_thdr);
        }
    } else {
        xTaskCreate(ramsynck_reset_entry,
                    "rs_reset",
                    RAMSYNC_RESET_SATCK_SIZE/sizeof(StackType_t),
                    uramsync,
                    RAMSYNC_RESET_TASK_PRI, &uramsync->reset_thdr);
    }
}

static void reset_signal_first_init(tp_uramsync_t *uramsync)
{
    uramsync->status = URAMSYNC_STATUE_IDLE;
    __reset_signal(uramsync);
}

static void reset_signal(tp_uramsync_t *uramsync)
{
    if (URAMSYNC_STATUE_RUNNING == uramsync->status) {
        uramsync->status = URAMSYNC_STATUE_IDLE;

        __reset_signal(uramsync);
        return;
    }
    tpdbg_log("logic error, never run here forever\r\n");
}

/*---------------------- app module -----------------------*/
static void __ramsync_low_cb(void *arg)
{
    tp_uramsync_t *uramsync = (tp_uramsync_t *)arg;

    BaseType_t xHigherPriorityTaskWoken, xResult;

    if (uramsync->emptyslot_evt) {
        if (xPortIsInsideInterrupt()) {
            xHigherPriorityTaskWoken = pdFALSE;
            xResult = xEventGroupSetBitsFromISR(
                    uramsync->emptyslot_evt,    /* The event group being updated. */
                    EVT_EMPTYSLOT_BIT,          /* The bits being set. */
                    &xHigherPriorityTaskWoken
            );

            /* Was the message posted successfully? */
            if( xResult != pdFAIL ) {
                portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
            }
        } else {
            xEventGroupSetBits(
                    uramsync->emptyslot_evt,    /* The event group being updated. */
                    EVT_EMPTYSLOT_BIT           /* The bits being set. */
            );
        }
    }
}

static void __ramsync_low_tx_cb(void *arg)
{
    tp_uramsync_t *uramsync = (tp_uramsync_t *)arg;

    BaseType_t xHigherPriorityTaskWoken, xResult;

    if (uramsync->emptyslot_evt) {
        if (xPortIsInsideInterrupt()) {
            xHigherPriorityTaskWoken = pdFALSE;
            xResult = xEventGroupSetBitsFromISR(
                    uramsync->emptyslot_evt,    /* The event group being updated. */
                    EVT_TX_DONE_BIT,          /* The bits being set. */
                    &xHigherPriorityTaskWoken
            );

            /* Was the message posted successfully? */
            if( xResult != pdFAIL ) {
                portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
            }
        } else {
            xEventGroupSetBits(
                    uramsync->emptyslot_evt,      /* The event group being updated. */
                    EVT_TX_DONE_BIT);           /* The bits being set. */
        }
    }
}

static void __reset_signal_cb(void *arg)
{
    tp_uramsync_t *uramsync = (tp_uramsync_t *)arg;

    if (URAMSYNC_STATUE_RUNNING == uramsync->status) {
        reset_signal(uramsync);
    }
    tpdbg_log("[%s] __reset_signal_cb\r\n", uramsync->name);
}

#if defined(CFG_USE_DTS_SPI_CONFIG)
static int get_dts_addr(const char *name, uint32_t *start, uint32_t *off)
{
    uint32_t addr = hal_board_get_factory_addr();
    const void *fdt = (const void *)addr;
    uint32_t offset;

    if (!name || !start || !off) {
        return -1;
    }

    offset = fdt_subnode_offset(fdt, 0, name);
    if (offset <= 0) {
        log_error("%s NULL.\r\n", name);
        return -1;
    }

    *start = (uint32_t)fdt;
    *off = offset;

    return 0;
}

static lramsync_spi_config_t* fdt_spi_module_init(const void *fdt, int offset, const char *node)
{
    #define PIN_INVALID_VALUE 0XFF

    int offset1 = 0, offset2 = 0;
    int j;
    const uint32_t *addr_prop = NULL;
    int lentmp = 0, countindex = 0;
    const char *result = NULL;
    uint8_t port, polar_phase;
    uint32_t freq;
    lramsync_spi_config_t* spi_cfg = NULL; 

    struct _feature_pin {
        char *name;
        uint8_t value;
    } spi_pin[4] = {
        {.name = "clk",  .value = PIN_INVALID_VALUE},
        {.name = "cs",   .value = PIN_INVALID_VALUE},
        {.name = "mosi", .value = PIN_INVALID_VALUE},
        {.name = "miso", .value = PIN_INVALID_VALUE},
    };

    offset1 = fdt_subnode_offset(fdt, offset, node);
    if (0 >= offset1) {
        blog_info("SPI %s NULL.\r\n", node);
        return NULL;
    }

    countindex = fdt_stringlist_count(fdt, offset1, "status");
    if (countindex != 1) {
        blog_info("SPI status_countindex = %d NULL.\r\n", countindex);
        return NULL;
    }
    result = fdt_stringlist_get(fdt, offset1, "status", 0, &lentmp);
    if ((lentmp != 4) || (memcmp("okay", result, 4) != 0)) {
        blog_info("SPI status = %s\r\n", result);
        return NULL;
    }

    /* set port */
    addr_prop = fdt_getprop(fdt, offset1, "port", &lentmp);
    if (addr_prop == NULL) {
        blog_info("SPI port NULL.\r\n");
        return NULL;
    }
    port = fdt32_to_cpu(*addr_prop);

    /* set polar_phase */
    addr_prop = fdt_getprop(fdt, offset1, "polar_phase", &lentmp);
    if (addr_prop == NULL) {
        blog_info("SPI polar_phase NULL.\r\n");
        return NULL;
    }
    polar_phase = fdt32_to_cpu(*addr_prop);

    /* set baudrate */
    addr_prop = fdt_getprop(fdt, offset1, "freq", &lentmp);
    if (addr_prop == NULL) {
        blog_info("SPI freq NULL.\r\n");
        return NULL;
    }
    freq = fdt32_to_cpu(*addr_prop);

    for (j = 0; j < 4; j++) {
        /* get pin_name */
        offset2 = fdt_subnode_offset(fdt, offset1, "pin");
        if (0 >= offset2) {
            blog_info("SPI pin NULL.\r\n");
            return NULL;
        }
        addr_prop = fdt_getprop(fdt, offset2, spi_pin[j].name, &lentmp);
        if (addr_prop == NULL) {
            blog_info("SPI %s NULL.\r\n", spi_pin[j].name);
            return NULL;
        }
        spi_pin[j].value = fdt32_to_cpu(*addr_prop);
    }
    spi_cfg = (lramsync_spi_config_t*)malloc(sizeof(lramsync_spi_config_t));
    if(spi_cfg == 0){
        blog_info("SPI malloc error\r\n");
        return NULL;
    }
    spi_cfg->port = port;
    spi_cfg->spi_mode = polar_phase;
    spi_cfg->spi_speed = freq;
    spi_cfg->clk = spi_pin[0].value;
    spi_cfg->cs = spi_pin[1].value;
    spi_cfg->mosi = spi_pin[2].value;
    spi_cfg->miso = spi_pin[3].value;
    blog_info("port[%d],spi_mode[%d],spi_speed[%d],clk[%d],cs[%d],mosi[%d],miso[%d].\r\n",
        spi_cfg->port, spi_cfg->spi_mode, spi_cfg->spi_speed,
        spi_cfg->clk, spi_cfg->cs, spi_cfg->mosi, spi_cfg->miso);
    #undef PIN_INVALID_VALUE
    return spi_cfg;
}

static int fdt_spi_module_deinit(lramsync_spi_config_t* spi_cfg)
{
    if(spi_cfg){
        free(spi_cfg);
    }
    return 0;
}
#endif /* CFG_USE_DTS_SPI_CONFIG */

static int __ramsync_low_init(tp_uramsync_t *arg, uint32_t type)
{
    uint32_t i;
    tp_uramsync_t *uramsync = (tp_uramsync_t *)arg;

    node_mem_t node_txbuf[TP_TXPAYLOAD_NUM * 2];
    node_mem_t node_rxbuf[TP_RXPAYLOAD_NUM * 2];

    memset(uramsync, 0, sizeof(tp_uramsync_t));
    uramsync->devtype = type;

    for (i = 0; i < TP_TXPAYLOAD_NUM; i++) {
        node_txbuf[i * 2].buf = &uramsync->tx.st;
        node_txbuf[i * 2].len = sizeof(tp_st_t);
        node_txbuf[i * 2 + 1].buf = &uramsync->tx.payload[i];
        node_txbuf[i * 2 + 1].len = sizeof(tp_payload_t);
    }

    for (i = 0; i < TP_RXPAYLOAD_NUM; i++) {
        node_rxbuf[i * 2].buf = &uramsync->rx.st;
        node_rxbuf[i * 2].len = sizeof(tp_st_t);
        node_rxbuf[i * 2 + 1].buf = &uramsync->rx.payload[i];
        node_rxbuf[i * 2 + 1].len = sizeof(tp_payload_t);
    }

    /* set magic */
    uramsync->tx.st.magic = TP_ST_MAGIC;

#if defined(CFG_USE_DTS_SPI_CONFIG)
    lramsync_spi_config_t* spi_cfg = NULL;
    uint32_t fdt = 0, offset = 0;

    const char *spi_node = {
        "spi@4000A200",
    };

    if (0 == get_dts_addr("spi", &fdt, &offset)) {
        spi_cfg = fdt_spi_module_init((const void *)fdt, offset, spi_node);
    }

    uramsync->hw.cfg = spi_cfg;
#endif /* CFG_USE_DTS_SPI_CONFIG */

    /* set type , and set name */
    if (URAMSYNC_MASTER_DEV_TYPE == uramsync->devtype) {
        strncpy(uramsync->name, "mst", sizeof(uramsync->name) - 1);
        lramsync_init(
            &uramsync->hw,
            node_txbuf, TP_TXPAYLOAD_NUM * 2,
            __ramsync_low_tx_cb, uramsync,
            node_rxbuf, TP_RXPAYLOAD_NUM * 2,
            __ramsync_low_cb, uramsync,
            NULL,NULL 
            );
    } else if (URAMSYNC_SLAVE_DEV_TYPE == uramsync->devtype) {
        strncpy(uramsync->name, "slv", sizeof(uramsync->name) - 1);
        lramsync_init(
            &uramsync->hw,
            node_txbuf, TP_TXPAYLOAD_NUM * 2,
            __ramsync_low_tx_cb, uramsync,
            node_rxbuf, TP_RXPAYLOAD_NUM * 2,
            __ramsync_low_cb, uramsync,
            __reset_signal_cb, uramsync
            );
    } else {
        strncpy(uramsync->name, "unk", sizeof(uramsync->name) - 1);
    }

#if defined(CFG_USE_DTS_SPI_CONFIG)
    if(spi_cfg){
        fdt_spi_module_deinit(spi_cfg);
    }
#endif /* CFG_USE_DTS_SPI_CONFIG */

    lramsync_start(&uramsync->hw);
    reset_signal_first_init(uramsync);

    return 0;
}

static int __ramsync_low_deinit(tp_uramsync_t *arg)
{
    tp_uramsync_t *uramsync = (tp_uramsync_t *)arg;

    lramsync_deinit(&uramsync->hw);

    return 0;
}

/*---------------------- tx moudule ------------------------*/
static void uramsync_task_tx(void *arg)
{
    uint32_t lentmp;
    uint32_t slottmp = 0;
    uint32_t poplentmp = 0;
    uint8_t ensure_empty_slot;
    uint32_t i;
    struct crc32_stream_ctx crc_ctx;

    tp_uramsync_t *uramsync = (tp_uramsync_t *)arg;

    if (NULL == uramsync) {
        return;
    }

    tpdbg_log("[%s] uramsync_task_tx\r\n", uramsync->name);

    while (1) {
        /* ensure status tx enter loop */
        while (URAMSYNC_STATUE_RUNNING != uramsync->status) {
            if (URAMSYNC_STATUE_IDLE == uramsync->status) {
                uramsync->status = URAMSYNC_STATUE_TXRESET_DONE;
            }
            vTaskDelay(pdMS_TO_TICKS(1));
        }

        /* ensure desc have valid data */
        while (1) {
            while (1) {
                //desc_peek(&uramsync->tx_desc, &lentmp, DESC_FOREVER);
                desc_peek(&uramsync->tx_desc, &lentmp, POLL_UNIT_TIME_MS);
                if (URAMSYNC_STATUE_RUNNING != uramsync->status) {
                    break;
                }
                if (0 != lentmp) {
                    break;
                }
            }

            if (URAMSYNC_STATUE_RUNNING != uramsync->status) {
                break;
            }

            if (lentmp > TP_PAYLOAD_LEN) {
                tpdbg_log("pop too long packet\r\n");
                desc_pop(&uramsync->tx_desc, NULL, NULL, 0);// 0-noblock DESC_FOREVER-block
                continue;
            }
            //tpdbg_log("[%s] [task_tx] 1. ensure desc have valid data.\r\n", uramsync->name);
            break;
        }
        if (URAMSYNC_STATUE_RUNNING != uramsync->status) {
            continue;
        }

        /* ensure empty slot */
        ensure_empty_slot = 0;
        while (1) {
        	xEventGroupWaitBits(uramsync->emptyslot_evt,
                             EVT_TX_DONE_BIT,
                             pdTRUE,/* should be cleared before returning */
                             pdTRUE,/* Don't wait for both bits, either bit will do */
                             POLL_UNIT_TIME_MS);//portMAX_DELAY
            for (i = 0; i < TP_TXPAYLOAD_NUM; i++) {
                if (uramsync->tx.payload[i].seq <= uramsync->rx.st.rseq) {
                    // have empty slot
                    slottmp = i;
                    ensure_empty_slot = 1;
                    break;// break for
                }
            }
            if (ensure_empty_slot) {
                //tpdbg_log("[%s] [task_tx] 2. ensure empty slot = %ld.\r\n", uramsync->name, slottmp);
                break;// break while
            }
            //vTaskDelay(pdMS_TO_TICKS(1));// fixme, can use isr notify
            if (URAMSYNC_STATUE_RUNNING != uramsync->status) {
                break;
            }
        }
        if (URAMSYNC_STATUE_RUNNING != uramsync->status) {
            continue;
        }

        /* desc to slot,  update buf + len */
        uramsync->tx.payload[slottmp].len = 4 + 4;// len(4) + seq(4) + buf(x) crc(4)

        desc_pop(&uramsync->tx_desc,
                uramsync->tx.payload[slottmp].buf,
                &poplentmp ,
                0);// 0-noblock DESC_FOREVER-block

        uramsync->tx.payload[slottmp].len += poplentmp;// len(4) + seq(4) + buf(x) crc(4)

        // seq crc update
        uramsync->tx_seq++;
        uramsync->tx.payload[slottmp].seq = uramsync->tx_seq;

        utils_crc32_stream_init(&crc_ctx);
        utils_crc32_stream_feed_block(&crc_ctx,
                (uint8_t *)&uramsync->tx.payload[slottmp],
                uramsync->tx.payload[slottmp].len);

        uramsync->tx.payload[slottmp].crc = utils_crc32_stream_results(&crc_ctx);

        tpdbg_log("[%s] [task_tx] 3. desc to slot, payload len = %ld, seq = %ld, poplentmp = %ld, crc = 0x%08lx\r\n",
                uramsync->name,
                uramsync->tx.payload[slottmp].len,
                uramsync->tx.payload[slottmp].seq,
                poplentmp,
                uramsync->tx.payload[slottmp].crc
                );

        //tpdbg_buf(uramsync->name, &uramsync->tx.st, sizeof(uramsync->tx.st));
        //tpdbg_buf(uramsync->name, &uramsync->tx.payload[slottmp], uramsync->tx.payload[slottmp].len);
    }
}

/*---------------------- rx moudule ------------------------*/
static void uramsync_task_rx(void *arg)
{
    uint32_t i;
    uint32_t lentmp;
    uint32_t crctmp;
    uint32_t seqtmp;

    uint32_t slot_have_validdata;
    uint32_t rseqtmp;
    struct crc32_stream_ctx crc_ctx;

    EventBits_t uxBits;
    int res;

    tp_uramsync_t *uramsync = (tp_uramsync_t *)arg;

    if (NULL == uramsync) {
        return;
    }

    tpdbg_log("[%s] uramsync_task_rx\r\n", uramsync->name);

    while (1) {
        /* ensure status rx enter loop */
        while (URAMSYNC_STATUE_RUNNING != uramsync->status) {
            if (URAMSYNC_STATUE_TXRESET_DONE == uramsync->status) {
                uramsync->status = URAMSYNC_STATUE_RXRESET_DONE;
            }
            vTaskDelay(pdMS_TO_TICKS(1));
        }

        /* ensure slot have new data */
        slot_have_validdata = 0;
        while (1) {
            if (URAMSYNC_STATUE_RUNNING != uramsync->status) {
                break;
            }
            while (1) {
                uxBits = xEventGroupWaitBits(uramsync->emptyslot_evt,
                                         EVT_EMPTYSLOT_BIT,
                                         pdTRUE,/* should be cleared before returning */
                                         pdTRUE,/* Don't wait for both bits, either bit will do */
                                         POLL_UNIT_TIME_MS);//portMAX_DELAY
                if (URAMSYNC_STATUE_RUNNING != uramsync->status) {
                    break;
                }
                if (uxBits & EVT_EMPTYSLOT_BIT) {
                    break;
                }
            }
            if (URAMSYNC_STATUE_RUNNING != uramsync->status) {
                break;
            }

            for (i = 0; i < TP_TXPAYLOAD_NUM; i++) {
                if (URAMSYNC_STATUE_RUNNING != uramsync->status) {
                    break;
                }
                if (URAMSYNC_MASTER_DEV_TYPE == uramsync->devtype) {
                    if (uramsync->rx.st.magic != TP_ST_MAGIC) {
                        reset_signal(uramsync);
                        break;
                    }
                        }
                // judge seq len valid
                seqtmp = uramsync->rx.payload[i].seq;
                lentmp = uramsync->rx.payload[i].len;
                rseqtmp = uramsync->tx.st.rseq;

                if ((lentmp > (TP_PAYLOAD_LEN + 4 + 4)) ||
                    (lentmp > (TP_PAYLOAD_LEN + 4 + 4)) ||
                    (seqtmp != (rseqtmp + 1))
                    ) {// seq len invalid
                    continue;
                }

                // memcpy + crc
                ramsync_fast_memcpy(&uramsync->rx_cache, &uramsync->rx.payload[i], lentmp);
                uramsync->rx_cache.crc = uramsync->rx.payload[i].crc;
                if ((uramsync->rx_cache.seq != seqtmp) ||
                    (uramsync->rx_cache.len != lentmp)
                    ) {
                    continue;
                }

                // judge crc
                utils_crc32_stream_init(&crc_ctx);
                utils_crc32_stream_feed_block(&crc_ctx,
                        (uint8_t *)&uramsync->rx_cache,
                        uramsync->rx_cache.len);
                crctmp = utils_crc32_stream_results(&crc_ctx);


                tpdbg_log("[%s] [task_rx] crctmp = 0x%08lx, cache.crc = 0x%08lx\r\n",
                        uramsync->name, crctmp, uramsync->rx_cache.crc);
                //vTaskDelay(pdMS_TO_TICKS(100));// fixme, can use isr notify

                if (uramsync->rx_cache.crc != crctmp) {
                    continue;
                }

                slot_have_validdata = 1;
                break;
            }

            if (slot_have_validdata) {
                //tpdbg_log("[%s] [task_rx] 1. ensure slot have new data.\r\n", uramsync->name);
                break;
            }
        }
        if (URAMSYNC_STATUE_RUNNING != uramsync->status) {
            continue;
        }

        /* slot to desc */
        while (1) {
            res = desc_push_toback(&uramsync->rx_desc,
                    &uramsync->rx_cache.buf,
                    uramsync->rx_cache.len,
                    POLL_UNIT_TIME_MS);//DESC_FOREVER
            if (URAMSYNC_STATUE_RUNNING != uramsync->status) {
                break;
            }
            if (res != 0) {
                continue;
            }
            break;
        }
        if (URAMSYNC_STATUE_RUNNING != uramsync->status) {
            continue;
        }

        tpdbg_log("[%s] [task_rx] 2. slot2desc,update rseq = %ld cache.crc = 0x%08lx.\r\n",
                uramsync->name, uramsync->rx_cache.seq, uramsync->rx_cache.crc);

        // update txtxtxtxtxtxtx!!!!! st rseq
        uramsync->tx.st.rseq = uramsync->rx_cache.seq;  // __rx_rseq_update
    }
}

int uramsync_init(tp_uramsync_t *uramsync, uint32_t type)
{
    if (NULL == uramsync) {
        return -1;
    }

    uramsync->reset_thdr = NULL;
    // init lramsync mem set 0, hw stream
    __ramsync_low_init(uramsync, type);

    uramsync->emptyslot_evt =
        xEventGroupCreateStatic(&uramsync->xEventGroupBuffer);// rx_isr -> rx_task
    if (NULL == uramsync->emptyslot_evt) {
        return -2;
    }

    desc_init(&uramsync->tx_desc, TP_TXDESC_NUM);
    desc_init(&uramsync->rx_desc, TP_RXDESC_NUM);

    // start tx/rx task
    xTaskCreate(
            uramsync_task_tx,
            (char*)"rs_tx",
            RAMSYNC_TX_STACK_SIZE/sizeof(StackType_t), uramsync,
            RAMSYNC_TX_TASK_PRI,
            &uramsync->tx_thdr);

    xTaskCreate(
            uramsync_task_rx,
            (char*)"rs_rx",
            RAMSYNC_RX_STACK_SIZE/sizeof(StackType_t), uramsync,
            RAMSYNC_TX_TASK_PRI,
            &uramsync->rx_thdr);

    return 0;
}

int uramsync_deinit(tp_uramsync_t *uramsync)
{
    /* stop hw mem or ops */
    __ramsync_low_deinit(uramsync);
    desc_deinit(&uramsync->tx_desc);
    desc_deinit(&uramsync->rx_desc);
    vEventGroupDelete(uramsync->emptyslot_evt);

    /* delete task */
    vTaskDelete(uramsync->tx_thdr);
    vTaskDelete(uramsync->rx_thdr);

    return 0;
}

int uramsync_rx_pop(tp_uramsync_t *uramsync, void *buf, uint32_t *len_p, uint32_t timeout_ms)
{
    int res;

    if (NULL == uramsync) {
        return -1;
    }

    res = (int)desc_pop(&uramsync->rx_desc, buf, len_p, timeout_ms);
    if (0 != res) {
        if (len_p) {
            *len_p = 0;
        }
    }

    return res;
}

int uramsync_tx_push_toback(tp_uramsync_t *uramsync, void *buf, uint32_t len, uint32_t timeout_ms)
{
    int res;

    if (NULL == uramsync) {
        return -1;
    }

    if (len > TP_PAYLOAD_LEN) {
        tpdbg_log("uramsync_tx_push_toback len(%ld) err, because of TP_PAYLOAD_LEN = %d\r\n", len, TP_PAYLOAD_LEN);
        return -2;
    }

    res = (int)desc_push_toback(&uramsync->tx_desc, buf, len, timeout_ms);

    return res;
}

int uramsync_tx_push_tofront(tp_uramsync_t *uramsync, void *buf, uint32_t len, uint32_t timeout_ms)
{
    int res;

    if (NULL == uramsync) {
        return -1;
    }

    if (len > TP_PAYLOAD_LEN) {
        tpdbg_log("uramsync_tx_push_tofront len(%ld) err, because of TP_PAYLOAD_LEN = %d\r\n", len, TP_PAYLOAD_LEN);
        return -2;
    }

    res = (int)desc_push_tofront(&uramsync->tx_desc, buf, len, timeout_ms);

    return res;
}

void test_reset_signal_cb(void *arg)
{
    __reset_signal_cb(arg);
}

