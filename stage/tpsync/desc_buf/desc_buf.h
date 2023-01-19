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
#ifndef __DESC_BUF_H__
#define __DESC_BUF_H__

#include <stdint.h>

typedef struct {
    void *hdl;
} desc_msg_t;

#define DESC_FOREVER 0xFFFFFFFF
#define DESC_BUF_DEBUG_ENABLE    (0)

int desc_init(desc_msg_t *q, uint32_t items);
int desc_deinit(desc_msg_t *q);
int desc_pop(desc_msg_t *q, void *buf, uint32_t *len_p, uint32_t timeout_ms);// 0-noblock DESC_FOREVER-block
int desc_push_toback(desc_msg_t *q, void *buf, uint32_t len, uint32_t timeout_ms);
int desc_push_tofront(desc_msg_t *q, void *buf, uint32_t len, uint32_t timeout_ms);//vip
int desc_peek(desc_msg_t *q, uint32_t *len_p, uint32_t timeout_ms);// 0-noblock DESC_FOREVER-block
int desc_deinit(desc_msg_t *q);

#endif

