#ifndef __BL_PDS_H__
#define __BL_PDS_H__


#include "bl702l_romdriver.h"


#ifndef ATTR_PDS_SECTION
#define ATTR_PDS_SECTION           __attribute__((section(".pds_code." ATTR_UNI_SYMBOL), noinline))
#endif

#define PDS_WAKEUP_BY_RTC          1
#define PDS_WAKEUP_BY_GPIO         2
#define PDS_WAKEUP_BY_KEY          3

#define PDS_GPIO_GROUP_0           0  // GPIO0/1/2/3 share same wakeup edge configuration
#define PDS_GPIO_GROUP_1           1  // GPIO7
#define PDS_GPIO_GROUP_2           2  // GPIO8
#define PDS_GPIO_GROUP_3           3  // GPIO14/15 share same wakeup edge configuration
#define PDS_GPIO_GROUP_4           4  // GPIO16/17/18/19 share same wakeup edge configuration
#define PDS_GPIO_GROUP_5           5  // GPIO20/21/22/23 share same wakeup edge configuration
#define PDS_GPIO_GROUP_6           6  // GPIO24/25/26/27 share same wakeup edge configuration
#define PDS_GPIO_GROUP_7           7  // GPIO28/29 share same wakeup edge configuration
#define PDS_GPIO_GROUP_8           8  // GPIO9/10/11/12/13/30/31 share same wakeup edge configuration

#define PDS_GPIO_EDGE_RISING       1
#define PDS_GPIO_EDGE_FALLING      0
#define PDS_GPIO_EDGE_BOTH         4

#define PDS_KEY_EVENT_PRESS        1
#define PDS_KEY_EVENT_RELEASE      2


typedef void (*bl_pds_cb_t)(void);


// Public Functions in Flash
void bl_pds_init(void);
void bl_pds_fastboot_cfg(uint32_t addr);
void bl_pds_set_psram_retention(uint8_t enable);
uint8_t bl_pds_get_gpio_group(uint8_t pin);
void bl_pds_gpio_group_wakeup_cfg(uint8_t group, uint8_t pin_list[], uint8_t pin_num, uint8_t edge_sel);  // edge_sel: PDS_GPIO_EDGE_RISING, PDS_GPIO_EDGE_FALLING or PDS_GPIO_EDGE_BOTH
void bl_pds_gpio_wakeup_cfg(uint8_t pin_list[], uint8_t pin_num, uint8_t edge_sel);  // edge_sel: PDS_GPIO_EDGE_RISING, PDS_GPIO_EDGE_FALLING or PDS_GPIO_EDGE_BOTH
void bl_pds_gpio_pull_get(uint32_t *pullup_bitmap, uint32_t *pulldown_bitmap);  // bitmap: bit 0 -> GPIO0 ... bit 31 -> GPIO31
void bl_pds_gpio_pull_set(uint32_t pullup_bitmap, uint32_t pulldown_bitmap);  // will enable gpio pu/pd when entering pds, and still take effect after wakeup
void bl_pds_gpio_pull_disable(void);  // disable gpio pu/pd, better after gpio reinitialization
void bl_pds_key_wakeup_cfg(uint8_t row_num, uint8_t col_num, uint8_t row_pins[], uint8_t col_pins[]);  // row_num: 1 - 8; col_num: 1 - 8
void bl_pds_set_white_keys(uint8_t white_key_num, uint8_t row_idx[], uint8_t col_idx[]);  // set pushed keys as white keys; release of any white key will also trigger key wakeup; white_key_num: 0 - 4
void bl_pds_register_fastboot_done_callback(bl_pds_cb_t cb);
uint32_t bl_pds_get_wakeup_gpio(void);  // return bitmap: bit 0 -> GPIO0 ... bit 31 -> GPIO31
int bl_pds_get_wakeup_key_index(uint8_t *row_idx, uint8_t *col_idx);  // return PDS_KEY_EVENT_PRESS or PDS_KEY_EVENT_RELEASE
int bl_pds_get_wakeup_key_code(uint8_t *key_code);  // return PDS_KEY_EVENT_PRESS or PDS_KEY_EVENT_RELEASE

// Public Functions in OCRAM
int bl_pds_enter(uint32_t pdsLevel, uint32_t pdsSleepCycles);
int bl_pds_get_wakeup_source(void);  // return PDS_WAKEUP_BY_RTC, PDS_WAKEUP_BY_GPIO, or PDS_WAKEUP_BY_KEY
int bl_pds_wakeup_by_rtc(void);
void bl_rtc_trigger_xtal_cnt_32k(void);
uint16_t bl_rtc_process_xtal_cnt_32k(void);
uint32_t bl_rtc_32k_to_32m(uint32_t cycles);
uint32_t bl_rtc_counter_to_ms(uint32_t cnt);
uint32_t bl_rtc_ms_to_counter(uint32_t ms);
void flash_powerdown(void);
void flash_restore(void);


#endif
