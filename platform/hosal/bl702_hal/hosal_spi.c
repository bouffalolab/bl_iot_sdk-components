#include <string.h>
#include <stdio.h>

#include <bl702_spi.h>
#include <bl702_gpio.h>
#include <bl702_glb.h>
#include <bl702_dma.h>
#include <bl702.h>
#include <bl_irq.h>
#include <bl_gpio.h>
#include <hosal_dma.h>
#include <hosal_spi.h>

#include <FreeRTOS.h>
#include <semphr.h>
#include <event_groups.h>

#include <blog.h>

#define LLI_BUFF_SIZE       2048

#define EVT_GROUP_SPI_TX    (1<<0)
#define EVT_GROUP_SPI_RX    (1<<1)
#define EVT_GROUP_SPI_TR    (EVT_GROUP_SPI_TX | EVT_GROUP_SPI_RX)

static void hosal_spi_int_handler_tx(void *arg, uint32_t flag);
static void hosal_spi_int_handler_rx(void *arg, uint32_t flag);
static void spi_irq_process(hosal_spi_dev_t *spi);

typedef struct {
    uint8_t *tx_data;
    uint8_t *rx_data;
    uint32_t length;
    uint32_t tx_index;
    uint32_t rx_index;
    SemaphoreHandle_t spi_semphr;  // much less latency then event group
} spi_priv_t;

typedef struct {
    int8_t tx_dma_ch;
    int8_t rx_dma_ch;
    EventGroupHandle_t spi_event_group;
} spi_dma_priv_t;

static void *hosal_spi_priv = NULL;
static uint8_t hosal_spi_cs_init = 0;

static void hosal_spi_basic_init(hosal_spi_dev_t *spi)
{
    SPI_CFG_Type spicfg;
    SPI_FifoCfg_Type fifocfg;
    SPI_ID_Type spi_id = SPI_ID_0; //spi->port;

    spicfg.deglitchEnable = DISABLE;
    spicfg.continuousEnable = ENABLE;
    spicfg.byteSequence = SPI_BYTE_INVERSE_BYTE0_FIRST;
    spicfg.bitSequence = SPI_BIT_INVERSE_MSB_FIRST;
    spicfg.frameSize = SPI_FRAME_SIZE_8;

    if (spi->config.polar_phase == 0) {
        spicfg.clkPhaseInv = SPI_CLK_PHASE_INVERSE_0;
        spicfg.clkPolarity = SPI_CLK_POLARITY_LOW;
    } else if (spi->config.polar_phase == 1) {
        spicfg.clkPhaseInv = SPI_CLK_PHASE_INVERSE_1;
        spicfg.clkPolarity = SPI_CLK_POLARITY_LOW;
    } else if (spi->config.polar_phase == 2) {
        spicfg.clkPhaseInv = SPI_CLK_PHASE_INVERSE_0;
        spicfg.clkPolarity = SPI_CLK_POLARITY_HIGH;
    } else {
        spicfg.clkPhaseInv = SPI_CLK_PHASE_INVERSE_1;
        spicfg.clkPolarity = SPI_CLK_POLARITY_HIGH;
    }

    SPI_Init(spi_id, &spicfg);
    SPI_SetClock(spi_id, spi->config.freq);
    SPI_IntMask(spi_id, SPI_INT_ALL, MASK);
    SPI_RxIgnoreDisable(spi_id);

    fifocfg.txFifoThreshold = 0;
    fifocfg.rxFifoThreshold = 0;
    if (spi->config.dma_enable) {
        fifocfg.txFifoDmaEnable = ENABLE;
        fifocfg.rxFifoDmaEnable = ENABLE;
        SPI_FifoConfig(spi_id, &fifocfg);
    } else {
        fifocfg.txFifoDmaEnable = DISABLE;
        fifocfg.rxFifoDmaEnable = DISABLE;
        SPI_FifoConfig(spi_id, &fifocfg);
    }
}

