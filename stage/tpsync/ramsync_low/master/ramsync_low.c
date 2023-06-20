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
#include <stdint.h>
#include <stdio.h>

#include <blog.h>
#include <utils_log.h>

#include <FreeRTOS.h>
#include <task.h>
#include <event_groups.h>

#include <bl_irq.h>
#include <bl_dma.h>
#include <bl_gpio.h>

#if defined BL702
#include <bl702_spi.h>
#include <bl702_gpio.h>
#include <bl702_glb.h>
#include <bl702_dma.h>
#include <bl702_clock.h>
#include <bl702.h>
#elif defined BL702L
#include <bl702l_spi.h>
#include <bl702l_gpio.h>
#include <bl702l_glb.h>
#include <bl702l_dma.h>
#include <bl702l_clock.h>
#include <bl702l.h>
#endif

#include <hosal_dma.h>
#include <hosal_spi.h>
#include <hal_gpio.h>
#include <bl_gpio.h>

#include "ramsync_low.h"
#include "tpdbg.h"

#if defined BL702
#define TPSYNC_MST_MISO_PIN 4
#define TPSYNC_MST_MOSI_PIN 5
#define TPSYNC_MST_CLK_PIN  3
#define TPSYNC_MST_CS_PIN   6
#elif defined BL702L
#define TPSYNC_MST_MISO_PIN 20
#define TPSYNC_MST_MOSI_PIN 21
#define TPSYNC_MST_CLK_PIN  19
#define TPSYNC_MST_CS_PIN   18
#endif

static lramsync_spi_config_t _spi_cfg = {
    .port = 0,
    .spi_mode = 0,           /* 0: phase 0, polarity low */
#if defined BL702
    .spi_speed = 18000000,
#elif defined BL702L
    /** SPI_SetClock use 80M for freqency calculation;
     * 32M for 16M; 25M for 10.67M; 20M for 8M;
     *  */
    .spi_speed = 8000000,
#endif
    .miso = TPSYNC_MST_MISO_PIN,
    .mosi = TPSYNC_MST_MOSI_PIN,
    .clk = TPSYNC_MST_CLK_PIN,
    .cs = TPSYNC_MST_CS_PIN,
};

#define RAMSYNC_LOW_DUMY_CNT 1

struct _ramsync_low_priv {
	DMA_LLI_Ctrl_Type *tx_lli;
	DMA_LLI_Ctrl_Type *rx_lli;
};

static int _lli_init(lramsync_ctx_t *ctx)
{
    uint32_t i = 0;
    struct _ramsync_low_priv *priv;
    volatile struct DMA_Control_Reg dmactrl;

    priv = ctx->priv;

    for (i = 0; i < ctx->items_tx; i++) {
        dmactrl.SBSize = DMA_BURST_SIZE_1;
        dmactrl.DBSize = DMA_BURST_SIZE_1;
        dmactrl.SWidth = DMA_TRNS_WIDTH_32BITS;
        dmactrl.DWidth = DMA_TRNS_WIDTH_32BITS;
        dmactrl.Prot = 0;
#ifdef BL702
        dmactrl.SLargerD = 0;
#endif
        dmactrl.dst_add_mode = DISABLE;
        dmactrl.dst_min_mode = DISABLE;
        dmactrl.fix_cnt = 0;

        dmactrl.TransferSize = ctx->node_tx[i].len / 4;
        dmactrl.SI = DMA_MINC_ENABLE;
        dmactrl.DI = DMA_MINC_DISABLE;
        dmactrl.I = 1;
        priv->tx_lli[i].srcDmaAddr  = (uint32_t)(uint32_t)ctx->node_tx[i].buf;
        priv->tx_lli[i].destDmaAddr = (uint32_t)(SPI_BASE + SPI_FIFO_WDATA_OFFSET);
        priv->tx_lli[i].dmaCtrl = dmactrl;

        if (i != ctx->items_tx - 1) {
        	priv->tx_lli[i].nextLLI = (uint32_t)&priv->tx_lli[i + 1];
        } else {
        	priv->tx_lli[i].nextLLI = (uint32_t)&priv->tx_lli[0];
        }
    }

    for (i = 0; i < ctx->items_rx; i++) {
        dmactrl.SBSize = DMA_BURST_SIZE_1;
        dmactrl.DBSize = DMA_BURST_SIZE_1;
        dmactrl.SWidth = DMA_TRNS_WIDTH_32BITS;
        dmactrl.DWidth = DMA_TRNS_WIDTH_32BITS;
        dmactrl.Prot = 0;
#ifdef BL702
        dmactrl.SLargerD = 0;
#endif
        dmactrl.I = 1;
        dmactrl.SI = DMA_MINC_DISABLE;
        dmactrl.DI = DMA_MINC_ENABLE;
 
        /* rx lli config */
        dmactrl.TransferSize = ctx->node_rx[i].len / 4;
        priv->rx_lli[i].srcDmaAddr  = (uint32_t)(SPI_BASE + SPI_FIFO_RDATA_OFFSET);
        priv->rx_lli[i].destDmaAddr = (uint32_t)ctx->node_rx[i].buf;
        priv->rx_lli[i].dmaCtrl     = dmactrl;

        if (i != ctx->items_rx - 1) {
        	priv->rx_lli[i].nextLLI = (uint32_t)&priv->rx_lli[i + 1];
        } else {
        	priv->rx_lli[i].nextLLI = (uint32_t)&priv->rx_lli[0];
        }
    }
    return 0;
}

