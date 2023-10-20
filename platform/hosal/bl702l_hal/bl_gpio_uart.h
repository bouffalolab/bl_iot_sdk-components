#ifndef __BL_GPIO_UART_H__
#define __BL_GPIO_UART_H__

#include <stdint.h>
#include <math.h>

int bl_gpio_uart_init(uint8_t tx_pin, uint32_t baudrate);  // tx_pin: 0 - 31; baudrate: max 2Mbps @128M or 1Mbps @32M
int bl_gpio_uart_send_byte(uint8_t data);
int bl_gpio_uart_send_data(uint8_t *data, uint32_t len);

#endif
