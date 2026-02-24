/*
 * Copyright (c) 2016-2026 Bouffalolab.
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
#include "bl_gpio_ir.h"
#include "bl_gpio.h"
#include "bl_irq.h"
#include "bl702l_glb.h"


static uint8_t ir_tx_ok = 0;
static uint8_t ir_tx_pin = 0;


int bl_gpio_ir_tx_init(uint8_t tx_pin)
{
    if(tx_pin > 31){
        ir_tx_ok = 0;
        return -1;
    }
    
    ir_tx_ok = 1;
    ir_tx_pin = tx_pin;
    
    // output high by default
    bl_gpio_output_set(tx_pin, 1);
    bl_gpio_enable_output(tx_pin, 0, 0);
    
    return 0;
}

//run this function in ram to avoid execution speed variation due to cache
//ATTR_TCM_SECTION
static __attribute__((noinline)) void bl_gpio_ir_tx_do(uint16_t time[], uint32_t len, uint32_t gpio_output[])
{
    uint32_t tick_low;
    uint32_t i;
    
    tick_low = *(volatile uint32_t *)0x0200BFF8;
    while(*(volatile uint32_t *)0x0200BFF8 == tick_low);
    tick_low += 1;
    
    for(i=0; i<len; i++){
        BL_WR_WORD(GLB_BASE + GLB_GPIO_OUTPUT_OFFSET, gpio_output[i % 2]);
        
        while(*(volatile uint32_t *)0x0200BFF8 < tick_low + time[i] * 2);
        tick_low += time[i] * 2;
    }
    
    // output high by default
    BL_WR_WORD(GLB_BASE + GLB_GPIO_OUTPUT_OFFSET, gpio_output[1]);
}

int bl_gpio_ir_tx(uint16_t time[], uint32_t len)
{
    uint32_t gpio_output_old;
    uint32_t gpio_output_new[2];
    
    if(!ir_tx_ok){
        return -1;
    }
    
    unsigned long mstatus_tmp;
    mstatus_tmp = read_csr(mstatus);
    clear_csr(mstatus, MSTATUS_MIE);
    
    gpio_output_old = BL_RD_WORD(GLB_BASE + GLB_GPIO_OUTPUT_OFFSET);
    
    gpio_output_new[0] = gpio_output_old & ~(0x1 << ir_tx_pin);
    gpio_output_new[1] = gpio_output_old | (0x1 << ir_tx_pin);
    
    bl_gpio_ir_tx_do(time, len, gpio_output_new);
    
    write_csr(mstatus, mstatus_tmp);
    
    return 0;
}