static ATTR_TCM_SECTION void _spi_int_handler_tx(
    void *arg, uint32_t flag)
{
    lramsync_ctx_t *ctx = (lramsync_ctx_t *)arg;

	if (HOSAL_DMA_INT_TRANS_COMPLETE != flag) {
		blog_info("spi_dma_tx error\r\n");
		return;
	}

    if (ctx->tx_cb) {
        ctx->tx_cb(ctx->tx_arg);
    }
}

static ATTR_TCM_SECTION void _spi_int_handler_rx(
    void *arg, uint32_t flag)
{
    lramsync_ctx_t *ctx = (lramsync_ctx_t *)arg;

	if (HOSAL_DMA_INT_TRANS_COMPLETE != flag) {
		blog_info("spi_dma_rx error\r\n");
		return;
	}

    if (ctx->rx_cb) {
        ctx->rx_cb(ctx->rx_arg);
    }
}

static void _spi_gpio_init(const lramsync_spi_config_t *config)
{
	GLB_GPIO_Type gpiopins[4];
    gpiopins[0] = config->cs;
    gpiopins[1] = config->clk;
    gpiopins[2] = config->mosi;
    gpiopins[3] = config->miso;
    GLB_GPIO_Func_Init(GPIO_FUN_SPI, gpiopins,
                       sizeof(gpiopins) / sizeof(gpiopins[0]));
    GLB_Set_SPI_0_ACT_MOD_Sel(GLB_SPI_PAD_ACT_AS_MASTER);
}

static int _spi_hw_init(const lramsync_spi_config_t *config)
{
    SPI_CFG_Type spicfg;
    SPI_FifoCfg_Type fifocfg;
    SPI_ID_Type spi_id;

    spi_id = config->port;

#ifdef BL702
    SPI_ClockCfg_Type clockcfg;
    uint8_t clk_div;

    blog_info("core clk %ld, bclk div %d\r\n", SystemCoreClockGet(), GLB_Get_BCLK_Div());
    clk_div = (uint8_t)((SystemCoreClockGet()/(GLB_Get_BCLK_Div()+1)) / 2 / config->spi_speed);
    GLB_Set_SPI_CLK(ENABLE, 0);

    clockcfg.startLen = clk_div;
    clockcfg.stopLen = clk_div;
    clockcfg.dataPhase0Len = clk_div;
    clockcfg.dataPhase1Len = clk_div;
    clockcfg.intervalLen = clk_div;
    SPI_ClockConfig(spi_id, &clockcfg);

    fifocfg.txFifoThreshold = 1;
    fifocfg.rxFifoThreshold = 1;

#elif defined BL702L
    spicfg.slavePin = SPI_SLAVE_PIN_4;

    // GLB_Set_SPI_CLK(ENABLE, GLB_SPI_CLK_SRC_BCLK, 0);
    SPI_SetClock(spi_id,config->spi_speed);

    fifocfg.txFifoThreshold = 0;
    fifocfg.rxFifoThreshold = 0;
#endif

    /* spi config */
    spicfg.deglitchEnable = DISABLE;
    spicfg.continuousEnable = ENABLE;
    spicfg.byteSequence = SPI_BYTE_INVERSE_BYTE0_FIRST,
    spicfg.bitSequence = SPI_BIT_INVERSE_MSB_FIRST,
    spicfg.frameSize = SPI_FRAME_SIZE_32;

    if (config->spi_mode == 0) {
        spicfg.clkPhaseInv = SPI_CLK_PHASE_INVERSE_0;
        spicfg.clkPolarity = SPI_CLK_POLARITY_LOW;
    } else if (config->spi_mode == 1) {
        spicfg.clkPhaseInv = SPI_CLK_PHASE_INVERSE_1;
        spicfg.clkPolarity = SPI_CLK_POLARITY_LOW;
    } else if (config->spi_mode == 2) {
        spicfg.clkPhaseInv = SPI_CLK_PHASE_INVERSE_0;
        spicfg.clkPolarity = SPI_CLK_POLARITY_HIGH;
    } else if (config->spi_mode == 3) {
        spicfg.clkPhaseInv = SPI_CLK_PHASE_INVERSE_1;
        spicfg.clkPolarity = SPI_CLK_POLARITY_HIGH;
    } else {
        blog_error("node support polar_phase \r\n");
        return -1;
    }
    SPI_Init(spi_id, &spicfg);

    SPI_Disable(spi_id, SPI_WORK_MODE_MASTER);

    SPI_IntMask(spi_id, SPI_INT_ALL, MASK);

    /* fifo */
    fifocfg.txFifoDmaEnable = ENABLE;
    fifocfg.rxFifoDmaEnable = ENABLE;
    SPI_FifoConfig(spi_id, &fifocfg);
    SPI_ClrTxFifo(spi_id);
    SPI_ClrRxFifo(spi_id);

    return 0;
}

