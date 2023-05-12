#ifndef __ROM_HAL_EXT_H__
#define __ROM_HAL_EXT_H__


#include "bl702l_romdriver.h"


#define rom_sprintf                ((int (*)(char *str, const char *format, ...))0x2101107a)
#define rom_vsnprintf              ((int (*)(char *buffer, size_t n, const char *format, va_list ap))0x210110a8)

/* if a function is defined by ATTR_PDS_SECTION, then this function CANNOT call functions in Flash and it can only call
   1, functions that are defined in ROM
   2, functions that are also defined by ATTR_PDS_SECTION
 */
#define ATTR_PDS_SECTION           __attribute__((section(".pds_code." ATTR_UNI_SYMBOL)))

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


typedef int (*bl_romhook_t)(uint32_t arg[], void *ret);
typedef void (*bl_pds_cb_t)(void);


// Public Functions
void rom_hal_init(void);
void bl_pds_init(void);
void bl_pds_gpio_group_wakeup_cfg(uint8_t group, uint8_t pin_list[], uint8_t pin_num, uint8_t edge_sel);  // edge_sel: PDS_GPIO_EDGE_RISING, PDS_GPIO_EDGE_FALLING or PDS_GPIO_EDGE_BOTH
void bl_pds_gpio_wakeup_cfg(uint8_t pin_list[], uint8_t pin_num, uint8_t edge_sel);  // edge_sel: PDS_GPIO_EDGE_RISING, PDS_GPIO_EDGE_FALLING or PDS_GPIO_EDGE_BOTH
void bl_pds_key_wakeup_cfg(uint8_t row_num, uint8_t col_num, uint8_t row_pins[], uint8_t col_pins[]);  // row_num: 1 - 8; col_num: 1 - 8
void bl_pds_set_white_keys(uint8_t white_key_num, uint8_t row_idx[], uint8_t col_idx[]);  // set pushed keys as white keys; release of any white key will also trigger key wakeup; white_key_num: 0 - 4
void bl_pds_register_fastboot_done_callback(bl_pds_cb_t cb);
void bl_pds_fastboot_entry(void);
int bl_pds_get_wakeup_source(void);  // return PDS_WAKEUP_BY_RTC, PDS_WAKEUP_BY_GPIO, or PDS_WAKEUP_BY_KEY
uint32_t bl_pds_get_wakeup_gpio(void);  // return bit map: bit 0 -> GPIO0 ... bit 31 -> GPIO31
int bl_pds_get_wakeup_key_index(uint8_t *row_idx, uint8_t *col_idx);  // return PDS_KEY_EVENT_PRESS or PDS_KEY_EVENT_RELEASE
int bl_pds_wakeup_by_rtc(void);
uint16_t bl_rtc_process_xtal_cnt_32k(void);
void flash_powerdown(void);
void flash_restore(void);
bool flash_enable(uint8_t enable);
bool flash_is_enabled(void);
void uart_init(uint8_t tx_pin, uint8_t rx_pin, uint32_t baudrate);
int printf_ram(const char *format, ...);

// ROM Variables
extern void (**bl_irq_handler_list)(void);
extern uint32_t bl_pds_bak_addr;
extern int (*bl_pds_pre_process_callback)(uint32_t pdsLevel, uint32_t pdsSleepCycles, uint32_t *store, uint32_t arg[]);
extern int (*bl_pds_start_callback)(uint32_t pdsLevel, uint32_t pdsSleepCycles);
extern int (*bl_pds_post_process_callback)(uint32_t pdsLevel, uint32_t pdsSleepCycles, uint32_t reset, uint32_t arg[]);
extern uint16_t bl_rtc_frequency;
extern bl_romhook_t bl_timer_romhook;

// ROM Functions
extern void rom_bl_irq_enable(unsigned int source);
extern void rom_bl_irq_disable(unsigned int source);
extern void rom_bl_irq_pending_set(unsigned int source);
extern void rom_bl_irq_pending_clear(unsigned int source);
extern void rom_bl_irq_register(int irqnum, void *handler);
extern void rom_bl_irq_unregister(int irqnum, void *handler);
extern void rom_bl_irq_handler_get(int irqnum, void **handler);

extern void rom_bl_srand(uint32_t seed);
extern int rom_bl_rand(void);
extern void *rom_bl_memmove(void *dest, const void *src, size_t n);
extern void *rom_bl_memcpy(void *dest, const void *src, size_t n);
extern uint32_t *rom_bl_memcpy4(uint32_t *dest, const uint32_t *src, size_t n);
extern void *rom_bl_memset(void *s, int c, size_t n);
extern uint32_t *rom_bl_memset4(uint32_t *s, uint32_t val, size_t n);
extern int rom_bl_memcmp(const void *s1, const void *s2, size_t n);
extern char *rom_bl_strcpy(char *dest, const char *src);
extern size_t rom_bl_strlen(const char *s);
//extern int rom_bl_sprintf(char *str, const char *format, ...);
extern int rom_bl_clz(unsigned int x);
extern int rom_bl_ctz(unsigned int x);
extern uint64_t rom_bl_uint64_div16(uint64_t dividend, uint16_t divisor);
extern uint64_t rom_bl_uint64_mod16(uint64_t dividend, uint16_t divisor);
extern uint64_t rom_bl_uint64_mul16(uint64_t a, uint16_t b);

