#ifndef _BL702L_RF_PUBLIC_H_
#define _BL702L_RF_PUBLIC_H_

#include <stdint.h>

uint32_t rf_set_init_tsen_value(int16_t tsen_value);
uint32_t rf_inc_cal_tsen_based(int16_t tsen_value);

// callback function, should be implemented by user
void rf_reset_done_callback(void);
void rf_full_cal_start_callback(uint32_t addr, uint32_t size);

#if CFG_FPGA
void rf_fpga_init();
#endif

/* RF API */
enum{
	MODE_BLE_ONLY = 0,
    MODE_ZB_ONLY = 1,
    MODE_BZ_COEX = 2,
};
void rf_set_bz_mode(uint8_t mode);

void rf_set_tx_bypass(uint8_t en);
uint8_t rf_get_rf_state(void);

#endif
