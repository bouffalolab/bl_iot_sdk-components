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
#include <stdarg.h>

#include <FreeRTOS.h>
#include <semphr.h>

static              char s_debug_buf[512];
static SemaphoreHandle_t s_debug_mutex = NULL;

static int buf_out(const char *str, const void *inbuf, int len)
{
#undef  ONE_LINE_MAX_NUM
#define ONE_LINE_MAX_NUM    (32)
    char *buf = (char *)inbuf;
    char *pbuffer = NULL;

    pbuffer = (char *)s_debug_buf;
    int m = 0, n = 0;
    int j = 0, k = 0, tmp = 0;

    tmp = (sizeof(s_debug_buf))/3;/* 数组最大长度 */
    if ((ONE_LINE_MAX_NUM > tmp) || (len < 1))
    {
        return -1;
    }

    if (NULL == s_debug_mutex) {
        s_debug_mutex = xSemaphoreCreateMutex();
    }

    m = len / ONE_LINE_MAX_NUM;
    n = len % ONE_LINE_MAX_NUM;
    if (n > 0) {
        m++;
    }

    if (n > 0) {
        /* 非整数倍 */
        for (k = 0; k < m; k++) {
            if ((k+1) == m) {
                /* 最后一帧数据 */
                tmp = 0;
                for (j = 0; j < n; j++) {
                    tmp += sprintf(pbuffer + tmp, "%02x ", (uint8_t)buf[k*ONE_LINE_MAX_NUM+j]);
                }
                printf("[%s] %.*s\r\n", str, tmp, pbuffer);
            } else {
                tmp = 0;
                for (j = 0; j < ONE_LINE_MAX_NUM; j++) {
                    tmp += sprintf(pbuffer + tmp, "%02x ", (uint8_t)buf[k*ONE_LINE_MAX_NUM+j]);
                }
                printf("[%s] %.*s\r\n", str, tmp, pbuffer);
            }
        }
    } else {
        /* 整数倍 */
        for (k = 0; k < m; k++) {
            tmp = 0;
            for (j = 0; j < ONE_LINE_MAX_NUM; j++) {
                tmp += sprintf(pbuffer + tmp, "%02x ", (uint8_t)buf[k*ONE_LINE_MAX_NUM+j]);
            }
            printf("[%s] %.*s\r\n", str, tmp, pbuffer);
        }
    }

    if (s_debug_mutex) {
        xSemaphoreGive(s_debug_mutex);
    }

    return 0;
}

void tpdbg_buf(const char *str, void *buf, uint32_t len)
{
    if ((NULL == buf) || (len == 0)) {
        printf("arg error buf = %p, len = %ld\r\n", buf, len);
        return;
    }

    buf_out(str, buf, len);
}

