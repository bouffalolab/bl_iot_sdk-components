#include <bl_gpio.h>
#include <bl702l_glb.h>
#include <bl702l_gpio.h>
#include <hosal_gpio.h>
#include <bl702l_common.h>
#include <blog.h>
#include <bl702l.h>
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

static int check_gpio_is_interrupt(GLB_GPIO_Type gpioPin)
{
    return GLB_Get_GPIO_IntStatus(gpioPin) == SET ? 0 : -1;
}

static void exec_gpio_handler(hosal_gpio_ctx_t *node)
{
    bl_gpio_intmask(node->pin, 1);

    if (node->handle) {
        node->handle(node->arg);
    }

    bl_gpio_intmask(node->pin, 0);
}

static void gpio_interrupt_entry(hosal_gpio_ctx_t **pstnode)
{
    int ret;
    hosal_gpio_ctx_t *node = *pstnode;

    while (node) {
        ret = check_gpio_is_interrupt(node->pin);
        if (ret == 0) {
            exec_gpio_handler(node);
        }

        node = node->next;
    }
}

int hosal_gpio_irq_set(hosal_gpio_dev_t *gpio, hosal_gpio_irq_trigger_t trigger_type, hosal_gpio_irq_handler_t handler, void *arg)
{
    if (gpio == NULL || trigger_type > 11) {
        blog_error("hosal irq register parameter is not correct!\r\n");
        return -1;
    }

    hosal_gpio_ctx_t *node = hosal_gpio_find_node(gpio);
    if (node == NULL) {
        blog_error("please hosal_gpio_init for gpio%d!\r\n", gpio->port);
        return -1;
    }

    node->handle = handler;
    node->arg = arg;

    bl_gpio_intmask(gpio->port, 1);
    bl_set_gpio_intmod(gpio->port, trigger_type);
    bl_irq_register_with_ctx(GPIO_INT0_IRQn, gpio_interrupt_entry, &gpio_head);
    bl_gpio_intmask(gpio->port, 0);
    bl_irq_enable(GPIO_INT0_IRQn);
    return 0;
}

int hosal_gpio_irq_mask(hosal_gpio_dev_t *gpio, uint8_t mask)
{
    if (gpio  == NULL) {
        return -1;
    }

    bl_gpio_intmask(gpio->port, mask ? 1 : 0);
    return 0;
}

int hosal_gpio_finalize(hosal_gpio_dev_t *gpio)
{
    if (gpio == NULL) {
        return -1;
    }

    hosal_gpio_ctx_t *node = hosal_gpio_find_node(gpio);
    if (node != NULL) {
        hosal_gpio_delete_node(node);
        free(node);
    }

    bl_gpio_intmask(gpio->port, 1);
    return 0;
}
