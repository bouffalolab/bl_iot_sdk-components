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
#include "bl_ir_rx.h"
#include "bl_irq.h"
#include "bl_gpio.h"
#include "hosal_gpio.h"
#include "blog.h"


static uint16_t *ir_rx_data = NULL;
static uint16_t ir_rx_max_num = 0;
static uint16_t ir_rx_len = 0;
static uint8_t ir_rx_overflow = 0;
static hosal_gpio_dev_t *ir_rx_gpio = NULL;


static void bl_ir_rx_gpio_handler(void *arg)
{
    int fifo_cnt;
    
    if(ir_rx_overflow){
        return;
    }
    
    fifo_cnt = IR_GetRxFIFOCount();
    
    /* rx fifo is full, gpio interrupt comes too late */
    blog_assert(fifo_cnt < 64);
    
    /* number of high/low level exceeds max_num specified by bl_ir_rx_cfg */
    if(ir_rx_len + fifo_cnt > ir_rx_max_num){
        fifo_cnt = ir_rx_max_num - ir_rx_len;
        ir_rx_overflow = 1;
    }
    
    /* read rx fifo */
    IR_SWMReceiveData(ir_rx_data + ir_rx_len, fifo_cnt);
    ir_rx_len += fifo_cnt;
}

static void bl_ir_rx_irq(void)
{
    IR_ClrIntStatus(IR_INT_RX);
    
    bl_ir_rx_gpio_handler(NULL);
    
    /* interrupt callback */
    bl_ir_rx_done_callback(ir_rx_data, ir_rx_len, ir_rx_overflow);
    
    /* enable rx again */
    if(BL_GET_REG_BITS_VAL(BL_RD_REG(IR_BASE, IRRX_CONFIG), IR_CR_IRRX_EN)){
        IR_Disable(IR_RX);
        IR_RxFIFOClear();
        IR_Enable(IR_RX);
        ir_rx_len = 0;
        ir_rx_overflow = 0;
    }
}

static void bl_ir_rx_interrupt_cfg(void)
{
    bl_irq_register(IRRX_IRQn, bl_ir_rx_irq);
    bl_irq_enable(IRRX_IRQn);
}


void bl_ir_custom_rx_cfg(uint8_t rx_pin, IR_RxCfg_Type *rx_cfg)
{
    blog_assert(rx_pin >= 17 && rx_pin <= 31);
    
    GLB_GPIO_Cfg_Type gpioCfg = {
        .gpioMode = GPIO_MODE_AF,
        .pullType = GPIO_PULL_NONE,
        .drive = 1,
        .smtCtrl = 1,
        .gpioPin = rx_pin,
        .gpioFun = GPIO_FUN_GPIO,
    };
    
    GLB_GPIO_Init(&gpioCfg);
    GLB_IR_RX_GPIO_Sel(rx_pin);
    
    GLB_PER_Clock_UnGate(GLB_AHB_CLOCK_IR);
    
    IR_Disable(IR_RX);
    IR_RxInit(rx_cfg);
    IR_IntMask(IR_INT_RX, UNMASK);
}

void bl_ir_rx_cfg(uint8_t rx_pin, uint16_t *rx_buf, uint16_t max_num, uint16_t max_width)
{
    IR_RxCfg_Type rxCfg = {
        .rxMode = IR_RX_SWM,           /* Set ir rx mode SWM */
        .inputInverse = ENABLE,        /* Enable signal of input inverse */
        .endThreshold = max_width,     /* Pulse width threshold to trigger end condition */
        .dataThreshold = 0,            /* Pulse width threshold for logic 0/1 detection */
        .rxDeglitch = DISABLE,         /* Disable input de-glitch function */
        .DeglitchCnt = 0,              /* De-glitch function cycle count */
    };
    
    /* Run IR at 1M */
    GLB_Set_IR_CLK(ENABLE, GLB_IR_CLK_SRC_XCLK, 31);
    
    bl_ir_custom_rx_cfg(rx_pin, &rxCfg);
    bl_ir_rx_interrupt_cfg();
    
    ir_rx_data = rx_buf;
    ir_rx_max_num = max_num;
    
    if(!ir_rx_gpio){
        ir_rx_gpio = malloc(sizeof(hosal_gpio_dev_t));
    }
    
    ir_rx_gpio->port = rx_pin;
    ir_rx_gpio->config = INPUT_PULL_UP;
    ir_rx_gpio->priv = NULL;
}

int bl_ir_rx_start(void)
{
    if(!ir_rx_data || !ir_rx_gpio){
        return -1;
    }
    
    /* rx has already started */
    if(BL_GET_REG_BITS_VAL(BL_RD_REG(IR_BASE, IRRX_CONFIG), IR_CR_IRRX_EN)){
        return -2;
    }
    
    int mstatus = bl_irq_save();
    
    hosal_gpio_init(ir_rx_gpio);
    hosal_gpio_irq_set(ir_rx_gpio, HOSAL_IRQ_TRIG_NEG_POS_PULSE, bl_ir_rx_gpio_handler, NULL);
    
    IR_Disable(IR_RX);
    IR_RxFIFOClear();
    IR_Enable(IR_RX);
    ir_rx_len = 0;
    ir_rx_overflow = 0;
    
    bl_irq_restore(mstatus);
    
    return 0;
}

int bl_ir_rx_stop(void)
{
    if(!ir_rx_data || !ir_rx_gpio){
        return -1;
    }
    
    int mstatus = bl_irq_save();
    
    hosal_gpio_finalize(ir_rx_gpio);
    
    IR_Disable(IR_RX);
    
    bl_irq_restore(mstatus);
    
    return 0;
}

__attribute__((weak)) void bl_ir_rx_done_callback(uint16_t *data, uint16_t len, uint8_t full)
{
    if(full){
        printf("rx_buf full\r\n");
        return;
    }
    
    printf("ir_rx[%u]:", len);
    for(int i=0; i<len; i++){
        printf(" %u", data[i]);
    }
    printf("\r\n");
}


#if 0
static void send_test_signal(uint8_t tx_pin, uint16_t end_width)
{
    uint16_t tx_buf[] = {20, 30, 40, 50, 60, 70, 80, 90, 100};
    uint32_t tick_low;
    
    bl_gpio_output_set(tx_pin, 1);
    bl_gpio_enable_output(tx_pin, 0, 0);
    
    int mstatus = bl_irq_save();
    for(int i = 0; i < sizeof(tx_buf)/sizeof(tx_buf[0]); i++){
        tick_low = *(volatile uint32_t *)0x0200BFF8;
        if(i % 2 == 0){
            *(volatile uint32_t *)0x40000188 &= ~(0x1 << tx_pin);
        }else{
            *(volatile uint32_t *)0x40000188 |= (0x1 << tx_pin);
        }
        while(*(volatile uint32_t *)0x0200BFF8 - tick_low < tx_buf[i] * 4);
    }
    *(volatile uint32_t *)0x40000188 |= (0x1 << tx_pin);
    bl_irq_restore(mstatus);
    
    arch_delay_us(end_width);
}

static uint16_t rx_buf[512];

// We implement a weak function bl_ir_rx_done_callback in this file for testing.
// To overwrite it, just implement your own bl_ir_rx_done_callback out of this file.
void bl_ir_rx_test(void)
{
    uint16_t max_width = 10000;
    
    // rx_pin: 21
    bl_ir_rx_cfg(21, rx_buf, sizeof(rx_buf)/sizeof(rx_buf[0]), max_width);
    bl_ir_rx_start();
    
    // tx_pin: 20, connected to rx_pin
    send_test_signal(20, max_width * 2);
}
#endif
