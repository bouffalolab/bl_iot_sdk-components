
#include <stdio.h>
#include "ramsync_low.h"

void lramsync_init(
        lramsync_ctx_t *ctx,
        node_mem_t *node_tx, uint32_t items_tx,
        lramsync_cb_func_t tx_cb, void *tx_arg,
        node_mem_t *node_rx, uint32_t items_rx,
        lramsync_cb_func_t rx_cb, void *rx_arg,
        lramsync_cb_func_t reset_signal_cb, void *reset_signal_arg
        )
{
    if (NULL == ctx) {
        printf("arg error.\r\n");
        return;
    }

    ctx->tx_cb             = tx_cb;
    ctx->tx_arg            = tx_arg;
    ctx->rx_cb             = rx_cb;
    ctx->rx_arg            = rx_arg;
    ctx->reset_signal_cb   = reset_signal_cb;
    ctx->reset_signal_arg  = reset_signal_arg;
}

void lramsync_start(lramsync_ctx_t *ctx)
{
}

void lramsync_reset(lramsync_ctx_t *ctx)
{
    if (ctx->reset_signal_cb) {
        ctx->reset_signal_cb(ctx->reset_signal_arg);
    }
}

void lramsync_deinit(lramsync_ctx_t *ctx)
{
}

