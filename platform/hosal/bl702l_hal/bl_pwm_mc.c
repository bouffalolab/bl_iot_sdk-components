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
#include "bl702l_glb.h"
#include "bl702l_glb_gpio.h"
#include "bl702l_pwm.h"
#include "bl702l_clock.h"
#include "bl_pwm_mc.h"


#define BL_PWM_CLK            (SystemCoreClockGet()/(GLB_Get_BCLK_Div()+1))


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

static void pwm_mc_port_init(uint16_t div, uint16_t period)
{
    PWMx_CFG_Type pwmxCfg = {
        .clk = PWM_CLK_BCLK,
        .stopMode = PWM_STOP_GRACEFUL,
        .clkDiv = div,
        .period = period,
        .intPulseCnt = 0,
        .extPol = PWM_BREAK_Polarity_LOW,
        .stpRept = DISABLE,
        .adcSrc = PWM_TRIGADC_SOURCE_NONE,
    };

    PWMx_Init(PWM0_ID, &pwmxCfg);
    PWMx_Enable(PWM0_ID);
}

static void pwm_mc_channel_init(uint8_t ch, uint8_t active_level)
{
    PWM_CHx_CFG_Type chxCfg = {
        .modP = PWM_MODE_DISABLE,
        .modN = PWM_MODE_DISABLE,
        .polP = (active_level == 0) ? PWM_POL_ACTIVE_LOW : PWM_POL_ACTIVE_HIGH,
        .polN = (active_level == 0) ? PWM_POL_ACTIVE_LOW : PWM_POL_ACTIVE_HIGH,
        .idlP = PWM_IDLE_STATE_INACTIVE,
        .idlN = PWM_IDLE_STATE_INACTIVE,
        .brkP = PWM_BREAK_STATE_INACTIVE,
        .brkN = PWM_BREAK_STATE_INACTIVE,
        .thresholdL = 0,
        .thresholdH = 0,
        .dtg = 0,
    };

    PWM_Channelx_Init(PWM0_ID, ch, &chxCfg);
}

static void pwm_mc_start(uint8_t ch)
{
    PWM_Channelx_Positive_Pwm_Mode_Set(PWM0_ID, ch, PWM_MODE_ENABLE);
}

static void pwm_mc_stop(uint8_t ch)
{
    PWM_Channelx_Positive_Pwm_Mode_Set(PWM0_ID, ch, PWM_MODE_DISABLE);
}

static void pwm_mc_set_duty(uint8_t ch, float duty, uint16_t *threshold1, uint16_t *threshold2)
{
    uint16_t period;

    PWMx_Period_Get(PWM0_ID, &period);

    *threshold2 = (uint16_t)(period * duty / 100) + *threshold1;

    if(*threshold2 > period || *threshold2 < *threshold1){
        *threshold2 -= *threshold1;
        *threshold1 = 0;
    }

    PWM_Channelx_Threshold_Set(PWM0_ID, ch, *threshold1, *threshold2);
}

static void pwm_mc_get_duty(uint8_t ch, float *p_duty)
{
    uint16_t period;
    uint16_t threshold1;
    uint16_t threshold2;

    PWMx_Period_Get(PWM0_ID, &period);
    PWM_Channelx_Threshold_Get(PWM0_ID, ch, &threshold1, &threshold2);

    *p_duty = (float)(threshold2 - threshold1) * 100 / period;
}


int32_t bl_pwm_mc_port_init(uint32_t freq)
{
    uint32_t div = 1;
    uint32_t period = BL_PWM_CLK/freq;
    while(period >= 65536){
        div <<= 1;
        period >>= 1;
    }

    GLB_PER_Clock_UnGate(GLB_AHB_CLOCK_PWM);

    pwm_mc_port_init((uint16_t)div, (uint16_t)period);

    return 0;
}

int32_t bl_pwm_mc_channel_init(bl_pwm_mc_ch_t ch, uint8_t pin, uint8_t active_level)
{
    if(ch > 4 || ch < 1 || pin > 31 || ch != pin % 5){
        return -1;
    }

    pwm_mc_channel_init(ch - 1, active_level);

    gpio_init(pin);

    return 0;
}

int32_t bl_pwm_mc_start(bl_pwm_mc_ch_t ch)
{
    if(ch > 4 || ch < 1){
        return -1;
    }

    pwm_mc_start(ch - 1);

    return 0;
}

int32_t bl_pwm_mc_stop(bl_pwm_mc_ch_t ch)
{
    if(ch > 4 || ch < 1){
        return -1;
    }

    pwm_mc_stop(ch - 1);

    return 0;
}

int32_t bl_pwm_mc_set_duty(bl_pwm_mc_ch_t ch, float duty)
{
    uint16_t threshold1 = 0;
    uint16_t threshold2;

    if(ch > 4 || ch < 1){
        return -1;
    }

    if(duty <= 0){
        duty = 0;
    }

    if(duty >= 100){
        duty = 100;
    }

    pwm_mc_set_duty(ch - 1, duty, &threshold1, &threshold2);

    return 0;
}

int32_t bl_pwm_mc_set_duty_ex(bl_pwm_mc_ch_t ch, float duty, uint16_t *threshold1, uint16_t *threshold2)
{
    if(ch > 4 || ch < 1){
        return -1;
    }

    if(duty <= 0){
        duty = 0;
    }

    if(duty >= 100){
        duty = 100;
    }

    pwm_mc_set_duty(ch - 1, duty, threshold1, threshold2);

    return 0;
}

int32_t bl_pwm_mc_get_duty(bl_pwm_mc_ch_t ch, float *p_duty)
{
    if(ch > 4 || ch < 1){
        return -1;
    }

    pwm_mc_get_duty(ch - 1, p_duty);

    return 0;
}
