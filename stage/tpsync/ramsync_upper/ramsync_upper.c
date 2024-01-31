
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

    for (i = 0; i < TP_TXPAYLOAD_NUM; i++) {
        tpdbg_log("rs->p_tx->payload[%ld].magic = 0x%08lx\r\n" , rs->p_tx->payload[i].magic);
        tpdbg_log("rs->p_tx->payload[%ld].rseq = %ld\r\n" , rs->p_tx->payload[i].rseq);
        tpdbg_log("rs->p_tx->payload[%ld].len = %ld\r\n" , i, rs->p_tx->payload[i].len);
        tpdbg_log("rs->p_tx->payload[%ld].seq = %ld\r\n" , i, rs->p_tx->payload[i].seq);
        tpdbg_log("rs->p_tx->payload[%ld].crc = 0x%08lx\r\n" , i, rs->p_tx->payload[i].crc);
        tpdbg_buf("tx", &rs->p_tx->payload[i], 20);
    }

    for (i = 0; i < TP_RXPAYLOAD_NUM; i++) {
        tpdbg_log("rs->p_rx->payload[%ld].magic = 0x%08lx\r\n" , rs->p_rx->payload[i].magic);
        tpdbg_log("rs->p_rx->payload[%ld].rseq = %ld\r\n" , rs->p_rx->payload[i].rseq);
        tpdbg_log("rs->p_rx->payload[%ld].len = %ld\r\n" , i, rs->p_rx->payload[i].len);
        tpdbg_log("rs->p_rx->payload[%ld].seq = %ld\r\n" , i, rs->p_rx->payload[i].seq);
        tpdbg_log("rs->p_rx->payload[%ld].crc = 0x%08lx\r\n" , i, rs->p_rx->payload[i].crc);
        tpdbg_buf("rx", &rs->p_rx->payload[i], 20);
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
    memset(uramsync->p_tx, 0, sizeof(tp_txbuf_t));
    memset(uramsync->p_rx, 0, sizeof(tp_rxbuf_t));

    for (int i = 0; i < TP_TXPAYLOAD_NUM; i++) {
        uramsync->p_tx->payload[i].magic = TP_ST_MAGIC;
    }

    /* local sequnce */
    memset(&uramsync->tx_seq, 0, sizeof(uint32_t));

    /* tx/rx desc hdr */
    // need tx_desc rx_desc set empty???

    /* for rx notify */
    xSemaphoreTake(uramsync->tx_sem, 0);
    xSemaphoreTake(uramsync->rx_sem, 0);

    /* calulate crc for rx */
    memset(uramsync->p_rx_cache, 0, sizeof(tp_payload_t));
}

