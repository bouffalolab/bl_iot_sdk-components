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
#include "bl_ir.h"
#include "bl_irq.h"
#include "hosal_dma.h"


static int8_t ir_tx_dma_ch = -1;
static uint32_t ir_tx_dma_data[32];


static void bl_ir_irq(void)
{
    IR_Disable(IR_TX);
    IR_IntMask(IR_INT_TX_END, MASK);
    IR_ClrIntStatus(IR_INT_TX_END);

    bl_ir_swm_tx_done_callback();
}

static void bl_ir_tx_interrupt_cfg(void)
{
    IR_FifoCfg_Type irFifoCfg = {
        .txFifoThreshold = 1,
        .txFifoDmaEnable = ENABLE,
    };

    IR_FifoConfig(&irFifoCfg);

    if(ir_tx_dma_ch < 0){
        hosal_dma_init();
        ir_tx_dma_ch = hosal_dma_chan_request(0);
    }

    DMA_Channel_Cfg_Type dmaChCfg = {
        0,                         /* Source address of DMA transfer */
        0x4000A688,                /* Destination address of DMA transfer */
        0,                         /* Transfer length, 0~4095, this is burst count */
        DMA_TRNS_M2P,              /* Transfer dir control. 0: Memory to Memory, 1: Memory to peripheral, 2: Peripheral to memory */
        ir_tx_dma_ch,              /* Channel select 0-7 */
        DMA_TRNS_WIDTH_32BITS,     /* Transfer width. 0: 8  bits, 1: 16  bits, 2: 32  bits */
        DMA_TRNS_WIDTH_32BITS,     /* Transfer width. 0: 8  bits, 1: 16  bits, 2: 32  bits */
        DMA_BURST_SIZE_1,          /* Number of data items for burst transaction length. Each item width is as same as tansfer width. 0: 1 item, 1: 4 items, 2: 8 items, 3: 16 items */
        DMA_BURST_SIZE_1,          /* Number of data items for burst transaction length. Each item width is as same as tansfer width. 0: 1 item, 1: 4 items, 2: 8 items, 3: 16 items */
        DISABLE,
        DISABLE,
        0,
        DMA_MINC_ENABLE,           /* Source address increment. 0: No change, 1: Increment */
        DMA_PINC_DISABLE,          /* Destination address increment. 0: No change, 1: Increment */
        DMA_REQ_NONE,              /* Source peripheral select */
        DMA_REQ_IR_TX,             /* Destination peripheral select */
    };

    GLB_PER_Clock_UnGate(GLB_AHB_CLOCK_DMA_0);
    DMA_Enable(DMA0_ID);
    bl_irq_enable(DMA_ALL_IRQn);

    DMA_Channel_Init(DMA0_ID, &dmaChCfg);

    bl_irq_register(IRTX_IRQn, bl_ir_irq);
    bl_irq_enable(IRTX_IRQn);
}


void bl_ir_led_drv_cfg(uint8_t led0_en, uint8_t led1_en)
{
    GLB_GPIO_Type pin;

    if(led0_en){
        pin = 22;
        GLB_GPIO_Func_Init(GPIO_FUN_ANALOG, &pin, 1);
        GLB_IR_LED_Driver_Output_Enable(GLB_IR_LED0);
    }else{
        GLB_IR_LED_Driver_Output_Disable(GLB_IR_LED0);
    }

    if(led1_en){
        pin = 18;
        GLB_GPIO_Func_Init(GPIO_FUN_ANALOG, &pin, 1);
        GLB_IR_LED_Driver_Output_Enable(GLB_IR_LED1);
    }else{
        GLB_IR_LED_Driver_Output_Disable(GLB_IR_LED1);
    }

    if(led0_en || led1_en){
        GLB_IR_LED_Driver_Enable();
    }else{
        GLB_IR_LED_Driver_Disable();
    }
}

