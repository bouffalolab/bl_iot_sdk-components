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