static void lli_list_init(DMA_LLI_Ctrl_Type **pptxlli, DMA_LLI_Ctrl_Type **pprxlli, uint8_t *ptx_data, uint8_t *prx_data, uint32_t length)
{
    uint32_t i;
    uint32_t count;
    uint32_t remainder;
    struct DMA_Control_Reg dmactrl = {
        .SBSize = DMA_BURST_SIZE_1,
        .DBSize = DMA_BURST_SIZE_1,
        .SWidth = DMA_TRNS_WIDTH_8BITS,
        .DWidth = DMA_TRNS_WIDTH_8BITS,
    };

    count = length / LLI_BUFF_SIZE;
    remainder = length % LLI_BUFF_SIZE;
    if (remainder != 0) {
        count = count + 1;
    }

    if (ptx_data) {
        *pptxlli = malloc(sizeof(DMA_LLI_Ctrl_Type) * count);
    }

    if (prx_data) {
        *pprxlli = malloc(sizeof(DMA_LLI_Ctrl_Type) * count);
    }

    for (i = 0; i < count; i++) {
        if (remainder == 0) {
            dmactrl.TransferSize = LLI_BUFF_SIZE;
        } else {
            if (i == count - 1) {
                dmactrl.TransferSize = remainder;
            } else {
                dmactrl.TransferSize = LLI_BUFF_SIZE;
            }
        }

        if (i == count - 1) {
            dmactrl.I = 1;
        } else {
            dmactrl.I = 0;
        }

        if (ptx_data) {
            dmactrl.SI = DMA_MINC_ENABLE;
            dmactrl.DI = DMA_MINC_DISABLE;
            (*pptxlli)[i].srcDmaAddr = (uint32_t)(ptx_data + i * LLI_BUFF_SIZE);
            (*pptxlli)[i].destDmaAddr = (uint32_t)(SPI_BASE + SPI_FIFO_WDATA_OFFSET);
            (*pptxlli)[i].dmaCtrl = dmactrl;
            if (i != 0) {
                (*pptxlli)[i-1].nextLLI = (uint32_t)&(*pptxlli)[i];
            }
            (*pptxlli)[i].nextLLI = 0;
        }

        if (prx_data) {
            dmactrl.SI = DMA_MINC_DISABLE;
            dmactrl.DI = DMA_MINC_ENABLE;
            (*pprxlli)[i].srcDmaAddr = (uint32_t)(SPI_BASE + SPI_FIFO_RDATA_OFFSET);
            (*pprxlli)[i].destDmaAddr = (uint32_t)(prx_data + i * LLI_BUFF_SIZE);
            (*pprxlli)[i].dmaCtrl = dmactrl;
            if (i != 0) {
                (*pprxlli)[i-1].nextLLI = (uint32_t)&(*pprxlli)[i];
            }
            (*pprxlli)[i].nextLLI = 0;
        }
    }
}