static void ramsynck_reset_entry(void *arg)
{
    tp_uramsync_t *uramsync = (tp_uramsync_t *)arg;

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
                vTaskDelay(pdMS_TO_TICKS(1000));// wait for one slot sync complete
                #if (TP_TXPAYLOAD_NUM > 1)
                if (TP_ST_MAGIC == uramsync->p_rx->payload[0].magic) {
                    break;
                }
                #endif /* (TP_TXPAYLOAD_NUM > 1) */
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
static void __ramsync_low_rx_cb(void *arg)
{
    tp_uramsync_t *uramsync = (tp_uramsync_t *)arg;

    BaseType_t xHigherPriorityTaskWoken, xResult;
    uint32_t flag = 0;

    for (int i = 0; i < TP_TXPAYLOAD_NUM; i++) {
        if (uramsync->p_rx->payload[i].magic == TP_ST_MAGIC && 
            ((uramsync->p_rx->payload[i].len <= (TP_PAYLOAD_LEN + 4 + 4)) &&
            (uramsync->p_rx->payload[i].seq == (uramsync->p_tx->payload[i].rseq + 1)))) {
            flag = 1;
        }
    }
    if(!(flag || uramsync->p_rx->payload[0].magic != TP_ST_MAGIC)){
        return;
    }
    
    if (uramsync->rx_sem) {
        if (xPortIsInsideInterrupt()) {
            xHigherPriorityTaskWoken = pdFALSE;
            xResult = xSemaphoreGiveFromISR(
                    uramsync->rx_sem,    /* The event group being updated. */
                    &xHigherPriorityTaskWoken
            );

            /* Was the message posted successfully? */
            if( xResult != pdFAIL ) {
                portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
            }
        } else {
             xSemaphoreGive(uramsync->rx_sem);
        }
    }
}

static void __ramsync_low_tx_cb(void *arg)
{
    tp_uramsync_t *uramsync = (tp_uramsync_t *)arg;

    BaseType_t xHigherPriorityTaskWoken, xResult;

    if (uramsync->tx_sem) {
        if (xPortIsInsideInterrupt()) {
            xHigherPriorityTaskWoken = pdFALSE;
            xResult = xSemaphoreGiveFromISR(
                    uramsync->tx_sem,    /* The event group being updated. */
                    &xHigherPriorityTaskWoken
            );

            /* Was the message posted successfully? */
            if( xResult != pdFAIL ) {
                portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
            }
        } else {
             xSemaphoreGive(uramsync->tx_sem);
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

    node_mem_t node_txbuf[TP_TXPAYLOAD_NUM];
    node_mem_t node_rxbuf[TP_RXPAYLOAD_NUM];

    memset(uramsync, 0, sizeof(tp_uramsync_t));

    uramsync->p_tx = (tp_txbuf_t *)pvPortMalloc(sizeof(tp_txbuf_t));
    uramsync->p_rx = (tp_rxbuf_t *)pvPortMalloc(sizeof(tp_rxbuf_t));
    uramsync->p_rx_cache = (tp_payload_t *)malloc(sizeof(tp_payload_t));

    memset(uramsync->p_tx, 0, sizeof(tp_txbuf_t));
    memset(uramsync->p_rx, 0, sizeof(tp_rxbuf_t));
    memset(uramsync->p_rx_cache, 0, sizeof(tp_payload_t));

    uramsync->devtype = type;

    for (i = 0; i < TP_TXPAYLOAD_NUM; i++) {
        node_txbuf[i].buf = &uramsync->p_tx->payload[i];
        node_txbuf[i].len = sizeof(tp_payload_t);
    }

    for (i = 0; i < TP_RXPAYLOAD_NUM; i++) {
        node_rxbuf[i].buf = &uramsync->p_rx->payload[i];
        node_rxbuf[i].len = sizeof(tp_payload_t);
    }

    /* set magic */
    for (i = 0; i < TP_TXPAYLOAD_NUM; i++) {
        uramsync->p_tx->payload[i].magic = TP_ST_MAGIC;
    }

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
            node_txbuf, TP_TXPAYLOAD_NUM,
            __ramsync_low_tx_cb, uramsync,
            node_rxbuf, TP_RXPAYLOAD_NUM,
            __ramsync_low_rx_cb, uramsync,
            NULL,NULL 
            );
    } else if (URAMSYNC_SLAVE_DEV_TYPE == uramsync->devtype) {
        strncpy(uramsync->name, "slv", sizeof(uramsync->name) - 1);
        lramsync_init(
            &uramsync->hw,
            node_txbuf, TP_TXPAYLOAD_NUM,
            __ramsync_low_tx_cb, uramsync,
            node_rxbuf, TP_RXPAYLOAD_NUM,
            __ramsync_low_rx_cb, uramsync,
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

    reset_signal_first_init(uramsync);

    return 0;
}

static int __ramsync_low_deinit(tp_uramsync_t *arg)
{
    tp_uramsync_t *uramsync = (tp_uramsync_t *)arg;

    lramsync_deinit(&uramsync->hw);
    vPortFree( uramsync->p_tx);
    vPortFree(uramsync->p_rx);
    free(uramsync->p_rx_cache);

    return 0;
}

/*---------------------- tx moudule ------------------------*/
static void  uramsync_task_tx(void *arg)
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
first_point:
        /* ensure status tx enter loop */
        while (URAMSYNC_STATUE_RUNNING != uramsync->status) {
            if (URAMSYNC_STATUE_IDLE == uramsync->status) {
                uramsync->status = URAMSYNC_STATUE_TXRESET_DONE;
            }
            vTaskDelay(pdMS_TO_TICKS(1));
        }

        /* ensure desc have valid data */
        while (1) {
            desc_peek(&uramsync->tx_desc, &lentmp, POLL_UNIT_TIME_MS);
            if (URAMSYNC_STATUE_RUNNING != uramsync->status) {
                goto first_point;
            }
            if (0 == lentmp) {
                continue;
            }

            if (lentmp > TP_PAYLOAD_LEN) {
                tpdbg_log("pop too long packet\r\n");
                desc_pop(&uramsync->tx_desc, NULL, NULL, 0);// 0-noblock DESC_FOREVER-block
                continue;
            }
            //tpdbg_log("[%s] [task_tx] 1. ensure desc have valid data.\r\n", uramsync->name);
            break;
        }

        /* ensure empty slot */
        ensure_empty_slot = 0;
        while (1) {
            xSemaphoreTake(uramsync->tx_sem, POLL_UNIT_TIME_MS);
            for (i = 0; i < TP_TXPAYLOAD_NUM; i++) {
                if (uramsync->p_tx->payload[i].seq <= uramsync->p_rx->payload[i].rseq) {
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
                goto first_point;
            }
        }

        /* desc to slot,  update buf + len */
        uramsync->p_tx->payload[slottmp].len = 4 + 4;// len(4) + seq(4) + buf(x) crc(4)

        desc_pop(&uramsync->tx_desc,
                uramsync->p_tx->payload[slottmp].buf,
                &poplentmp ,
                0);// 0-noblock DESC_FOREVER-block

        uramsync->p_tx->payload[slottmp].len += poplentmp;// len(4) + seq(4) + buf(x) crc(4)

        // seq crc update
        uramsync->tx_seq++;
        uramsync->p_tx->payload[slottmp].seq = uramsync->tx_seq;

        utils_crc32_stream_init(&crc_ctx);
        utils_crc32_stream_feed_block(&crc_ctx,
                (uint8_t *)&uramsync->p_tx->payload[slottmp].len,
                uramsync->p_tx->payload[slottmp].len);

        uramsync->p_tx->payload[slottmp].crc = utils_crc32_stream_results(&crc_ctx);

        tpdbg_log("[%s] [task_tx] 3. desc to slot, payload len = %ld, seq = %ld, poplentmp = %ld, crc = 0x%08lx\r\n",
                uramsync->name,
                uramsync->p_tx->payload[slottmp].len,
                uramsync->p_tx->payload[slottmp].seq,
                poplentmp,
                uramsync->p_tx->payload[slottmp].crc
                );

        //tpdbg_buf(uramsync->name, &uramsync->p_tx->st, sizeof(uramsync->p_tx->st));
        //tpdbg_buf(uramsync->name, &uramsync->p_tx->payload[slottmp], uramsync->p_tx->payload[slottmp].len);
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

    BaseType_t ret;
    int res;

    tp_uramsync_t *uramsync = (tp_uramsync_t *)arg;

    if (NULL == uramsync) {
        return;
    }

    tpdbg_log("[%s] uramsync_task_rx\r\n", uramsync->name);

    while (1) {
first_point:
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
                goto first_point;
            }
            while (1) {
                ret = xSemaphoreTake(uramsync->rx_sem, POLL_UNIT_TIME_MS);
                if (URAMSYNC_STATUE_RUNNING != uramsync->status) {
                    goto first_point;
                }
                if (ret == pdPASS){
                    break;
                }
            }

            for (i = 0; i < TP_TXPAYLOAD_NUM; i++) {
                if (URAMSYNC_STATUE_RUNNING != uramsync->status) {
                    goto first_point;
                }
                if (URAMSYNC_MASTER_DEV_TYPE == uramsync->devtype) {
                    if (uramsync->p_rx->payload[i].magic != TP_ST_MAGIC) {
                        reset_signal(uramsync);
                        break;
                    }
                }
                // judge seq len valid
                seqtmp = uramsync->p_rx->payload[i].seq;
                lentmp = uramsync->p_rx->payload[i].len;
                rseqtmp = uramsync->p_tx->payload[i].rseq;

                if ((lentmp > (TP_PAYLOAD_LEN + 4 + 4)) ||
                    (seqtmp != (rseqtmp + 1))
                    ) {// seq len invalid
                    continue;
                }

                // memcpy + crc
                ramsync_fast_memcpy(uramsync->p_rx_cache, &uramsync->p_rx->payload[i], lentmp + 8);
                uramsync->p_rx_cache->crc = uramsync->p_rx->payload[i].crc;
                if ((uramsync->p_rx_cache->seq != seqtmp) ||
                    (uramsync->p_rx_cache->len != lentmp)
                    ) {
                    continue;
                }

                // judge crc
                utils_crc32_stream_init(&crc_ctx);
                utils_crc32_stream_feed_block(&crc_ctx,
                        (uint8_t *)&uramsync->p_rx_cache->len,
                        uramsync->p_rx_cache->len);
                crctmp = utils_crc32_stream_results(&crc_ctx);

                //vTaskDelay(pdMS_TO_TICKS(100));// fixme, can use isr notify

                if (uramsync->p_rx_cache->crc != crctmp) {
                    tpdbg_log("[%s] [task_rx] crctmp = 0x%08lx, cache.crc = 0x%08lx\r\n",
                        uramsync->name, crctmp, uramsync->p_rx_cache->crc);
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

        /* slot to desc */
        while (1) {
            res = desc_push_toback(&uramsync->rx_desc,
                    uramsync->p_rx_cache->buf,
                    uramsync->p_rx_cache->len,
                    POLL_UNIT_TIME_MS);//DESC_FOREVER
            if (URAMSYNC_STATUE_RUNNING != uramsync->status) {
                goto first_point;
            }
            if (res != 0) {
                continue;
            }
            break;
        }

        tpdbg_log("[%s] [task_rx] 2. slot2desc,update rseq = %ld cache.crc = 0x%08lx.\r\n",
                uramsync->name, uramsync->p_rx_cache->seq, uramsync->p_rx_cache->crc);

        // update txtxtxtxtxtxtx!!!!! st rseq
        taskENTER_CRITICAL();
        for (i = 0; i < TP_TXPAYLOAD_NUM; i++) {
            uramsync->p_tx->payload[i].rseq = uramsync->p_rx_cache->seq;  // __rx_rseq_update
        }
        taskEXIT_CRITICAL();
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
    uramsync->tx_sem = xSemaphoreCreateBinary();
    if(uramsync->tx_sem == NULL){
        return -2;
    }
    uramsync->rx_sem = xSemaphoreCreateBinary();
    if(uramsync->tx_sem == NULL){
        vSemaphoreDelete(uramsync->tx_sem);
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
    if(uramsync->tx_sem){
        vSemaphoreDelete(uramsync->tx_sem);
    }
    if(uramsync->rx_sem){ 
        vSemaphoreDelete(uramsync->rx_sem);
    }

    /* delete task */
    if(uramsync->reset_thdr)
        vTaskDelete(uramsync->reset_thdr);
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

