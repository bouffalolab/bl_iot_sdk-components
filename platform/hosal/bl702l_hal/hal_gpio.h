#ifndef __HAL_GPIO_H__
#define __HAL_GPIO_H__

typedef enum {
    GPIO_INT_TRIG_SYNC_FALLING_EDGE = 0,        /*!< GPIO interrupt sync mode, GPIO falling edge trigger interrupt */
    GPIO_INT_TRIG_SYNC_RISING_EDGE = 1,         /*!< GPIO interrupt sync mode, GPIO rising edge trigger interrupt */
    GPIO_INT_TRIG_SYNC_LOW_LEVEL = 2,           /*!< GPIO interrupt sync mode, GPIO low level trigger interrupt (32k 3T) */
    GPIO_INT_TRIG_SYNC_HIGH_LEVEL = 3,          /*!< GPIO interrupt sync mode, GPIO high level trigger interrupt (32k 3T) */
    GPIO_INT_TRIG_SYNC_FALLING_RISING_EDGE = 4, /*!< GPIO interrupt sync mode, GPIO falling and rising edge trigger interrupt */
    GPIO_INT_TRIG_ASYNC_FALLING_EDGE = 8,       /*!< GPIO interrupt async mode, GPIO falling edge trigger interrupt */
    GPIO_INT_TRIG_ASYNC_RISING_EDGE = 9,        /*!< GPIO interrupt async mode, GPIO rising edge trigger interrupt */
    GPIO_INT_TRIG_ASYNC_LOW_LEVEL = 10,         /*!< GPIO interrupt async mode, GPIO low level trigger interrupt (32k 3T) */
    GPIO_INT_TRIG_ASYNC_HIGH_LEVEL = 11,        /*!< GPIO interrupt async mode, GPIO high level trigger interrupt (32k 3T) */
} hal_gpio_int_trig_type;

int hal_gpio_register_handler(void *func, int gpioPin, int intTrgMod, void *arg);
int hal_gpio_init_from_dts(uint32_t fdt, uint32_t dtb_offset);
int hal_gpio_led_on(void);
int hal_gpio_led_off(void);

#endif
