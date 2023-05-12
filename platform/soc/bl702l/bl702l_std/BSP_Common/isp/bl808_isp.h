#ifndef __BL808_ISP_H__
#define __BL808_ISP_H__

#include "bl808_common.h"
#include "bl808_comm_mm.h"



#define ISP_AWB2_STA_Y_NUM        (5)
#define ISP_AWB2_STA_C_NUM        (8)
#define ISP_AE_HIST_Y_BIN_NUM     (257)
#define ISP_AE_HIST_BAYER_BIN_NUM (65)
#define ISP_AWB3_WEIGHT_NUM       (9)
#define ISP_AWB3_HIST_BIN_NUM_MAX (512)
#define ISP_EE_LUMA_WEIGHT_NUM    (32)
#define ISP_AWB3_CAL_BIN_NUM      (88)

#define AE_STATS_GRID_NUM_X   (12)
#define AE_STATS_GRID_NUM_Y   (8)
#define AE_STATS_GRID_NUM     ((AE_STATS_GRID_NUM_X)*(AE_STATS_GRID_NUM_Y))
#define AE_HIST_BIN_NUM       (256)
#define AE_METERING_TBL_NUM    (4)

/*
    ISP SYNC MODE
 */
typedef enum {
    ISP_SYNC_MODE_INPUT = 0,
    ISP_SYNC_MODE_OUTPUT,
    ISP_SYNC_MODE_OUTPUT_INPUT,
    ISP_SYNC_MODE_INPUT_VBLANK_NO_TOGGLE
}ISP_SYNC_MODE_TYPE;


/*
    R,Gr,Gb,B black level
*/
typedef struct blISP_CLAMP_S {
    int clamp_r;
    int clamp_gr;
    int clamp_gb;
    int clamp_b;
} ISP_CLAMP_S;

/*
    OB avg output R,Gr,Gb,B 
*/
typedef struct blISP_OB_S {
    int16_t ob_r;
    int16_t ob_gr;
    int16_t ob_gb;
    int16_t ob_b;
} ISP_OB_S;

/*
    Color Correction Matrix Coefficient
*/
typedef enum {
    ISP_CCM_11 = 0,
    ISP_CCM_12 = 1,
    ISP_CCM_13 = 2,
    ISP_CCM_21 = 3,
    ISP_CCM_22 = 4,
    ISP_CCM_23 = 5,
    ISP_CCM_31 = 6,
    ISP_CCM_32 = 7,
    ISP_CCM_33 = 8,
} ISP_CCM_COEF_TYPE_E;
    
typedef enum {
    ISP_CCM_OFFSET_0 = 0,
    ISP_CCM_OFFSET_1 = 1,
    ISP_CCM_OFFSET_2 = 2,
} ISP_CCM_CFFSET_TYPE_E;

typedef struct blISP_CCM_OFFSET_GROUP_S
{
    int offset[CCM_OFFSET_NUM];
} ISP_CCM_OFFSET_S;

typedef struct blISP_CCM_COEF_GROUP_S
{
    int ccm_coef[CCM_COEF_NUM];
} ISP_CCM_COEF_S;

typedef struct blISP_GAMMA_COEF_GROUP_S{
    uint16_t curve_coef[GAMMA_COEF_NUM];
} ISP_Gamma_Curve_Type;

/*
    YUV conversion mode
*/
typedef enum {
    ISP_YUV_CONV_FULL_SCALE = 0,
    ISP_YUV_CONV_BT601
}ISP_YUV_CONV_TYPE;

typedef struct blISP_SATURATION_S
{
    uint32_t saturation_cb;
    uint32_t saturation_cr;
} ISP_SATURATION_S;


/*
    Defect correction enhance state
*/
typedef enum {
    ISP_DPC_Enhance1 = 0,
    ISP_DPC_Enhance2,
    ISP_DPC_Enhance3,
    ISP_DPC_Enhance_Num  /* 3 */
}ISP_DPC_ENHANCE_TYPE;

/*
    Defect correction enhance state group
*/
typedef struct {
    BL_Fun_Type data[ISP_DPC_Enhance_Num];
}ISP_DPC_ENHANCE_GROUP_TYPE;

/*
    Defect correction threadhold
*/
typedef enum {
    ISP_DPC_th1 = 0,
    ISP_DPC_th2,
    ISP_DPC_th3,
    ISP_DPC_th4,
    ISP_DPC_th5,
    ISP_DPC_th_Num /* 5 */
}ISP_DPC_THRESHOLD_TYPE;

/*
    Defect correction threadhold group
*/
typedef struct {
    int data[ISP_DPC_th_Num];
}ISP_DPC_THRESHOLD_GROUP_TYPE;

/*
    Lens Shading Coef
    Acoef range: -4194304-4194303
    Bcoef range: -4096-4095
    Ccoef range: 0-255
    Dcoef range: -4194304-4194303
    Ecoef range: -4096-4095
    Gcoef range: -4194304-4194303
 */

