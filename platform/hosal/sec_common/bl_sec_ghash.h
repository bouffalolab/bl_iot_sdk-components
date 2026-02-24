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
#pragma once

#include <stdint.h>
#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct {
    uint32_t :9;
    uint32_t gmacIntClr:1;
    uint32_t gmacIntSet:1;
    uint32_t :5;
    uint32_t gmacMsgLen:16;
    uint32_t gmacSrcAddr;
    uint32_t gmacKey0;
    uint32_t gmacKey1;
    uint32_t gmacKey2;
    uint32_t gmacKey3;
    uint32_t result[4];
} __attribute__((aligned(4))) bl_SEC_Eng_GMAC_Link_Config_Type;

typedef struct {
    uint32_t guard0_[7];
    bl_SEC_Eng_GMAC_Link_Config_Type link_cfg;
    uint32_t dummy_;
    uint8_t tmp_buf[16];
    uint32_t guard1_[7];
} bl_sec_ghash_t;

int bl_sec_ghash_init();

int bl_ghash_init(bl_sec_ghash_t *ctx, const uint8_t key[16]);
int bl_ghash_clear(bl_sec_ghash_t *ctx);
int bl_ghash_acquire_hw();
int bl_ghash_release_hw();
int bl_ghash_update(bl_sec_ghash_t *ctx, const uint8_t *input, size_t input_len);
int bl_ghash_finish(bl_sec_ghash_t *ctx, uint8_t result[16]);
int bl_sec_ghash_test();

#if defined(__cplusplus)
}
#endif
