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
#include <bl702l_glb.h>
#include <bl702l_glb_gpio.h>
#include <bl702l_pwm.h>
#include "bl_pwm.h"

#define PWM_USE_ID PWM0_ID

int bl_pwm_init(uint8_t source, uint16_t div, uint16_t period)
{
    PWMx_CFG_Type pwmxCfg = {
        .clk = source,
        .stopMode = PWM_STOP_GRACEFUL,
        .clkDiv = div,
        .period = period,
        .intPulseCnt = 0,
        .extPol = PWM_BREAK_Polarity_LOW,
        .stpRept = DISABLE,
        .adcSrc = PWM_TRIGADC_SOURCE_NONE,
    };
    PWM_CHx_CFG_Type chxCfg = {
        .modP = PWM_MODE_DISABLE,
        .modN = PWM_MODE_DISABLE,
        .polP = PWM_POL_ACTIVE_HIGH,
        .polN = PWM_POL_ACTIVE_HIGH,
        .idlP = PWM_IDLE_STATE_INACTIVE,
        .idlN = PWM_IDLE_STATE_INACTIVE,
        .brkP = PWM_BREAK_STATE_INACTIVE,
        .brkN = PWM_BREAK_STATE_INACTIVE,
        .thresholdL = 0,
        .thresholdH = 0,
        .dtg = 0,
    };

    PWMx_Disable(PWM_USE_ID);
    PWMx_Init(PWM_USE_ID, &pwmxCfg);
    for (PWM_CHx_Type ch = PWM_CH0; ch < PWM_CHx_MAX; ch++) {
        PWM_Channelx_Init(PWM_USE_ID, ch, &chxCfg);
    }
    PWMx_Enable(PWM_USE_ID);

    return 0;
}

int bl_pwm_channel_init(uint8_t ch, uint8_t pin, uint8_t defaultLevel)
{
    GLB_GPIO_Cfg_Type gpioCfg;

    gpioCfg.gpioPin = pin;
    gpioCfg.gpioFun = GPIO_FUN_PWM;
    gpioCfg.gpioMode = GPIO_MODE_AF;
    gpioCfg.pullType = GPIO_PULL_NONE;
    gpioCfg.drive = 1;
    gpioCfg.smtCtrl = 1;
    GLB_GPIO_Init(&gpioCfg);

    PWM_Channelx_Positive_Pwm_Mode_Set(PWM_USE_ID, ch, PWM_MODE_DISABLE);
    if (defaultLevel) {
        PWM_Channelx_Positive_Polarity_Set(PWM_USE_ID, ch, PWM_POL_ACTIVE_LOW);
    } else {
        PWM_Channelx_Positive_Polarity_Set(PWM_USE_ID, ch, PWM_POL_ACTIVE_HIGH);
    }
    PWM_Channelx_Threshold_Set(PWM_USE_ID, ch, 0, 0);
    PWM_Channelx_Positive_Idle_State_Set(PWM_USE_ID, ch, PWM_IDLE_STATE_INACTIVE);
    PWM_Channelx_Positive_Pwm_Mode_Set(PWM_USE_ID, ch, PWM_MODE_ENABLE);

    return 0;
}

int bl_pwm_channel_duty_set(uint8_t ch, float duty)
{
    uint16_t period;
    uint16_t threshold;

    PWMx_Period_Get(PWM_USE_ID, &period);

    if (duty <= 0) {
        PWM_Channelx_Threshold_Set(PWM_USE_ID, ch, 0, 0);
    } else if (duty >= 100) {
        PWM_Channelx_Threshold_Set(PWM_USE_ID, ch, 0, period);
    } else {
        threshold = (uint16_t)(period * duty / 100);
        PWM_Channelx_Threshold_Set(PWM_USE_ID, ch, 0, threshold);
    }

    return 0;
}
