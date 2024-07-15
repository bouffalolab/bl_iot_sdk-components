#ifndef __BL_IR_H__
#define __BL_IR_H__

#include "bl702_ir.h"
#include "bl702_glb.h"

void bl_ir_led_drv_cfg(uint8_t led0_en, uint8_t led1_en);

// normal mode
void bl_ir_custom_tx_cfg(IR_TxCfg_Type *txCfg, IR_TxPulseWidthCfg_Type *txPWCfg);
void bl_ir_nec_tx_cfg(void);
void bl_ir_rc5_tx_cfg(void);
void bl_ir_nec_tx(uint32_t wdata);
void bl_ir_rc5_tx(uint32_t wdata);

// software mode
void bl_ir_swm_tx_cfg(float freq_hz, float duty_cycle);
// k: coefficient, default 1
// data: number of carriers ranging from 1 to 16*k (should also be multiple of k)
// len: number of data ranging from 1 to 64
int bl_ir_swm_tx(uint8_t k, uint16_t data[], uint8_t len);
int bl_ir_swm_tx_busy(void);
void bl_ir_swm_tx_done_callback(void);

#endif
