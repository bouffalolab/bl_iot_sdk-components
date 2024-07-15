#ifndef __BL_DMA_UART_H__
#define __BL_DMA_UART_H__

#include "bl702l_glb.h"
#include "bl702l_uart.h"
#include "bl702l_dma.h"

typedef void(*bl_dma_uart_callback_t)(void);

typedef struct
{
    uint8_t uart_id;
    uint32_t uart_baudrate;
    uint8_t uart_tx_pin;
    uint8_t uart_rx_pin;
    uint8_t uart_tx_dma_enable;
    uint8_t *uart_rx_buf;
    uint16_t uart_rx_buf_size;
    bl_dma_uart_callback_t uart_tx_event;
    bl_dma_uart_callback_t uart_rx_event;
}bl_dma_uart_cfg_t;

int bl_dma_uart_init(bl_dma_uart_cfg_t *cfg);
int bl_dma_uart_change_baudrate(uint32_t baudrate);
uint16_t bl_dma_uart_get_rx_count(void);
uint16_t bl_dma_uart_read(uint8_t *data, uint16_t len);
int bl_dma_uart_write(uint8_t *data, uint16_t len, uint8_t wait_tx_done);
int bl_dma_uart_deinit(void);

#endif