typedef struct  {
    uint16_t th_base_r; // r/w, shadow, 10'b, range: 0 ~ (2 ^ TH_BW - 1)
    uint16_t th_base_g; // r/w, shadow, 10'b, range: 0 ~ (2 ^ TH_BW - 1)
    uint16_t th_base_b; // r/w, shadow, 10'b, range: 0 ~ (2 ^ TH_BW - 1)
    uint16_t th_slope_r; // r/w, shadow, 8'b, = TH_SLOPE_PRECISION, range: 0 ~ (2 ^ TH_SLOPE_PRECISION - 1)
    uint16_t th_slope_g; // r/w, shadow, 8'b, = TH_SLOPE_PRECISION, range: 0 ~ (2 ^ TH_SLOPE_PRECISION - 1)
    uint16_t th_slope_b; // r/w, shadow, 8'b, = TH_SLOPE_PRECISION, range: 0 ~ (2 ^ TH_SLOPE_PRECISION - 1)
    uint16_t slope_gain; // r/w, shadow, 6'b, = 2 + SLOPE_GAIN_PRECISION, 2.4 fixed-point, range: 0 ~ (2 ^ 6 - 1)
} ISP_BNR_THRESHOLD_TYPE;

typedef struct  {
    uint16_t fb_lpf_weight; // r/w, shadow, 5'b, = 1 + LPF_WEIGHT_PRECISION, range: 0 ~ (2 ^ LPF_WEIGHT_PRECISION)
    uint16_t fb_level;   // r/w, shadow, 3'b, = FB_LEVEL_BW, range: 0 ~ (2 ^ FB_LEVEL_BW - 1)
} ISP_BNR_FALLBACK_TYPE;

typedef struct {
    int coef[LSC_COEF_NUM];
}ISP_LSC_COEF_TYPE;

typedef struct blISP_LSC_COEF_S {
    ISP_LSC_COEF_TYPE lsc_coef_r;
    ISP_LSC_COEF_TYPE lsc_coef_g;
    ISP_LSC_COEF_TYPE lsc_coef_b;
} ISP_LSC_COEF_S;

typedef struct blISP_LSC_MAX_S {
    int max_r;
    int max_g;
    int max_b;
} ISP_LSC_MAX_S;

typedef enum {
    AE_GRID_DATA_RGB = 0,
    AE_GRID_DATA_Y = 1,
} AE_GRID_DATA_TYPE_E;

typedef enum {
    AWB_GRID_DATA_RG = 0,
    AWB_GRID_DATA_B = 1,
} AWB_GRID_DATA_TYPE_E;

typedef enum {
    ISP_CLK_160MHz = 0,
    ISP_CLK_240MHz = 1,
    ISP_CLK_400MHz = 2,
    ISP_CLK_BCLK = 3,
} ISP_CLK_SEL_E;


typedef struct {
    ISP_SYNC_MODE_TYPE      sync_mode_value;
    uint32_t                ISP_clk;
    uint32_t                pix_clk;
    uint16_t                fps;
    uint16_t                total_width;
    uint16_t                total_height;
    uint16_t                active_width;
    uint16_t                active_height;
    uint16_t                out_width;
    uint16_t                out_height;
} ISP_TG_Cfg_Type;

typedef struct {
    uint8_t                reg_act_vsync_rst_en;
    ISP_SYNC_MODE_TYPE     sync_mode;
    uint8_t                hsync_in_inv;
    uint8_t                vsync_in_inv;
    uint8_t                hcnt_rst_mode;
    uint8_t                vcnt_rst_mode;
    uint8_t                hblk_line_on;
    uint8_t                fhblk_line_on;
} ISP_DVP_MODE_CFG_T;

typedef struct _isp_ae_stats_conf {
    uint16_t            roi_start_x;
    uint16_t            roi_start_y;
    uint16_t            roi_width;
    uint16_t            roi_height;
    uint16_t            img_width;
    uint16_t            img_height;
    uint8_t             grid_width;
    uint8_t             grid_height;
    uint8_t             grid_num_x;
    uint8_t             grid_num_y;
} isp_ae_stats_conf_t;

typedef struct _isp_ae_sum {
    uint32_t            r_sum           :   8;
    uint32_t            g_sum           :   8;
    uint32_t            b_sum           :   8;
    uint32_t            reserved_24_31  :   8;
    uint32_t            y_sum           :   8;
    uint32_t            reserved_8_31   :   24;
} isp_ae_sum_t;

typedef enum {
    AE_HIST_MODE_BAYER = 0,
    AE_HIST_MODE_Y
} AE_HIST_MODE_TYPE_E;

typedef struct {
    uint16_t    ae_hist_x_min;
    uint16_t    ae_hist_x_max;
    uint16_t    ae_hist_y_min;
    uint16_t    ae_hist_y_max;
} isp_ae_hist_roi_t;

