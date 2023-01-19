/*
 * Copyright (c) 2016-2023 Bouffalolab.
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
#include <utils_list.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <stdint.h>

#ifndef __RAMSYNC_LOW_H__
#define __RAMSYNC_LOW_H__

typedef void (*lramsync_cb_func_t)(void *arg);

typedef struct __node_mem {
    void        *buf;
    uint32_t    len;
} node_mem_t;

typedef struct lramsync_spi_config {
	uint8_t port;
	uint8_t spi_mode;
	uint32_t spi_speed;
    uint8_t miso;
    uint8_t mosi;
    uint8_t clk;
    uint8_t cs;
} lramsync_spi_config_t;

typedef struct lramsync_ctx {
    const lramsync_spi_config_t *cfg;

    node_mem_t *node_tx;
    node_mem_t *node_rx;
    uint32_t items_tx;
    uint32_t items_rx;

    lramsync_cb_func_t tx_cb;
    void *tx_arg;
    lramsync_cb_func_t rx_cb;
    void *rx_arg;
    lramsync_cb_func_t reset_signal_cb;
    void *reset_signal_arg;

    int8_t dma_tx_chan;
    int8_t dma_rx_chan;

    void *priv;
} lramsync_ctx_t;

void lramsync_init(
        lramsync_ctx_t *ctx,
        node_mem_t *node_tx, uint32_t items_tx,
        lramsync_cb_func_t tx_cb, void *tx_arg,
        node_mem_t *node_rx, uint32_t items_rx,
        lramsync_cb_func_t rx_cb, void *rx_arg,
        lramsync_cb_func_t reset_signal_cb, void *reset_signal_arg
        );

void lramsync_start(lramsync_ctx_t *ctx);

void lramsync_reset(lramsync_ctx_t *ctx);
void lramsync_deinit(lramsync_ctx_t *ctx);

#endif



