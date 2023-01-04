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

/*
 * memcpy.c
 */

#include <string.h>
#include <stdint.h>

void *memcpy8(void *dst, const void *src, size_t n)
{
    const uint8_t *p = src;
    uint8_t *q = dst;

    while (n--) {
        *q++ = *p++;
    }

    return dst;
}

/****************************************************************************/ /**
 * @brief  Word memcpy
 *
 * @param  dst: Destination
 * @param  src: Source
 * @param  n:  Count of words
 *
 * @return Destination pointer
 *
 *******************************************************************************/
uint32_t *memcpy32(uint32_t *dst, const uint32_t *src, size_t n)
{
    const uint32_t *p = src;
    uint32_t *q = dst;

    while (n--) {
        *q++ = *p++;
    }

    return dst;
}

/****************************************************************************/ /**
 * @brief  Double word memcpy
 *
 * @param  dst: Destination
 * @param  src: Source
 * @param  n:  Count of words
 *
 * @return Destination pointer
 *
 *******************************************************************************/
uint64_t *memcpy64(uint64_t *dst, const uint64_t *src, size_t n)
{
    const uint64_t *p = src;
    uint64_t *q = dst;

    while (n--) {
        *q++ = *p++;
    }

    return dst;
}
/****************************************************************************/ /**
 * @brief  Fast memcpy
 *
 * @param  dst: Destination
 * @param  src: Source
 * @param  n:  Count of bytes
 *
 * @return Destination pointer
 *
 *******************************************************************************/
void *memcpy(void *dst, const void *src, size_t n)
{
    uint32_t left, done, i = 0;
    uint8_t *pdst = (uint8_t *)dst;
    uint8_t *psrc = (uint8_t *)src;

#if __riscv_xlen == 64
    if (((uint64_t)(uintptr_t)pdst & 0x7) == 0 && ((uint64_t)(uintptr_t)psrc & 0x7) == 0) {
        memcpy64((uint64_t *)pdst, (const uint64_t *)psrc, n >> 3);
        left = n & 0x07;
        goto remain;
    }
#endif
    if (((uint32_t)(uintptr_t)pdst & 0x3) == 0 && ((uint32_t)(uintptr_t)psrc & 0x3) == 0) {
        memcpy32((uint32_t *)pdst, (const uint32_t *)psrc, n >> 2);
        left = n & 0x03;
#if __riscv_xlen == 64
remain:
#endif
        done = n - left;
        while (i < left) {
            pdst[done + i] = psrc[done + i];
            i++;
        }
    } else {
        memcpy8(pdst, psrc, n);
    }

    return dst;

}
