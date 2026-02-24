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
#include <stdio.h>
#include <stdint.h>
#include <bl_pwm.h>
#include <hosal_pwm.h>
#include <blog.h>


static uint8_t pwm_v2_ch_map = 0;  // use bits [4:1] for pwm_v2 (port 1)
static uint32_t pwm_v2_freq = 0;  // four pwm_v2 (port 1) channels share same frequency


/**
 * Initialises a PWM device
 *
 * @param[in]  pwm  the PWM device
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int hosal_pwm_init(hosal_pwm_dev_t *pwm)
{
    if (pwm == NULL) {
        blog_error("arg null.\r\n");
        return -1;
    }

    if (pwm->port > 1) {
        blog_error("arg error.\r\n");
        return -1;
    }

    if (pwm->config.pin > 31) {
        blog_error("arg error.\r\n");
        return -1;
    }

    uint8_t id = pwm->port;
    uint8_t ch = pwm->config.pin % 5;

    if ((id == 0 && ch != 0) || (id != 0 && ch == 0)) {
        blog_error("pwm port and pin do not match!\r\n");
        return -1;
    }

    if (pwm->port == 1) {
        if (pwm_v2_ch_map != 0) {  // pwm port 1 already initialized
            if (pwm->config.freq != pwm_v2_freq) {  // frequency change required
                blog_error("pwm port 1 has already been initialized with frequency %lu!\r\n", pwm_v2_freq);
                return -1;
            }
        } else {
            bl_pwm_port_init(pwm->port, pwm->config.freq);
            pwm_v2_freq = pwm->config.freq;
        }
        pwm_v2_ch_map |= 1 << ch;
    } else {
        bl_pwm_port_init(pwm->port, pwm->config.freq);
    }

    bl_pwm_gpio_init(ch, pwm->config.pin);
    bl_pwm_set_duty(ch, (float)pwm->config.duty_cycle / 100);

    return 0;
}

/**
 * Starts Pulse-Width Modulation signal output on a PWM pin
 *
 * @param[in]  pwm  the PWM device
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int hosal_pwm_start(hosal_pwm_dev_t *pwm)
{
    if (pwm == NULL) {
        blog_error("arg null.\r\n");
        return -1;
    }

    if (pwm->config.pin > 31) {
        blog_error("arg error.\r\n");
        return -1;
    }

    uint8_t ch = pwm->config.pin % 5;

    bl_pwm_start(ch);

    return 0;
}

/**
 * Changes parameters of the PWM device
 *
 * @param[in]  pwm  the PWM device
 * @param[in]  para  the new parameters (frequency and duty cycle)
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int hosal_pwm_para_chg(hosal_pwm_dev_t *pwm, hosal_pwm_config_t para)
{
    if (pwm == NULL) {
        blog_error("arg null.\r\n");
        return -1;
    }

    if (pwm->port > 1) {
        blog_error("arg error.\r\n");
        return -1;
    }

    if (pwm->config.pin > 31) {
        blog_error("arg error.\r\n");
        return -1;
    }

    if (pwm->port == 1) {
        if (pwm_v2_ch_map != 0) {  // pwm port 1 already initialized
            if (para.freq != pwm_v2_freq) {  // frequency change required
                blog_error("pwm port 1 does not support frequency change!\r\n");
                return -1;
            }
        } else {
            blog_error("pwm port 1 has not been initialized yet!\r\n");
            return -1;
        }
    } else {
        pwm->config.freq = para.freq;
        bl_pwm_port_init(pwm->port, pwm->config.freq);
    }

    uint8_t ch = pwm->config.pin % 5;

    pwm->config.duty_cycle = para.duty_cycle;
    bl_pwm_set_duty(ch, (float)pwm->config.duty_cycle / 100);

    bl_pwm_start(ch);

    return 0;
}

/**
 * Changes frequency of the PWM device
 *
 * @param[in]  pwm  the PWM device
 * @param[in]  freq  the new frequency
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int hosal_pwm_freq_set(hosal_pwm_dev_t *pwm, uint32_t freq)
{
    if (pwm == NULL) {
        blog_error("arg null.\r\n");
        return -1;
    }

    if (pwm->port > 1) {
        blog_error("arg error.\r\n");
        return -1;
    }

    if (pwm->config.pin > 31) {
        blog_error("arg error.\r\n");
        return -1;
    }

    if (pwm->port == 1) {
        if (pwm_v2_ch_map != 0) {  // pwm port 1 already initialized
            if (freq != pwm_v2_freq) {  // frequency change required
                blog_error("pwm port 1 does not support frequency change!\r\n");
                return -1;
            }
        } else {
            blog_error("pwm port 1 has not been initialized yet!\r\n");
            return -1;
        }
    } else {
        pwm->config.freq = freq;
        bl_pwm_port_init(pwm->port, pwm->config.freq);
    }

    uint8_t ch = pwm->config.pin % 5;

    bl_pwm_set_duty(ch, (float)pwm->config.duty_cycle / 100);

    bl_pwm_start(ch);

    return 0;
}

int hosal_pwm_freq_get(hosal_pwm_dev_t *pwm,  uint32_t *p_freq)
{
    if (pwm == NULL) {
        blog_error("arg null.\r\n");
        return -1;
    }

    if (pwm->port > 1) {
        blog_error("arg error.\r\n");
        return -1;
    }

    *p_freq = pwm->config.freq;

    return 0;
}

/**
 * Changes duty cycle of the PWM device
 *
 * @param[in]  pwm  the PWM device
 * @param[in]  duty  the new duty cycle
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int hosal_pwm_duty_set(hosal_pwm_dev_t *pwm, uint32_t duty)
{
    if (pwm == NULL) {
        blog_error("arg null.\r\n");
        return -1;
    }

    if (pwm->config.pin > 31) {
        blog_error("arg error.\r\n");
        return -1;
    }

    uint8_t ch = pwm->config.pin % 5;

    pwm->config.duty_cycle = duty;
    bl_pwm_set_duty(ch, (float)pwm->config.duty_cycle / 100);

    return 0;
}

int hosal_pwm_duty_get(hosal_pwm_dev_t *pwm, uint32_t *p_duty)
{
    float duty;

    if (pwm == NULL) {
        blog_error("arg null.\r\n");
        return -1;
    }

    if (pwm->config.pin > 31) {
        blog_error("arg error.\r\n");
        return -1;
    }

    uint8_t ch = pwm->config.pin % 5;

    bl_pwm_get_duty(ch, &duty);
    *p_duty = (uint32_t)(duty * 100);

    return 0;
}

/**
 * Stops output on a PWM pin
 *
 * @param[in]  pwm  the PWM device
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int hosal_pwm_stop(hosal_pwm_dev_t *pwm)
{
    if (pwm == NULL) {
        blog_error("arg null.\r\n");
        return -1;
    }

    if (pwm->config.pin > 31) {
        blog_error("arg error.\r\n");
        return -1;
    }

    uint8_t ch = pwm->config.pin % 5;

    bl_pwm_stop(ch);

    return 0;
}

/**
 * De-initialises the PWM device
 *
 * @param[in]  pwm  the PWM device
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int hosal_pwm_finalize(hosal_pwm_dev_t *pwm)
{
    if (pwm == NULL) {
        blog_error("arg null.\r\n");
        return -1;
    }

    if (pwm->port > 1) {
        blog_error("arg error.\r\n");
        return -1;
    }

    if (pwm->config.pin > 31) {
        blog_error("arg error.\r\n");
        return -1;
    }

    uint8_t ch = pwm->config.pin % 5;

    bl_pwm_stop(ch);

    if (pwm->port == 1) {
        pwm_v2_ch_map &= ~(1 << ch);
        if (pwm_v2_ch_map == 0) {
            pwm_v2_freq = 0;
        }
    }

    return 0;
}
