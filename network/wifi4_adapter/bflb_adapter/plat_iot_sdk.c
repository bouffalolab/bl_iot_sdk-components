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
#include <assert.h>
#include <stdio.h>
#include "wifi_mgmr_ext.h"

#include "FreeRTOS.h"
#include "task.h"
#include "platform_al.h"
#include "stdbool.h"
#include "bl_irq.h"
#include "hal_sys.h"
#include "hosal_rng.h"
#include "bl_wifi.h"

int platform_register_event(int catalogue, pfn_wifi_event cb, void *arg)
{
    return aos_register_event_filter(catalogue, (aos_event_cb)cb, arg);
}

int platform_unregister_event(int catalogue, pfn_wifi_event cb, void *arg)
{
    return aos_unregister_event_filter(catalogue, (aos_event_cb)cb, arg);
}

/**
****************************************************************************************
* @brief Post Event to upper layer
*
* @param[in] catalogue Type of event.
* @param[in] code Code of event.
****************************************************************************************
*/

void platform_post_event(int catalogue, int code1, int code2)
{
    aos_post_event((uint16_t)catalogue, (uint16_t)code1, (uint16_t)code2);
}

int platform_wifi_enable_irq(void)
{
    bl_wifi_enable_irq();

    return 0;
}

void platform_post_delayed_action(int delay_ms, void (*callback)(void* arg), void *arg)
{
    aos_post_delayed_action(delay_ms, callback, arg);
}

void platform_sys_capcode_update(uint8_t capin, uint8_t capout)
{
    hal_sys_capcode_update(capin, capout);
}

uint8_t platform_sys_capcode_get(void)
{
    return hal_sys_capcode_get();
}

int platform_get_random(unsigned char *buf, size_t len)
{
    int ret;

    taskENTER_CRITICAL();
    ret = hosal_random_num_read(buf, len);
    taskEXIT_CRITICAL();
    return ret;
}

long platform_rand(void)
{
    return random();
}

#ifdef CFG_CHIP_BL602
__attribute__((weak)) void BL602_Delay_US(uint32_t cnt)
{
    arch_delay_us(cnt);
}

__attribute__((weak)) void BL602_Delay_MS(uint32_t cnt)
{
    ARCH_Delay_MS(cnt);
}
#endif
