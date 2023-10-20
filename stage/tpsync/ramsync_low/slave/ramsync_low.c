#include <stdio.h>
#include <stdint.h>
#include <utils_list.h>
#include <FreeRTOS.h>
#include <task.h>
#include <blog.h>
#include <hosal_dma.h>
#include "bl602_dma.h"
#include "bl602_spi.h"
#include "bl602_gpio.h"
#include "bl602_glb.h"
#include "ramsync_low.h"
#include "bl_gpio.h"
#include "tpdbg.h"

#define TPSYNC_SLV_MISO_PIN 0
#define TPSYNC_SLV_MOSI_PIN 1
#define TPSYNC_SLV_CLK_PIN  3
#define TPSYNC_SLV_CS_PIN   2

static lramsync_spi_config_t _spi_cfg = {
    .port = 0,
    .spi_mode = 0,           /* 0: phase 0, polarity low */
    .spi_speed = 12000000,
    .miso = TPSYNC_SLV_MISO_PIN,
    .mosi = TPSYNC_SLV_MOSI_PIN,
    .clk = TPSYNC_SLV_CLK_PIN,
    .cs = TPSYNC_SLV_CS_PIN,
};

struct _ramsync_low_priv {
    DMA_LLI_Ctrl_Type *tx_lli;
    DMA_LLI_Ctrl_Type *rx_lli;
    gpio_ctx_t         gpio;
};

static ATTR_TCM_SECTION void _spi_dma_rx_irq(void *p_arg, uint32_t flag)
{
    lramsync_ctx_t *ctx = (lramsync_ctx_t *)p_arg;

    if (HOSAL_DMA_INT_TRANS_COMPLETE != flag) {
        blog_info("spi_dma_rx error\r\n");
        return;
    }

    if (ctx->rx_cb) {
        ctx->rx_cb(ctx->rx_arg);
    }
}

static ATTR_TCM_SECTION void _spi_dma_tx_irq(void *p_arg, uint32_t flag)
{
    lramsync_ctx_t *ctx = (lramsync_ctx_t *)p_arg;

    if (HOSAL_DMA_INT_TRANS_COMPLETE != flag) {
        blog_info("spi_dma_tx error\r\n");
        return;
    }

    if (ctx->tx_cb) {
        ctx->tx_cb(ctx->tx_arg);
    }
}

static void _gpio_irq(void *arg)
{
    gpio_ctx_t *pstnode = (gpio_ctx_t *)arg;
    lramsync_ctx_t *ctx = (lramsync_ctx_t *)pstnode->arg;
    bl_gpio_intmask(pstnode->gpioPin, 0);
    lramsync_reset(ctx);
    
    if (ctx->reset_signal_cb) {
        ctx->reset_signal_cb(ctx->reset_signal_arg);
    }
    blog_info("slave reset\r\n");
}

static void _spi_gpio_init(const lramsync_spi_config_t *config)
{
    GLB_GPIO_Type gpiopins[4];

    gpiopins[0] = config->cs;  //pin cs
    gpiopins[1] = config->clk;
    gpiopins[2] = config->mosi;
    gpiopins[3] = config->miso;

    GLB_GPIO_Func_Init(GPIO_FUN_SPI, gpiopins, sizeof(gpiopins)/sizeof(gpiopins[0]));
    GLB_Set_SPI_0_ACT_MOD_Sel(GLB_SPI_PAD_ACT_AS_SLAVE);
}

static int _spi_hw_init(const lramsync_spi_config_t *config)
{
    SPI_CFG_Type spicfg;
    SPI_FifoCfg_Type fifocfg;
    SPI_ID_Type spi_id; //TODO change SPI_ID_Type

    spi_id = config->port;

#if 0
     *2  --->  20 Mhz
     *5  --->  8  Mhz
     *6  --->  6.66 Mhz
     *10 --->  4 Mhz
     * */
    clk_div = (uint8_t)(40000000 / ctx->config->spi_speed);
    GLB_Set_SPI_CLK(ENABLE, 0);
    clockcfg.startLen = clk_div;
    clockcfg.stopLen = clk_div;
    clockcfg.dataPhase0Len = clk_div;
    clockcfg.dataPhase1Len = clk_div;
    clockcfg.intervalLen = clk_div;
    SPI_ClockConfig(spi_id, &clockcfg);
#endif

    /* Set SPI clock */
    GLB_Set_SPI_CLK(ENABLE, 0);

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

    SPI_Init(0, &spicfg);

    SPI_ClrTxFifo(spi_id);
    SPI_ClrRxFifo(spi_id);

    SPI_Disable(spi_id, SPI_WORK_MODE_SLAVE);

    SPI_IntMask(spi_id, SPI_INT_ALL, MASK);
    SPI_IntMask(spi_id, SPI_INT_FIFO_ERROR, UNMASK);

    /* fifo */
    fifocfg.txFifoThreshold = 1;
    fifocfg.rxFifoThreshold = 1;
    fifocfg.txFifoDmaEnable = ENABLE;
    fifocfg.rxFifoDmaEnable = ENABLE;
    SPI_FifoConfig(spi_id, &fifocfg);

    return 0;
}