typedef struct _isp_awb_stats_conf {
    uint16_t            roi_start_x;
    uint16_t            roi_start_y;
    uint16_t            roi_width;
    uint16_t            roi_height;
    uint16_t            img_width;
    uint16_t            img_height;
    uint8_t             grid_width;
    uint8_t             grid_height;
    uint8_t             grid_num_x;
    uint8_t             grid_num_y;
    uint16_t            r_max_thr;
    uint16_t            r_min_thr;
    uint16_t            g_max_thr;
    uint16_t            g_min_thr;
    uint16_t            b_max_thr;
    uint16_t            b_min_thr;
} isp_awb_stats_conf_t;

typedef struct _isp_awb_sum{
    uint32_t r_sum          :  12;
    uint32_t reserved_12_15 :  4;
    uint32_t g_sum          :  12;
    uint32_t reserved_28_31 :  4;
    uint32_t b_sum          :  12;
    uint32_t reserved_12_31 :  20;
} isp_awb_sum_t;

typedef struct _isp_awb2_stats_conf {
    uint16_t    stat_x_min; // r/w, shadow, 11'b, range: 0 ~ (width - 1)
    uint16_t    stat_x_max; // r/w, shadow, 11'b, range: 0 ~ (width - 1)
    uint16_t    stat_y_min; // r/w, shadow, 11'b, range: 0 ~ (height - 1)
    uint16_t    stat_y_max; // r/w, shadow, 11'b, range: 0 ~ (height - 1)
    uint32_t    stat_pixel_num; // r/w, shadow, 21'b, = (stat_x_max - stat_x_min + 1) * (stat_y_max - stat_y_min + 1)
    uint16_t    stat_r_gain; // r/w, shadow, 16'b, = 4 + GAIN_PRECISION, 4.12 fixed-point, range: 0 ~ (2 ^ 16 - 1)
    uint16_t    stat_g_gain; // r/w, shadow, 16'b, = 4 + GAIN_PRECISION, 4.12 fixed-point, range: 0 ~ (2 ^ 16 - 1)
    uint16_t    stat_b_gain; // r/w, shadow, 16'b, = 4 + GAIN_PRECISION, 4.12 fixed-point, range: 0 ~ (2 ^ 16 - 1)
    uint16_t    stat_u_gain; // r/w, shadow, 10'b, = 2 + C_GAIN_PRECISION, 2.8 fixed-point, range: 0 ~ (2 ^ 10 - 1)
    uint16_t    stat_v_gain; // r/w, shadow, 10'b, = 2 + C_GAIN_PRECISION, 2.8 fixed-point, range: 0 ~ (2 ^ 10 - 1)
    uint16_t    reserved0;
    uint8_t     y_th[ISP_AWB2_STA_Y_NUM]; // r/w, shadow, 8'b, = STAT_PIXEL_BW, range: 0 ~ (2 ^ 8 - 1)
    uint8_t     y_weight[ISP_AWB2_STA_Y_NUM]; // r/w, shadow, 8'b, = STAT_PIXEL_BW, range: 0 ~ (2 ^ 8 - 1)
    uint16_t    reserved1;
    int16_t     y_slope[ISP_AWB2_STA_Y_NUM]; // r/w, shadow, 9'b, = 7 + SLOPE_PRECISION, signed 2's complement, range: -256 ~ +255
    uint16_t    reserved2;
    int8_t      u[ISP_AWB2_STA_C_NUM]; // r/w, shadow, 8'b, = STAT_PIXEL_BW, signed 2's complement, range: -128 ~ +127
    int8_t      v[ISP_AWB2_STA_C_NUM]; // r/w, shadow, 8'b, = STAT_PIXEL_BW, signed 2's complement, range: -128 ~ +127
    uint8_t     c_th[ISP_AWB2_STA_C_NUM]; // r/w, shadow, 6'b, = STAT_PIXEL_BW - 2, range: 0 ~ (2 ^ 6 - 1)
    uint8_t     c_slope[ISP_AWB2_STA_C_NUM]; // r/w, shadow, 8'b, = 6 + SLOPE_PRECISION, range: 0 ~ (2 ^ 8 - 1)
} isp_awb2_stats_conf_t;

typedef struct _isp_awb2_sum{
    uint32_t r_avg; // r, ping-pong, 12'b, = PIXEL_BW
    uint32_t g_avg; // r, ping-pong, 12'b, = PIXEL_BW
    uint32_t b_avg; // r, ping-pong, 12'b, = PIXEL_BW
    uint64_t white_r_sum; // r, ping-pong, 41'b, = max. width * height in bit + WEIGHT_PRECISION + PIXEL_BW
    uint64_t white_g_sum; // r, ping-pong, 41'b, = max. width * height in bit + WEIGHT_PRECISION + PIXEL_BW
    uint64_t white_b_sum; // r, ping-pong, 41'b, = max. width * height in bit + WEIGHT_PRECISION + PIXEL_BW
    uint32_t white_ratio; // r, ping-pong, 15'b, = 1 + RATIO_PRECISION, 1.14 fixed-point
} isp_awb2_sum_t;

