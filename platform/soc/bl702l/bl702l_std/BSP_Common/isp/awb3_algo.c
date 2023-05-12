#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "awb3_algo.h"
#include "bl_img_utils.h"
#include "img_param_gc2053.h"
#include "bl808_isp.h"

typedef struct blWB3_cal_S {
    int32_t g_divisor;
    int32_t r_ratio_ofst;
    int32_t b_ratio_ofst;
    int32_t r_ratio_mult;
    int32_t b_ratio_mult;
    int32_t ratio_sum_min;
    int32_t ratio_sum_max;
    int32_t weight_ratio[ISP_AWB3_WEIGHT_NUM][ISP_AWB3_WEIGHT_NUM];
} wb3_cal_t;

static void iir_filter(int32_t* iir, int32_t curr, int32_t* result);

#define PIXEL_BW                    (12)
#define PIXEL_MAX                   ((1 << PIXEL_BW) - 1)
#define GAIN_INT_BW                 (4)
#define GAIN_FRAC_BW                (PIXEL_BW)
#define GAIN_BW                     (GAIN_INT_BW + GAIN_FRAC_BW)
#define GAIN_UNITY                  (1 << GAIN_FRAC_BW)
#define RATIO_INT_BW                (4)
#define RATIO_FRAC_BW               (8)
#define RATIO_BW                    (RATIO_INT_BW + RATIO_FRAC_BW)
#define RATIO_MAX                   ((1 << RATIO_BW) - 1)
#define NORM_BW                     (8)
#define NORM_MAX                    ((1 << NORM_BW) - 1)
#define NORM_PRECISION              (5)
#define NORM_NUM                    ((1 << (NORM_BW - NORM_PRECISION)) + 1)
#define COEFF_INT_BW                (GAIN_INT_BW)
#define COEFF_FRAC_BW               (8)
#define COEFF_BW                    (COEFF_INT_BW + COEFF_FRAC_BW)
#define Y_BW                        (8)
#define Y_MAX                       ((1 << Y_BW) - 1)
#define Y_PRECISION                 (5)
#define Y_NUM                       ((1 << (Y_BW - Y_PRECISION)) + 1)
#define TEMP_PRECISION              (5)
#define TEMP_NUM                    ((1 << (NORM_BW - TEMP_PRECISION)) + 1)
#define WEIGHT_BW                   (8)
#define WEIGHT_MAX                  ((1 << WEIGHT_BW) - 1)
#define WEIGHT_GAIN_INT_BW          (4)
#define WEIGHT_GAIN_FRAC_BW         (4)
#define WEIGHT_GAIN_BW              (WEIGHT_GAIN_INT_BW + WEIGHT_GAIN_FRAC_BW)
#define BIN_LEVEL_MAX               (9)
#define BIN_NUM                     (1 << BIN_LEVEL_MAX)
#define PIXEL_BINNING_BW            (1)
#define PIXEL_NUM_BW                (21 - PIXEL_BINNING_BW) // (max. width * height in bit) - PIXEL_BINNING_B
#define CLIP_PRECISION              (16)
#define INK_BW                      (8)
#define INK_MAX                     ((1 << INK_BW) - 1)

// #define BIN_MODE_FULL                (0)
// #define BIN_MODE_RATIO               (1)
// #define BIN_MODE_Y                   (2)
// #define BIN_MODE_NONE                (3)

#define INK_TYPE_WEIGHT              (0)
#define INK_TYPE_WEIGHT_RATIO        (1)
#define INK_TYPE_WEIGHT_Y            (2)

#define GAIN_ALPHA_PRECISION         (8)
#define GAIN_ALPHA_MAX               ((1 << GAIN_ALPHA_PRECISION))
#define EFF_PIXEL_SLOPE_PRECISION    (20)
#define TEMPORAL_SMOOTH_PRECISION    (8)
#define TEMPORAL_IIR_EXTRA_PRECISION (4)
#define SPEED_LEVEL_PRECISION        (5)
#define PREFERENCE_GAIN_IDX_PRECISION  (54)

struct __awb3_ctx_t{
    awb_info_t        awb_info;
    uint32_t          awb_configed;
    uint32_t          awb_config_update;
    AWB_MODE          new_awb_mode;
    BL_COLOR_TEMP     new_color_temp;
    int               awb_start_skip_num;
};

static struct __awb3_ctx_t awb3_ctx;

