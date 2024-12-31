#ifndef __BL_ADC_H__
#define __BL_ADC_H__

#include <stdint.h>

typedef void(*bl_adc_tsen_callback_t)(int16_t tsen_val);

typedef struct bl_adc_tsen_cfg
{
    bl_adc_tsen_callback_t tsen_event;
}bl_adc_tsen_cfg_t;

typedef void(*bl_adc_voice_callback_t)(int buf_idx);

// adc_pos_ch/adc_neg_ch: 0 - 11, corresponding to gpio {8, 15, 17, 11, 12, 14, 7, 9, 18, 19, 20, 21}
// pcm_frame_size: 1 - 4095
typedef struct bl_adc_voice_cfg
{
    uint8_t adc_pos_ch;
    uint8_t adc_neg_ch;
    uint16_t pcm_frame_size;
    int16_t *pcm_frame_buf[2];
    bl_adc_voice_callback_t pcm_frame_event;
}bl_adc_voice_cfg_t;

// single_ended: 0 - differential mode (pos + neg), 1 - single ended mode (pos only)
// pos_ch/neg_ch: 0 - 11, corresponding to gpio {8, 15, 17, 11, 12, 14, 7, 9, 18, 19, 20, 21}
int bl_adc_init(uint8_t single_ended, uint8_t pos_ch, uint8_t neg_ch);
int bl_adc_get_conversion_result(void);
float bl_adc_get_val(void);
int bl_adc_disable(void);

int bl_adc_vbat_init(void);
float bl_adc_vbat_get_val(void);

int bl_adc_tsen_init(void);
int16_t bl_adc_tsen_get_val(void);

int bl_adc_tsen_dma_init(bl_adc_tsen_cfg_t *cfg);
int bl_adc_tsen_dma_trigger(void);
int bl_adc_tsen_dma_is_busy(void);

int bl_adc_voice_init(bl_adc_voice_cfg_t *cfg);
int bl_adc_voice_start(void);
int bl_adc_voice_stop(void);

#endif
