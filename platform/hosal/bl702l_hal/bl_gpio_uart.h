#ifndef __BL_GPIO_UART_H__
#define __BL_GPIO_UART_H__

#include <stdint.h>
#include <math.h>

#define GPIO_UART_TX_NUM           1

// tx_pin: 0 - 31
// baudrate: max 2Mbps @128M or 1Mbps @32M
int bl_gpio_uart_tx_init(uint8_t id, uint8_t tx_pin, uint32_t baudrate);
int bl_gpio_uart_send_byte(uint8_t id, uint8_t data);
int bl_gpio_uart_send_data(uint8_t id, uint8_t *data, uint32_t len);

// rx_pin: 0 - 31, two rx pins (connected together) are required
// baudrate: max 9600bps
int bl_gpio_uart_rx_init(uint8_t rx_pin_1, uint8_t rx_pin_2, uint32_t baudrate, uint8_t *rx_fifo, uint32_t fifo_size);
uint32_t bl_gpio_uart_get_rx_length(void);
uint32_t bl_gpio_uart_read_data(uint8_t *data, uint32_t len);

#endif