struct Reg
{
    // control register
    int32_t stat_en; // r/w, shadow, 1'b
    int32_t stat_x_min; // r/w, shadow, 11'b, range: 0 ~ (width - 1), must be even
    int32_t stat_x_max; // r/w, shadow, 11'b, range: 0 ~ (width - 1), must be odd
    int32_t stat_y_min; // r/w, shadow, 11'b, range: 0 ~ (height - 1)
    int32_t stat_y_max; // r/w, shadow, 11'b, range: 0 ~ (height - 1)
    int32_t g_divisor; // r/w, shadow, 1'b
    int32_t r_ratio_ofst; // r/w, shadow, 12'b, = RATIO_BW, 4.8 fixed-point, range: 0 ~ (2 ^ 12 - 1)
    int32_t b_ratio_ofst; // r/w, shadow, 12'b, = RATIO_BW, 4.8 fixed-point, range: 0 ~ (2 ^ 12 - 1)
    int32_t r_ratio_mult; // r/w, shadow, 12'b, integer, range: 0 ~ (2 ^ 12 - 1)
    int32_t b_ratio_mult; // r/w, shadow, 12'b, integer, range: 0 ~ (2 ^ 12 - 1)
    int32_t ratio_sum_min;  // r/w, shadow, 9'b, = NORM_BW + 1, range: 0 ~ (2 ^ 9 - 1)
    int32_t ratio_sum_max; // r/w, shadow, 9'b, = NORM_BW + 1, range: 0 ~ (2 ^ 9 - 1)
    int32_t weight_ratio_en; // r/w, shadow, 1'b
    int32_t weight_ratio_gain; // r/w, shadow, 8'b, = WEIGHT_GAIN_BW, 4.4 fixed-point, range: 0 ~ (2 ^ 8 - 1)
    int32_t weight_ratio[NORM_NUM][NORM_NUM]; // r/w, shadow, 8'b, = WEIGHT_BW, range: 0 ~ (2 ^ 8 - 1)
    int32_t y_r_coeff; // r/w, shadow, 12'b, = COEFF_BW, 4.8 fixed-point, range: 0 ~ (2 ^ 12 - 1)
    int32_t y_g_coeff; // r/w, shadow, 12'b, = COEFF_BW, 4.8 fixed-point, range: 0 ~ (2 ^ 12 - 1)
    int32_t y_b_coeff; // r/w, shadow, 12'b, = COEFF_BW, 4.8 fixed-point, range: 0 ~ (2 ^ 12 - 1)
    int32_t weight_y_use_g; // r/w, shadow, 1'b
    int32_t weight_y_en; // r/w, shadow, 1'b
    int32_t weight_y_gain; // r/w, shadow, 8'b, = WEIGHT_GAIN_BW, 4.4 fixed-point, range: 0 ~ (2 ^ 8 - 1)
    int32_t weight_y[TEMP_NUM][Y_NUM]; // r/w, shadow, 8'b, = WEIGHT_BW, range: 0 ~ (2 ^ 8 - 1)
    int32_t count_zero_weight; // r/w, shadow, 1'b
    int32_t hist_y_use_g; // r/w, shadow, 1'b
    int32_t bin_y_level; // r/w, shadow, 3'b, range: 0 ~ 7
    int32_t bin_ratio_level; // r/w, shadow, 3'b, range: 0 ~ 4
    int32_t bin_clip_en; // r/w, shadow, 1'b
    int32_t bin_pixel_clip; // r/w, shadow, 20'b, = PIXEL_NUM_BW, range: 0 ~ (2 ^ 20 - 1)
    int32_t bin_mode; // r/w, shadow, 2'b, range: 0 ~ 3
    int32_t ink_type; // r/w, shadow, 2'b, range: 0 ~ 2
    int32_t ink_gain; // r/w, shadow, 4'b, integer, range: 0 ~ (2 ^ 4 - 1)

    // status register
    int64_t r_sum; // r, ping-pong, 32'b, = PIXEL_NUM_BW + PIXEL_BW
    int64_t g_sum; // r, ping-pong, 32'b, = PIXEL_NUM_BW + PIXEL_BW
    int64_t b_sum; // r, ping-pong, 32'b, = PIXEL_NUM_BW + PIXEL_BW
    int64_t r_sum_hex; // r, ping-pong, 32'b, = PIXEL_NUM_BW + PIXEL_BW
    int64_t g_sum_hex; // r, ping-pong, 32'b, = PIXEL_NUM_BW + PIXEL_BW
    int64_t b_sum_hex; // r, ping-pong, 32'b, = PIXEL_NUM_BW + PIXEL_BW
    int32_t pixel_hex; // r, ping-pong, 20'b, = PIXEL_NUM_BW
    int64_t r_ratio_hex_y; // r, ping-pong, 40'b, = PIXEL_NUM_BW + WEIGHT_BW + RATIO_BW
    int64_t b_ratio_hex_y; // r, ping-pong, 40'b, = PIXEL_NUM_BW + WEIGHT_BW + RATIO_BW
    int32_t weight_hex_y; // r, ping-pong, 28'b, = PIXEL_NUM_BW + WEIGHT_BW
    int32_t pixel_hex_y; // r, ping-pong, 20'b, = PIXEL_NUM_BW
    int64_t r_ratio_hist; // r, ping-pong, 40'b, = PIXEL_NUM_BW + WEIGHT_BW + RATIO_BW
    int64_t b_ratio_hist; // r, ping-pong, 40'b, = PIXEL_NUM_BW + WEIGHT_BW + RATIO_BW
    int32_t weight_hist; // r, ping-pong, 28'b, = PIXEL_NUM_BW + WEIGHT_BW
    int32_t pixel_hist; // r, ping-pong, 20'b, = PIXEL_NUM_BW
    int64_t r_ratio_hist_clip; // r, ping-pong, 40'b, = PIXEL_NUM_BW + WEIGHT_BW + RATIO_BW
    int64_t b_ratio_hist_clip; // r, ping-pong, 40'b, = PIXEL_NUM_BW + WEIGHT_BW + RATIO_BW
    int32_t weight_hist_clip; // r, ping-pong, 28'b, = PIXEL_NUM_BW + WEIGHT_BW
    int32_t pixel_hist_clip; // r, ping-pong, 20'b, = PIXEL_NUM_BW
};

struct Sram
{
    int64_t r_ratio[BIN_NUM]; // r, ping-pong, 40'b, = PIXEL_NUM_BW + WEIGHT_BW + RATIO_BW
    int64_t b_ratio[BIN_NUM]; // r, ping-pong, 40'b, = PIXEL_NUM_BW + WEIGHT_BW + RATIO_BW
    int32_t weight[BIN_NUM]; // r, ping-pong, 28'b, = PIXEL_NUM_BW + WEIGHT_BW
    int32_t pixel[BIN_NUM]; // r, ping-pong, 20'b, = PIXEL_NUM_BW
};

