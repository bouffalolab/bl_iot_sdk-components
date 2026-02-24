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
#ifndef __BL_PWM_MOTOR_H__
#define __BL_PWM_MOTOR_H__


#include <stdint.h>
#include <math.h>


#ifndef PWM_MAIN_PIN
#define PWM_MAIN_PIN               10
#endif

#ifndef PWM_COMPLEMENTARY_PIN
#define PWM_COMPLEMENTARY_PIN      11
#endif


/**
 * @brief initialize pwm
 *
 * @param [in] freq pwm frequency in Hz
 * @param [in] duty pwm duty cycle from 0 to 100
 * @param [in] dead dead zone time in ns
 * @return device handle
 */
void *bl_pwm_motor_init(uint32_t freq, float duty, uint32_t dead);

/**
 * @brief update pwm parameters
 *
 * @param [in] handle device handle
 * @param [in] duty pwm duty cycle from 0 to 100
 * @param [in] dead dead zone time in ns
 * @return error code
 */
int bl_pwm_motor_update(void *handle, float duty, uint32_t dead);

/**
 * @brief start pwm
 *
 * @param [in] handle device handle
 * @return error code
 */
int bl_pwm_motor_start(void *handle);

/**
 * @brief stop pwm
 *
 * @param [in] handle device handle
 * @return error code
 */
int bl_pwm_motor_stop(void *handle);

/**
 * @brief finalize pwm
 *
 * @param [in] handle device handle
 * @return error code
 */
int bl_pwm_motor_finalize(void *handle);


#endif
