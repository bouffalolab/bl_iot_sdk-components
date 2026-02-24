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

#include "blog.h"
#include "bl_irq.h"
#include "bl_adc.h"
#include "bl702_gpio.h"
#include "bl702_adc.h"
#include "bl702_dma.h"
#include "bl702_glb.h"
#include "hosal_adc.h"
#include "hosal_dma.h"

static hosal_adc_dev_t *pgdevice;

static int adc_get_channel_by_gpio(GLB_GPIO_Type pin)
{
    int channel = -1;

    switch (pin) {
        case GLB_GPIO_PIN_7:
            channel = 6;
            break;
        case GLB_GPIO_PIN_8:
            channel = 0;
            break;
        case GLB_GPIO_PIN_9:
            channel = 7;
            break;
        case GLB_GPIO_PIN_11:
            channel = 3;
            break;
        case GLB_GPIO_PIN_12:
            channel = 4;
            break;
        case GLB_GPIO_PIN_14:
            channel = 5;
            break;
        case GLB_GPIO_PIN_15:
            channel = 1;
            break;
        case GLB_GPIO_PIN_17:
            channel = 2;
            break;
        case GLB_GPIO_PIN_18:
            channel = 8;
            break;
        case GLB_GPIO_PIN_19:
            channel = 9;
            break;
        case GLB_GPIO_PIN_20:
            channel = 10;
            break;
        case GLB_GPIO_PIN_21:
            channel = 11;
            break;
        
        default :
            channel = -1;
            break;
    }

    return channel;
}


int hosal_adc_init(hosal_adc_dev_t *adc)
{
    if (NULL == adc) {
        blog_error("parameter is error!\r\n");
        return -1;
    }

    int ch = adc_get_channel_by_gpio(adc->config.pin);
    if (ch == -1) {
        blog_error("pin is error!\r\n");
        return -1;
    }

    bl_adc_init(1, ch, 0);

    pgdevice = adc;
    return 0;
}

int hosal_adc_add_channel(hosal_adc_dev_t *adc, uint32_t channel)
{
    return 0;
}

int hosal_adc_remove_channel(hosal_adc_dev_t *adc, uint32_t channel)
{
    return 0;
}

int hosal_adc_add_reference_channel(hosal_adc_dev_t *adc, uint32_t refer_channel, float refer_voltage)
{
    return 0;
}

int hosal_adc_remove_reference_channel(hosal_adc_dev_t *adc)
{
    return 0;
}

hosal_adc_dev_t *hosal_adc_device_get(void)
{
    if (NULL == pgdevice) {
        blog_error("please init adc first!\r\n");
        return NULL;
    }

    return pgdevice;
}

int hosal_adc_value_get(hosal_adc_dev_t *adc, uint32_t channel, uint32_t timeout)
{
    if (NULL == pgdevice) {
        blog_error("please init adc first!\r\n");
        return -1;
    }

    return (int)(bl_adc_get_val() * 1000);
}

int hosal_adc_tsen_value_get(hosal_adc_dev_t *adc)
{
    blog_error("not support now!\r\n");
    return -1;
}

int hosal_adc_sample_cb_reg(hosal_adc_dev_t *adc, hosal_adc_cb_t cb)
{
    log_error("not support now!\r\n");
    return -1;
}

int hosal_adc_start(hosal_adc_dev_t *adc, void *data, uint32_t size)
{
    return 0;
}

int hosal_adc_stop(hosal_adc_dev_t *adc)
{
   return 0;
}

int hosal_adc_finalize(hosal_adc_dev_t *adc)
{
    bl_adc_disable();

    pgdevice = NULL;
    return 0;
}