struct Reg_gain
{
    int32_t r_gain; // r/w, shadow, 16'b, = GAIN_BW, 4.12 fixed-point, range: 0 ~ (2 ^ 16 - 1)
    int32_t g_gain; // r/w, shadow, 16'b, = GAIN_BW, 4.12 fixed-point, range: 0 ~ (2 ^ 16 - 1)
    int32_t b_gain; // r/w, shadow, 16'b, = GAIN_BW, 4.12 fixed-point, range: 0 ~ (2 ^ 16 - 1)
};

struct Fw_param
{
    int32_t r_gain_default;
    int32_t b_gain_default;
    int32_t eff_pixel_th_min_ppm;
    int32_t eff_pixel_th_max_ppm;
    int32_t bias_r_gain;
    int32_t bias_b_gain;
    int32_t speed_level;
    int32_t temporal_smooth; // 0 ~ ((2 ^ TEMPORAL_SMOOTH_PRECISION) - 1)
    BL_GAIN_DB sys_gain;
};

struct Cal_tool
{
    // GC2053
    int32_t wb_gain_map[COLOR_TEMP_IDX_NUM][2];

    int32_t g_divisor;

    int32_t r_ratio_ofst;
    int32_t b_ratio_ofst;
    int32_t r_ratio_mult;
    int32_t b_ratio_mult;

    int32_t ratio_sum_min;
    int32_t ratio_sum_max;

    int32_t weight_ratio[NORM_NUM][NORM_NUM];

};

static struct Reg reg;

static struct Reg_gain reg_gain;
static struct Reg_gain fw_curr;
static struct Reg_gain fw_iir;

static struct Fw_param fw_param;
static isp_awb3_sum_t awb3_sum;

static int32_t width;
static int32_t height;

static int32_t eff_pixel_th_min;
static int32_t eff_pixel_th_max;
static int32_t eff_pixel_slope;

static int awb3_calib_bin_to_reg(wb3_cal_bin_t* cal_bin, wb3_cal_t* reg)
{
    int i, j, cnt = 0;
    
    reg->g_divisor = cal_bin->bin[0];
    reg->r_ratio_ofst = cal_bin->bin[1];
    reg->b_ratio_ofst = cal_bin->bin[2];
    reg->r_ratio_mult = cal_bin->bin[3];
    reg->b_ratio_mult = cal_bin->bin[4];
    reg->ratio_sum_min = cal_bin->bin[5];
    reg->ratio_sum_max = cal_bin->bin[6];
    for(j = 0; j < ISP_AWB3_WEIGHT_NUM; j++)
        for(i = 0; i < ISP_AWB3_WEIGHT_NUM; i++)
            reg->weight_ratio[j][i] = cal_bin->bin[7 + cnt++];
    return 0;
}

static int awb3_get_info(awb_info_t **info)
{
    *info = &(awb3_ctx.awb_info);
    return 0;
}

static void awb3_hw_init(isp_awb3_stats_conf_t *awb_stats_conf)
{
    const wb3_cal_bin_t *wb3_cal_bin;
    const wb3_cal_t wb3_cal;

    // typical register
    awb_stats_conf->stat_x_min = 0;
    awb_stats_conf->stat_x_max = width - 1;
    awb_stats_conf->stat_y_min = 0;
    awb_stats_conf->stat_y_max = height - 1;
    awb_stats_conf->weight_ratio_en = 1;
    awb_stats_conf->weight_ratio_gain = 1 << WEIGHT_GAIN_FRAC_BW;
    awb_stats_conf->y_r_coeff = 77;
    awb_stats_conf->y_g_coeff = 150;
    awb_stats_conf->y_b_coeff = 29;
    awb_stats_conf->weight_y_use_g = 1;
    awb_stats_conf->weight_y_en = 1;
    awb_stats_conf->weight_y_gain = 1 << WEIGHT_GAIN_FRAC_BW;

    for (int32_t it = 0; it < TEMP_NUM; it++) {
        for (int32_t iy = 0; iy < Y_NUM; iy++) {
            awb_stats_conf->weight_y[it][iy] = MIN(WEIGHT_MAX, iy << Y_PRECISION);
        }
    }

    awb_stats_conf->count_zero_weight = 0;
    awb_stats_conf->hist_y_use_g = 1;
    awb_stats_conf->bin_y_level = 3;
    awb_stats_conf->bin_ratio_level = 3;
    awb_stats_conf->bin_clip_en = 1;
    awb_stats_conf->bin_pixel_clip = (width * height) >> PIXEL_BINNING_BW;
    awb_stats_conf->bin_mode = AWB3_BIN_MODE_FULL;
    awb_stats_conf->ink_type = INK_TYPE_WEIGHT;
    awb_stats_conf->ink_gain = 1;

    reg_gain.r_gain = fw_param.r_gain_default;
    reg_gain.g_gain = GAIN_UNITY;
    reg_gain.b_gain = fw_param.b_gain_default;

	wb3_cal_bin = (const wb3_cal_bin_t *)&wb3_cal_use.wb3_cal.bin;
    awb3_calib_bin_to_reg((wb3_cal_bin_t*)wb3_cal_bin, (wb3_cal_t*)&wb3_cal);

    // register from sensor calibration tool
    // TODO: optimize when verificaition done
    awb_stats_conf->g_divisor     = wb3_cal.g_divisor;
    awb_stats_conf->r_ratio_ofst  = wb3_cal.r_ratio_ofst;
    awb_stats_conf->b_ratio_ofst  = wb3_cal.b_ratio_ofst;
    awb_stats_conf->r_ratio_mult  = wb3_cal.r_ratio_mult;
    awb_stats_conf->b_ratio_mult  = wb3_cal.b_ratio_mult;
    awb_stats_conf->ratio_sum_min = wb3_cal.ratio_sum_min;
    awb_stats_conf->ratio_sum_max = wb3_cal.ratio_sum_max;

    for (int32_t ib = 0; ib < NORM_NUM; ib++) {
        for (int32_t ir = 0; ir < NORM_NUM; ir++) {
          awb_stats_conf->weight_ratio[ib][ir] = wb3_cal.weight_ratio[ib][ir];
        }
    }
}

