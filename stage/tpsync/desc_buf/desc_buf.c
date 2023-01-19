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
#include <string.h>

#include <FreeRTOS.h>
#include <queue.h>

#include <tpdbg.h>
#include "desc_buf.h"

typedef struct{
    uint8_t   *buf;
    uint32_t  len;
} desc_t;

int desc_init(desc_msg_t *q, uint32_t items)
{
    if (NULL == q) {
        return -1;
    }

    //printf("desc_init = %p\r\n", q);
    q->hdl = xQueueCreate(items, sizeof(desc_t));

    if (NULL == q->hdl) {
        return -2;
    }

    tpdbg_log("creat success.\r\n");
    return 0;
}

int desc_deinit(desc_msg_t *q)
{
    desc_t msg;

    if (NULL == q) {
        return 0;
    }

    while (xQueueReceive(q->hdl, &msg, 0)) {
        vPortFree(msg.buf);
    }

    vQueueDelete(q->hdl);

    q->hdl = NULL;

    return 0;
}

int desc_push_tofront(desc_msg_t *q, void *buf, uint32_t len, uint32_t timeout_ms)//vip
{
    desc_t msg;
    int res;

    if ((NULL == q) || (NULL == buf) || (0 == len)) {
        return -1;
    }

    msg.buf = pvPortMalloc(len);
    if (NULL == msg.buf) {
        return -2;
    }

    memcpy(msg.buf, buf, len);
    msg.len = len;

    res = xQueueSendToFront(q->hdl, &msg, (DESC_FOREVER == timeout_ms) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms));// fixme return
    if (pdTRUE != res) {
        //tpdbg_log("xQueueSendToFront res = %d\r\n", res);
        return -3;
    }

    tpdbg_log("desc_push_tofront success.\r\n");
    return 0;
}

int desc_push_toback(desc_msg_t *q, void *buf, uint32_t len, uint32_t timeout_ms)
{
    desc_t msg;
    int res;

    if ((NULL == q) || (NULL == buf) || (0 == len)) {
        return -1;
    }

    msg.buf = pvPortMalloc(len);
    if (NULL == msg.buf) {
        return -2;
    }

    memcpy(msg.buf, buf, len);
    msg.len = len;

    res = xQueueSendToBack(q->hdl, &msg, (DESC_FOREVER == timeout_ms) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms));// fixme return
    if (pdTRUE != res) {
        //tpdbg_log("xQueueSendToBack res = %d\r\n", res);
        return -3;
    }

    tpdbg_log("desc_push_toback success.\r\n");
    return 0;
}

int desc_pop(desc_msg_t *q, void *buf, uint32_t *len_p, uint32_t timeout_ms)//timerout = 0xFFFFFFFF ms
{
    desc_t msg;
    int res;

    if (NULL == q) {
        return -1;
    }

    //printf("desc_pop = %p\r\n", desc_pop);

    res = xQueueReceive(q->hdl, &msg,
            (DESC_FOREVER == timeout_ms) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms));

    if (pdTRUE != res) {
        return -2;
    }

    if (buf) {
        memcpy(buf, msg.buf, msg.len);
    }

    if (len_p) {
        *len_p = msg.len;
    }

    vPortFree(msg.buf);

    return 0;
}

int desc_peek(desc_msg_t *q, uint32_t *len_p, uint32_t timeout_ms)
{
    desc_t msg;

    if ((NULL == q) || (NULL == len_p)) {
        return -1;
    }

    if (pdTRUE == xQueuePeek(q->hdl, &msg,
                (DESC_FOREVER == timeout_ms) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms))) {
        *len_p = msg.len;
    } else {
        *len_p = 0;
        return -2;
    }

    return 0;
}