static int _spi_lli_list_init(lramsync_ctx_t *ctx)
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
        dmactrl.SLargerD = 0;
        dmactrl.I = 1;
        dmactrl.SI = DMA_MINC_ENABLE;
        dmactrl.DI = DMA_MINC_DISABLE;

        dmactrl.TransferSize = ctx->node_tx[i].len / 4;
        priv->tx_lli[i].srcDmaAddr  = (uint32_t)ctx->node_tx[i].buf;
        priv->tx_lli[i].destDmaAddr = (uint32_t)(SPI_BASE + SPI_FIFO_WDATA_OFFSET);
        priv->tx_lli[i].dmaCtrl     = dmactrl;

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
        dmactrl.SLargerD = 0;
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

static int _spi_dma_init(lramsync_ctx_t *ctx)
{
    int ret;
    DMA_LLI_Cfg_Type txllicfg;
    DMA_LLI_Cfg_Type rxllicfg;
    struct _ramsync_low_priv *priv;

    if (ctx->dma_rx_chan == -1) {
        ctx->dma_rx_chan = hosal_dma_chan_request(0);
        if (ctx->dma_rx_chan < 0) {
            return -1;
        }
    }
    if (ctx->dma_tx_chan == -1) {
        ctx->dma_tx_chan = hosal_dma_chan_request(0);
        if (ctx->dma_tx_chan < 0) {
            return -1;
        }
    }

    txllicfg.dir = DMA_TRNS_M2P;
    txllicfg.srcPeriph = DMA_REQ_NONE;
    txllicfg.dstPeriph = DMA_REQ_SPI_TX;
    rxllicfg.dir = DMA_TRNS_P2M;
    rxllicfg.srcPeriph = DMA_REQ_SPI_RX;
    rxllicfg.dstPeriph = DMA_REQ_NONE;
    ret = _spi_lli_list_init(ctx);
    if (ret < 0) {
        blog_error("init lli failed. \r\n");
        return -1;
    }
  
    priv = (struct _ramsync_low_priv *)ctx->priv;
    DMA_LLI_Init(ctx->dma_rx_chan, &rxllicfg);
    DMA_LLI_Update(ctx->dma_rx_chan,(uint32_t)&priv->rx_lli[0]);
    hosal_dma_irq_callback_set(ctx->dma_rx_chan, _spi_dma_rx_irq, (void *)ctx);

    DMA_LLI_Init(ctx->dma_tx_chan, &txllicfg);
    DMA_LLI_Update(ctx->dma_tx_chan, (uint32_t)&priv->tx_lli[0]);
    hosal_dma_irq_callback_set(ctx->dma_tx_chan, _spi_dma_tx_irq, (void *)ctx);

    SPI_Enable(ctx->cfg->port, SPI_WORK_MODE_SLAVE);

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
    printf("lramsync_reset\r\n");
    GLB_AHB_Slave1_Reset(BL_AHB_SLAVE1_SPI);
    SPI_Disable(ctx->cfg->port, SPI_WORK_MODE_SLAVE);

    hosal_dma_chan_stop(ctx->dma_tx_chan);
    hosal_dma_chan_stop(ctx->dma_rx_chan);

    _spi_gpio_init(ctx->cfg);
    bl_gpio_enable_input(ctx->cfg->cs, 0, 1);

    _spi_hw_init(ctx->cfg);
    _spi_dma_init(ctx);

    SPI_ClrTxFifo(ctx->cfg->port);
    SPI_ClrRxFifo(ctx->cfg->port);

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

    SPI_Disable(ctx->cfg->port, SPI_WORK_MODE_SLAVE);

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
    struct _ramsync_low_priv *priv;

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

    lramsync_callback_register(ctx, tx_cb, tx_arg, rx_cb, rx_arg);

    ctx->priv = pvPortMalloc(sizeof(struct _ramsync_low_priv));
    if (ctx->priv == NULL) {
        return;
    }
    priv = (struct _ramsync_low_priv *)ctx->priv;

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

    _spi_gpio_init(ctx->cfg);
    bl_gpio_enable_input(ctx->cfg->cs, 0, 1);
    priv->gpio.gpioPin = ctx->cfg->cs;
    priv->gpio.gpio_handler = _gpio_irq;
    priv->gpio.intCtrlMod = GLB_GPIO_INT_CONTROL_SYNC;
    priv->gpio.intTrgMod = GLB_GPIO_INT_TRIG_POS_PULSE;
    priv->gpio.next = NULL;
    priv->gpio.arg = ctx;
    bl_gpio_enable_input(ctx->cfg->cs, 1, 0);
    bl_gpio_register(&priv->gpio);

    _spi_hw_init(ctx->cfg); 
    _spi_dma_init(ctx);

    tpdbg_log("lramsync_init slave\r\n");
}
