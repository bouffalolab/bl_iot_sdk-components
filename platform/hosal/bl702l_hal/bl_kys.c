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
#include "bl_kys.h"
#include "bl_irq.h"


#ifdef CFG_USE_ROM_CODE
extern void rom_bl_irq_enable(unsigned int source);
extern void rom_bl_irq_disable(unsigned int source);
extern void rom_bl_irq_register(int irqnum, void *handler);
#define bl_irq_enable              rom_bl_irq_enable
#define bl_irq_disable             rom_bl_irq_disable
#define bl_irq_register            rom_bl_irq_register
#endif


ATTR_TCM_SECTION
static void bl_kys_get_result(kys_result_t *result)
{
    uint8_t row_num;
    uint8_t col_num;
    uint8_t key_code;
    uint32_t tmpVal;
    int i;

    tmpVal = BL_RD_REG(KYS_BASE, KYS_KS_INT_STS);
    result->ghost_det = (tmpVal >> 12) & 0x01;
    result->fifo_full = (tmpVal >> 8) & 0x01;

    tmpVal = BL_RD_REG(KYS_BASE, KYS_KEYFIFO_IDX);
    result->key_num = (tmpVal >> 16) & 0x0F;

    tmpVal = BL_RD_REG(KYS_BASE, KYS_KS_CTRL);
    row_num = BL_GET_REG_BITS_VAL(tmpVal, KYS_ROW_NUM) + 1;
    col_num = BL_GET_REG_BITS_VAL(tmpVal, KYS_COL_NUM) + 1;

    for(i=0; i<result->key_num; i++){
        key_code = KYS_ReadKeyfifo() & 0x7F;
        result->key_code[i] = key_code;
        result->row_idx[i] = key_code % row_num;
        result->col_idx[i] = key_code / row_num;
    }

    KYS_Disable();

#if 1
    uint8_t state[8];
    uint8_t common;
    int c1, c2;

    // Patch for ghost key detection
    if(result->ghost_det == 0 && result->key_num >= 4){
        for(i=0; i<sizeof(state)/sizeof(state[0]); i++){
            state[i] = 0;
        }

        for(i=0; i<result->key_num; i++){
            state[result->col_idx[i]] |= 0x1 << result->row_idx[i];
        }

        for(c1=0; c1<col_num; c1++){
            if(!state[c1]){
                continue;
            }

            for(c2=c1+1; c2<col_num; c2++){
                common = state[c1] & state[c2];
                if(common & (common - 1)){
                    result->ghost_det = 1;
                }
            }
        }
    }
#endif
}

ATTR_TCM_SECTION
static void bl_kys_irq(void)
{
    kys_result_t result;

    bl_kys_get_result(&result);

    bl_kys_interrupt_callback(&result);
}


ATTR_TCM_SECTION
void bl_kys_gpio_init(uint8_t row_num, uint8_t col_num, uint8_t row_pins[], uint8_t col_pins[])
{
    uint32_t row_sel[2] = {0, 0};
    uint32_t col_sel[2] = {0, 0};
    GLB_GPIO_Cfg_Type gpioCfg;
    int i;

    for(i=0; i<row_num; i++){
        gpioCfg.gpioPin = row_pins[i];
        gpioCfg.gpioFun = GPIO_FUN_KEY_SCAN_ROW;
        gpioCfg.gpioMode = GPIO_MODE_AF;
        gpioCfg.pullType = GPIO_PULL_UP;
        gpioCfg.drive = 1;
        gpioCfg.smtCtrl = 1;
        GLB_GPIO_Init(&gpioCfg);

        row_sel[i/4] |= (uint32_t)row_pins[i] << (8 * (i%4));
    }

    for(i=row_num; i<8; i++){
        row_sel[i/4] |= 4 << (8 * (i%4));  // use GPIO4 as row padding
    }

    for(i=0; i<col_num; i++){
        gpioCfg.gpioPin = col_pins[i];
        gpioCfg.gpioFun = GPIO_FUN_KEY_SCAN_COL;
        gpioCfg.gpioMode = GPIO_MODE_AF;
        gpioCfg.pullType = GPIO_PULL_NONE;
        gpioCfg.drive = 1;
        gpioCfg.smtCtrl = 1;
        GLB_GPIO_Init(&gpioCfg);

        col_sel[i/4] |= (uint32_t)col_pins[i] << (8 * (i%4));
    }

    BL_WR_REG(PDS_BASE, PDS_KYS_ROW_I_GPIO_SEL0, row_sel[0] | (row_num << 29));
    BL_WR_REG(PDS_BASE, PDS_KYS_ROW_I_GPIO_SEL1, row_sel[1]);
    BL_WR_REG(PDS_BASE, PDS_KYS_COL_O_GPIO_SEL0, col_sel[0]);
    BL_WR_REG(PDS_BASE, PDS_KYS_COL_O_GPIO_SEL1, col_sel[1]);
}

ATTR_TCM_SECTION
void bl_kys_init(uint8_t row_num, uint8_t col_num, uint8_t row_pins[], uint8_t col_pins[])
{
    KYS_CFG_Type kysCfg = {
        col_num,        /* Col of keyboard,max:8 */
        row_num,        /* Row of keyboard,max:8 */
        0,              /* Idle duration between column scans */
        ENABLE,         /* Fifo mode */
        ENABLE,         /* Enable ghost key event detection */
        DISABLE,        /* Disable deglitch function */
        0,              /* Deglitch count */
        0,
        0,
    };

    // Run KYS at 1M (better no greater than 1M)
    GLB_PER_Clock_UnGate(GLB_AHB_CLOCK_KYS);
    GLB_Set_KYS_CLK(GLB_KYS_CLK_SRC_XCLK, 31);

    KYS_Disable();
    KYS_Init(&kysCfg);

    // Enable ks_done, fifo_full, ghost interrupt
    BL_WR_REG(KYS_BASE, KYS_KS_INT_EN, (0x1<<7)|(0x1<<8)|(0x1<<12));

    bl_kys_gpio_init(row_num, col_num, row_pins, col_pins);

    bl_irq_register(KYS_IRQn, bl_kys_irq);
}

ATTR_TCM_SECTION
void bl_kys_trigger_poll(kys_result_t *result)
{
    bl_irq_disable(KYS_IRQn);

    KYS_Enable();
    while(!KYS_GetIntStatus(KYS_INT_KS_DONE));

    bl_kys_get_result(result);
}

ATTR_TCM_SECTION
void bl_kys_trigger_interrupt(void)
{
    bl_irq_enable(KYS_IRQn);

    KYS_Enable();
}

ATTR_TCM_SECTION
void bl_kys_abort(void)
{
    KYS_Disable();
}

ATTR_TCM_SECTION
__attribute__((weak)) void bl_kys_interrupt_callback(const kys_result_t *result)
{
    
}
