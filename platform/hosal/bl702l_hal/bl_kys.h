#ifndef __BL_KYS_H__
#define __BL_KYS_H__


#include "bl702l_kys.h"
#include "bl702l_glb.h"


#define KEY_EVENT_NONE             0  // the key state is unchanged
#define KEY_EVENT_RELEASE          1  // the key state is changed from pressed state to released state
#define KEY_EVENT_PRESS            2  // the key state is changed from released state to pressed state


typedef struct {
    uint8_t ghost_det;
    uint8_t fifo_full;
    uint8_t key_num;
    uint8_t key_code[8];
    uint8_t row_idx[8];
    uint8_t col_idx[8];
}bl_kys_result_t;


typedef void (*bl_kys_cb_t)(bl_kys_result_t *result);


void bl_kys_init(uint8_t row_num, uint8_t col_num, uint8_t row_pins[], uint8_t col_pins[]);  // row_num: 1 - 8; col_num: 1 - 8
void bl_kys_trigger_poll(bl_kys_result_t *result);
void bl_kys_register_interrupt_callback(bl_kys_cb_t cb);
void bl_kys_trigger_interrupt(void);
void bl_kys_abort(void);

void bl_kys_update_event(const bl_kys_result_t *result);
int bl_kys_is_key_event_detected(void);
int bl_kys_get_key_event_by_coordinates(uint8_t row_idx, uint8_t col_idx);
int bl_kys_get_key_event_by_key_code(uint8_t key_code);


#endif
