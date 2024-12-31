#ifndef __BL_HBN_H__
#define __BL_HBN_H__


#include "bl702l_romdriver.h"
#include "bl702l_acomp.h"


#define ATTR_HBN_CODE_SECTION      __attribute__((section(".hbn_code." ATTR_UNI_SYMBOL), noinline))
#define ATTR_HBN_DATA_SECTION      __attribute__((section(".hbn_data")))
#define ATTR_HBN_NOINIT_SECTION    __attribute__((section(".hbn_noinit")))

#define HBN_WAKEUP_BY_RTC          1
#define HBN_WAKEUP_BY_GPIO         2
#define HBN_WAKEUP_BY_ACOMP0       3
#define HBN_WAKEUP_BY_ACOMP1       4

#define HBN_GPIO_EDGE_RISING       1
#define HBN_GPIO_EDGE_FALLING      0
#define HBN_GPIO_EDGE_BOTH         4

#define HBN_ACOMP_EDGE_RISING      1
#define HBN_ACOMP_EDGE_FALLING     2
#define HBN_ACOMP_EDGE_BOTH        3


void bl_hbn_fastboot_init(void);
void bl_hbn_gpio_wakeup_cfg(uint8_t pin_list[], uint8_t pin_num, uint8_t edge_sel);  // available gpio: 9, 10, 11, 12, 13, 30, 31
void bl_hbn_acomp_wakeup_cfg(uint8_t acomp_id, uint8_t ch_sel, uint8_t edge_sel);  // available channel: 0 - 7, corresponding to gpio {14, 15, 17, 18, 19, 20, 21, 7}
void bl_hbn_enter_with_fastboot(uint32_t hbnSleepCycles);
void bl_hbn_fastboot_done_callback(void);
int bl_hbn_get_wakeup_source(void);
uint32_t bl_hbn_get_wakeup_gpio(void);
uint64_t bl_hbn_get_wakeup_time(void);  // in rtc cycles


#endif