static int hosal_spi_dma_trans(hosal_spi_dev_t *spi, uint8_t *TxData, uint8_t *RxData, uint32_t Len, uint32_t timeout)
{
    EventBits_t uxBits;
    DMA_LLI_Cfg_Type txllicfg;
    DMA_LLI_Cfg_Type rxllicfg;
    DMA_LLI_Ctrl_Type *ptxlli = NULL;
    DMA_LLI_Ctrl_Type *prxlli = NULL;
    spi_dma_priv_t *dma_arg = (spi_dma_priv_t *)hosal_spi_priv;
    SPI_ID_Type spi_id = SPI_ID_0; //spi->port;

    if (TxData) {
        dma_arg->tx_dma_ch = hosal_dma_chan_request(0);
        if (dma_arg->tx_dma_ch == -1) {
            blog_error("SPI TX DMA CHANNEL get failed!\r\n");
            return -1;
        }
    }

    if (RxData) {
        dma_arg->rx_dma_ch = hosal_dma_chan_request(0);
        if (dma_arg->rx_dma_ch == -1) {
            if (dma_arg->tx_dma_ch >= 0) {
                hosal_dma_chan_release(dma_arg->tx_dma_ch);
                dma_arg->tx_dma_ch = -1;
            }
            blog_error("SPI RX DMA CHANNEL get failed!\r\n");
            return -1;
        }
    }

    txllicfg.dir = DMA_TRNS_M2P;
    txllicfg.srcPeriph = DMA_REQ_NONE; 
    txllicfg.dstPeriph = DMA_REQ_SPI_TX;

    rxllicfg.dir = DMA_TRNS_P2M;
    rxllicfg.srcPeriph = DMA_REQ_SPI_RX;
    rxllicfg.dstPeriph = DMA_REQ_NONE;

    if (spi->config.mode == 0) {
        SPI_Enable(spi_id, SPI_WORK_MODE_MASTER);
    } else {
        SPI_Enable(spi_id, SPI_WORK_MODE_SLAVE);
    }

    lli_list_init(&ptxlli, &prxlli, TxData, RxData, Len);

    if (RxData) {
        hosal_dma_irq_callback_set(dma_arg->rx_dma_ch, hosal_spi_int_handler_rx, spi);
        DMA_LLI_Init(dma_arg->rx_dma_ch, &rxllicfg);
        DMA_LLI_Update(dma_arg->rx_dma_ch,(uint32_t)prxlli);
        hosal_dma_chan_start(dma_arg->rx_dma_ch);
    }

    if (TxData) {
        hosal_dma_irq_callback_set(dma_arg->tx_dma_ch, hosal_spi_int_handler_tx, spi);
        DMA_LLI_Init(dma_arg->tx_dma_ch, &txllicfg);
        DMA_LLI_Update(dma_arg->tx_dma_ch,(uint32_t)ptxlli);
        hosal_dma_chan_start(dma_arg->tx_dma_ch);
    }

    uxBits = xEventGroupWaitBits(dma_arg->spi_event_group, EVT_GROUP_SPI_TR, pdTRUE, pdTRUE, timeout);
    xEventGroupClearBits(dma_arg->spi_event_group, EVT_GROUP_SPI_TR);

    if (dma_arg->tx_dma_ch >= 0) {
        hosal_dma_chan_stop(dma_arg->tx_dma_ch);
        hosal_dma_chan_release(dma_arg->tx_dma_ch);
        dma_arg->tx_dma_ch = -1;
    }
    if (dma_arg->rx_dma_ch >= 0) {
        hosal_dma_chan_stop(dma_arg->rx_dma_ch);
        hosal_dma_chan_release(dma_arg->rx_dma_ch);
        dma_arg->rx_dma_ch = -1;
    }

    if (ptxlli) {
        free(ptxlli);
    }
    if (prxlli) {
        free(prxlli);
    }

    if ((uxBits & EVT_GROUP_SPI_TR) == EVT_GROUP_SPI_TR) {
        if (spi->cb) {
            spi->cb(spi->p_arg);
        }
    } else {
        blog_error("transmission timeout\r\n");
        return -1;
    }

	return 0;
}