void bl_ir_custom_tx_cfg(IR_TxCfg_Type *txCfg, IR_TxPulseWidthCfg_Type *txPWCfg)
{
    /* Run IR at 2M */
    GLB_PER_Clock_UnGate(GLB_AHB_CLOCK_IR);
    GLB_Set_IR_CLK(ENABLE, GLB_IR_CLK_SRC_XCLK, 15);

    IR_Disable(IR_TX);
    IR_TxSWM(DISABLE);
    IR_TxInit(txCfg);
    IR_TxPulseWidthConfig(txPWCfg);
}

void bl_ir_nec_tx_cfg(void)
{
    IR_TxCfg_Type txCfg = {
        32,              /* 32-bit data */
        DISABLE,         /* Disable signal of tail pulse inverse */
        ENABLE,          /* Enable signal of tail pulse */
        DISABLE,         /* Disable signal of head pulse inverse */
        ENABLE,          /* Enable signal of head pulse */
        DISABLE,         /* Disable signal of logic 1 pulse inverse */
        DISABLE,         /* Disable signal of logic 0 pulse inverse */
        ENABLE,          /* Enable signal of data pulse */
        ENABLE,          /* Enable signal of output modulation */
        DISABLE,         /* Disable signal of output inverse */
        DISABLE,         /* Disable tx freerun mode */
        DISABLE,         /* Disable tx freerun continuous mode */
        IR_FRAME_SIZE_32 /* Frame size */
    };

    IR_TxPulseWidthCfg_Type txPWCfg = {
        1,   /* Pulse width of logic 0 pulse phase 1, 562.5us @2MHz source clock*/
        1,   /* Pulse width of logic 0 pulse phase 0 */
        3,   /* Pulse width of logic 1 pulse phase 1, 1687.5us */
        1,   /* Pulse width of logic 1 pulse phase 0 */
        8,   /* Pulse width of head pulse phase 1, 4.5ms */
        16,  /* Pulse width of head pulse phase 0, 9ms */
        1,   /* Pulse width of tail pulse phase 1 */
        1,   /* Pulse width of tail pulse phase 0 */
        35,  /* Modulation phase 1 width, 37.7kHz, duty=1/3 */
        18,  /* Modulation phase 0 width, 37.7kHz, duty=1/3 */
        1125 /* Pulse width unit */
    };

    bl_ir_custom_tx_cfg(&txCfg, &txPWCfg);
}

void bl_ir_rc5_tx_cfg(void)
{
    IR_TxCfg_Type txCfg = {
        13,              /* 13-bit data, head pulse as the first start bit */
        DISABLE,         /* Disable signal of tail pulse inverse */
        DISABLE,         /* Disable signal of tail pulse */
        ENABLE,          /* Enable signal of head pulse inverse */
        ENABLE,          /* Enable signal of head pulse */
        ENABLE,          /* Enable signal of logic 1 pulse inverse */
        DISABLE,         /* Disable signal of logic 0 pulse inverse */
        ENABLE,          /* Enable signal of data pulse */
        ENABLE,          /* Enable signal of output modulation */
        DISABLE,         /* Disable signal of output inverse */
        DISABLE,         /* Disable tx freerun mode */
        DISABLE,         /* Disable tx freerun continuous mode */
        IR_FRAME_SIZE_32 /* Frame size */
    };

    IR_TxPulseWidthCfg_Type txPWCfg = {
        1,   /* Pulse width of logic 0 pulse phase 1, 889us @2MHz source clock*/
        1,   /* Pulse width of logic 0 pulse phase 0 */
        1,   /* Pulse width of logic 1 pulse phase 1 */
        1,   /* Pulse width of logic 1 pulse phase 0 */
        1,   /* Pulse width of head pulse phase 1 */
        1,   /* Pulse width of head pulse phase 0 */
        1,   /* Pulse width of tail pulse phase 1 */
        1,   /* Pulse width of tail pulse phase 0 */
        35,  /* Modulation phase 1 width, 37.7kHz, duty=1/3 */
        18,  /* Modulation phase 0 width, 37.7kHz, duty=1/3 */
        1778 /* Pulse width unit */
    };

    bl_ir_custom_tx_cfg(&txCfg, &txPWCfg);
}

