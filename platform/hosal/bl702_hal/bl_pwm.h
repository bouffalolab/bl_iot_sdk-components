#ifndef __BL_PWM_H__
#define __BL_PWM_H__

#include <stdint.h>

typedef enum {
    BL_PWM0 = 0,
    BL_PWM1 = 1,
    BL_PWM2 = 2,
    BL_PWM3 = 3,
    BL_PWM4 = 4,
}bl_pwm_t;

int32_t bl_pwm_port_init(bl_pwm_t id, uint32_t freq);
int32_t bl_pwm_gpio_init(bl_pwm_t id, uint8_t pin);  // id == pin % 5
int32_t bl_pwm_init(bl_pwm_t id, uint8_t pin, uint32_t freq);  // id == pin % 5
int32_t bl_pwm_start(bl_pwm_t id);
int32_t bl_pwm_stop(bl_pwm_t id);
int32_t bl_pwm_set_duty(bl_pwm_t id, float duty);
int32_t bl_pwm_get_duty(bl_pwm_t id, float *p_duty);

#endif
