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
#include <bl_sec.h>
#include <stdint.h>
#include "bl_sec_common.h"
#include "bl_sec_hw_common.h"

#define TRNG_SIZE_IN_WORD (8)
#define TRNG_SIZE_IN_BYTES (32)
#define TRNG_BUF_LEFT (TRNG_SIZE_IN_BYTES - trng_idx)
static uint8_t trng_buffer[TRNG_SIZE_IN_BYTES];
static uint8_t trng_idx = 0;

static int feed_trng_buffer()
{
    uint8_t val = 0;

    if (SUCCESS != Sec_Eng_Trng_Read(trng_buffer))
        return -1;

    for (size_t i = 0; i < TRNG_SIZE_IN_BYTES; ++i)
        val |= trng_buffer[i];
    // assert would be better?
    if (val == 0)
        return 1;

    trng_idx = 0;

    return 0;
}

int bl_rand_stream(uint8_t *buf, int len)
{
    int ret = 0;
    bl_sec_enter_critical();
    while (len > 0) {
        int left = TRNG_BUF_LEFT;
        int this_len = len;
        if (this_len > left) {
            this_len = left;
        }
        memcpy(buf, &trng_buffer[trng_idx], this_len);
        trng_idx += this_len;
        buf += this_len;
        len -= this_len;
        if (TRNG_BUF_LEFT == 0) {
            if ((ret = feed_trng_buffer())) {
                goto exit;
            }
        }
    }
exit:
    bl_sec_exit_critical(0);

    return ret;
}

int bl_rand()
{
    int val;

    bl_rand_stream((uint8_t *)&val, sizeof(val));
    return val;
}

uint32_t bl_sec_get_random_word(void)
{
    uint32_t val;

    bl_rand_stream((uint8_t *)&val, sizeof(val));
    return val;
}

int bl_sec_trng_init(void)
{
#if defined(BL616) || defined(BL808) || defined(BL606P)
    Sec_Eng_Group0_Request_Trng_Access();
#endif
    Sec_Eng_Trng_Enable();

    trng_idx = TRNG_SIZE_IN_BYTES;
    return 0;
}
