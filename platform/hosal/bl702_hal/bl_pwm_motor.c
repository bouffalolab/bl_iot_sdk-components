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
#include "bl702_glb.h"
#include "bl702_pwm.h"
#include "bl702_clock.h"
#include "bl_pwm_motor.h"
#include "bl_irq.h"


#define PWM_MAIN_CHANNEL           ((PWM_MAIN_PIN) % 5)
#define PWM_COMPLEMENTARY_CHANNEL  ((PWM_COMPLEMENTARY_PIN) % 5)
#if PWM_MAIN_CHANNEL == PWM_COMPLEMENTARY_CHANNEL
#error PWM_MAIN_PIN and PWM_COMPLEMENTARY_PIN share the same pwm channel
#endif

#define PWM_TH1_REG(CH)            (0x4000A424 + (CH) * 0x20)
#define PWM_TH2_REG(CH)            (0x4000A428 + (CH) * 0x20)
#define PWM_CFG_REG(CH)            (0x4000A430 + (CH) * 0x20)

#define PWM_START_INTERVAL         100  // unit: ns

#define PWM_NS2CNT(NS)             ((float)(pwm_ctx->freq * pwm_ctx->period) / 1000000000 * (int)(NS))
#define PWM_DUTY2CNT(DUTY)         ((float)pwm_ctx->period / 100 * (float)(DUTY))


typedef struct {
    uint32_t freq;
    uint32_t period;
} pwm_ctx_t;

pwm_ctx_t *pwm_ctx = NULL;


static void gpio_init(uint8_t pin)
{
    GLB_GPIO_Cfg_Type cfg;

    cfg.drive = 3;
    cfg.smtCtrl = 1;
    cfg.gpioMode = GPIO_MODE_OUTPUT;
    cfg.pullType = GPIO_PULL_NONE;
    cfg.gpioPin = pin;
    cfg.gpioFun = 8;

    GLB_GPIO_Init(&cfg);
}

static void pwm_init(uint8_t ch, uint16_t div, uint16_t period, uint16_t threshold1, uint16_t threshold2)
{
    PWM_CH_CFG_Type pwmCfg = {
        .ch = ch,
        .clk = PWM_CLK_XCLK,
        .stopMode = PWM_STOP_ABRUPT,
        .pol = PWM_POL_NORMAL,
        .clkDiv = div,
        .period = period,
        .threshold1 = threshold1,
        .threshold2 = threshold2,
        .intPulseCnt = 0,
    };

    PWM_Channel_Init(&pwmCfg);
}

static void pwm_calc_main_threshold(float duty, uint32_t dead, uint16_t *threshold1, uint16_t *threshold2)
{
    *threshold1 = round(PWM_NS2CNT(PWM_START_INTERVAL + dead));
    if((int)duty >= 50){
        *threshold2 = round(PWM_DUTY2CNT(duty) + PWM_NS2CNT(PWM_START_INTERVAL - dead));
    }else{
        *threshold2 = round(PWM_DUTY2CNT(duty) + PWM_NS2CNT(PWM_START_INTERVAL + dead));
    }
}

static void pwm_calc_comp_threshold(float duty, uint32_t dead, uint16_t *threshold1, uint16_t *threshold2)
{
    *threshold1 = 0;
    if((int)duty >= 50){
        *threshold2 = round(PWM_DUTY2CNT(duty));
    }else{
        *threshold2 = round(PWM_DUTY2CNT(duty) + PWM_NS2CNT(dead * 2));
    }
}

static void pwm_calc_threshold(float duty, uint32_t dead, uint16_t *main_threshold1, uint16_t *main_threshold2, uint16_t *comp_threshold1, uint16_t *comp_threshold2)
{
    pwm_calc_main_threshold(duty, dead, main_threshold1, main_threshold2);
    pwm_calc_comp_threshold(duty, dead, comp_threshold1, comp_threshold2);

    if(*main_threshold2 == *main_threshold1){
        *comp_threshold2 = *comp_threshold1;
    }

    if(*comp_threshold2 >= pwm_ctx->period){
        *main_threshold1 = 0;
        *main_threshold2 = pwm_ctx->period;
    }
}

ATTR_TCM_SECTION
__attribute__((noinline)) static void pwm_update_threshold(uint16_t main_threshold1, uint16_t main_threshold2, uint16_t comp_threshold1, uint16_t comp_threshold2)
{
    BL_WR_WORD(PWM_TH1_REG(PWM_MAIN_CHANNEL), main_threshold1);
    BL_WR_WORD(PWM_TH2_REG(PWM_MAIN_CHANNEL), main_threshold2);
    BL_WR_WORD(PWM_TH1_REG(PWM_COMPLEMENTARY_CHANNEL), comp_threshold1);
    BL_WR_WORD(PWM_TH2_REG(PWM_COMPLEMENTARY_CHANNEL), comp_threshold2);
}