static void awb3_set_gain_default()
{
    ISP_RGB_Gain_Type rgb_gain;
    rgb_gain.r_gain = fw_param.r_gain_default;
    rgb_gain.g_gain = GAIN_UNITY;
    rgb_gain.b_gain = fw_param.b_gain_default;
    ISP_Set_AWB_Gain(&rgb_gain);
}

static int awb3_config(awb_config_t *config, bool is_virtual_sensor)
{
    if (is_virtual_sensor) {
        fw_param.temporal_smooth = 0;
    }
    isp_awb3_stats_conf_t awb_stats_conf;
    // awb_stats_enable(&(config->stats_cfg));
    // awb3_ctx.awb_configed = 1;

    width  = config->stats_cfg.act_win.width;
    height = config->stats_cfg.act_win.height;

    // calculate firmware variable
    eff_pixel_th_min = (width * height * fw_param.eff_pixel_th_min_ppm) >> (PIXEL_BINNING_BW + 20);
    eff_pixel_th_max = (width * height * fw_param.eff_pixel_th_max_ppm) >> (PIXEL_BINNING_BW + 20);
    eff_pixel_slope = (1 << (GAIN_ALPHA_PRECISION + EFF_PIXEL_SLOPE_PRECISION)) / (eff_pixel_th_max - eff_pixel_th_min);

    awb3_hw_init(&awb_stats_conf);
    awb3_set_gain_default();
    ISP_Set_AWB3_Stats_Conf(&awb_stats_conf);
    ISP_Set_AWB3_Hist_State(ENABLE);
    // TODO: rename
    //ISP_Set_AWB2_Gain_State(ENABLE);

    return 0;
}

static void calculate_sum_gain(int64_t r_sum, int64_t g_sum, int64_t b_sum, int32_t *r_gain, int32_t *b_gain)
{
    *r_gain = (int32_t)(((int64_t)g_sum << GAIN_FRAC_BW) / MAX(1, r_sum));
    *b_gain = (int32_t)(((int64_t)g_sum << GAIN_FRAC_BW) / MAX(1, b_sum));
}

static void calculate_ratio_gain(int32_t g_divisor, int64_t r_ratio, int64_t b_ratio, int32_t weight, int32_t* r_gain, int32_t* b_gain)
{
    if (g_divisor == 0) {
        *r_gain = (int32_t)((r_ratio << (GAIN_FRAC_BW - RATIO_FRAC_BW)) / MAX(1, weight));
        *b_gain = (int32_t)((b_ratio << (GAIN_FRAC_BW - RATIO_FRAC_BW)) / MAX(1, weight));
    }
    else {
        *r_gain = (int32_t)(((int64_t)weight << (GAIN_FRAC_BW + RATIO_FRAC_BW)) / MAX(1, r_ratio));
        *b_gain = (int32_t)(((int64_t)weight << (GAIN_FRAC_BW + RATIO_FRAC_BW)) / MAX(1, b_ratio));
    }
}

static void decide_awb_gain()
{
    // calculate 3 gains from 3 statistics with different white point strictness
    int32_t r_gain_hex;
    int32_t b_gain_hex;
    int32_t r_gain_hex_y;
    int32_t b_gain_hex_y;
    int32_t r_gain_hist_clip;
    int32_t b_gain_hist_clip;

    calculate_sum_gain(reg.r_sum_hex, reg.g_sum_hex, reg.b_sum_hex, &r_gain_hex, &b_gain_hex);
    calculate_ratio_gain(reg.g_divisor, reg.r_ratio_hex_y, reg.b_ratio_hex_y, reg.weight_hex_y, &r_gain_hex_y, &b_gain_hex_y);
    calculate_ratio_gain(reg.g_divisor, reg.r_ratio_hist_clip, reg.b_ratio_hist_clip, reg.weight_hist_clip, &r_gain_hist_clip, &b_gain_hist_clip);



    // blending priority: hist_clip > hex_y > hex > default
    int32_t r_gain_blend;
    int32_t b_gain_blend;

    int32_t alpha_hex = clip(0, GAIN_ALPHA_MAX, (int32_t)((((int64_t)reg.pixel_hex - eff_pixel_th_min) * eff_pixel_slope) >> EFF_PIXEL_SLOPE_PRECISION));
    int32_t alpha_hex_y = clip(0, GAIN_ALPHA_MAX, (int32_t)((((int64_t)reg.pixel_hex_y - eff_pixel_th_min) * eff_pixel_slope) >> EFF_PIXEL_SLOPE_PRECISION));
    int32_t alpha_hist_clip = clip(0, GAIN_ALPHA_MAX, (int32_t)((((int64_t)reg.pixel_hist_clip - eff_pixel_th_min) * eff_pixel_slope) >> EFF_PIXEL_SLOPE_PRECISION));

    r_gain_blend = alpha_blend(r_gain_hex, fw_param.r_gain_default, alpha_hex, GAIN_ALPHA_PRECISION);
    b_gain_blend = alpha_blend(b_gain_hex, fw_param.b_gain_default, alpha_hex, GAIN_ALPHA_PRECISION);

    r_gain_blend = alpha_blend(r_gain_hex_y, r_gain_blend, alpha_hex_y, GAIN_ALPHA_PRECISION);
    b_gain_blend = alpha_blend(b_gain_hex_y, b_gain_blend, alpha_hex_y, GAIN_ALPHA_PRECISION);

    r_gain_blend = alpha_blend(r_gain_hist_clip, r_gain_blend, alpha_hist_clip, GAIN_ALPHA_PRECISION);
    b_gain_blend = alpha_blend(b_gain_hist_clip, b_gain_blend, alpha_hist_clip, GAIN_ALPHA_PRECISION);

    fw_curr.r_gain = r_gain_blend;
    fw_curr.b_gain = b_gain_blend;

}