typedef struct _isp_awb3_stats_conf {
    // control register
    // int8_t stat_en; // r/w, shadow, 1'b
    uint16_t stat_x_min; // r/w, shadow, 11'b, range: 0 ~ (width - 1), must be even
    uint16_t stat_x_max; // r/w, shadow, 11'b, range: 0 ~ (width - 1), must be odd
    uint16_t stat_y_min; // r/w, shadow, 11'b, range: 0 ~ (height - 1)
    uint16_t stat_y_max; // r/w, shadow, 11'b, range: 0 ~ (height - 1)
    uint16_t r_ratio_ofst; // r/w, shadow, 12'b, = RATIO_BW, 4.8 fixed-point, range: 0 ~ (2 ^ 12 - 1)
    uint16_t b_ratio_ofst; // r/w, shadow, 12'b, = RATIO_BW, 4.8 fixed-point, range: 0 ~ (2 ^ 12 - 1)
    uint16_t r_ratio_mult; // r/w, shadow, 12'b, integer, range: 0 ~ (2 ^ 12 - 1)
    uint16_t b_ratio_mult; // r/w, shadow, 12'b, integer, range: 0 ~ (2 ^ 12 - 1)
    uint16_t ratio_sum_min;  // r/w, shadow, 9'b, = NORM_BW + 1, range: 0 ~ (2 ^ 9 - 1)
    uint16_t ratio_sum_max; // r/w, shadow, 9'b, = NORM_BW + 1, range: 0 ~ (2 ^ 9 - 1)
    uint8_t g_divisor; // r/w, shadow, 1'b
    uint8_t weight_ratio_en; // r/w, shadow, 1'b
    uint8_t weight_ratio_gain; // r/w, shadow, 8'b, = WEIGHT_GAIN_BW, 4.4 fixed-point, range: 0 ~ (2 ^ 8 - 1)
    uint8_t _pad1[1];
    uint8_t weight_ratio[ISP_AWB3_WEIGHT_NUM][ISP_AWB3_WEIGHT_NUM]; // r/w, shadow, 8'b, = WEIGHT_BW, range: 0 ~ (2 ^ 8 - 1)
    uint8_t _pad2[3];
    uint16_t y_r_coeff; // r/w, shadow, 12'b, = COEFF_BW, 4.8 fixed-point, range: 0 ~ (2 ^ 12 - 1)
    uint16_t y_g_coeff; // r/w, shadow, 12'b, = COEFF_BW, 4.8 fixed-point, range: 0 ~ (2 ^ 12 - 1)
    uint16_t y_b_coeff; // r/w, shadow, 12'b, = COEFF_BW, 4.8 fixed-point, range: 0 ~ (2 ^ 12 - 1)
    uint8_t weight_y_use_g; // r/w, shadow, 1'b
    uint8_t weight_y_en; // r/w, shadow, 1'b
    uint8_t weight_y_gain; // r/w, shadow, 8'b, = WEIGHT_GAIN_BW, 4.4 fixed-point, range: 0 ~ (2 ^ 8 - 1)
    uint8_t _pad3[3];
    uint8_t weight_y[ISP_AWB3_WEIGHT_NUM][ISP_AWB3_WEIGHT_NUM]; // r/w, shadow, 8'b, = WEIGHT_BW, range: 0 ~ (2 ^ 8 - 1)
    uint8_t count_zero_weight; // r/w, shadow, 1'b
    uint8_t hist_y_use_g; // r/w, shadow, 1'b
    int32_t bin_pixel_clip; // r/w, shadow, 20'b, = PIXEL_NUM_BW, range: 0 ~ (2 ^ 20 - 1)
    uint8_t bin_y_level; // r/w, shadow, 3'b, range: 0 ~ 7
    uint8_t bin_ratio_level; // r/w, shadow, 3'b, range: 0 ~ 4
    uint8_t bin_clip_en; // r/w, shadow, 1'b
    uint8_t bin_mode; // r/w, shadow, 2'b, range: 0 ~ 3
    uint8_t ink_type; // r/w, shadow, 2'b, range: 0 ~ 2
    uint8_t ink_gain; // r/w, shadow, 4'b, integer, range: 0 ~ (2 ^ 4 - 1)
} isp_awb3_stats_conf_t;

