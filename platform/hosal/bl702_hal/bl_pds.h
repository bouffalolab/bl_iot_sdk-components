#ifndef __BL_PDS_H__
#define __BL_PDS_H__


#include "bl702_romdriver.h"


#ifndef ATTR_PDS_SECTION
#define ATTR_PDS_SECTION           __attribute__((section(".pds_code." ATTR_UNI_SYMBOL), noinline))
#endif

#define PDS_WAKEUP_BY_SLEEP_CNT    1
#define PDS_WAKEUP_BY_GPIO         2


void bl_pds_init(void);
void bl_pds_gpio_wakeup_cfg(uint8_t pin_list[], uint8_t pin_num);  // available gpio: 0 - 7, 9 - 12
void bl_pds_gpio_wakeup_cfg_ex(uint32_t bitmap);  // one bit per gpio, only for pds0 - pds3
void bl_pds_fastboot_cfg(uint32_t addr);
int bl_pds_enter(uint32_t pdsLevel, uint32_t pdsSleepCycles);
void bl_pds_fastboot_done_callback(void);
int bl_pds_get_wakeup_source(void);  // return PDS_WAKEUP_BY_SLEEP_CNT or PDS_WAKEUP_BY_GPIO
uint32_t bl_pds_get_wakeup_gpio(void);  // return bitmap: bit 0 -> GPIO0 ... bit 31 -> GPIO31
void flash_powerdown(void);
void flash_restore(void);


#endif