static int _spi_dma_init(lramsync_ctx_t *ctx)
{
    int ret;
    DMA_LLI_Cfg_Type txllicfg;
    DMA_LLI_Cfg_Type rxllicfg;
    const lramsync_spi_config_t *config = ctx->cfg;
    struct _ramsync_low_priv *priv;

    if (ctx->dma_tx_chan == -1) {
        ctx->dma_tx_chan = hosal_dma_chan_request(0);
        if (ctx->dma_tx_chan < 0) {
            blog_error("SPI TX DMA CHANNEL get failed!\r\n");
            return -1;
        }
    }

    if (ctx->dma_rx_chan == -1) {
        ctx->dma_rx_chan = hosal_dma_chan_request(0);
        if (ctx->dma_rx_chan < 0) {
            blog_error("SPI RX DMA CHANNEL get failed!\r\n");
            return -1;
        }
    }
    ret = _lli_init(ctx);
    if (ret < 0) {
        blog_error("init lli failed. \r\n");
        return -1;
    }

    priv = (struct _ramsync_low_priv *)ctx->priv;
    txllicfg.dir = DMA_TRNS_M2P;
    txllicfg.srcPeriph = DMA_REQ_NONE;
    txllicfg.dstPeriph = DMA_REQ_SPI_TX;

    rxllicfg.dir = DMA_TRNS_P2M;
    rxllicfg.srcPeriph = DMA_REQ_SPI_RX;
    rxllicfg.dstPeriph = DMA_REQ_NONE;

#ifdef BL702
    DMA_LLI_Init(ctx->dma_rx_chan, &rxllicfg);
    DMA_LLI_Update(ctx->dma_rx_chan, (uint32_t)&priv->rx_lli[0]);
    hosal_dma_irq_callback_set(ctx->dma_rx_chan, _spi_int_handler_rx, ctx);

    DMA_LLI_Init(ctx->dma_tx_chan, &txllicfg);
    DMA_LLI_Update(ctx->dma_tx_chan, (uint32_t)&priv->tx_lli[0]);
    hosal_dma_irq_callback_set(ctx->dma_tx_chan, _spi_int_handler_tx, ctx);
#elif defined BL702L
    DMA_LLI_Init(ctx->dma_rx_chan, ctx->dma_rx_chan, &rxllicfg);
    DMA_LLI_Update(DMA0_ID, ctx->dma_rx_chan, (uint32_t)&priv->rx_lli[0]);
    hosal_dma_irq_callback_set(ctx->dma_rx_chan, _spi_int_handler_rx, ctx);

    DMA_LLI_Init(ctx->dma_tx_chan, ctx->dma_tx_chan, &txllicfg);
    DMA_LLI_Update(DMA0_ID, ctx->dma_tx_chan, (uint32_t)&priv->tx_lli[0]);
    hosal_dma_irq_callback_set(ctx->dma_tx_chan, _spi_int_handler_tx, ctx);
#endif

    SPI_Enable(config->port, SPI_WORK_MODE_MASTER);
    return 0;
}

void lramsync_callback_register(
        lramsync_ctx_t *ctx,
        lramsync_cb_func_t tx_cb, void *tx_arg,
        lramsync_cb_func_t rx_cb, void *rx_arg)
{
    ctx->tx_cb  = tx_cb;
    ctx->tx_arg = tx_arg;
    ctx->rx_cb  = rx_cb;
    ctx->rx_arg = rx_arg;
}

void lramsync_start(lramsync_ctx_t *ctx)
{
	hosal_dma_chan_start(ctx->dma_rx_chan);
	hosal_dma_chan_start(ctx->dma_tx_chan);
}