typedef struct _isp_awb3_sum{
    // status register
    uint32_t r_sum; // r, ping-pong, 32'b, = PIXEL_NUM_BW + PIXEL_BW
    uint32_t g_sum; // r, ping-pong, 32'b, = PIXEL_NUM_BW + PIXEL_BW
    uint32_t b_sum; // r, ping-pong, 32'b, = PIXEL_NUM_BW + PIXEL_BW
    uint32_t r_sum_hex; // r, ping-pong, 32'b, = PIXEL_NUM_BW + PIXEL_BW
    uint32_t g_sum_hex; // r, ping-pong, 32'b, = PIXEL_NUM_BW + PIXEL_BW
    uint32_t b_sum_hex; // r, ping-pong, 32'b, = PIXEL_NUM_BW + PIXEL_BW
    uint32_t pixel_hex; // r, ping-pong, 20'b, = PIXEL_NUM_BW
    uint64_t r_ratio_hex_y; // r, ping-pong, 40'b, = PIXEL_NUM_BW + WEIGHT_BW + RATIO_BW
    uint64_t b_ratio_hex_y; // r, ping-pong, 40'b, = PIXEL_NUM_BW + WEIGHT_BW + RATIO_BW
    uint32_t weight_hex_y; // r, ping-pong, 28'b, = PIXEL_NUM_BW + WEIGHT_BW
    uint32_t pixel_hex_y; // r, ping-pong, 20'b, = PIXEL_NUM_BW
    uint64_t r_ratio_hist; // r, ping-pong, 40'b, = PIXEL_NUM_BW + WEIGHT_BW + RATIO_BW
    uint64_t b_ratio_hist; // r, ping-pong, 40'b, = PIXEL_NUM_BW + WEIGHT_BW + RATIO_BW
    uint32_t weight_hist; // r, ping-pong, 28'b, = PIXEL_NUM_BW + WEIGHT_BW
    uint32_t pixel_hist; // r, ping-pong, 20'b, = PIXEL_NUM_BW
    uint64_t r_ratio_hist_clip; // r, ping-pong, 40'b, = PIXEL_NUM_BW + WEIGHT_BW + RATIO_BW
    uint64_t b_ratio_hist_clip; // r, ping-pong, 40'b, = PIXEL_NUM_BW + WEIGHT_BW + RATIO_BW
    uint32_t weight_hist_clip; // r, ping-pong, 28'b, = PIXEL_NUM_BW + WEIGHT_BW
    uint32_t pixel_hist_clip; // r, ping-pong, 20'b, = PIXEL_NUM_BW
} isp_awb3_sum_t;

typedef struct _isp_awb3_bucket{
    int32_t footprint[4];
} isp_awb3_bucket_t;

typedef enum {
    AWB3_BIN_MODE_FULL = 0,
    AWB3_BIN_MODE_RATIO,
    AWB3_BIN_MODE_Y,
    AWB3_BIN_MODE_NONE,
} isp_awb3_bin_mode_e;

typedef struct _isp_af_stats_conf{
    int16_t wa_hpf1_sum;
    int16_t wa_hpf2_sum;
    int16_t wb_hpf1_sum;
    int16_t wb_hpf2_sum;
} isp_af_stats_conf_t;

typedef struct {
    uint8_t  dirShpnessThresh;
    uint8_t  noDirShpnessThresh;
    uint8_t  flatThresh;
    uint16_t textureThresh;
    uint16_t edgeThresh;
} ISP_EE_Thresh_Type;

typedef struct {
    uint8_t posEdgeOvrshtStr;
    uint8_t negEdgeOvrshtStr;
    uint8_t posEdgeStr;
    uint8_t negEdgeStr;
    uint8_t nrStr;
} ISP_EE_Str_Type;

typedef struct {
    uint16_t lumaWgtTbl[ISP_EE_LUMA_WEIGHT_NUM];
} ISP_EE_Luma_Weight_Type;

typedef struct {
    uint16_t gain;
    uint8_t  weight;
    uint8_t  grey_thr;
} ISP_Chroma_Suppress_Type;

typedef struct {
    uint16_t r_gain;
    uint16_t g_gain;
    uint16_t b_gain;
} ISP_RGB_Gain_Type;

typedef struct {
    uint16_t r_offset;
    uint16_t g_offset;
    uint16_t b_offset;
} ISP_RGB_Offset_Type;

typedef struct {
    uint16_t r_gain;
    uint16_t b_gain;
} ISP_RB_Gain_Type;

typedef struct {
    uint8_t gamma_pattern;
    uint8_t fcore1;
    uint8_t fcore2;
} ISP_AF_CONF_Type;

typedef struct {
    uint8_t start_x;
    uint8_t start_y;
    uint8_t end_x;
    uint8_t end_y;
    uint8_t sum_shift;
} ISP_AF_WINDOW_Type;

#define IS_ISP_YUV_CONV_TYPE(type)                      (((type) == ISP_YUV_CONV_FULL_SCALE) || \
                                                          ((type) == ISP_YUV_CONV_BT601))

void ISP_Init(void);

void ISP_Get_IP_Info(int *version, int *ip_id);
void ISP_Set_TG_Cfg(const ISP_TG_Cfg_Type *cfg);
void ISP_Set_TG_Reshape_Cfg();

