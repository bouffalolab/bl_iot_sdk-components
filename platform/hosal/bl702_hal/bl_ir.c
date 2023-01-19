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


void bl_ir_led_drv_cfg(uint8_t led0_en, uint8_t led1_en)
{
    GLB_GPIO_Type pin;

    if(led0_en){
        pin = 22;
        GLB_GPIO_Func_Init(GPIO_FUN_ANALOG, &pin, 1);
        GLB_IR_LED_Driver_Output_Enable(pin);
    }else{
        pin = 22;
        GLB_IR_LED_Driver_Output_Disable(pin);
    }

    if(led1_en){
        pin = 23;
        GLB_GPIO_Func_Init(GPIO_FUN_ANALOG, &pin, 1);
        GLB_IR_LED_Driver_Output_Enable(pin);
    }else{
        pin = 23;
        GLB_IR_LED_Driver_Output_Disable(pin);
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

void bl_ir_swm_tx_cfg(void)
{
    IR_TxCfg_Type txCfg = {
        22,                                                  /* Send 22 tx fifo data */
        DISABLE,                                             /* Don't care when SWM is enabled */
        DISABLE,                                             /* Don't care when SWM is enabled */
        DISABLE,                                             /* Don't care when SWM is enabled */
        DISABLE,                                             /* Don't care when SWM is enabled */
        DISABLE,                                             /* Don't care when SWM is enabled */
        DISABLE,                                             /* Don't care when SWM is enabled */
        DISABLE,                                             /* Don't care when SWM is enabled */
        ENABLE,                                              /* Enable signal of output modulation */
        DISABLE,                                             /* Disable signal of output inverse */
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
        35,                                                  /* Modulation phase 1 width, 37.7kHz, duty=1/3 */
        18,                                                  /* Modulation phase 0 width, 37.7kHz, duty=1/3 */
        1778                                                 /* Pulse width unit */
    };

    bl_ir_custom_tx_cfg(&txCfg, &txPWCfg);
}

void bl_ir_nec_tx(uint32_t wdata)
{
    IR_TxSWM(DISABLE);
    IR_SendCommand(0, wdata);
}

void bl_ir_rc5_tx(uint32_t wdata)
{
    IR_TxSWM(DISABLE);
    IR_SendCommand(0, wdata);
}

void bl_ir_swm_tx(uint16_t *data, uint8_t len)
{
    IR_TxSWM(ENABLE);
    IR_SWMSendCommand(data, len);
}
