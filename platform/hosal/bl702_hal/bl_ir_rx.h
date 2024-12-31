#ifndef __BL_IR_RX_H__
#define __BL_IR_RX_H__

#include "bl702_ir.h"
#include "bl702_glb.h"

// rx_pin: 17 - 31
// rx_buf: rx buffer
// max_num: max number of high level and low level
// max_width: max width (time duration in us) of high level or low level to trigger end condition
void bl_ir_rx_cfg(uint8_t rx_pin, uint16_t *rx_buf, uint16_t max_num, uint16_t max_width);
int bl_ir_rx_start(void);
int bl_ir_rx_stop(void);
// data: width (time duration in us) of high level and low level alternately
// len: number of high level and low level
// full: rx_buf full flag (actual number of high level and low level exceeds max_num)
void bl_ir_rx_done_callback(uint16_t *data, uint16_t len, uint8_t full);

#endif