void ISP_Set_DVP_Mode_Cfg(ISP_DVP_MODE_CFG_T *cfg);
void ISP_Get_DVP_Mode_Cfg(ISP_DVP_MODE_CFG_T *cfg);
void ISP_Set_Total_Pixels(BL_Img_Size_T *out_win);
void ISP_Get_Total_Pixels(BL_Img_Size_T *out_win);
void ISP_Set_Active_Pixels(int start_pixel, int start_line, BL_Img_Size_T *out_win);
void ISP_Get_Active_Pixels(int *start_pixel, int *start_line, BL_Img_Size_T *out_win);
void ISP_Set_Output_Pixels(int start_pixel, int start_line, BL_Img_Size_T *out_win);
void ISP_Get_Output_Pixels(int *start_pixel, int *start_line, BL_Img_Size_T *out_win);
void ISP_Set_Hsync_Pixels(int start_pixel, int start_line, BL_Img_Size_T *out_win);
void ISP_Get_Hsync_Pixels(int *start_pixel, int *start_line, BL_Img_Size_T *out_win);
void ISP_Set_Vsync_Pixels(int start_pixel, int start_line, int end_pixel, int line_height);
void ISP_Get_Vsync_Pixels(int *start_pixel, int *start_line, int *end_pixel, int *line_height);
void ISP_Set_Bayer_Pattern(const BAYER_PATTERN_E pattern);
void ISP_Get_Bayer_Pattern(BAYER_PATTERN_E *pattern);

void ISP_Set_AE_State(BL_Fun_Type state);
void ISP_Set_AE_Stats_Conf(const isp_ae_stats_conf_t *ae_stats_conf);
void ISP_Get_AE_Stats_Conf(isp_ae_stats_conf_t *ae_stats_conf);
void ISP_Get_AE_Sum(int grid_idx, AE_GRID_DATA_TYPE_E data_type, uint32_t *data);
void ISP_Set_AE_Ysum_Coeff(uint8_t coeff_r, uint8_t coeff_g, uint8_t coeff_b);
void ISP_Get_AE_Ysum_Coeff(uint8_t *coeff_r, uint8_t *coeff_g, uint8_t *coeff_b);
int  ISP_Get_AE_Stats(int grid_num, uint32_t *data);

void ISP_Set_AE_Hist_State(BL_Fun_Type state);
int  ISP_Get_AE_Hist_State(BL_Fun_Type *state);
void ISP_Set_AE_Hist_Mode(AE_HIST_MODE_TYPE_E mode);
int  ISP_Get_AE_Hist_Mode(AE_HIST_MODE_TYPE_E *mode);
void ISP_Set_AE_Hist_ROI_State(BL_Fun_Type state);
int  ISP_Get_AE_Hist_ROI_State(BL_Fun_Type *state);
int  ISP_Set_AE_Hist_ROI(BL_Fun_Type state, const isp_ae_hist_roi_t *roi);
int  ISP_Get_AE_Hist_ROI(BL_Fun_Type state, isp_ae_hist_roi_t *roi);
int  ISP_Get_AE_Hist_SUM_Y(uint32_t *y_sum);
int  ISP_Get_AE_Hist_SUM_Bayer(uint32_t *r_sum, uint32_t *gr_sum, uint32_t *gb_sum, uint32_t *b_sum);

void ISP_Set_AWB_State(BL_Fun_Type state);
int  ISP_Get_AWB_Stats(int grid_num, uint32_t *data);
void ISP_Set_AWB_Stats_Conf(const isp_awb_stats_conf_t *awb_stats_conf);
void ISP_Get_AWB_Stats_Conf(isp_awb_stats_conf_t *awb_stats_conf);
void ISP_Get_AWB_Sum(int grid_idx, RGB_SUM_S *rgb_sum);
void ISP_Get_AWB_ValidGridNum(int *valid_grid_num);

void ISP_Set_AWB_Gain(const ISP_RGB_Gain_Type* rgb_gain);
void ISP_Get_AWB_Gain(ISP_RGB_Gain_Type *rgb_gain);
void ISP_Set_AWB_Offset(const ISP_RGB_Offset_Type *Offset);
void ISP_Get_AWB_Offset(ISP_RGB_Offset_Type *Offset);
void ISP_Set_RB_Gain(const ISP_RB_Gain_Type *rb_gain);

void ISP_Set_AWB2_Stats_State(BL_Fun_Type state);
void ISP_Set_AWB2_Stats_Conf(const isp_awb2_stats_conf_t *awb_stats_conf);
int  ISP_Get_AWB2_Sum(isp_awb2_sum_t *sum);
void ISP_Set_AWB2_Gain_State(BL_Fun_Type state);
void ISP_Set_AWB2_Gain(const ISP_RGB_Gain_Type *gain);
void ISP_Get_AWB2_Gain(ISP_RGB_Gain_Type *rgb_gain);

void ISP_Set_AWB3_Hist_State(BL_Fun_Type state);
void ISP_Get_AWB3_Hist_State(BL_Fun_Type *state);
void ISP_Set_AWB3_Stats_Conf(const isp_awb3_stats_conf_t *awb_stats_conf);
void ISP_Get_AWB3_Stats_Conf(isp_awb3_stats_conf_t *awb_stats_conf);
int  ISP_Get_AWB3_Sum(isp_awb3_sum_t *sum);
int  ISP_Get_AWB3_Bin_Mode(isp_awb3_bin_mode_e *mode);
void ISP_Set_AWB3_Bin_Mode(isp_awb3_bin_mode_e mode);
int  ISP_Get_AWB3_Hist_Bin_Num(uint32_t *num);
int  ISP_Get_AWB3_Hist(uint32_t *data);
int  ISP_Get_AWB3_G_Divisor(int *value);
void ISP_Set_AWB3_G_Divisor(int value);