static void temporal_smooth()
{
   iir_filter(&(fw_iir.r_gain), fw_curr.r_gain, &(reg_gain.r_gain));
   iir_filter(&(fw_iir.b_gain), fw_curr.b_gain, &(reg_gain.b_gain));
}

static void iir_filter(int32_t* iir, int32_t curr, int32_t* result)
{
    if (fw_param.speed_level == 5) fw_param.temporal_smooth = 51;//0.2
    else if (fw_param.speed_level == 4) fw_param.temporal_smooth = 179;//0.7
    else if (fw_param.speed_level == 3) fw_param.temporal_smooth = 204;//0.8
    else if (fw_param.speed_level == 2) fw_param.temporal_smooth = 230;//0.9
    else fw_param.temporal_smooth = 243;//0.95

    *iir = alpha_blend(*iir, curr << TEMPORAL_IIR_EXTRA_PRECISION, fw_param.temporal_smooth, TEMPORAL_SMOOTH_PRECISION);
    *result = round_off(*iir, TEMPORAL_IIR_EXTRA_PRECISION);
}

static void linear_intpl_by_gain(int value_L, int value_H, BL_GAIN_DB gain, int *value_out)
{
    BL_GAIN_DB gain_L = (gain / 1536) * 1536;
    BL_GAIN_DB gain_H = gain_L + 1536;

    intpl_point_t point_L = create_intpl_point(gain_L, value_L);
    intpl_point_t point_H = create_intpl_point(gain_H, value_H);
    linear_interpolate(&point_L, &point_H, gain, value_out);
}

static int calc_awb_fw_config_by_sys_gain(BL_GAIN_DB sys_gain, BL_COLOR_TEMP color_temp, ISP_RB_Gain_Type *rb_bias)
{
    int i = 0;
    int gain_idx;
    int ct_idx = 2;
    const awb_bais_t *awb_bias_L, *awb_bias_H, *awb_bias_P;
    intpl_point_t point_L, point_H;
    int num_awb_bias = 19;
    int ct_idx_tbl[] = {2700, 5000, 7000};
    int num_ct = 3;
    int sys_gain_db;

	awb_bias_P = &awb_fw_use.awb_auto_fw.preference_gain[0];
    for (i = 0; i < num_ct - 1; i++) {
        if (color_temp <= ct_idx_tbl[i]) {
            ct_idx = i;
            break;
        }
    }
    //MSG("============= ct_idx %d color_temp %d sys_gain %d\r\n", ct_idx, color_temp, sys_gain);
    if (sys_gain > 69120) {// 54 * 1536
		awb_bias_P = &awb_fw_use.awb_auto_fw.preference_gain[num_awb_bias - 1];
    } else if (sys_gain < -69120) {// -54 * 1536
		awb_bias_P = &awb_fw_use.awb_auto_fw.preference_gain[0];
    } else {

		//awb_bias_P = &awb_fw_use.awb_auto_fw.preference_gain[0];

        gain_idx = (sys_gain / 1536) + 9;

		awb_bias_L = &awb_fw_use.awb_auto_fw.preference_gain[gain_idx];
		awb_bias_H = &awb_fw_use.awb_auto_fw.preference_gain[gain_idx + 1];
       // MSG("============= gain_idx %d awb_bias %d %d\r\n", gain_idx, awb_bias_L->bais[ct_idx].r_gain, awb_bias_H->bais[ct_idx].r_gain);

        if (ct_idx == num_ct - 1) {
            sys_gain_db = sys_gain/ 1536;
            linear_intpl_by_gain((int)awb_bias_L->bais[ct_idx].r_gain, (int)awb_bias_H->bais[ct_idx].r_gain, sys_gain_db, (int *)&awb_bias_P->bais[ct_idx].r_gain);
           // MSG("============= 2 awb_bias %d\r\n", awb_bias_P.bais[ct_idx].r_gain);
            linear_intpl_by_gain((int)awb_bias_L->bais[ct_idx].b_gain, (int)awb_bias_H->bais[ct_idx].b_gain, sys_gain_db, (int *)&awb_bias_P->bais[ct_idx].b_gain);
            //MSG("============= 2 awb_bias %d %d\r\n", awb_bias_P.bais[ct_idx].r_gain, awb_bias_P->bais[ct_idx].b_gain);
        } else {
            linear_intpl_by_gain((int)awb_bias_L->bais[ct_idx].r_gain, (int)awb_bias_H->bais[ct_idx].r_gain, sys_gain, (int *)&awb_bias_P->bais[ct_idx].r_gain);
            linear_intpl_by_gain((int)awb_bias_L->bais[ct_idx].b_gain, (int)awb_bias_H->bais[ct_idx].b_gain, sys_gain, (int *)&awb_bias_P->bais[ct_idx].b_gain);
            linear_intpl_by_gain((int)awb_bias_L->bais[ct_idx+1].r_gain, (int)awb_bias_H->bais[ct_idx+1].r_gain, sys_gain, (int *)&awb_bias_P->bais[ct_idx+1].r_gain);
            linear_intpl_by_gain((int)awb_bias_L->bais[ct_idx+1].b_gain, (int)awb_bias_H->bais[ct_idx+1].b_gain, sys_gain, (int *)&awb_bias_P->bais[ct_idx+1].b_gain);
            //MSG("=============  awb_bias %d %d\r\n", awb_bias_P.bais[ct_idx].r_gain, awb_bias_P->bais[ct_idx].b_gain);
        }
        //MSG("============= awb_bias_P %d %d\r\n", awb_bias_P.bais[ct_idx].r_gain, awb_bias_P->bais[ct_idx+1].r_gain);

    }

    if (ct_idx == num_ct - 1) {
        rb_bias->r_gain = awb_bias_P->bais[ct_idx].r_gain;
        rb_bias->b_gain = awb_bias_P->bais[ct_idx].b_gain;
        //MSG("a ============= rb_bias %d %d\r\n", rb_bias->r_gain, rb_bias->b_gain);
        return 0;
    }

    /* interpolate r_gain */
    point_L = create_intpl_point(ct_idx_tbl[ct_idx],   awb_bias_P->bais[ct_idx].r_gain);
    point_H = create_intpl_point(ct_idx_tbl[ct_idx+1], awb_bias_P->bais[ct_idx+1].r_gain);
    linear_interpolate(&point_L, &point_H, color_temp, (int*)&rb_bias->r_gain);

    /* interpolate r_gain */
    point_L = create_intpl_point(ct_idx_tbl[ct_idx],   awb_bias_P->bais[ct_idx].b_gain);
    point_H = create_intpl_point(ct_idx_tbl[ct_idx+1], awb_bias_P->bais[ct_idx+1].b_gain);
    linear_interpolate(&point_L, &point_H, color_temp, (int*)&rb_bias->b_gain);
    //MSG("============= rb_bias %d %d\r\n", rb_bias->r_gain, rb_bias->b_gain);

    return 0;
}

