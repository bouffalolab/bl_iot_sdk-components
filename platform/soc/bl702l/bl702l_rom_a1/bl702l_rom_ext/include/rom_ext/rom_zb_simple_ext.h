#ifndef __ROM_ZB_SIMPLE_EXT_H__
#define __ROM_ZB_SIMPLE_EXT_H__

#include "lmac154.h"
#include "hosal_gpio.h"

typedef struct{
    uint8_t uart_enable; //1: enable uart print in rom code; 0: disable uart print in rom code
    uint8_t uart_tx_pin;
    uint8_t uart_rx_pin;
    uint32_t uart_baudrate;
    uint8_t gpio_irq_restore; //1: restore gpio irq after pds wakeup; 0: do not restore gpio irq after pds wakeup
    uint8_t gpio_num;
    uint8_t gpio_index[4];
    hosal_gpio_config_t pull_type[4];
    hosal_gpio_irq_trigger_t trigger_type[4];
}zb_app_conf_t;

void rom_zb_simple_init(void);
void zb_pds_init(zb_app_conf_t *app_conf);
//enable=1:device will enter into low power mode when it is idle;
//enable=0:device won't enter into low power mode when it
void zb_pds_enable(uint8_t enable);
void zb_set_ota_status(bool otaOngoing);
#endif