int  ISP_Set_Output_Window(BL_Img_Size_T *out_win);
int  ISP_Get_Output_Window(BL_Img_Size_T *out_win);
void ISP_Set_BL444to422(int mode);
void ISP_Get_BL444to422(int *mode);


/*
    Set R/G/B BLC
    value range:
        0-1023
 */
int ISP_Set_BLC(const ISP_CLAMP_S *clamp);
int ISP_Get_BLC(ISP_CLAMP_S *clamp);
int ISP_Get_OB_Avg(ISP_OB_S *ob);

void ISP_Set_DPC_State(BL_Fun_Type state);
void ISP_Get_DPC_State(BL_Fun_Type *state);
void ISP_Enable_DPC(void);
void ISP_Disable_DPC(void);
int  ISP_Set_Defect_Correction_Enhance_Group(const ISP_DPC_ENHANCE_GROUP_TYPE *value);
int  ISP_Get_Defect_Correction_Enhance_Group(ISP_DPC_ENHANCE_GROUP_TYPE *value);
int  ISP_Set_Defect_Correction_Threshold_Group(const ISP_DPC_THRESHOLD_GROUP_TYPE *value);
int  ISP_Get_Defect_Correction_Threshold_Group(ISP_DPC_THRESHOLD_GROUP_TYPE *value);
int  ISP_Set_Defect_Postion(const int pos_x, const int pos_y);
int  ISP_Get_Defect_Postion(int *pos_x, int *pos_y);

void ISP_Set_BNR_State(BL_Fun_Type state);
void ISP_Get_BNR_State(BL_Fun_Type *state);
void ISP_Set_BNR_Weight(int value);
void ISP_Get_BNR_Weight(int *value);
void ISP_Set_BNR_LPF_Weight(int value);
void ISP_Get_BNR_LPF_Weight(int *value);
void ISP_Set_BNR_Threshold(const ISP_BNR_THRESHOLD_TYPE *value);
void ISP_Get_BNR_Threshold(ISP_BNR_THRESHOLD_TYPE *value);
void ISP_Set_BNR_Fallback(const ISP_BNR_FALLBACK_TYPE *value);
void ISP_Get_BNR_Fallback(ISP_BNR_FALLBACK_TYPE *value);

void ISP_Set_Brightness(int value);
void ISP_Get_Brightness(int *value);

void ISP_Set_LSC_State(BL_Fun_Type state);
void ISP_Enable_LSC(void);
void ISP_Disable_LSC(void);
void ISP_Get_LSC_State(BL_Fun_Type *state);
/*
    Set Lens Shading Correction offset
*/
int ISP_Set_LSC_Offset(int xoffset, int yoffset);
int ISP_Get_LSC_Offset(int *xoffset, int *yoffset);

/*
    Set/Get Lens Shading Correction setting
    value range: please see ISP_LSC_COEF_S
*/
int ISP_Set_LSC_Coeff(const ISP_LSC_COEF_S *lsc);
int ISP_Get_LSC_Coeff(ISP_LSC_COEF_S *lsc);
int ISP_Set_LSC_Coeff_Type(const int type_idx, ISP_LSC_COEF_TYPE *lsc);
int ISP_Get_LSC_Coeff_Type(const int type_idx, ISP_LSC_COEF_TYPE *lsc);

/*
    Set/Get Lens Shading Correction R,G,B MAX
    value range: 0-255 (refs to demo app)
*/
int ISP_Set_LSC_Max(const ISP_LSC_MAX_S *lsc_max);
int ISP_Get_LSC_Max(ISP_LSC_MAX_S *lsc_max);

void ISP_Set_Digital_Gain(int r_gain, int gr_gain, int gb_gain, int b_gain);
void ISP_Get_Digital_Gain(int *r_gain, int *gr_gain, int *gb_gain, int *b_gain);

void ISP_Set_CFA_Int_State(BL_Fun_Type state);
void ISP_Get_CFA_Int_State(BL_Fun_Type *state);

void ISP_Set_CCM_State(BL_Fun_Type state);
void ISP_Get_CCM_State(BL_Fun_Type *state);

/*
    Set/Get Color Correction Matrix Coefficient Value
    value range: 0-4095, 512 normalize to 1
 */
void  ISP_Set_CCM_Coef(const ISP_CCM_COEF_S *ccm);
void  ISP_Get_CCM_Coef(ISP_CCM_COEF_S *ccm);
void ISP_Set_CCM_Pre_Offset(const ISP_CCM_OFFSET_S *ccm);
void ISP_Get_CCM_Pre_Offset(ISP_CCM_OFFSET_S *ccm);
void ISP_Set_CCM_Post_Offset(const ISP_CCM_OFFSET_S *ccm);
void ISP_Get_CCM_Post_Offset(ISP_CCM_OFFSET_S *ccm);


