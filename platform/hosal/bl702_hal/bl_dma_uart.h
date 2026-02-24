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
#ifndef __BL_DMA_UART_H__
#define __BL_DMA_UART_H__

#include "bl702_glb.h"
#include "bl702_uart.h"
#include "bl702_dma.h"

typedef void (*bl_dma_uart_callback_t)(uint8_t uart_id);

typedef struct
{
    uint8_t uart_id;
    uint8_t uart_tx_pin;
    uint8_t uart_rx_pin;
    uint8_t uart_tx_dma_enable;
    uint32_t uart_baudrate;
    uint8_t *uart_rx_buf;
    uint16_t uart_rx_buf_size;  // maximum 8190 bytes
    bl_dma_uart_callback_t uart_tx_event;
    bl_dma_uart_callback_t uart_rx_event;
}bl_dma_uart_cfg_t;

int bl_dma_uart_init(bl_dma_uart_cfg_t *cfg);
int bl_dma_uart_change_baudrate(uint8_t uart_id, uint32_t baudrate);
uint16_t bl_dma_uart_get_rx_count(uint8_t uart_id);
uint16_t bl_dma_uart_read(uint8_t uart_id, uint8_t *data, uint16_t len);
int bl_dma_uart_write(uint8_t uart_id, uint8_t *data, uint16_t len, uint8_t wait_tx_done);
int bl_dma_uart_deinit(uint8_t uart_id);

#endif
