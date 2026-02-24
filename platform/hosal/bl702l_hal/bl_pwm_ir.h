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
#ifndef __BL_PWM_IR_H__
#define __BL_PWM_IR_H__

#include "bl702l_pwm.h"

void bl_pwm_ir_tx_cfg(float freq_hz, float duty_cycle);
void bl_pwm_ir_tx_pin_cfg(uint8_t pin);  // pin: pin % 5 != 0
int bl_pwm_ir_tx(uint16_t data[], uint32_t len);  // transmit mark and space alternately
int bl_pwm_ir_tx_ex(uint32_t data[], uint32_t len);  // transmit mark or space according to the msb of each data

void bl_pwm_ir_nec_tx_init(uint8_t pin);  // pin: pin % 5 != 0
int bl_pwm_ir_nec_tx(uint8_t addr, uint8_t cmd);  // addr: 8-bit address; cmd: 8-bit command
int bl_pwm_ir_nec_tx_repeat(void);  // transmit repeat code

void bl_pwm_ir_rc5_tx_init(uint8_t pin);  // pin: pin % 5 != 0
int bl_pwm_ir_rc5_tx(uint8_t t, uint8_t addr, uint8_t cmd);  // t: 1-bit toggle; addr: 5-bit address; cmd: 6-bit command

#endif