ATTR_TCM_SECTION
__attribute__((noinline)) static void pwm_ctrl(uint32_t main_cfg, uint32_t comp_cfg)
{
    BL_WR_WORD(PWM_CFG_REG(PWM_MAIN_CHANNEL), main_cfg);
    BL_WR_WORD(PWM_CFG_REG(PWM_COMPLEMENTARY_CHANNEL), comp_cfg);
}


void *bl_pwm_motor_init(uint32_t freq, float duty, uint32_t dead)
{
    uint32_t div;
    uint32_t period;
    uint16_t main_threshold1;
    uint16_t main_threshold2;
    uint16_t comp_threshold1;
    uint16_t comp_threshold2;

    if(pwm_ctx == NULL){
        pwm_ctx = malloc(sizeof(pwm_ctx_t));
    }

    div = 1;
    period = 32000000 / freq;
    while(period >= 65536){
        div <<= 1;
        period >>= 1;
    };

    pwm_ctx->freq = freq;
    pwm_ctx->period = period;

    if(duty <= 0){
        duty = 0;
    }

    if(duty >= 100){
        duty = 100;
    }

    GLB_PER_Clock_UnGate(GLB_AHB_CLOCK_PWM);

    pwm_calc_threshold(duty, dead, &main_threshold1, &main_threshold2, &comp_threshold1, &comp_threshold2);

    gpio_init(PWM_MAIN_PIN);
    pwm_init(PWM_MAIN_CHANNEL, div, period, main_threshold1, main_threshold2);

    gpio_init(PWM_COMPLEMENTARY_PIN);
    pwm_init(PWM_COMPLEMENTARY_CHANNEL, div, period, comp_threshold1, comp_threshold2);

    return pwm_ctx;
}

int bl_pwm_motor_update(void *handle, float duty, uint32_t dead)
{
    uint16_t main_threshold1;
    uint16_t main_threshold2;
    uint16_t comp_threshold1;
    uint16_t comp_threshold2;
    int flags;

    if(pwm_ctx == NULL || pwm_ctx != handle){
        return -1;
    }

    if(duty <= 0){
        duty = 0;
    }

    if(duty >= 100){
        duty = 100;
    }

    pwm_calc_threshold(duty, dead, &main_threshold1, &main_threshold2, &comp_threshold1, &comp_threshold2);

    flags = bl_irq_save();
    pwm_update_threshold(main_threshold1, main_threshold2, comp_threshold1, comp_threshold2);
    bl_irq_restore(flags);

    return 0;
}

int bl_pwm_motor_start(void *handle)
{
    uint32_t main_cfg;
    uint32_t comp_cfg;
    int flags;

    if(pwm_ctx == NULL || pwm_ctx != handle){
        return -1;
    }

    flags = bl_irq_save();

    main_cfg = BL_RD_WORD(PWM_CFG_REG(PWM_MAIN_CHANNEL));
    main_cfg &= ~(1UL << 6);
    comp_cfg = BL_RD_WORD(PWM_CFG_REG(PWM_COMPLEMENTARY_CHANNEL));
    comp_cfg &= ~(1UL << 6);
    comp_cfg |= (1UL << 2);

    pwm_ctrl(main_cfg, comp_cfg);

    bl_irq_restore(flags);

    return 0;
}

int bl_pwm_motor_stop(void *handle)
{
    uint32_t main_cfg;
    uint32_t comp_cfg;
    int flags;

    if(pwm_ctx == NULL || pwm_ctx != handle){
        return -1;
    }

    flags = bl_irq_save();

    main_cfg = BL_RD_WORD(PWM_CFG_REG(PWM_MAIN_CHANNEL));
    main_cfg |= (1UL << 6);
    comp_cfg = BL_RD_WORD(PWM_CFG_REG(PWM_COMPLEMENTARY_CHANNEL));
    comp_cfg |= (1UL << 6);
    comp_cfg &= ~(1UL << 2);

    pwm_ctrl(main_cfg, comp_cfg);

    bl_irq_restore(flags);

    return 0;
}

int bl_pwm_motor_finalize(void *handle)
{
    if(pwm_ctx == NULL || pwm_ctx != handle){
        return -1;
    }

    bl_pwm_motor_stop(handle);

    free(pwm_ctx);
    pwm_ctx = NULL;

    return 0;
}


#if 0
void bl_pwm_motor_test(void)
{
    void *handle;

    handle = bl_pwm_motor_init(50000, 20, 300);
    arch_delay_ms(3000);

    bl_pwm_motor_start(handle);
    arch_delay_ms(3000);

    bl_pwm_motor_update(handle, 50, 400);
    arch_delay_ms(3000);

    bl_pwm_motor_update(handle, 80, 500);
    arch_delay_ms(3000);

    bl_pwm_motor_stop(handle);
    arch_delay_ms(3000);

    bl_pwm_motor_finalize(handle);
}
#endif
