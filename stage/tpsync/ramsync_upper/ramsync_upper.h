#ifndef __RAMSYNC_UPPER_H__
#define __RAMSYNC_UPPER_H__

#include <stdint.h>
#include "desc_buf.h"
#include "ramsync_low.h"

#include <FreeRTOS.h>
#include <queue.h>
#include <event_groups.h>


/* ramsync_upper config macro */
#define TP_ST_MAGIC              (0x11223344)
#define TP_TXPAYLOAD_NUM         (2)
#define TP_TXDESC_NUM            (2)
#define TP_RXPAYLOAD_NUM         (2)
#define TP_RXDESC_NUM            (2)
#define TP_PAYLOAD_LEN           (1024 + 512)// 512

#define RAMSYNC_TX_STACK_SIZE    (1024)// Bytes
#define RAMSYNC_RX_STACK_SIZE    (1024)// Bytes
#define RAMSYNC_RESET_SATCK_SIZE (1024)
#ifndef RAMSYNC_TX_TASK_PRI
#define RAMSYNC_TX_TASK_PRI      (19)
#endif
#ifndef RAMSYNC_RX_TASK_PRI
#define RAMSYNC_RX_TASK_PRI      (19)
#endif
#define RAMSYNC_RESET_TASK_PRI   (10)

#define RAMSYNC_FAST_COPY_ENABLE (0)

#define URAMSYNC_FOREVER         (0xFFFFFFFF)
#define POLL_UNIT_TIME_MS        (50)


#define URAMSYNC_MASTER_DEV_TYPE (1)
#define URAMSYNC_SLAVE_DEV_TYPE  (2)

/* Structure for tp_uramsync_t */
typedef struct __tp_payload {
    uint32_t magic;
    uint32_t rseq;
    uint32_t len;
    uint32_t seq;
    uint8_t buf[TP_PAYLOAD_LEN];
    uint32_t crc;
} tp_payload_t;

typedef struct __tp_txbuf {
    tp_payload_t payload[TP_TXPAYLOAD_NUM];
} tp_txbuf_t;

typedef struct __tp_rxbuf {
    tp_payload_t payload[TP_RXPAYLOAD_NUM];
} tp_rxbuf_t;

typedef struct __tp_buf {
    char name[4];

    /* slot */
    tp_txbuf_t * p_tx;
    tp_rxbuf_t * p_rx;

    /* local sequnce */
    uint32_t tx_seq;

    /* tx/rx desc hdr */
    desc_msg_t tx_desc;
    desc_msg_t rx_desc;
    SemaphoreHandle_t tx_sem;
    SemaphoreHandle_t rx_sem;

    /* calulate crc for rx */
    tp_payload_t *p_rx_cache;

    /* tx/rx task */
    TaskHandle_t tx_thdr;
    TaskHandle_t rx_thdr;

    TaskHandle_t reset_thdr;

    lramsync_ctx_t hw;
    uint32_t status;
    uint32_t devtype;// 1 master, 2 slave
} tp_uramsync_t;

/**
 * This function init upper ramsync
 *
 * @param[in]  uramsync id hdr, mem from app
 *
 * @return  0 on success, error code otherwise.
 */
int uramsync_init(tp_uramsync_t *uramsync, uint32_t type);

/**
 * This function dump upper arg
 *
 * @param[in]  uramsync is hdr
 *
 * @return
 */
void uramsync_dump(tp_uramsync_t *rs);

/**
 * This function deinit upper ramsync
 *
 * @param[in]  uramsync id hdr, mem from app
 *
 * @return  0 on success, error code otherwise.
 */
int uramsync_deinit(tp_uramsync_t *uramsync);

/**
 * Receive an item from a queue.  The item is received by copy so a buffer of
 * adequate size must be provided.  The number of bytes copied into the buffer
 * was defined when the queue was created.
 *
 * Successfully received items are removed from the queue.
 *
 * @param[in] uramsync The handle to the queue from which the item is to be
 * received.
 *
 * @param[in] buf Pointer to the buffer into which the received item will
 * be copied.
 *
 * @param[in] len the buf len
 *
 * @param[in] timeout_ms xTicksToWait The maximum amount of time the task should block
 * waiting for an item to receive should the queue be empty at the time
 * of the call. xQueueReceive() will return immediately if xTicksToWait
 * is zero and the queue is empty.  The time is defined in tick periods so the
 * constant portTICK_PERIOD_MS should be used to convert to real time if this is
 * required.
 *
 * @return pdTRUE if an item was successfully received from the queue,
 * otherwise pdFALSE.
 */
int uramsync_rx_pop(tp_uramsync_t *uramsync, void *buf, uint32_t *len_p, uint32_t timeout_ms);

/**
 * Post an item to the front of a queue.  The item is queued by copy, not by
 * reference.
 *
 * @param[in] uramsync: The handle to the queue on which the item is to be posted.
 *
 * @param[in] buf: A pointer to the item that is to be placed on the
 * queue.  The size of the items the queue will hold was defined when the
 * queue was created, so this many bytes will be copied from pvItemToQueue
 * into the queue storage area.
 *
 * @param[in] len: the buf size
 *
 * @param xTicksToWait The maximum amount of time the task should block
 * waiting for space to become available on the queue, should it already
 * be full.  The call will return immediately if this is set to 0 and the
 * queue is full. URAMSYNC_FOREVER will wait forever for received data.
 *
 * @return pdTRUE if the item was successfully posted, otherwise errQUEUE_FULL.
 *
 */
int uramsync_tx_push_toback(tp_uramsync_t *uramsync, void *buf, uint32_t len, uint32_t timeout_ms);

/*
 * Post an item to the front of a queue vip.  The item is queued by copy, not by
 * reference.
 *
 * @param[in] uramsync: The handle to the queue on which the item is to be posted.
 *
 * @param[in] buf: A pointer to the item that is to be placed on the
 * queue.  The size of the items the queue will hold was defined when the
 * queue was created, so this many bytes will be copied from pvItemToQueue
 * into the queue storage area.
 *
 * @param[in] len: the buf size
 *
 * @param xTicksToWait The maximum amount of time the task should block
 * waiting for space to become available on the queue, should it already
 * be full.  The call will return immediately if this is set to 0 and the
 * queue is full. URAMSYNC_FOREVER will wait forever for received data.
 *
 * @return pdTRUE if the item was successfully posted, otherwise errQUEUE_FULL.
 *
 */
int uramsync_tx_push_tofront(tp_uramsync_t *uramsync, void *buf, uint32_t len, uint32_t timeout_ms);

#endif
