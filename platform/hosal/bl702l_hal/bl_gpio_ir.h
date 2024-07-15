#ifndef __BL_GPIO_IR_H__
#define __BL_GPIO_IR_H__

#include <stdint.h>

// suitable for ir without carriers
int bl_gpio_ir_tx_init(uint8_t tx_pin);
int bl_gpio_ir_tx(uint16_t time[], uint32_t len);  // pull high and low alternately in us

#endif
