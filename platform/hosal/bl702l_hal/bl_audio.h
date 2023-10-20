#ifndef __BL_AUDIO_H__
#define __BL_AUDIO_H__

#include "bl702l_auadc.h"
#include "bl702l_glb.h"
#include "bl702l_dma.h"

typedef void (*bl_audio_callback_t)(int buf_idx);

// amic_single_ended: 0 - differential mode (pos + neg), 1 - single ended mode (pos only)
// amic_pos_ch/amic_neg_ch: 0 - 7, corresponding to gpio {2, 3, 7, 8, 10, 11, 14, 15}
// pcm_frame_size: 1 - 4095
typedef struct {
    uint8_t amic_single_ended;
    uint8_t amic_pos_ch;
    uint8_t amic_neg_ch;
    uint16_t pcm_frame_size;
    int16_t *pcm_frame_buf[2];
    bl_audio_callback_t pcm_frame_event;
}bl_audio_amic_cfg_t;

// pdm_clk_pin: 0, 2, 8
// pdm_in_pin: 1, 3, 7
// pcm_frame_size: 1 - 4095
typedef struct {
    uint8_t pdm_clk_pin;
    uint8_t pdm_in_pin;
    uint16_t pcm_frame_size;
    int16_t *pcm_frame_buf[2];
    bl_audio_callback_t pcm_frame_event;
}bl_audio_pdm_cfg_t;

int bl_audio_amic_init(bl_audio_amic_cfg_t *cfg);
int bl_audio_pdm_init(bl_audio_pdm_cfg_t *cfg);

int bl_audio_start(void);
int bl_audio_stop(void);

// 1-order high pass filter
//                  1 - z^-1
// H(z) = -----------------------------
//         1 - (1 - 2^-k1 - 2^-k2)z^-1
// k1: available range 3 - 13; set 0 to disable it; default 8
// k2: available range 3 - 13; set 0 to disable it; default disabled
void bl_audio_set_high_pass_filter(int k1, int k2);

void bl_audio_set_analog_gain(int gain_db);  // gain_db: 6dB to 42dB in 3dB step
void bl_audio_set_digital_gain(float gain_db);  // gain_db: -95.5dB to +18dB in 0.5dB step

int bl_audio_get_analog_gain(void);
float bl_audio_get_digital_gain(void);

#endif
