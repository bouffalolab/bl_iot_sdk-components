/**
 * Copyright (c) 2016-2021 Bouffalolab Co., Ltd.
 *
 * Contact information:
 * web site:    https://www.bouffalolab.com/
 */

#ifndef __HOSAL_GPIO_H_
#define __HOSAL_GPIO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/** @addtogroup hosal_gpio GPIO
 *  HOSAL GPIO API
 *
 *  @{
 */

/**
 * @brief gpio config struct
 */
typedef enum {
    ANALOG_MODE,               /**< @brief Used as a function pin, input and output analog */
    INPUT_PULL_UP,             /**< @brief Input with an internal pull-up resistor - use with devices that actively drive the signal low - e.g. button connected to ground */
    INPUT_PULL_DOWN,           /**< @brief Input with an internal pull-down resistor - use with devices that actively drive the signal high - e.g. button connected to a power rail */
    INPUT_HIGH_IMPEDANCE,      /**< @brief Input - must always be driven, either actively or by an external pullup resistor */
    OUTPUT_PUSH_PULL,          /**< @brief Output actively driven high and actively driven low - must not be connected to other active outputs - e.g. LED output */
    OUTPUT_OPEN_DRAIN_NO_PULL, /**< @brief Output actively driven low but is high-impedance when set high - can be connected to other open-drain/open-collector outputs.  Needs an external pull-up resistor */
    OUTPUT_OPEN_DRAIN_PULL_UP, /**< @brief Output actively driven low and is pulled high with an internal resistor when set high - can be connected to other open-drain/open-collector outputs. */
    OUTPUT_OPEN_DRAIN_AF,      /**< @brief Alternate Function Open Drain Mode. */
    OUTPUT_PUSH_PULL_AF,       /**< @brief Alternate Function Push Pull Mode. */
} hosal_gpio_config_t;

/**
 * @brief GPIO interrupt trigger
 */
typedef enum {
    HOSAL_IRQ_TRIG_NEG_PULSE,        /**< @brief GPIO negedge pulse trigger interrupt */
    HOSAL_IRQ_TRIG_POS_PULSE,        /**< @brief GPIO posedge pulse trigger interrupt */
    HOSAL_IRQ_TRIG_NEG_LEVEL,        /**< @brief  GPIO negedge level trigger interrupt (32k 3T)*/
    HOSAL_IRQ_TRIG_POS_LEVEL,        /**< @brief  GPIO posedge level trigger interrupt (32k 3T)*/

#if defined(BL702L)
    HOSAL_IRQ_TRIG_SYNC_FALLING_EDGE = 0,        /*!< GPIO interrupt sync mode, GPIO falling edge trigger interrupt */
    HOSAL_IRQ_TRIG_SYNC_RISING_EDGE = 1,         /*!< GPIO interrupt sync mode, GPIO rising edge trigger interrupt */
    HOSAL_IRQ_TRIG_SYNC_LOW_LEVEL = 2,           /*!< GPIO interrupt sync mode, GPIO low level trigger interrupt (32k 3T) */
    HOSAL_IRQ_TRIG_SYNC_HIGH_LEVEL = 3,          /*!< GPIO interrupt sync mode, GPIO high level trigger interrupt (32k 3T) */
    HOSAL_IRQ_TRIG_SYNC_FALLING_RISING_EDGE = 4, /*!< GPIO interrupt sync mode, GPIO falling and rising edge trigger interrupt */
    HOSAL_IRQ_TRIG_ASYNC_FALLING_EDGE = 8,       /*!< GPIO interrupt async mode, GPIO falling edge trigger interrupt */
    HOSAL_IRQ_TRIG_ASYNC_RISING_EDGE = 9,        /*!< GPIO interrupt async mode, GPIO rising edge trigger interrupt */
    HOSAL_IRQ_TRIG_ASYNC_LOW_LEVEL = 10,         /*!< GPIO interrupt async mode, GPIO low level trigger interrupt (32k 3T) */
    HOSAL_IRQ_TRIG_ASYNC_HIGH_LEVEL = 11,        /*!< GPIO interrupt async mode, GPIO high level trigger interrupt (32k 3T) */
#endif

#if defined(BL616)
    HOSAL_IRQ_TRIG_SYNC_FALLING_EDGE = 10,        /*!< GPIO interrupt sync mode, GPIO falling edge trigger interrupt */
    HOSAL_IRQ_TRIG_SYNC_RISING_EDGE = 11,         /*!< GPIO interrupt sync mode, GPIO rising edge trigger interrupt */
    HOSAL_IRQ_TRIG_SYNC_LOW_LEVEL = 12,           /*!< GPIO interrupt sync mode, GPIO low level trigger interrupt (32k 3T) */
    HOSAL_IRQ_TRIG_SYNC_HIGH_LEVEL = 13,          /*!< GPIO interrupt sync mode, GPIO high level trigger interrupt (32k 3T) */
    HOSAL_IRQ_TRIG_SYNC_FALLING_RISING_EDGE = 14, /*!< GPIO interrupt sync mode, GPIO falling and rising edge trigger interrupt */
#endif
} hosal_gpio_irq_trigger_t;

/**
 * @brief GPIO interrupt callback handler
 *
 *@param[in] parg  ：Set the custom parameters specified
 */
typedef void (*hosal_gpio_irq_handler_t)(void *arg);

/**
 * @brief hosal gpio ctx, use for multi gpio irq
 */
typedef struct hosal_gpio_ctx {
    struct hosal_gpio_ctx *next;
    hosal_gpio_irq_handler_t handle;
    void *arg;
    uint8_t pin;
    uint8_t intCtrlMod;
    uint8_t intTrigMod;
}hosal_gpio_ctx_t;

/**
 * @brief GPIO dev struct
 */
typedef struct {
    uint8_t        port;         /**< @brief gpio port */
    hosal_gpio_config_t  config; /**< @brief gpio config */
    void          *priv;         /**< @brief priv data */
} hosal_gpio_dev_t;

/**
 * @brief Initialises a GPIO pin
 *
 * @note  Prepares a GPIO pin for use.
 *
 * @param[in]  gpio           the gpio pin which should be initialised
 *
 * @return  
 *	- 0    on success
 *	- EIO  if an error occurred with any step
 */
int hosal_gpio_init(hosal_gpio_dev_t *gpio);

/**
 * @brief Set GPIO output high or low
 *
 * @note  Using this function on a gpio pin which is set to input mode is undefined.
 *
 * @param[in]  gpio  the gpio pin which should be set 
 * @param[in]  value 0 : output low | >0 : output high
 *
 * @return  
 *	- 0    on success
 *	- EIO  if an error occurred with any step
 */
int hosal_gpio_output_set(hosal_gpio_dev_t *gpio, uint8_t value);

/**
 * @brief Get the state of an input GPIO pin. Using this function on a
 * gpio pin which is set to output mode will return an undefined value.
 *
 * @param[in]  gpio   the gpio pin which should be read
 * @param[out] value  gpio value
 *
 * @return  
 *	- 0    on success
 *	- EIO  if an error occurred with any step
 */
int hosal_gpio_input_get(hosal_gpio_dev_t *gpio, uint8_t *value);

/**
 * @brief Enables an interrupt trigger for an input GPIO pin.
 * Using this function on a gpio pin which is set to
 * output mode is undefined.
 *
 * @param[in]  gpio     the gpio pin which will provide the interrupt trigger
 * @param[in]  trigger  the type of trigger (rising/falling edge or both)
 * @param[in]  handler  a function pointer to the interrupt handler
 * @param[in]  arg      an argument that will be passed to the interrupt handler
 *
 * @return  
 *	- 0    on success
 *	- EIO  if an error occurred with any step
 */
int hosal_gpio_irq_set(hosal_gpio_dev_t *gpio, hosal_gpio_irq_trigger_t trigger_type, hosal_gpio_irq_handler_t handler, void *arg);

/**
 * @brief Mask or unmask an interrupt for an input GPIO pin.
 *
 * @param[in]  gpio  the gpio pin which provided the interrupt trigger
 * @param[in]  mask  0 : unmask (enable interrupt) | 1 : mask (disable interrupt)
 *
 * @return  
 *	- 0    on success
 *	- EIO  if an error occurred with any step
 */
int hosal_gpio_irq_mask(hosal_gpio_dev_t *gpio, uint8_t mask);

/**
 * @brief Set a GPIO pin in default state.
 *
 * @param[in]  gpio  the gpio pin which should be deinitialised
 *
 * @return  
 *	- 0    on success
 *	- EIO  if an error occurred with any step
 */
int hosal_gpio_finalize(hosal_gpio_dev_t *gpio);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* HAL_GPIO_H */

