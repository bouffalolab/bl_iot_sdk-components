/**
 * Copyright (c) 2016-2021 Bouffalolab Co., Ltd.
 *
 * Contact information:
 * web site:    https://www.bouffalolab.com/
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