void bl_ir_swm_tx_cfg(float freq_hz, float duty_cycle)
{
    uint16_t pw_unit = (uint16_t)(2000000 / freq_hz);
    uint8_t mod_width_0 = (uint8_t)(pw_unit * duty_cycle + 0.5);
    uint8_t mod_width_1 = pw_unit - mod_width_0;

    IR_TxCfg_Type txCfg = {
        1,                                                   /* Number of pulse */
        DISABLE,                                             /* Don't care when SWM is enabled */
        DISABLE,                                             /* Don't care when SWM is enabled */
        DISABLE,                                             /* Don't care when SWM is enabled */
        DISABLE,                                             /* Don't care when SWM is enabled */
        DISABLE,                                             /* Don't care when SWM is enabled */
        DISABLE,                                             /* Don't care when SWM is enabled */
        DISABLE,                                             /* Don't care when SWM is enabled */
        ENABLE,                                              /* Enable signal of output modulation */
        DISABLE,                                             /* Disable signal of output inverse */
        DISABLE,                                             /* Disable tx freerun mode */
        DISABLE,                                             /* Disable tx freerun continuous mode */
        IR_FRAME_SIZE_32                                     /* Frame size */
    };

    IR_TxPulseWidthCfg_Type txPWCfg = {
        0,                                                   /* Don't care when SWM is enabled */
        0,                                                   /* Don't care when SWM is enabled */
        0,                                                   /* Don't care when SWM is enabled */
        0,                                                   /* Don't care when SWM is enabled */
        0,                                                   /* Don't care when SWM is enabled */
        0,                                                   /* Don't care when SWM is enabled */
        0,                                                   /* Don't care when SWM is enabled */
        0,                                                   /* Don't care when SWM is enabled */
        mod_width_1,                                         /* Modulation phase 1 width */
        mod_width_0,                                         /* Modulation phase 0 width */
        pw_unit                                              /* Pulse width unit */
    };

    bl_ir_custom_tx_cfg(&txCfg, &txPWCfg);
    bl_ir_tx_interrupt_cfg();
}

void bl_ir_nec_tx(uint32_t wdata)
{
    IR_SendCommand(&wdata, 1);
}

void bl_ir_rc5_tx(uint32_t wdata)
{
    IR_SendCommand(&wdata, 1);
}

int bl_ir_swm_tx(uint16_t data[], uint8_t len)
{
    uint32_t tmpVal;
    uint32_t pwVal;
    int i, j;

    tmpVal = BL_RD_REG(IR_BASE, IRTX_CONFIG);
    if(BL_GET_REG_BITS_VAL(tmpVal, IR_CR_IRTX_EN)){
        return -1;
    }

    tmpVal = BL_RD_REG(IR_BASE, IRTX_CONFIG);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, IR_CR_IRTX_DATA_NUM, len - 1);
    BL_WR_REG(IR_BASE, IRTX_CONFIG, tmpVal);

    pwVal = 0;
    for(i=0; i<len; i++){
        j = i % 4;
        pwVal |= ((data[i] - 1) & 0xFF) << 8 * j;
        if(i == len - 1 || j == 3){
            ir_tx_dma_data[i / 4] = pwVal;
            pwVal = 0;
        }
    }

    DMA_Channel_Update_SrcMemcfg(DMA0_ID, ir_tx_dma_ch, (uint32_t)ir_tx_dma_data, (len + 3) / 4);
    DMA_Channel_Enable(DMA0_ID, ir_tx_dma_ch);

    IR_IntMask(IR_INT_TX_END, UNMASK);
    IR_TxSWM(ENABLE);
    IR_Enable(IR_TX);

    return 0;
}

int bl_ir_swm_tx_busy(void)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(IR_BASE, IRTX_CONFIG);
    return BL_GET_REG_BITS_VAL(tmpVal, IR_CR_IRTX_EN);
}

__attribute__((weak)) void bl_ir_swm_tx_done_callback(void)
{
    
}