void ISP_Set_Gamma_State(BL_Fun_Type state);
void ISP_Get_Gamma_State(BL_Fun_Type *state);

/*
    Set/Get Gamma
    value range: 0-4095
 */
void ISP_Set_Gamma_Coef(const ISP_Gamma_Curve_Type *gamma_curve);
void ISP_Get_Gamma_Coef(ISP_Gamma_Curve_Type *gamma_curve);


void ISP_Init_Color_Enhancement();
/*
    Set/Get YUV Conversion mode
    @ deprecated
    if you want to change the yuv range, please set yuv adjust
 */
void ISP_Set_YUV_Conv_Mode(ISP_YUV_CONV_TYPE  type);
void ISP_Get_YUV_Conv_Mode(ISP_YUV_CONV_TYPE *type);


/*
    Set/Get Brightness
    value range: TODO  -128-127
 */
void ISP_Set_Brightness(int value);
void ISP_Get_Brightness(int *value);

/*
    Set/Get Contrast
    value range: TODO 0-255
*/
void ISP_Set_Contrast(int value);
void ISP_Get_Contrast(int *value);

/*
    Set/Get Saturation
    value range:TODO  0-255
*/
void ISP_Set_Saturation(const ISP_SATURATION_S *saturation);
void ISP_Get_Saturation(ISP_SATURATION_S *saturation);

void ISP_Set_Special_Mode(int value);
void ISP_Get_Special_Mode(int *value);
void ISP_Set_LPF_Filting_Mode(int value);
void ISP_Get_LPF_Filting_Mode(int *value);
void ISP_Set_Sepia(int SepiaCr, int SepiaCb);
void ISP_Get_Sepia(int *SepiaCr, int *SepiaCb);
void ISP_Set_Solar_Thred(int value);
void ISP_Get_Solar_Thred(int *value);

/*
    Set/Get Noise Level
    value range: 0-63
*/
void ISP_Set_Noise_Level(int value);
void ISP_Get_Noise_Level(int *value);

/*
    Set/Get 2D Noise Reduction
    value range: 0-31
*/
void ISP_Set_2D_NR(int value);
void ISP_Get_2D_NR(int *value);

/*
    Set/Get 3D Noise Reduction
    value range: 0-31
*/
void ISP_Set_3D_NR(int value);
void ISP_Get_3D_NR(int *value);
void ISP_Set_NR_HSize(const int value);
void ISP_Get_NR_HSize(int *value);

void ISP_Set_EE_State(BL_Fun_Type state);
void ISP_Get_EE_State(BL_Fun_Type *state);
void ISP_Enable_EE(void);
void ISP_Disable_EE(void);
void ISP_EnableEEOverShoot(void);
void ISP_DisableEEOverShoot(void);
void ISP_Set_EEOverShoot_State(BL_Fun_Type state);
void ISP_Get_EEOverShoot_State(BL_Fun_Type *state);
void ISP_Set_EE_Thresh(const ISP_EE_Thresh_Type *threshCfg);
void ISP_Get_EE_Thresh(ISP_EE_Thresh_Type *threshCfg);
void ISP_Set_EE_Strength(const ISP_EE_Str_Type *strCfg);
void ISP_Get_EE_Strength(ISP_EE_Str_Type *strCfg);
void ISP_Set_EE_LumaWeight(const ISP_EE_Luma_Weight_Type *lumaWgtCfg);
void ISP_Get_EE_LumaWeight(ISP_EE_Luma_Weight_Type *lumaWgtCfg);

void ISP_Set_ChromaSuppress_State(BL_Fun_Type state);
void ISP_Get_ChromaSuppress_State(BL_Fun_Type *state);
void ISP_Enable_ChromaSuppress(void);
void ISP_Disable_ChromaSuppress(void);

void ISP_Set_ChromaSuppress(const ISP_Chroma_Suppress_Type *chromaSupprCfg);
void ISP_Get_ChromaSuppress(ISP_Chroma_Suppress_Type *chromaSupprCfg);

void ISP_Get_AF_Stats(isp_af_stats_conf_t *afStats);
void ISP_Set_AF_Stats_Conf(const ISP_AF_CONF_Type *afCfg);
void ISP_Get_AF_Stats_Conf(ISP_AF_CONF_Type *afCfg);
void ISP_Set_AF_Window_A(const ISP_AF_WINDOW_Type *afWinCfg);
void ISP_Get_AF_Window_A(ISP_AF_WINDOW_Type *afWinCfg);
void ISP_Set_AF_Window_B(const ISP_AF_WINDOW_Type *afWinCfg);
void ISP_Get_AF_Window_B(ISP_AF_WINDOW_Type *afWinCfg);

void ISP_SW_Reset(void);
void ISP_Clock_Sel(ISP_CLK_SEL_E clk_sel, BL_Fun_Type clk_div_en, int clk_div);
void ISP_Enable(void);
void ISP_Disable(void);

#endif  /* __BL808_ISP_H__ */