void lramsync_reset(lramsync_ctx_t *ctx)
{
    struct _ramsync_low_priv *priv;

    priv = (struct _ramsync_low_priv *)ctx->priv;

    blog_info("lramsync_reset\r\n");
#ifdef BL702
    GLB_AHB_Slave1_Reset(BL_AHB_SLAVE1_SPI);
#elif defined BL702L
    GLB_AHB_MCU_Software_Reset(GLB_AHB_MCU_SW_SPI);
#endif
    SPI_Disable(ctx->cfg->port, SPI_WORK_MODE_MASTER);
	hosal_dma_chan_stop(ctx->dma_tx_chan);
	hosal_dma_chan_stop(ctx->dma_rx_chan);

    _spi_gpio_init(ctx->cfg);
    bl_gpio_output_set(ctx->cfg->cs, 1);
    _spi_hw_init(ctx->cfg);

#ifdef BL702
    DMA_LLI_Update(ctx->dma_rx_chan, (uint32_t)&priv->rx_lli[0]);
    DMA_LLI_Update(ctx->dma_tx_chan, (uint32_t)&priv->tx_lli[0]);
#elif defined BL702L
    DMA_LLI_Update(DMA0_ID, ctx->dma_rx_chan, (uint32_t)&priv->rx_lli[0]);
    DMA_LLI_Update(DMA0_ID, ctx->dma_tx_chan, (uint32_t)&priv->tx_lli[0]);
#endif

    vTaskDelay(pdMS_TO_TICKS(5));

    SPI_ClrTxFifo(ctx->cfg->port);
    SPI_ClrRxFifo(ctx->cfg->port);
    SPI_Enable(ctx->cfg->port, SPI_WORK_MODE_MASTER);
    bl_gpio_output_set(ctx->cfg->cs, 0);

    hosal_dma_chan_start(ctx->dma_rx_chan);
    hosal_dma_chan_start(ctx->dma_tx_chan);
}

void lramsync_deinit(lramsync_ctx_t *ctx)
{
    struct _ramsync_low_priv *priv;
    
    if (ctx == NULL) {
        return;
    }

    priv = (struct _ramsync_low_priv *)ctx->priv;

    SPI_Disable(ctx->cfg->port, SPI_WORK_MODE_MASTER);

	hosal_dma_chan_release(ctx->dma_tx_chan);
	hosal_dma_chan_release(ctx->dma_rx_chan);

    vPortFree(ctx->node_rx);
    vPortFree(ctx->node_tx);
    vPortFree(priv->tx_lli);
    vPortFree(priv->rx_lli);
    vPortFree(ctx->priv);
}

void lramsync_init(
        lramsync_ctx_t *ctx,
        node_mem_t *node_tx, uint32_t items_tx,
        lramsync_cb_func_t tx_cb, void *tx_arg,
        node_mem_t *node_rx, uint32_t items_rx,
        lramsync_cb_func_t rx_cb, void *rx_arg,
        lramsync_cb_func_t reset_signal_cb, void *reset_signal_arg)
{
    if (ctx == NULL) {
        return;
    }
    if(ctx->cfg != NULL){
        _spi_cfg = *ctx->cfg;
    }
    memset(ctx, 0, sizeof(lramsync_ctx_t));
    ctx->cfg         = &_spi_cfg;
    ctx->dma_tx_chan = -1;
    ctx->dma_rx_chan = -1;
    ctx->reset_signal_cb = reset_signal_cb;
    ctx->reset_signal_arg = reset_signal_arg;

    ctx->node_tx = pvPortMalloc(sizeof(node_mem_t) * items_tx);
    if (ctx->node_tx == NULL) {
        return;
    }
    ctx->node_rx = pvPortMalloc(sizeof(node_mem_t) * items_rx);
    if (ctx->node_rx == NULL) {
        return;
    }
    memcpy(ctx->node_tx, node_tx, sizeof(node_mem_t) * items_tx);
    memcpy(ctx->node_rx, node_rx, sizeof(node_mem_t) * items_rx);
    ctx->items_tx    = items_tx;
    ctx->items_rx    = items_rx;

    ctx->priv = pvPortMalloc(sizeof(struct _ramsync_low_priv));
    if (ctx->priv == NULL) {
        return;
    }
    memset(ctx->priv, 0, sizeof(struct _ramsync_low_priv));

    struct _ramsync_low_priv *priv = ctx->priv;

    /* tx lli config */
    priv->tx_lli = pvPortMalloc(sizeof(DMA_LLI_Ctrl_Type) * ctx->items_tx);
    if (priv->tx_lli == NULL) {
        return;
    }
    /* rx lli config */
    priv->rx_lli = pvPortMalloc(sizeof(DMA_LLI_Ctrl_Type) * ctx->items_rx);
    if (priv->rx_lli == NULL) {
        return;
    }

    lramsync_callback_register(ctx, tx_cb, tx_arg, rx_cb, rx_arg);

    _spi_gpio_init(ctx->cfg);
    bl_gpio_enable_output(ctx->cfg->cs, 0, 1);
    _spi_hw_init(ctx->cfg);
    _spi_dma_init(ctx);

    tpdbg_log("lramsync_init master\r\n");
}

