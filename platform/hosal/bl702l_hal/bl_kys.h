#ifndef __BL_KYS_H__
#define __BL_KYS_H__

#include "bl702l_kys.h"
#include "bl702l_glb.h"

typedef struct {
    uint8_t ghost_det;
    uint8_t fifo_full;
    uint8_t key_num;
    uint8_t key_code[8];
    uint8_t row_idx[8];
    uint8_t col_idx[8];
}kys_result_t;

void bl_kys_init(uint8_t row_num, uint8_t col_num, uint8_t row_pins[], uint8_t col_pins[]);  // row_num: 1 - 8; col_num: 1 - 8
void bl_kys_trigger_poll(kys_result_t *result);
void bl_kys_trigger_interrupt(void);
void bl_kys_abort(void);
void bl_kys_interrupt_callback(const kys_result_t *result);

#endif