static void apply_preference_gain() 
{
    ISP_RB_Gain_Type rb_bias;
    calc_awb_fw_config_by_sys_gain(fw_param.sys_gain, awb3_ctx.awb_info.color_temp, &rb_bias);

    fw_param.bias_r_gain = MIN(MAX(rb_bias.r_gain, 2048), 8192);
    fw_param.bias_b_gain = MIN(MAX(rb_bias.b_gain, 2048), 8192);
    reg_gain.r_gain = (reg_gain.r_gain * fw_param.bias_r_gain) >> GAIN_FRAC_BW;
    reg_gain.b_gain = (reg_gain.b_gain * fw_param.bias_b_gain) >> GAIN_FRAC_BW;
}

static void awb3_get_g_divisor()
{
    int value;
    ISP_Get_AWB3_G_Divisor(&value);
    reg.g_divisor = value;
}

static void awb3_get_stats()
{
    ISP_Get_AWB3_Sum(&awb3_sum);

    // TODO: optimize awb3_sum as member of reg when verificaition done
    reg.r_sum             =    awb3_sum.r_sum;
    reg.g_sum             =    awb3_sum.g_sum;
    reg.b_sum             =    awb3_sum.b_sum;
    reg.r_sum_hex         =    awb3_sum.r_sum_hex;
    reg.g_sum_hex         =    awb3_sum.g_sum_hex;
    reg.b_sum_hex         =    awb3_sum.b_sum_hex;
    reg.pixel_hex         =    awb3_sum.pixel_hex;
    reg.r_ratio_hex_y     =    awb3_sum.r_ratio_hex_y;
    reg.b_ratio_hex_y     =    awb3_sum.b_ratio_hex_y;
    reg.weight_hex_y      =    awb3_sum.weight_hex_y;
    reg.pixel_hex_y       =    awb3_sum.pixel_hex_y;
    reg.r_ratio_hist      =    awb3_sum.r_ratio_hist;
    reg.b_ratio_hist      =    awb3_sum.b_ratio_hist;
    reg.weight_hist       =    awb3_sum.weight_hist;
    reg.pixel_hist        =    awb3_sum.pixel_hist;
    reg.r_ratio_hist_clip =    awb3_sum.r_ratio_hist_clip;
    reg.b_ratio_hist_clip =    awb3_sum.b_ratio_hist_clip;
    reg.weight_hist_clip  =    awb3_sum.weight_hist_clip;
    reg.pixel_hist_clip   =    awb3_sum.pixel_hist_clip;
}

static int32_t calculate_rb_gain_by_color_temp(int32_t color_temp, int32_t *r_gain, int32_t *b_gain)
{
    int32_t ic;
    intpl_point_t point_L, point_H;

    const wb_info_t *wb_info_L, *wb_info_H;

    for (ic = 0; ic < COLOR_TEMP_IDX_NUM; ic++) {
		wb_info_H = &wb_info_use.wb_info[ic];

        if (color_temp <= wb_info_H->color_temp) {
          break;
        }
    }

    if (ic == 0) {
        *r_gain = wb_info_H->rb_gain.r_gain;
        *b_gain = wb_info_H->rb_gain.b_gain;
    } else if (ic == COLOR_TEMP_IDX_NUM) {
        *r_gain = wb_info_H->rb_gain.r_gain;
        *b_gain = wb_info_H->rb_gain.b_gain;
    } else {
		wb_info_L = &wb_info_use.wb_info[ic - 1];

        point_L = create_intpl_point(wb_info_L->color_temp, wb_info_L->rb_gain.r_gain);
        point_H = create_intpl_point(wb_info_H->color_temp, wb_info_H->rb_gain.r_gain);
        linear_interpolate(&point_L, &point_H, color_temp, (int*)r_gain);

        point_L = create_intpl_point(wb_info_L->color_temp, wb_info_L->rb_gain.b_gain);
        point_H = create_intpl_point(wb_info_H->color_temp, wb_info_H->rb_gain.b_gain);
        linear_interpolate(&point_L, &point_H, color_temp, (int*)b_gain);
    }

    return 0;
}

