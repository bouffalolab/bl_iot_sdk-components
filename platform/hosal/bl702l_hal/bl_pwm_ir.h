#ifndef __BL_PWM_IR_H__
#define __BL_PWM_IR_H__

#include "bl702l_pwm.h"
#include "bl702l_glb.h"
#include "bl702l_clock.h"

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
