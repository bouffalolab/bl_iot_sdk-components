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
#include <bl_gpio.h>
#include <bl702_glb.h>
#include <bl702_gpio.h>
#include <hosal_gpio.h>
#include <bl702_common.h>
#include <blog.h>
#include <bl702.h>
#include <bl_irq.h>

static hosal_gpio_ctx_t *gpio_head = NULL;

static hosal_gpio_ctx_t *hosal_gpio_find_node(hosal_gpio_dev_t *gpio)
{
    hosal_gpio_ctx_t *ctx;

    for (ctx = gpio_head; ctx != NULL; ctx = ctx->next) {
        if (ctx->pin == gpio->port) {
            break;
        }
    }

    return ctx;
}

static void hosal_gpio_add_node(hosal_gpio_ctx_t *node)
{
    node->next = gpio_head;
    gpio_head = node;
}

static void hosal_gpio_delete_node(hosal_gpio_ctx_t *node)
{
    hosal_gpio_ctx_t *ctx;

    if (gpio_head == node) {
        gpio_head = node->next;
    } else {
        for (ctx = gpio_head; ctx != NULL; ctx = ctx->next) {
            if (ctx->next == node) {
                ctx->next = node->next;
                break;
            }
        }
    }
}

int hosal_gpio_init(hosal_gpio_dev_t *gpio)
{
    if (gpio == NULL) {
        return -1;
    }

    hosal_gpio_ctx_t *node = hosal_gpio_find_node(gpio);
    if (node == NULL) {
        node = malloc(sizeof(hosal_gpio_ctx_t));
        node->pin = gpio->port;
        node->handle = NULL;
        hosal_gpio_add_node(node);
    }

    switch (gpio->config) {
    case INPUT_PULL_DOWN:
        bl_gpio_enable_input(gpio->port, 0, 1);
        break;
    case INPUT_PULL_UP:
        bl_gpio_enable_input(gpio->port, 1, 0);
        break;
    case INPUT_HIGH_IMPEDANCE:
        bl_gpio_enable_input(gpio->port, 0, 0);
        break;
    case OUTPUT_PUSH_PULL:
        bl_gpio_enable_output(gpio->port, 1, 0);
        break;
    case OUTPUT_OPEN_DRAIN_NO_PULL:
        bl_gpio_enable_output(gpio->port, 0, 0);
        break;
    case OUTPUT_OPEN_DRAIN_PULL_UP:
        bl_gpio_enable_output(gpio->port, 0, 0);
        break;
    default:
        return -1;
    }
    return 0;
}

int hosal_gpio_output_set(hosal_gpio_dev_t *gpio, uint8_t value)
{
    if (gpio == NULL) {
        return -1;
    }

    bl_gpio_output_set(gpio->port, value);
    return 0;
}

int hosal_gpio_input_get(hosal_gpio_dev_t *gpio, uint8_t *value)
{
    if (gpio == NULL) {
        return -1;
    }

    bl_gpio_input_get(gpio->port, value);
    return 0;
}

static void set_gpio_intmask(hosal_gpio_ctx_t *node, uint8_t mask)
{
    GLB_GPIO_IntMask(node->pin, mask ? MASK : UNMASK);

    if (node->intTrigMod == HOSAL_IRQ_TRIG_NEG_POS_PULSE) {
        GLB_GPIO_Int2Mask(node->pin, mask ? MASK : UNMASK);
    }
}

static int check_gpio_is_interrupt(hosal_gpio_ctx_t *node)
{
    if (GLB_Get_GPIO_IntStatus(node->pin) == SET) {
        return 0;
    }

    if (node->intTrigMod == HOSAL_IRQ_TRIG_NEG_POS_PULSE) {
        if (GLB_Get_GPIO_Int2Status(node->pin) == SET) {
            return 0;
        }
    }

    return -1;
}

static void exec_gpio_handler(hosal_gpio_ctx_t *node)
{
    set_gpio_intmask(node, 1);

    if (node->handle) {
        node->handle(node->arg);
    }

    set_gpio_intmask(node, 0);
}

static void gpio_interrupt_entry(hosal_gpio_ctx_t **pstnode)
{
    int ret;
    hosal_gpio_ctx_t *node = *pstnode;

    while (node) {
        ret = check_gpio_is_interrupt(node);
        if (ret == 0) {
            exec_gpio_handler(node);
        }

        node = node->next;
    }
}

int hosal_gpio_irq_set(hosal_gpio_dev_t *gpio, hosal_gpio_irq_trigger_t trigger_type, hosal_gpio_irq_handler_t handler, void *arg)
{
    if (gpio == NULL || trigger_type > 4) {
        blog_error("hosal irq register parameter is not correct!\r\n");
        return -1;
    }

    hosal_gpio_ctx_t *node = hosal_gpio_find_node(gpio);
    if (node == NULL) {
        blog_error("please hosal_gpio_init for gpio%d!\r\n", gpio->port);
        return -1;
    }

    node->intTrigMod = trigger_type;
    node->handle = handler;
    node->arg = arg;

    if (trigger_type != HOSAL_IRQ_TRIG_NEG_POS_PULSE) {
        GLB_GPIO_IntMask(gpio->port, MASK);
        GLB_GPIO_Int2Mask(gpio->port, MASK);
        GLB_Set_GPIO_IntMod(gpio->port, GLB_GPIO_INT_CONTROL_ASYNC, (GLB_GPIO_INT_TRIG_Type)trigger_type);
        GLB_GPIO_IntMask(gpio->port, UNMASK);
    } else {
        GLB_GPIO_IntMask(gpio->port, MASK);
        GLB_GPIO_Int2Mask(gpio->port, MASK);
        GLB_Set_GPIO_IntMod(gpio->port, GLB_GPIO_INT_CONTROL_ASYNC, GLB_GPIO_INT_TRIG_NEG_PULSE);
        GLB_Set_GPIO_Int2Mod(gpio->port, GLB_GPIO_INT_CONTROL_ASYNC, GLB_GPIO_INT_TRIG_POS_PULSE);
        GLB_GPIO_IntMask(gpio->port, UNMASK);
        GLB_GPIO_Int2Mask(gpio->port, UNMASK);
    }

    bl_irq_register_with_ctx(GPIO_INT0_IRQn, gpio_interrupt_entry, &gpio_head);
    bl_irq_enable(GPIO_INT0_IRQn);
    return 0;
}

int hosal_gpio_irq_mask(hosal_gpio_dev_t *gpio, uint8_t mask)
{
    if (gpio == NULL) {
        return -1;
    }

    hosal_gpio_ctx_t *node = hosal_gpio_find_node(gpio);
    if (node == NULL) {
        blog_error("please hosal_gpio_init for gpio%d!\r\n", gpio->port);
        return -1;
    }

    set_gpio_intmask(node, mask);
    return 0;
}

int hosal_gpio_finalize(hosal_gpio_dev_t *gpio)
{
    if (gpio == NULL) {
        return -1;
    }

    hosal_gpio_ctx_t *node = hosal_gpio_find_node(gpio);
    if (node != NULL) {
        set_gpio_intmask(node, 1);
        hosal_gpio_delete_node(node);
        free(node);
    }

    return 0;
}