static int32_t calculate_color_temp(int32_t r_gain, int32_t b_gain)
{
    int32_t color_temp;
    int32_t diff_cal[COLOR_TEMP_IDX_NUM];
    int32_t diff;
    int32_t ic;

    const wb_info_t *wb_info_L, *wb_info_H;

    diff = r_gain - b_gain;

    for (ic = 0; ic < COLOR_TEMP_IDX_NUM; ic++) {
        // diff_cal[ic] = wb_gain_map[ic][0] - wb_gain_map[ic][1];
		wb_info_H = &wb_info_use.wb_info[ic];
        diff_cal[ic] = wb_info_H->rb_gain.r_gain - wb_info_H->rb_gain.b_gain;

        if (diff < diff_cal[ic]) {
          break;
        }
    }

    if (ic == 0) {
        // color_temp = wb_gain_map[0][2];
		wb_info_H = &wb_info_use.wb_info[ic];
        color_temp = wb_info_H->color_temp;
    }
    else if (ic == COLOR_TEMP_IDX_NUM) {
        // color_temp = wb_gain_map[COLOR_TEMP_NUM - 1][2];
		wb_info_H = &wb_info_use.wb_info[COLOR_TEMP_IDX_NUM - 1];
        color_temp = wb_info_H->color_temp;
    }
    else {
        // color_temp = ((diff_cal[ic] - diff) * wb_gain_map[ic - 1][2] + (diff - diff_cal[ic - 1]) * wb_gain_map[ic][2]) / (diff_cal[ic] - diff_cal[ic - 1]);
		wb_info_L = &wb_info_use.wb_info[ic - 1];
		wb_info_H = &wb_info_use.wb_info[ic];
        color_temp = ((diff_cal[ic] - diff) * wb_info_L->color_temp \
            + (diff - diff_cal[ic - 1]) * wb_info_H->color_temp) \
            / (diff_cal[ic] - diff_cal[ic - 1]);
    }

    return color_temp;
}

static int awb3_set_gain(ISP_RGB_Gain_Type *rgb_gain)
{
    ISP_Set_AWB_Gain(rgb_gain);

    awb3_ctx.awb_info.rb_gain.r_gain = rgb_gain->r_gain;
    awb3_ctx.awb_info.rb_gain.b_gain = rgb_gain->b_gain;
    awb3_ctx.awb_info.rb_gain_update = 1;

    return 0;
}

static int awb3_get_gain(ISP_RGB_Gain_Type *rgb_gain)
{
    ISP_Get_AWB_Gain(rgb_gain);

    return 0;
}

/* for debug purpose only */
static int awb3_set_color_temp(BL_COLOR_TEMP color_temp)
{
    awb3_ctx.new_color_temp = color_temp;

    awb3_ctx.awb_config_update |= AWB_UPDATE_BIT_COLOR_TEMP;

    return 0;
}

static int awb3_get_color_temp(BL_COLOR_TEMP *color_temp)
{
    *color_temp = (BL_COLOR_TEMP)(awb3_ctx.awb_info.color_temp);

    return 0;
}
static int awb3_set_speed_level(int level)
{
    if (!is_param_valid(level, 1, SPEED_LEVEL_PRECISION)) {
        BL_LOGE("awb3_set_speed_level: invalid AWB speed level %d!\r\n", level);
        return -1;
    }

    fw_param.speed_level = level;

    return 0;
}

static int awb3_get_speed_level(int *level)
{
    *level = (int)(fw_param.speed_level);

    return 0;
}

static int awb3_set_sys_gain(BL_GAIN_DB sys_gain)
{
    if (!is_param_valid(sys_gain, -PREFERENCE_GAIN_IDX_PRECISION * 1536, PREFERENCE_GAIN_IDX_PRECISION *1536)) {
        BL_LOGE("awb3_set_sys_gain: invalid AWB set sys gain %d!\r\n", sys_gain);
        return -1;
    }

    fw_param.sys_gain = sys_gain;

    return 0;
}

static int awb3_set_mode(AWB_MODE mode)
{
    if (!is_param_valid(mode, AWB_MODE_AUTO, AWB_MODE_NUM - 1)) {
        BL_LOGE("awb3_set_mode: invalid AWB mode %d!\r\n", mode);
        return -1;
    }

    awb3_ctx.new_awb_mode = mode;
    awb3_ctx.awb_config_update |= AWB_UPDATE_BIT_MODE;

    return 0;
}

static int awb3_get_mode(AWB_MODE *mode)
{
    *mode = awb3_ctx.awb_info.awb_mode;

    return 0;
}

static int awb3_clear_info()
{
    awb3_ctx.awb_info.rb_gain_update = 0;
    awb3_ctx.awb_info.ct_update = 0;

    return 0;
}

#if 0
static void test_register()
{
// print read-only register
    MSG( "%lu\r\n", (int64_t)reg.r_sum);
    MSG( "%lu\r\n", (int64_t)reg.g_sum);
    MSG( "%lu\r\n", (int64_t)reg.b_sum);
    MSG( "%lu\r\n", (int64_t)reg.r_sum_hex);
    MSG( "%lu\r\n", (int64_t)reg.g_sum_hex);
    MSG( "%lu\r\n", (int64_t)reg.b_sum_hex);
    MSG( "%lu\r\n", (int64_t)reg.pixel_hex);
    MSG( "%lu\r\n", (int64_t)reg.r_ratio_hex_y);
    MSG( "%lu\r\n", (int64_t)reg.b_ratio_hex_y);
    MSG( "%lu\r\n", (int64_t)reg.weight_hex_y);
    MSG( "%lu\r\n", (int64_t)reg.pixel_hex_y);
    MSG( "%lu\r\n", (int64_t)reg.r_ratio_hist);
    MSG( "%lu\r\n", (int64_t)reg.b_ratio_hist);
    MSG( "%lu\r\n", (int64_t)reg.weight_hist);
    MSG( "%lu\r\n", (int64_t)reg.pixel_hist);
    MSG( "%lu\r\n", (int64_t)reg.r_ratio_hist_clip);
    MSG( "%lu\r\n", (int64_t)reg.b_ratio_hist_clip);
    MSG( "%lu\r\n", (int64_t)reg.weight_hist_clip);
    MSG( "%lu\r\n", (int64_t)reg.pixel_hist_clip);

    // print SRAM
    //int32_t bin_num = 1 << (reg.bin_y_level + (reg.bin_ratio_level << 1));
}
#endif