extern void rom_bl_pds_get_sf_ctrl_cfg(SPI_Flash_Cfg_Type *pFlashCfg, SF_Ctrl_Cfg_Type *pSfCtrlCfg);
extern void rom_bl_pds_restore_sf_swap(void);
extern void rom_bl_pds_restore_sf_gpio(void);
extern void rom_bl_pds_restore_sf_pad(void);
extern void rom_bl_pds_restore_flash(SF_Ctrl_Cfg_Type *pSfCtrlCfg, SPI_Flash_Cfg_Type *pFlashCfg, uint32_t flashImageOffset, uint8_t cacheWayDisable);
extern void rom_bl_pds_restore_data(uint32_t src, uint32_t dst, uint32_t end);
extern void rom_bl_pds_restore_cpu_reg(void);
extern void rom_bl_pds_fastboot_cfg(uint32_t addr);
extern void rom_bl_pds_start(void);
extern int rom_bl_pds_enter(uint32_t pdsLevel, uint32_t pdsSleepCycles);
extern void rom_bl_pds_set_wakeup_source(uint8_t src);
extern uint8_t rom_bl_pds_get_wakeup_event(void);
extern void rom_bl_pds_clear_interrupt_wakeup(void);
extern void rom_bl_pds_gpio_set_pu(uint32_t bitmap);
extern void rom_bl_pds_gpio_set_pd(uint32_t bitmap);
extern void rom_bl_pds_gpio_set_ie(uint32_t bitmap);
extern void rom_bl_pds_gpio_set_oe(uint32_t bitmap);
extern void rom_bl_pds_gpio_set_int_mode(uint8_t group, uint8_t mode);
extern void rom_bl_pds_gpio_set_int_mask(uint8_t group, uint8_t mask);
extern void rom_bl_pds_gpio_set_output(uint8_t group, uint8_t output);
extern uint32_t rom_bl_pds_gpio_get_int_status(void);
extern void rom_bl_pds_gpio_clear_int_status(void);
extern void rom_bl_pds_kyd_init(uint8_t row_num, uint8_t col_num, uint8_t col_out, uint8_t row_pull);
extern void rom_bl_pds_kyd_enable(void);
extern void rom_bl_pds_kyd_disable(void);
extern uint8_t rom_bl_pds_kyd_get_wakeup_key_index(uint8_t *row_index, uint8_t *col_index);
extern uint8_t rom_bl_pds_kyd_get_wakeup_key_gpio(uint8_t *row_gpio, uint8_t *col_gpio);
extern void rom_bl_pds_kyd_clear_wakeup(void);
extern void rom_bl_pds_kyd_set_white_key(uint8_t id, uint8_t row_index, uint8_t col_index, uint8_t mode);

extern uint64_t rom_bl_rtc_get_counter(void);
extern uint64_t rom_bl_rtc_get_delta_counter(uint64_t ref_cnt);
extern uint32_t rom_bl_rtc_counter_to_ms(uint32_t cnt);
extern uint32_t rom_bl_rtc_ms_to_counter(uint32_t ms);
extern void rom_bl_rtc_set_sleep_cycles(uint32_t cycles);
extern void rom_bl_rtc_trigger_xtal_cnt_32k(void);
extern uint8_t rom_bl_rtc_is_xtal_cnt_32k_done(void);
extern uint16_t rom_bl_rtc_get_xtal_cnt_32k_counter(uint8_t trigger);
extern uint16_t rom_bl_rtc_get_xtal_cnt_32k_frequency(uint8_t trigger);

extern void rom_bl_timer_init(void);
extern uint32_t rom_bl_timer_get_overflow_cnt(void);
extern uint32_t rom_bl_timer_get_current_time(void);
extern uint32_t rom_bl_timer_get_remaining_time(uint8_t ch);
extern uint32_t rom_bl_timer_is_active(uint8_t ch);
extern void rom_bl_timer_start(uint8_t ch, uint32_t target_time, void* cb);
extern void* rom_bl_timer_stop(uint8_t ch);
extern void rom_bl_timer_store(void);
extern void rom_bl_timer_store_time(void);
extern void rom_bl_timer_store_events(void);
extern void rom_bl_timer_restore(uint32_t jump_time, uint8_t run_expired);
extern void rom_bl_timer_restore_time(uint32_t jump_time);
extern void rom_bl_timer_restore_events(uint8_t run_expired);

extern uint64_t rom_hal_mtimer_now_us64(void);
extern void rom_hal_mtimer_delay_us64(uint64_t us);
extern uint32_t rom_hal_pds_enter_with_time_compensation(uint32_t pdsLevel, uint32_t pdsSleepCycles);


#endif
