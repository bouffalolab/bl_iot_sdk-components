#ifndef __ROM_BTBLE_EXT_H__
#define __ROM_BTBLE_EXT_H__

#include "hosal_gpio.h"
#include "FreeRTOS.h"

#define BTBLE_PDS_VERSION_NUMBER "1.6.1"
#define BTBLE_PDS_VERSION_MAJOR 1
#define BTBLE_PDS_VERSION_MINOR 6
#define BTBLE_PDS_VERSION_PATCH 1

typedef struct{
    uint8_t print_enable;//1: enable uart print in library; 0: disable uart print in library
    uint8_t gpio_irq_restore; //1: restore gpio irq after pds wakeup; 0: do not restore gpio irq after pds wakeup
    uint8_t gpio_num;
    uint8_t gpio_index[4];
    hosal_gpio_config_t pull_type[4];
    hosal_gpio_irq_trigger_t trigger_type[4];
}btble_app_conf_t;

void btble_pds_init(btble_app_conf_t *app_conf);
void btble_pds_enable(uint8_t enable);
void btble_vApplicationSleepExt(TickType_t xExpectedIdleTime);
#endif