static void hosal_spi_int_handler_tx(void *arg, uint32_t flag)
{
    BaseType_t xResult = pdFAIL;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    spi_dma_priv_t *priv = (spi_dma_priv_t *)hosal_spi_priv;

    if (priv->rx_dma_ch == -1) {
        xResult = xEventGroupSetBitsFromISR(priv->spi_event_group, EVT_GROUP_SPI_TR, &xHigherPriorityTaskWoken);
    } else {
        xResult = xEventGroupSetBitsFromISR(priv->spi_event_group, EVT_GROUP_SPI_TX, &xHigherPriorityTaskWoken);
    }
    if(xResult != pdFAIL) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

static void hosal_spi_int_handler_rx(void *arg, uint32_t flag)
{
    BaseType_t xResult = pdFAIL;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    spi_dma_priv_t *priv = (spi_dma_priv_t *)hosal_spi_priv;

    if (priv->tx_dma_ch == -1) {
        xResult = xEventGroupSetBitsFromISR(priv->spi_event_group, EVT_GROUP_SPI_TR, &xHigherPriorityTaskWoken);
    } else {
        xResult = xEventGroupSetBitsFromISR(priv->spi_event_group, EVT_GROUP_SPI_RX, &xHigherPriorityTaskWoken);
    }
    if(xResult != pdFAIL) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

static void spi_irq_process(hosal_spi_dev_t *spi)
{
    BaseType_t xResult = pdFAIL;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    spi_priv_t *spi_priv = (spi_priv_t *)hosal_spi_priv;
    SPI_ID_Type spi_id = SPI_ID_0; //spi->port;

    /* TX fifo ready interrupt(fifo count > fifo threshold) */
    if (SPI_GetIntStatus(spi_id, SPI_INT_TX_FIFO_REQ) == SET) {
        SPI_ClrIntStatus(spi_id, SPI_INT_TX_FIFO_REQ);
        if (spi_priv->tx_index < spi_priv->length) {
            if (spi_priv->tx_data) {
                BL_WR_REG(SPI_BASE, SPI_FIFO_WDATA, (uint32_t)spi_priv->tx_data[spi_priv->tx_index]);
            } else {
                BL_WR_REG(SPI_BASE, SPI_FIFO_WDATA, 0);
            }
            spi_priv->tx_index++;
        }
    }

    /* RX fifo ready interrupt(fifo count > fifo threshold) */
    if (SPI_GetIntStatus(spi_id, SPI_INT_RX_FIFO_REQ) == SET) {
        SPI_ClrIntStatus(spi_id, SPI_INT_RX_FIFO_REQ);
        if (spi_priv->rx_data) {
            spi_priv->rx_data[spi_priv->rx_index] = BL_RD_REG(SPI_BASE, SPI_FIFO_RDATA) & 0xff;
        }
        spi_priv->rx_index++;
        if (spi_priv->rx_index == spi_priv->length) {
            bl_irq_disable(SPI_IRQn);

            xResult = xSemaphoreGiveFromISR(spi_priv->spi_semphr, &xHigherPriorityTaskWoken);
            if(xResult != pdFAIL) {
                portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            }
        }
    }
}

static int hosal_spi_trans(hosal_spi_dev_t *spi, uint8_t *tx_data, uint8_t *rx_data, uint32_t length, uint32_t timeout)
{
    spi_priv_t *spi_priv = (spi_priv_t *)hosal_spi_priv;
    SPI_ID_Type spi_id = SPI_ID_0; //spi->port;

    spi_priv->tx_data = tx_data;
    spi_priv->rx_data = rx_data;
    spi_priv->length  = length;
    spi_priv->tx_index  = 0;
    spi_priv->rx_index  = 0;

    SPI_ClrTxFifo(spi_id);
    SPI_ClrRxFifo(spi_id);

    if (spi->config.mode == 0) {
        SPI_Enable(spi_id, SPI_WORK_MODE_MASTER);
    } else {
        SPI_Enable(spi_id, SPI_WORK_MODE_SLAVE);
    }

    SPI_IntMask(spi_id, SPI_INT_TX_FIFO_REQ, UNMASK);
    SPI_IntMask(spi_id, SPI_INT_RX_FIFO_REQ, UNMASK);
    bl_irq_register_with_ctx(SPI_IRQn, spi_irq_process, spi);
    bl_irq_enable(SPI_IRQn);

    if (xSemaphoreTake(spi_priv->spi_semphr, timeout)) {
        if (spi->cb) {
            spi->cb(spi->p_arg);
        }
    } else {
        blog_error("transmission timeout\r\n");
        return -1;
    }

    return 0;
}

static void hosal_spi_gpio_init(hosal_spi_dev_t *spi)
{
    GLB_GPIO_Type gpiopins[4];
    gpiopins[0] = spi->config.pin_cs;
    gpiopins[1] = spi->config.pin_clk;
    gpiopins[2] = spi->config.pin_mosi;
    gpiopins[3] = spi->config.pin_miso;
    GLB_GPIO_Func_Init(GPIO_FUN_SPI, gpiopins, sizeof(gpiopins)/sizeof(gpiopins[0]));

    if (spi->config.mode == 0) {
        GLB_Set_SPI_0_ACT_MOD_Sel(GLB_SPI_PAD_ACT_AS_MASTER);
    } else {
        GLB_Set_SPI_0_ACT_MOD_Sel(GLB_SPI_PAD_ACT_AS_SLAVE);
    }
}

int hosal_spi_init(hosal_spi_dev_t *spi)
{
    if (spi == NULL) {
        return -1;
    }

    if (hosal_spi_priv == NULL) {
        if (spi->config.dma_enable) {
            spi_dma_priv_t *priv = malloc(sizeof(spi_dma_priv_t));
            priv->tx_dma_ch = -1;
            priv->rx_dma_ch = -1;
            priv->spi_event_group = xEventGroupCreate();
            hosal_spi_priv = priv;
        } else {
            spi_priv_t *priv = malloc(sizeof(spi_priv_t));
            priv->spi_semphr = xSemaphoreCreateBinary();
            hosal_spi_priv = priv;
        }
    }

    hosal_spi_cs_init = 0;

    hosal_spi_gpio_init(spi);
    hosal_spi_basic_init(spi);
    return 0;
}

int hosal_spi_set_cs(uint8_t pin, uint8_t value)
{
    if (hosal_spi_cs_init == 0) {
        bl_gpio_enable_output(pin, 0, 0);
        hosal_spi_cs_init = 1;
    }

    bl_gpio_output_set(pin, value);
    return 0;
}

int hosal_spi_irq_callback_set(hosal_spi_dev_t *spi, hosal_spi_irq_t pfn, void *p_arg)
{
    if (spi == NULL) {
        return -1;
    }

    spi->cb = pfn;
    spi->p_arg = p_arg;
    return 0;
}

int hosal_spi_finalize(hosal_spi_dev_t *spi)
{
    if (spi == NULL) {
        return -1;
    }

    if (hosal_spi_priv != NULL) {
        if (spi->config.dma_enable) {
            spi_dma_priv_t *priv = (spi_dma_priv_t *)hosal_spi_priv;
            vEventGroupDelete(priv->spi_event_group);
            free(priv);
        } else {
            spi_priv_t *priv = (spi_priv_t *)hosal_spi_priv;
            vSemaphoreDelete(priv->spi_semphr);
            free(priv);
        }
        hosal_spi_priv = NULL;
    }

    hosal_spi_cs_init = 0;

    SPI_ID_Type spi_id = SPI_ID_0; //spi->port;
    SPI_DeInit(spi_id);
    return 0;
}

int hosal_spi_send(hosal_spi_dev_t *spi, const uint8_t *data, uint16_t size, uint32_t timeout)
{
    int ret;

    if (spi == NULL || data == NULL) {
        blog_error("wrong para\r\n");
        return -1;
    }

    if (hosal_spi_priv == NULL) {
        blog_error("please hosal_spi_init!\r\n");
        return -1;
    }

    if (spi->config.dma_enable) {
        ret = hosal_spi_dma_trans(spi, (uint8_t *)data, NULL, size, timeout);
    } else {
        ret = hosal_spi_trans(spi, (uint8_t *)data, NULL, size, timeout);
    }
    return ret;
}

int hosal_spi_recv(hosal_spi_dev_t *spi, uint8_t *data, uint16_t size, uint32_t timeout)
{
    int ret;

    if (spi == NULL || data == NULL) {
        blog_error("wrong para\r\n");
        return -1;
    }

    if (hosal_spi_priv == NULL) {
        blog_error("please hosal_spi_init!\r\n");
        return -1;
    }

    if (spi->config.dma_enable) {
        ret = hosal_spi_dma_trans(spi, NULL, data, size, timeout);
    } else {
        ret = hosal_spi_trans(spi, NULL, data, size, timeout);
    }
    return ret;
}

int hosal_spi_send_recv(hosal_spi_dev_t *spi, uint8_t *tx_data, uint8_t *rx_data, uint16_t size, uint32_t timeout)
{
    int ret;

    if (spi == NULL || tx_data == NULL || rx_data == NULL) {
        blog_error("wrong para\r\n");
        return -1;
    }

    if (hosal_spi_priv == NULL) {
        blog_error("please hosal_spi_init!\r\n");
        return -1;
    }

    if (spi->config.dma_enable) {
        ret = hosal_spi_dma_trans(spi, tx_data, rx_data, size, timeout);
    } else {
        ret = hosal_spi_trans(spi, tx_data, rx_data, size, timeout);
    }
    return ret;
}