static int awb3_control()
{
    ISP_RGB_Gain_Type rgb_gain;
    awb3_clear_info();

    /* Check if AWB config is updated */
    if (awb3_ctx.awb_config_update) {
        if (awb3_ctx.awb_config_update & AWB_UPDATE_BIT_MODE) {
            awb3_ctx.awb_info.awb_mode = awb3_ctx.new_awb_mode;
        }

        /* if new color temp is not equal to previous one, awb_info.ct_update color temp related param */
        if (awb3_ctx.awb_config_update & AWB_UPDATE_BIT_COLOR_TEMP &&
            awb3_ctx.new_color_temp != awb3_ctx.awb_info.color_temp) {
            awb3_ctx.awb_info.color_temp = awb3_ctx.new_color_temp;
            awb3_ctx.awb_info.ct_update = 1;
        }

        awb3_ctx.awb_config_update = 0;
    }

    if (awb3_ctx.awb_info.awb_mode != AWB_MODE_AUTO) {
        if (awb3_ctx.awb_info.ct_update) {
            calculate_rb_gain_by_color_temp(awb3_ctx.awb_info.color_temp, &reg_gain.r_gain, &reg_gain.b_gain);
            apply_preference_gain();
            rgb_gain.r_gain = (uint16_t)reg_gain.r_gain;
            rgb_gain.g_gain = (uint16_t)GAIN_UNITY;
            rgb_gain.b_gain = (uint16_t)reg_gain.b_gain;;
            awb3_set_gain(&rgb_gain);
        }
        return 0;
    }

    // run after every frame hardware interrupt
    awb3_get_stats();
    awb3_get_g_divisor();

    if (0 != awb3_ctx.awb_start_skip_num && awb3_ctx.awb_start_skip_num-- > 0) {
        BL_LOGD("AWB booting!\r\n");
        return -1;
    }

    decide_awb_gain();
    temporal_smooth();
    
    awb3_ctx.awb_info.color_temp = calculate_color_temp(reg_gain.r_gain, reg_gain.b_gain);
    awb3_ctx.awb_info.ct_update = 1;

    // bias for tuning and adaptivity
    apply_preference_gain();

    rgb_gain.r_gain = (uint16_t)reg_gain.r_gain;
    rgb_gain.g_gain = (uint16_t)GAIN_UNITY;
    rgb_gain.b_gain = (uint16_t)reg_gain.b_gain;;
    awb3_set_gain(&rgb_gain);

    return 0;
}

#if 0
static void test_awb3_histogram()
{
    uint32_t bucket_num;
    void *awb3_hist;

    ISP_Get_AWB3_Hist_Bin_Num(&bucket_num);
    awb3_hist = pvPortMalloc(bucket_num*sizeof(isp_awb3_bucket_t));
    if (NULL == awb3_hist) {
        //MSG("pvPortMalloc awb3_hist error\r\n");
        return;
    }

    ISP_Get_AWB3_Hist((uint32_t *)awb3_hist);
    // parse bucket and compare with cmodel

    vPortFree(awb3_hist);
}
#endif

static void awb3_func_init(awb_func_t *func)
{
    func->config          = awb3_config;
    func->control         = awb3_control;
    func->get_info        = awb3_get_info;
    func->set_mode        = awb3_set_mode;
    func->get_mode        = awb3_get_mode;
    func->set_gain        = awb3_set_gain;
    func->get_gain        = awb3_get_gain;
    func->set_color_temp  = awb3_set_color_temp;
    func->get_color_temp  = awb3_get_color_temp;
    func->set_speed_level = awb3_set_speed_level;
    func->get_speed_level = awb3_get_speed_level;
    func->set_sys_gain    = awb3_set_sys_gain;
}

int awb3_init(awb_func_t *func)
{
    awb3_ctx.awb_start_skip_num = IMG_START_SKIP_NUM;

    const wb_info_t *wb_info_D50;
    // TODO: check the color temp table
    awb3_func_init(func);

	wb_info_D50 = &wb_info_use.wb_info[COLOR_TEMP_IDX_D50];
    fw_param.r_gain_default = wb_info_D50->rb_gain.r_gain;
    fw_param.b_gain_default = wb_info_D50->rb_gain.b_gain;
    
    awb_auto_param_fw_t *awb_auto_config;
	awb_auto_config = (awb_auto_param_fw_t *)&awb_fw_use.awb_auto_fw;
    fw_param.speed_level = awb_auto_config->speed_level;

    fw_param.eff_pixel_th_min_ppm = 256;
    fw_param.eff_pixel_th_max_ppm = 1024;
    fw_param.bias_r_gain = GAIN_UNITY;
    fw_param.bias_b_gain = GAIN_UNITY;
    fw_param.temporal_smooth = 179;
    fw_param.sys_gain = 0;

    // initialize IIR
    fw_iir.r_gain = fw_param.r_gain_default;
    fw_iir.b_gain = fw_param.b_gain_default;

    // increase IIR register precision to avoid permanent residue
    fw_iir.r_gain <<= TEMPORAL_IIR_EXTRA_PRECISION;
    fw_iir.b_gain <<= TEMPORAL_IIR_EXTRA_PRECISION;

    return 0;
}
