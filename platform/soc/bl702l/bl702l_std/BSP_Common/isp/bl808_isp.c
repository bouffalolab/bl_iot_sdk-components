#include <string.h>

#include <bl808_isp.h>
#include <isp_tg_reg.h>
#include <isp_auto_reg.h>
#include <isp_blae_reg.h>
#include <isp_blawb_reg.h>
#include <isp_gamma_reg.h>

#include <isp_front_reg.h>
#include <isp_middle_reg.h>
#include <isp_middle2_reg.h>
#include <isp_middle3_reg.h>
#include <isp_middle4_reg.h>
#include <isp_middle5_reg.h>
#include <isp_blback_reg.h>

#include <mm_glb_reg.h>

#include "bflb_platform.h"

/* accrording TestPatternClampRGBgainDlg.cpp */
#define ISP_BLC_MAX  1023
#define ISP_BLC_MIN 0

#define ISP_BNR_WEIGHT_MAX  64
#define ISP_BNR_WEIGHT_MIN  0
#define ISP_BNR_LPF_WEIGHT_MAX  16
#define ISP_BNR_LPF_WEIGHT_MIN  0
#define ISP_BNR_BASE_MAX 1023
#define ISP_BNR_SLOPE_MAX 255
#define ISP_BNR_SLOPE_GAIN_MAX 63
#define ISP_BNR_FB_LPF_WGT_MAX 16
#define ISP_BNR_FB_LVL_MAX 7

#define ISP_GAIN_MAX 2047
#define ISP_GAIN_MIN 0

#define ISP_CCM_MAX 2047
#define ISP_CCM_MIN -2048

#define ISP_CCM_OFFSET_MAX 4095
#define ISP_CCM_OFFSET_MIN -4096

#define ISP_DIGITAL_GAIN_MAX 65535
#define ISP_DIGITAL_GAIN_MIN 0

#define ISP_BRIGHTNESS_MAX  127
#define ISP_BRIGHTNESS_MIN  -128

#define ISP_CONTRAST_MAX  255
#define ISP_CONTRAST_MIN  0

#define ISP_SATURATION_MAX  255
#define ISP_SATURATION_MIN  0

#define ISP_SOLAR_SEPIA_MAX 255
#define ISP_SOLAR_SEPIA_MIN 0

#define ISP_SOLAR_THRED_MAX 255
#define ISP_SOLAR_THRED_MIN 0

#define ISP_NOISE_LEVEL_MAX  63
#define ISP_NOISE_LEVEL_MIN  0

#define ISP_NOISE_THRESHOLD_MAX  127
#define ISP_NOISE_THRESHOLD_MIN  0

#define ISP_NR_HSIZE_MAX  65535
#define ISP_NR_HSIZE_MIN  0

#define ISP_NR_STRENGTH_MAX  15
#define ISP_NR_STRENGTH_MIN  0

#define ISP_Edge_Enhance_LEVEL_MAX  127
#define ISP_Edge_Enhance_LEVEL_MIN  0

#define ISP_Edge_Enhance_LIMIT_MAX  255
#define ISP_Edge_Enhance_LIMIT_MIN  0

#define ISP_Edge_Enhance_HPF_MAX  8
#define ISP_Edge_Enhance_HPF_MIN  0

#define ISP_DPC_TH_MAX  255
#define ISP_DPC_TH_MIN  0

#define ISP_DPC_POS_MAX  65535
#define ISP_DPC_POS_MIN  0

#define ISP_AE_SRAM_BANK1      (AE_SRAM_BASE)            // 0x30019000
#define ISP_AE_SRAM_BANK2      (AE_SRAM_BASE +  (768))   // 0x30019300

#define ISP_AE_ROI_START_X_RANGE_MAX        2047
#define ISP_AE_ROI_START_Y_RANGE_MAX        2047
#define ISP_AE_ROI_WIDTH_RANGE_MAX          2047
#define ISP_AE_ROI_WIDTH_RANGE_MIN          32
#define ISP_AE_ROI_HEIGHT_RANGE_MAX         2047
#define ISP_AE_ROI_HEIGHT_RANGE_MIN         2

#define ISP_AE_HIST_ROI_MAX 2047
#define ISP_AE_HIST_ROI_MIN 0

#define ISP_AE_GRID_WIDTH_RANGE_MAX     255
#define ISP_AE_GRID_WIDTH_RANGE_MIN     32
#define ISP_AE_GRID_HEIGHT_RANGE_MAX    255
#define ISP_AE_GRID_HEIGHT_RANGE_MIN    2
#define ISP_AE_GRID_NUM_X_RANGE_MAX     12
#define ISP_AE_GRID_NUM_X_RANGE_MIN     1
#define ISP_AE_GRID_NUM_Y_RANGE_MAX     8
#define ISP_AE_GRID_NUM_Y_RANGE_MIN     1

#define AE_HIST_SUM_ADDR_Y     ((uint32_t)0x30019600)
#define AE_HIST_SUM_ADDR_R     ((uint32_t)0x30019600)
#define AE_HIST_SUM_ADDR_GR    ((uint32_t)0x30019704)
#define AE_HIST_SUM_ADDR_GB    ((uint32_t)0x30019808)
#define AE_HIST_SUM_ADDR_B     ((uint32_t)0x3001990C)

#define ISP_AWB_SRAM_BANK1     (AWB_SRAM_BASE)            // 3001c000
#define ISP_AWB_SRAM_BANK2     (AWB_SRAM_BASE + (1024*8)) // 3001e000

#define AWB3_HIST_BUCKET_ADDR  ((uint32_t)0x30017000)
#define AWB3_HIST_BUCKET_NUM_MIN  1
#define AWB3_HIST_BUCKET_NUM_MAX  ISP_AWB3_HIST_BIN_NUM_MAX

#define ISP_AWB_ROI_START_X_RANGE_MAX       2047
#define ISP_AWB_ROI_START_Y_RANGE_MAX       2047
#define ISP_AWB_ROI_WIDTH_RANGE_MAX         2047
#define ISP_AWB_ROI_WIDTH_RANGE_MIN         32
#define ISP_AWB_ROI_HEIGHT_RANGE_MAX        2047
#define ISP_AWB_ROI_HEIGHT_RANGE_MIN        2

#define ISP_AWB_GRID_WIDTH_RANGE_MAX     255
#define ISP_AWB_GRID_WIDTH_RANGE_MIN     32
#define ISP_AWB_GRID_HEIGHT_RANGE_MAX    255
#define ISP_AWB_GRID_HEIGHT_RANGE_MIN    2
#define ISP_AWB_GRID_NUM_X_RANGE_MAX     32
#define ISP_AWB_GRID_NUM_X_RANGE_MIN     1
#define ISP_AWB_GRID_NUM_Y_RANGE_MAX     32
#define ISP_AWB_GRID_NUM_Y_RANGE_MIN     1

#define ISP_AWB_R_MAX_THR_RANGE_MAX     4095
#define ISP_AWB_R_MIN_THR_RANGE_MAX     4095
#define ISP_AWB_G_MAX_THR_RANGE_MAX     4095
#define ISP_AWB_G_MIN_THR_RANGE_MAX     4095
#define ISP_AWB_B_MAX_THR_RANGE_MAX     4095
#define ISP_AWB_B_MIN_THR_RANGE_MAX     4095

#define ISP_LSC_OFFSET_MAX      127
#define ISP_LSC_OFFSET_MIN      -128

#define ISP_EE_DIR_SHPNSS_THRESH_MAX        127
#define ISP_EE_NO_DIR_SHPNSS_THRESH_MAX     127
#define ISP_EE_FLAT_REGION_THRESH_MAX       255
#define ISP_EE_TEXTURE_REGION_THRESH_MAX    511
#define ISP_EE_EDGE_REGION_THRESH_MAX       65535

#define ISP_EE_STRENGTH_MAX         127
#define ISP_EE_NR_STRENGTH_MAX     255

#define ISP_EE_LUMA_WEIGHT_MAX     1023

#define ISP_CS_GAIN_MAX         511
#define ISP_CS_WEIGHT_MAX       31
#define ISP_CS_GAIN_THR_MAX     15

#define ISP_AF_GAMMA_PATTERN_MAX    7
#define ISP_AF_WIN_START_MAX        255
#define ISP_AF_WIN_END_MAX          255
#define ISP_AF_SUM_SHIFT_A_MAX        3
#define ISP_AF_SUM_SHIFT_B_MAX        7
#define ISP_AF_FCORE_MAX            15

typedef enum {
    ISP_DPC_OFF = 0,
    ISP_DPC_FLY
} ISP_DPC_MODE_TYPE;

#if 0
static const int ISP_LSC_COEF_LIMIT[LSC_COEF_NUM][2] =
{
    { -4194304, 4194303 },      // A
    { -4096, 4095 },            // B
    { 0, 255 },                 // C
    { -4194304, 4194303 },      // D
    { -4096, 4095 },            // E
    { -4194304, 4194303 },      // G
};
#endif

#define ISP_LSC_MAX_MAX  255
#define ISP_LSC_MAX_MIN  0


#define ISP_ERROR_ADDR 0xffffffff

/* For 16bits reg, high 8bits offset 1Byte is low 8bits */
#define ISP_REG_H_L_OFFSET 0X1

/* read 16 bits data consits of high 8bits and low 8bits */
#define BL_RD_16_H_L_REG(addr_h, addr_l) \
    ((BL_RD_BYTE(addr_h)) << 8) + (BL_RD_BYTE(addr_l))

#define ISP_RD_16_H_L_REG(regname_h, regname_l) \
    ((BL_RD_BYTE_REG(ISP_BASE, regname_h)) << 8) + (BL_RD_BYTE_REG(ISP_BASE, regname_l))


#define IS_BLACK_WHITE_MODE(mode)     (mode == BLACK_WHITE_MODE_ON)

typedef enum {
    ISP_HSYNC_INVERT_NORMAL = 0,
    ISP_HSYNC_INVERT_INVERT
}ISP_HSYNC_INVERT_TYPE;

typedef enum {
    ISP_VSYNC_INVERT_NORMAL = 0,
    ISP_VSYNC_INVERT_INVERT
}ISP_VSYNC_INVERT_TYPE;

typedef enum {
    ISP_HCNT_RST_RISING = 0,
    ISP_HCNT_RST_FALLING
}ISP_HCNT_RST_TYPE;

typedef enum {
    ISP_VCNT_RST_RISING = 0,
    ISP_VCNT_RST_FALLING
}ISP_VCNT_RST_TYPE;

typedef enum {
    ISP_VCNT_RST_REVERT_FALLING = 0,
    ISP_VCNT_RST_REVERT_RISING
}ISP_VCNT_RST_REVERT_TYPE;

typedef enum {
    ISP_HBLK_LINE_OFF = 0,
    ISP_HBLK_LINE_ON = 1,
}ISP_HBLK_LINE_TYPE;

typedef enum {
    ISP_FHBLK_LINE_OFF = 0,
    ISP_FHBLK_LINE_ON = 1,
}ISP_FHBLK_LINE_TYPE;


static void ISP_Print_TG(void)
{
#if 0
    isp_tg_reg_t *isp_tg_reg = (isp_tg_reg_t*)ISP_BASE;
    BL_LOGD("tot win: %dx%d\r\n", isp_tg_reg->total_pixels.BF.total_hpixels_w,
        isp_tg_reg->total_pixels.BF.total_vlines_w);
    BL_LOGD("act win: %dx%d, start (%d, %d)\r\n", isp_tg_reg->act_pixels_1.BF.act_pixel_width_w,
        isp_tg_reg->act_pixels_2.BF.act_line_height_w, isp_tg_reg->act_pixels_1.BF.act_start_pixel_w,
        isp_tg_reg->act_pixels_2.BF.act_start_line_w);
#endif
}

static uint16_t ISP_Calc_Total_Width(const ISP_TG_Cfg_Type *config)
{
    // uint32_t pix_clk = config->pix_clk;
    // uint32_t ISP_clk = config->ISP_clk;
    // uint16_t sns_tot_width = config->total_width;

    return (uint16_t)((uint64_t)config->ISP_clk/config->total_height/config->fps + 48) & 0xFFFE;
#if 0
#if (BL80X_HW_VER <= BL80X_HW_A0)
    if (pix_clk == 0)
        return sns_tot_width;

    /* TODO: workaround bayer issue and top garbage lines issue */
    if (sns_tot_width == 1600)
        return 0x716;   // bitfile: VCU118, update interrupt _d2b739ea86_23364bd51e

    /* OV9732 */
    if (sns_tot_width == 1478)
        return 0x6B6;

#if 0
    /* SC2232 */
    if (sns_tot_width == 2200)
        return 0xe14;
#endif

    /* SC2232 */
    if (sns_tot_width == 2080)
#if (BL80X_HW_VER <= BL80X_HW_A0)
        return 0x886;
#else
        return 0xDE80;
#endif
#endif
#endif
    /* the final total width must be even */

    // return (uint16_t)((uint64_t)ISP_clk/1125/25) & 0xFFFE;

    // return (uint16_t)((uint64_t)sns_tot_width * ISP_clk / pix_clk + 48) & 0xFFFE;    //TODO: remove magic number
}

static bool is_LSC_Coeff_Valid(const ISP_LSC_COEF_S *lsc)
{
    int i;
    for (i = 0; i < LSC_COEF_NUM; ++i) {
        CHECK_PARAM_RANGE(lsc->lsc_coef_r.coef[i], ISP_LSC_COEF_LIMIT[i][0], ISP_LSC_COEF_LIMIT[i][1]);
        CHECK_PARAM_RANGE(lsc->lsc_coef_g.coef[i], ISP_LSC_COEF_LIMIT[i][0], ISP_LSC_COEF_LIMIT[i][1]);
        CHECK_PARAM_RANGE(lsc->lsc_coef_b.coef[i], ISP_LSC_COEF_LIMIT[i][0], ISP_LSC_COEF_LIMIT[i][1]);
    }

    return true;
}

static bool is_LSC_Coeff_Type_Valid(const ISP_LSC_COEF_TYPE *lsc)
{
    int i;
    for (i = 0; i < LSC_COEF_NUM; ++i) {
        CHECK_PARAM_RANGE(lsc->coef[i], ISP_LSC_COEF_LIMIT[i][0], ISP_LSC_COEF_LIMIT[i][1]);
    }

    return true;
}

static int ISP_Calc_Clamp_Gain(int black_level)
{
    return DIV_ROUND(4095 * 4096, (4095 - black_level));
}

void ISP_Init(void)
{
    uint32_t tmpVal;

#if (BL80X_HW_VER <= BL80X_HW_A0)
    tmpVal = BL_RD_REG(ISP_BASE, ISP_FRONT_AE_COEFF);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_FRONT_AE_YSUM_COEFF_R_PRE, 77);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_FRONT_AE_YSUM_COEFF_G_PRE, 150);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_FRONT_AE_YSUM_COEFF_B_PRE, 29);
    BL_WR_REG(ISP_BASE, ISP_FRONT_AE_COEFF, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BACK_DNR_B);
    /* yuv output 16bit mode */
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_BACK_OUT_FORMAT_W, 1);
    /* YUYV order */
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_BACK_CBCR_SELECT_W, 1);
    /* 3DNR dram delay match, proposed by MingChun */
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_BACK_DELAYNUM3DNR_W, 4);
#else

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_AE_COEFF);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AE_YSUM_COEFF_R_PRE, 77);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AE_YSUM_COEFF_G_PRE, 150);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AE_YSUM_COEFF_B_PRE, 29);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE_AE_COEFF, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_DNR_B);
    /* yuv output 16bit mode */
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_BLBACK_OUT_FORMAT_W, 1);
    /* YUYV order */
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_BLBACK_CBCR_SELECT_W, 1);

    BL_WR_REG(ISP_BASE, ISP_BLBACK_DNR_B, tmpVal);
#endif
    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_AWB_SETTING);
    tmpVal = BL_SET_REG_BIT(tmpVal, ISP_MIDDLE_BL_AWB_GAIN_SEL_W);
#if (BL80X_HW_VER >= BL80X_HW_B0)
    tmpVal = BL_SET_REG_BIT(tmpVal, ISP_MIDDLE_BL_DIGITAL_GAIN_SEL_W);
#endif
    BL_WR_REG(ISP_BASE, ISP_MIDDLE_AWB_SETTING, tmpVal);
}

void ISP_Get_IP_Info(int *version, int *ip_id)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_TG_IP_INFO);
    *version = BL_GET_REG_BITS_VAL(tmpVal, ISP_TG_VERSION_R);
    tmpVal = BL_RD_REG(ISP_BASE, ISP_TG_IP_INFO);
    *ip_id = BL_GET_REG_BITS_VAL(tmpVal, ISP_TG_IP_ID_R);

}

void ISP_Set_TG_Reshape_Cfg()
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_TG_HVCOUNT_CTRL);
    tmpVal = BL_SET_REG_BIT(tmpVal, ISP_TG_TG_CTRL_START_W);
    tmpVal = BL_CLR_REG_BIT(tmpVal, ISP_TG_AUTO_HVSYNC_GEN_W);
    tmpVal = BL_SET_REG_BIT(tmpVal, ISP_TG_TG_CTRL_OSTART_W);
    tmpVal = BL_CLR_REG_BIT(tmpVal, ISP_TG_AUTO_OHVSYNC_GEN_W);
    BL_WR_REG(ISP_BASE, ISP_TG_HVCOUNT_CTRL, tmpVal);
}

void ISP_Set_TG_Cfg(const ISP_TG_Cfg_Type *cfg)
{
    isp_tg_reg_t *isp_tg_reg = (isp_tg_reg_t*)ISP_BASE;

    uint16_t act_start_pixel = 0;
    uint16_t act_pixel_width = 0;
    uint16_t act_start_line  = 0;
    uint16_t act_line_height = 0;

    uint16_t out_pixel_width = 0;
    uint16_t out_line_height = 0;

    uint16_t out_start_pixel = 0;
    uint16_t out_start_line = 0;

    /* TODO: crop the best window */
    if (ISP_SYNC_MODE_INPUT_VBLANK_NO_TOGGLE == cfg->sync_mode_value) {
        act_start_pixel = 0;    /* hcount reset in rising edge, so H blanking is put after valid line */
        act_pixel_width = cfg->active_width;
        act_start_line  = 0;
        act_line_height = cfg->active_height;
    }
    else {
        act_start_pixel = cfg->total_width - cfg->active_width;
        act_pixel_width = cfg->active_width;
        act_start_line  = cfg->total_height - cfg->active_height;
        act_line_height = cfg->active_height;
    }

#ifdef COLOR_HDMI_480P25
    out_pixel_width = 720;
    out_line_height = 480;
#else
    out_pixel_width = cfg->out_width;
    out_line_height = cfg->out_height;
#endif

    out_start_pixel = (act_pixel_width - out_pixel_width)/2;
    out_start_line  = (act_line_height - out_line_height)/2;

#if (BL80X_HW_VER >= BL80X_HW_B0)
    /* further reduce frame latency by enabling act_vsync reset mode on vsync edge */
    isp_tg_reg->dvp_mode.BF.reg_act_vsync_rst_en_w = 1;
#endif

    /* set sync mode */
    isp_tg_reg->dvp_mode.BF.sync_mode_w = cfg->sync_mode_value;
    isp_tg_reg->dvp_mode.BF.hsync_in_inv_w = ISP_HSYNC_INVERT_NORMAL;
    /* dvp_tsrc would do vsync invert so vsync pulse is always active low */
    isp_tg_reg->dvp_mode.BF.vsync_in_inv_w = ISP_VSYNC_INVERT_NORMAL;
    isp_tg_reg->dvp_mode.BF.fhblk_line_on_w = ISP_FHBLK_LINE_OFF;

    /* note: if vsync invert, vcnt reset mode set 0, means falling
        else if vsync not invert,  vcont reset mode set 1, also means falling
        this value will not open to user now.
    */
    if (ISP_SYNC_MODE_INPUT_VBLANK_NO_TOGGLE == cfg->sync_mode_value) {
        /* hcnt reset in rising edge, so H blanking is put after valid line
            vcnt reset mode must be 0, to make sure vcnt is reset to 4095 instead of 0. */
        isp_tg_reg->dvp_mode.BF.hcnt_rst_mode_w = ISP_HCNT_RST_RISING;
        isp_tg_reg->dvp_mode.BF.vcnt_rst_mode_w = ISP_VCNT_RST_RISING;
    }
    else {
        isp_tg_reg->dvp_mode.BF.hcnt_rst_mode_w = ISP_HCNT_RST_FALLING;
        isp_tg_reg->dvp_mode.BF.vcnt_rst_mode_w = ISP_HCNT_RST_FALLING;
    }

    /* set total pixels and total lines */
    isp_tg_reg->total_pixels.BF.total_hpixels_w =
        ISP_Calc_Total_Width(cfg);

    isp_tg_reg->total_pixels.BF.total_vlines_w = cfg->total_height;;

    /* act start pixel, act pixel width */
    isp_tg_reg->act_pixels_1.BF.act_start_pixel_w = act_start_pixel;
    isp_tg_reg->act_pixels_1.BF.act_pixel_width_w = act_pixel_width;

    /* act start line, act line height */
    isp_tg_reg->act_pixels_2.BF.act_start_line_w = act_start_line;
    isp_tg_reg->act_pixels_2.BF.act_line_height_w = act_line_height;

    /* out start pixel, out pixel width */
    isp_tg_reg->fact_pixels_1.BF.fact_start_pixel_w = out_start_pixel;
    isp_tg_reg->fact_pixels_1.BF.fact_pixel_width_w = out_pixel_width;

    /* out start line, out line height*/
    isp_tg_reg->fact_pixels_2.BF.fact_start_line_w = out_start_line;

#if !ENCODE_1080P_SVGA
    isp_tg_reg->fact_pixels_2.BF.fact_line_height_w = out_line_height;
#else
    isp_tg_reg->fact_pixels_2.BF.fact_line_height_w = ENC_HEIGHT(out_line_height);
#endif

    ISP_Print_TG();
}

void ISP_Set_DVP_Mode_Cfg(ISP_DVP_MODE_CFG_T *cfg)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_TG_DVP_MODE);
    if (cfg->fhblk_line_on == 1) tmpVal = BL_SET_REG_BIT(tmpVal, ISP_TG_FHBLK_LINE_ON_W);
    else tmpVal = BL_CLR_REG_BIT(tmpVal, ISP_TG_FHBLK_LINE_ON_W);

    if (cfg->hblk_line_on == 1) tmpVal = BL_SET_REG_BIT(tmpVal, ISP_TG_HBLK_LINE_ON_W);
    else tmpVal = BL_CLR_REG_BIT(tmpVal, ISP_TG_HBLK_LINE_ON_W);

    if (cfg->vcnt_rst_mode == 1) tmpVal = BL_SET_REG_BIT(tmpVal, ISP_TG_VCNT_RST_MODE_W);
    else tmpVal = BL_CLR_REG_BIT(tmpVal, ISP_TG_VCNT_RST_MODE_W);

    if (cfg->hcnt_rst_mode == 1) tmpVal = BL_SET_REG_BIT(tmpVal, ISP_TG_HCNT_RST_MODE_W);
    else tmpVal = BL_CLR_REG_BIT(tmpVal, ISP_TG_HCNT_RST_MODE_W);

    if (cfg->vsync_in_inv == 1) tmpVal = BL_SET_REG_BIT(tmpVal, ISP_TG_VSYNC_IN_INV_W);
    else tmpVal = BL_CLR_REG_BIT(tmpVal, ISP_TG_VSYNC_IN_INV_W);

    if (cfg->hsync_in_inv == 1) tmpVal = BL_SET_REG_BIT(tmpVal, ISP_TG_HSYNC_IN_INV_W);
    else tmpVal = BL_CLR_REG_BIT(tmpVal, ISP_TG_HSYNC_IN_INV_W);

    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_TG_SYNC_MODE_W, cfg->sync_mode);

    if (cfg->reg_act_vsync_rst_en == 1) tmpVal = BL_SET_REG_BIT(tmpVal, ISP_TG_REG_ACT_VSYNC_RST_EN_W);
    else tmpVal = BL_CLR_REG_BIT(tmpVal, ISP_TG_REG_ACT_VSYNC_RST_EN_W);

    BL_WR_REG(ISP_BASE, ISP_TG_DVP_MODE, tmpVal);

}

void ISP_Get_DVP_Mode_Cfg(ISP_DVP_MODE_CFG_T *cfg)
{
    isp_tg_reg_t *isp_tg_reg = (isp_tg_reg_t*)ISP_BASE;

    cfg->reg_act_vsync_rst_en = isp_tg_reg->dvp_mode.BF.reg_act_vsync_rst_en_w;
    cfg->sync_mode      = isp_tg_reg->dvp_mode.BF.sync_mode_w;
    cfg->hsync_in_inv   = isp_tg_reg->dvp_mode.BF.hsync_in_inv_w;
    cfg->vsync_in_inv   = isp_tg_reg->dvp_mode.BF.vsync_in_inv_w;
    cfg->hcnt_rst_mode  = isp_tg_reg->dvp_mode.BF.hcnt_rst_mode_w;
    cfg->vcnt_rst_mode  = isp_tg_reg->dvp_mode.BF.vcnt_rst_mode_w;
    cfg->hblk_line_on   = isp_tg_reg->dvp_mode.BF.hblk_line_on_w;
    cfg->fhblk_line_on  = isp_tg_reg->dvp_mode.BF.fhblk_line_on_w;

}

void ISP_Set_Total_Pixels(BL_Img_Size_T *out_win)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_TG_TOTAL_PIXELS);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_TG_TOTAL_HPIXELS_W, out_win->width);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_TG_TOTAL_VLINES_W, out_win->height);
    BL_WR_REG(ISP_BASE, ISP_TG_TOTAL_PIXELS, tmpVal);
}

void ISP_Get_Total_Pixels(BL_Img_Size_T *out_win)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_TG_TOTAL_PIXELS);
    out_win->width = BL_GET_REG_BITS_VAL(tmpVal, ISP_TG_TOTAL_HPIXELS_W);
    out_win->height = BL_GET_REG_BITS_VAL(tmpVal, ISP_TG_TOTAL_VLINES_W);
}

void ISP_Set_Active_Pixels(int start_pixel, int start_line, BL_Img_Size_T *out_win)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_TG_ACT_PIXELS_1);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_TG_ACT_PIXEL_WIDTH_W, out_win->width);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_TG_ACT_START_PIXEL_W, start_pixel);
    BL_WR_REG(ISP_BASE, ISP_TG_ACT_PIXELS_1, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_TG_ACT_PIXELS_2);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_TG_ACT_LINE_HEIGHT_W, out_win->height);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_TG_ACT_START_LINE_W, start_line);
    BL_WR_REG(ISP_BASE, ISP_TG_ACT_PIXELS_2, tmpVal);
}

void ISP_Get_Active_Pixels(int *start_pixel, int *start_line, BL_Img_Size_T *out_win)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_TG_ACT_PIXELS_1);
    out_win->width = BL_GET_REG_BITS_VAL(tmpVal, ISP_TG_ACT_PIXEL_WIDTH_W);
    *start_pixel = BL_GET_REG_BITS_VAL(tmpVal, ISP_TG_ACT_START_PIXEL_W);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_TG_ACT_PIXELS_2);
    out_win->height = BL_GET_REG_BITS_VAL(tmpVal, ISP_TG_ACT_LINE_HEIGHT_W);
    *start_line = BL_GET_REG_BITS_VAL(tmpVal, ISP_TG_ACT_START_LINE_W);
}

void ISP_Set_Output_Pixels(int start_pixel, int start_line, BL_Img_Size_T *out_win)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_TG_FACT_PIXELS_1);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_TG_FACT_PIXEL_WIDTH_W, out_win->width);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_TG_FACT_START_PIXEL_W, start_pixel);
    BL_WR_REG(ISP_BASE, ISP_TG_FACT_PIXELS_1, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_TG_FACT_PIXELS_2);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_TG_FACT_LINE_HEIGHT_W, out_win->height);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_TG_FACT_START_LINE_W, start_line);
    BL_WR_REG(ISP_BASE, ISP_TG_FACT_PIXELS_2, tmpVal);
}

void ISP_Get_Output_Pixels(int *start_pixel, int *start_line, BL_Img_Size_T *out_win)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_TG_FACT_PIXELS_1);
    out_win->width = BL_GET_REG_BITS_VAL(tmpVal, ISP_TG_FACT_PIXEL_WIDTH_W);
    *start_pixel = BL_GET_REG_BITS_VAL(tmpVal, ISP_TG_FACT_START_PIXEL_W);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_TG_FACT_PIXELS_2);
    out_win->height = BL_GET_REG_BITS_VAL(tmpVal, ISP_TG_FACT_LINE_HEIGHT_W);
    *start_line = BL_GET_REG_BITS_VAL(tmpVal, ISP_TG_FACT_START_LINE_W);
}

void ISP_Set_Hsync_Pixels(int start_pixel, int start_line, BL_Img_Size_T *out_win)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_TG_HSYNC_PIXEL);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_TG_HSYNC_PIXEL_WIDTH_W, out_win->width);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_TG_HSYNC_START_PIXEL_W, start_pixel);
    BL_WR_REG(ISP_BASE, ISP_TG_HSYNC_PIXEL, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_TG_HSYNC_LINE);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_TG_HSYNC_LINE_HEIGHT_W, out_win->height);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_TG_HSYNC_START_LINE_W, start_line);
    BL_WR_REG(ISP_BASE, ISP_TG_HSYNC_LINE, tmpVal);
}

void ISP_Get_Hsync_Pixels(int *start_pixel, int *start_line, BL_Img_Size_T *out_win)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_TG_HSYNC_PIXEL);
    out_win->width = BL_GET_REG_BITS_VAL(tmpVal, ISP_TG_HSYNC_PIXEL_WIDTH_W);
    *start_pixel = BL_GET_REG_BITS_VAL(tmpVal, ISP_TG_HSYNC_START_PIXEL_W);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_TG_HSYNC_LINE);
    out_win->height = BL_GET_REG_BITS_VAL(tmpVal, ISP_TG_HSYNC_LINE_HEIGHT_W);
    *start_line = BL_GET_REG_BITS_VAL(tmpVal, ISP_TG_HSYNC_START_LINE_W);
}

void ISP_Set_Vsync_Pixels(int start_pixel, int start_line, int end_pixel, int line_height)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_TG_VSYNC_LINE);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_TG_VSYNC_LINE_HEIGHT_W, line_height);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_TG_VSYNC_START_LINE_W, start_line);
    BL_WR_REG(ISP_BASE, ISP_TG_VSYNC_LINE, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_TG_VSYNC_PIXEL);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_TG_VSYNC_END_PIXEL_W, end_pixel);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_TG_VSYNC_START_PIXEL_W, start_pixel);
    BL_WR_REG(ISP_BASE, ISP_TG_VSYNC_PIXEL, tmpVal);
}

void ISP_Get_Vsync_Pixels(int *start_pixel, int *start_line, int *end_pixel, int *line_height)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_TG_VSYNC_LINE);
    *line_height = BL_GET_REG_BITS_VAL(tmpVal, ISP_TG_VSYNC_LINE_HEIGHT_W);
    *start_line = BL_GET_REG_BITS_VAL(tmpVal, ISP_TG_VSYNC_START_LINE_W);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_TG_VSYNC_PIXEL);
    *end_pixel = BL_GET_REG_BITS_VAL(tmpVal, ISP_TG_VSYNC_END_PIXEL_W);
    *start_pixel = BL_GET_REG_BITS_VAL(tmpVal, ISP_TG_VSYNC_START_PIXEL_W);
}

int ISP_Set_Output_Window(BL_Img_Size_T *out_win)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_TG_FACT_PIXELS_1);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_TG_FACT_PIXEL_WIDTH_W, out_win->width);
    BL_WR_REG(ISP_BASE, ISP_TG_FACT_PIXELS_1, tmpVal);
    
    tmpVal = BL_RD_REG(ISP_BASE, ISP_TG_FACT_PIXELS_2);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_TG_FACT_LINE_HEIGHT_W, out_win->height);
    BL_WR_REG(ISP_BASE, ISP_TG_FACT_PIXELS_2, tmpVal);

    return 0;
}

int ISP_Get_Output_Window(BL_Img_Size_T *out_win)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_TG_FACT_PIXELS_1);
    out_win->width = BL_GET_REG_BITS_VAL(tmpVal, ISP_TG_FACT_PIXEL_WIDTH_W);
    tmpVal = BL_RD_REG(ISP_BASE, ISP_TG_FACT_PIXELS_2);
    out_win->height = BL_GET_REG_BITS_VAL(tmpVal, ISP_TG_FACT_LINE_HEIGHT_W);

    return 0;
}

void ISP_Set_Bayer_Pattern(const BAYER_PATTERN_E pattern)
{
    isp_front_reg_t *isp_front_reg = (isp_front_reg_t*)ISP_BASE;

    isp_front_reg->pattern_mode.BF.pattern_mode_w = 0;

#if (BL80X_HW_VER >= BL80X_HW_B0)
    /* fix issue: bayer pattern highly sensitive to total width setting */
    isp_front_reg->pattern_mode.BF.reg_bl_lpid_sel_w = 1;
#endif

    if (pattern == BAYER_PATTERN_BG) {
        /* lid invert depends on the total height */
        isp_front_reg->pattern_mode.BF.pid_inv_rgb_pre = 1;
        isp_front_reg->pattern_mode.BF.lid_inv_rgb_pre = 1;
    } else if (pattern == BAYER_PATTERN_RG) {
        isp_front_reg->pattern_mode.BF.pid_inv_rgb_pre = 0;
        isp_front_reg->pattern_mode.BF.lid_inv_rgb_pre = 0;
    } else if (pattern == BAYER_PATTERN_GR) {
        isp_front_reg->pattern_mode.BF.pid_inv_rgb_pre = 1;
        isp_front_reg->pattern_mode.BF.lid_inv_rgb_pre = 0;
    } else if (pattern == BAYER_PATTERN_GB) {
        isp_front_reg->pattern_mode.BF.pid_inv_rgb_pre = 0;
        isp_front_reg->pattern_mode.BF.lid_inv_rgb_pre = 1;
    } else {
        BL_LOGE("[error] error bayer pattern! \r\n");
        return;
    }
    /* hack for CISTA sensor */
    //isp_front_reg->pattern_mode.BF.pid_inv_rgb_pre = 1;
    //isp_front_reg->pattern_mode.BF.lid_inv_rgb_pre = 0;
}

void ISP_Get_Bayer_Pattern(BAYER_PATTERN_E *pattern)
{
    isp_front_reg_t *isp_front_reg = (isp_front_reg_t*)ISP_BASE;

    if ((isp_front_reg->pattern_mode.BF.pid_inv_rgb_pre == 1) &&
        (isp_front_reg->pattern_mode.BF.lid_inv_rgb_pre == 1)) {
        *pattern = BAYER_PATTERN_BG;
    } else if ((isp_front_reg->pattern_mode.BF.pid_inv_rgb_pre == 0) &&
        (isp_front_reg->pattern_mode.BF.lid_inv_rgb_pre == 0)) {
        *pattern = BAYER_PATTERN_RG;
    } else if ((isp_front_reg->pattern_mode.BF.pid_inv_rgb_pre == 1) &&
        (isp_front_reg->pattern_mode.BF.lid_inv_rgb_pre == 0)) {
        *pattern = BAYER_PATTERN_GR;
    } else if ((isp_front_reg->pattern_mode.BF.pid_inv_rgb_pre == 0) &&
        (isp_front_reg->pattern_mode.BF.lid_inv_rgb_pre == 1)) {
        *pattern = BAYER_PATTERN_GB;
    } else {
        BL_LOGE("[error] error bayer pattern! \r\n");
        return;
    }
}

void ISP_Set_BL444to422(int mode)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_DUMMY_CTRL);
    if (ENABLE == mode) {
        tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_BLBACK_REG_BL444TO422_EN_W, 1);
    } else {
        tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_BLBACK_REG_BL444TO422_EN_W, 0);
    }
    BL_WR_REG(ISP_BASE, ISP_BLBACK_DUMMY_CTRL, tmpVal);
}

void ISP_Get_BL444to422(int *mode)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_DUMMY_CTRL);
    *mode = BL_GET_REG_BITS_VAL(tmpVal, ISP_BLBACK_REG_BL444TO422_EN_W);
}

void ISP_Set_AE_State(BL_Fun_Type state)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_AE_GRID_SIZE);

    if (ENABLE == state) {
        tmpVal = BL_SET_REG_BIT(tmpVal, ISP_MIDDLE_NEW_AE_STA_ENABLE_W);
    } else {
        tmpVal = BL_CLR_REG_BIT(tmpVal, ISP_MIDDLE_NEW_AE_STA_ENABLE_W);
    }

    BL_WR_REG(ISP_BASE, ISP_MIDDLE_AE_GRID_SIZE, tmpVal);
}

void ISP_Set_AE_Stats_Conf(const isp_ae_stats_conf_t *ae_stats_conf)
{
    uint32_t tmpVal;

    CHECK_PARAM_MAX(ae_stats_conf->roi_start_x, ISP_AE_ROI_START_X_RANGE_MAX);
    CHECK_PARAM_MAX(ae_stats_conf->roi_start_y, ISP_AE_ROI_START_Y_RANGE_MAX);
    CHECK_PARAM_RANGE(ae_stats_conf->roi_width,
        ISP_AE_ROI_WIDTH_RANGE_MIN, ISP_AE_ROI_WIDTH_RANGE_MAX);
    CHECK_PARAM_RANGE(ae_stats_conf->roi_height,
        ISP_AE_ROI_HEIGHT_RANGE_MIN, ISP_AE_ROI_HEIGHT_RANGE_MAX);
    CHECK_PARAM_MIN(ae_stats_conf->grid_width, ISP_AE_GRID_WIDTH_RANGE_MIN);
    CHECK_PARAM_MIN(ae_stats_conf->grid_height, ISP_AE_GRID_HEIGHT_RANGE_MIN);
    CHECK_PARAM_RANGE(ae_stats_conf->grid_num_x,
        ISP_AE_GRID_NUM_X_RANGE_MIN, ISP_AE_GRID_NUM_X_RANGE_MAX);
    CHECK_PARAM_RANGE(ae_stats_conf->grid_num_y,
        ISP_AE_GRID_NUM_Y_RANGE_MIN, ISP_AE_GRID_NUM_Y_RANGE_MAX);

    /* special check */

    /* AE ROI must be in the active image region */
    if ((ae_stats_conf->roi_start_x + ae_stats_conf->roi_width > ae_stats_conf->img_width) ||
        (ae_stats_conf->roi_start_y + ae_stats_conf->roi_height > ae_stats_conf->img_height)) {
        BL_LOGE("AE ROI must be in the active image region!\r\n");
        BL_ABORT();
    }

    /* grid width/height must be even number */
    if ((ae_stats_conf->grid_width & 0x1) & (ae_stats_conf->grid_height & 0x1)) {
        BL_LOGE("Grid width/height must be even number!\r\n");
        BL_ABORT();
    }

#if (BL80X_HW_VER <= BL80X_HW_A0)
    tmpVal = BL_RD_REG(ISP_BASE, ISP_FRONT_AE_ROI_X);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_FRONT_AE_ROI_START_X_PRE, ae_stats_conf->roi_start_x);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_FRONT_AE_ROI_WIDTH_PRE, ae_stats_conf->roi_width);
    BL_WR_REG(ISP_BASE, ISP_FRONT_AE_ROI_X, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_FRONT_AE_ROI_Y);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_FRONT_AE_ROI_START_Y_PRE, ae_stats_conf->roi_start_y);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_FRONT_AE_ROI_HEIGHT_PRE, ae_stats_conf->roi_height);
    BL_WR_REG(ISP_BASE, ISP_FRONT_AE_ROI_Y, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_FRONT_AE_IMAGE_SIZE);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_FRONT_AE_IMAGE_WIDTH_PRE, ae_stats_conf->img_width);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_FRONT_AE_IMAGE_HEIGHT_PRE, ae_stats_conf->img_height);
    BL_WR_REG(ISP_BASE, ISP_FRONT_AE_IMAGE_SIZE, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_FRONT_AE_GRID_SIZE);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_FRONT_AE_GRID_WIDTH_PRE, ae_stats_conf->grid_width);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_FRONT_AE_GRID_HEIGHT_PRE, ae_stats_conf->grid_height);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_FRONT_AE_GRID_X_MAX_PRE, ae_stats_conf->grid_num_x);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_FRONT_AE_GRID_Y_MAX_PRE, ae_stats_conf->grid_num_y);
    tmpVal = BL_SET_REG_BIT(tmpVal, ISP_FRONT_NEW_AE_STA_ENABLE_W);
    /* AE occupies 1 bank of memory (doesn't switch for each image) */
    tmpVal = BL_CLR_REG_BIT(tmpVal, ISP_FRONT_AE_SWITCH_BANK_EN_PRE);
    BL_WR_REG(ISP_BASE, ISP_FRONT_AE_GRID_SIZE, tmpVal);
#else
    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_AE_ROI_X);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AE_ROI_START_X_PRE, ae_stats_conf->roi_start_x);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AE_ROI_WIDTH_PRE, ae_stats_conf->roi_width);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE_AE_ROI_X, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_AE_ROI_Y);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AE_ROI_START_Y_PRE, ae_stats_conf->roi_start_y);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AE_ROI_HEIGHT_PRE, ae_stats_conf->roi_height);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE_AE_ROI_Y, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_AE_IMAGE_SIZE);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AE_IMAGE_WIDTH_PRE, ae_stats_conf->img_width);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AE_IMAGE_HEIGHT_PRE, ae_stats_conf->img_height);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE_AE_IMAGE_SIZE, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_AE_GRID_SIZE);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AE_GRID_WIDTH_PRE, ae_stats_conf->grid_width);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AE_GRID_HEIGHT_PRE, ae_stats_conf->grid_height);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AE_GRID_X_MAX_PRE, ae_stats_conf->grid_num_x);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AE_GRID_Y_MAX_PRE, ae_stats_conf->grid_num_y);
    tmpVal = BL_SET_REG_BIT(tmpVal, ISP_MIDDLE_NEW_AE_STA_ENABLE_W);
    /* AE occupies 2 bank of memory*/
    tmpVal = BL_SET_REG_BIT(tmpVal, ISP_MIDDLE_AE_SWITCH_BANK_EN_PRE);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE_AE_GRID_SIZE, tmpVal);
#endif
}

void ISP_Get_AE_Stats_Conf(isp_ae_stats_conf_t *ae_stats_conf)
{
    uint32_t tmpVal;

#if (BL80X_HW_VER <= BL80X_HW_A0)
    tmpVal = BL_RD_REG(ISP_BASE, ISP_FRONT_AE_ROI_X);
    ae_stats_conf->roi_start_x = BL_GET_REG_BITS_VAL(tmpVal, ISP_FRONT_AE_ROI_START_X_PRE);
    ae_stats_conf->roi_width = BL_GET_REG_BITS_VAL(tmpVal, ISP_FRONT_AE_ROI_WIDTH_PRE);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_FRONT_AE_ROI_Y);
    ae_stats_conf->roi_start_y = BL_GET_REG_BITS_VAL(tmpVal, ISP_FRONT_AE_ROI_START_Y_PRE);
    ae_stats_conf->roi_height = BL_GET_REG_BITS_VAL(tmpVal, ISP_FRONT_AE_ROI_HEIGHT_PRE);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_FRONT_AE_IMAGE_SIZE);
    ae_stats_conf->img_width = BL_GET_REG_BITS_VAL(tmpVal, ISP_FRONT_AE_IMAGE_WIDTH_PRE);
    ae_stats_conf->img_height = BL_GET_REG_BITS_VAL(tmpVal, ISP_FRONT_AE_IMAGE_HEIGHT_PRE);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_FRONT_AE_GRID_SIZE);
    ae_stats_conf->grid_width = BL_GET_REG_BITS_VAL(tmpVal, ISP_FRONT_AE_GRID_WIDTH_PRE);
    ae_stats_conf->grid_height = BL_GET_REG_BITS_VAL(tmpVal, ISP_FRONT_AE_GRID_HEIGHT_PRE);
    ae_stats_conf->grid_width = BL_GET_REG_BITS_VAL(tmpVal, ISP_FRONT_AE_GRID_X_MAX_PRE);
    ae_stats_conf->grid_height = BL_GET_REG_BITS_VAL(tmpVal, ISP_FRONT_AE_GRID_Y_MAX_PRE);
#else
    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_AE_ROI_X);
    ae_stats_conf->roi_start_x = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AE_ROI_START_X_PRE);
    ae_stats_conf->roi_width = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AE_ROI_WIDTH_PRE);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_AE_ROI_Y);
    ae_stats_conf->roi_start_y = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AE_ROI_START_Y_PRE);
    ae_stats_conf->roi_height = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AE_ROI_HEIGHT_PRE);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_AE_IMAGE_SIZE);
    ae_stats_conf->img_width = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AE_IMAGE_WIDTH_PRE);
    ae_stats_conf->img_height = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AE_IMAGE_HEIGHT_PRE);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_AE_GRID_SIZE);
    ae_stats_conf->grid_width = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AE_GRID_WIDTH_PRE);
    ae_stats_conf->grid_height = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AE_GRID_HEIGHT_PRE);
    ae_stats_conf->grid_width = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AE_GRID_X_MAX_PRE);
    ae_stats_conf->grid_height = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AE_GRID_Y_MAX_PRE);
#endif
}

void ISP_Set_AE_Ysum_Coeff(uint8_t coeff_r, uint8_t coeff_g, uint8_t coeff_b)
{
    uint32_t tmpVal;

#if (BL80X_HW_VER <= BL80X_HW_A0)
    tmpVal = BL_RD_REG(ISP_BASE, ISP_FRONT_AE_COEFF);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_FRONT_AE_YSUM_COEFF_R_PRE, coeff_r);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_FRONT_AE_YSUM_COEFF_G_PRE, coeff_g);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_FRONT_AE_YSUM_COEFF_B_PRE, coeff_b);
    BL_WR_REG(ISP_BASE, ISP_FRONT_AE_COEFF, tmpVal);
#else
    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_AE_COEFF);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AE_YSUM_COEFF_R_PRE, coeff_r);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AE_YSUM_COEFF_G_PRE, coeff_g);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AE_YSUM_COEFF_B_PRE, coeff_b);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE_AE_COEFF, tmpVal);
#endif
}

void ISP_Get_AE_Ysum_Coeff(uint8_t *coeff_r, uint8_t *coeff_g, uint8_t *coeff_b)
{
    uint32_t tmpVal;

#if (BL80X_HW_VER <= BL80X_HW_A0)
    tmpVal = BL_RD_REG(ISP_BASE, ISP_FRONT_AE_COEFF);
    *coeff_r = BL_GET_REG_BITS_VAL(tmpVal, ISP_FRONT_AE_YSUM_COEFF_R_PRE);
    *coeff_g = BL_GET_REG_BITS_VAL(tmpVal, ISP_FRONT_AE_YSUM_COEFF_G_PRE);
    *coeff_b = BL_SET_REG_BITS_VAL(tmpVal, ISP_FRONT_AE_YSUM_COEFF_B_PRE);

#else
    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_AE_COEFF);
    *coeff_r = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AE_YSUM_COEFF_R_PRE);
    *coeff_g = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AE_YSUM_COEFF_G_PRE);
    *coeff_b = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AE_YSUM_COEFF_B_PRE);

#endif
}

#if USE_AEAWB_SRAM
int ISP_Get_AE_Stats(int grid_num, uint32_t *data)
{
    uint32_t tmpVal;
    uint32_t buf_idx;
    uint32_t buf_addr;
    uint32_t w_cnt_idx_start;
    uint32_t w_cnt_idx_end;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_AE_STATUS);
    buf_idx = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_BLAE_BUF_IDX_R);
    w_cnt_idx_start = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_BLAE_W_CNT_R);

    buf_addr = (0 == buf_idx) ? ISP_AE_SRAM_BANK1: ISP_AE_SRAM_BANK2;
    memcpy(data, (uint32_t*)buf_addr, grid_num*2); // two word per grid

    w_cnt_idx_end = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_BLAE_W_CNT_R);
    if (w_cnt_idx_start != w_cnt_idx_end) {
        BL_LOGW("ae statistics overwrite!");
        return -1;
    }

    return 0;
}

void ISP_Get_AE_Sum(int grid_idx, AE_GRID_DATA_TYPE_E data_type, uint32_t *data)
{
    isp_ae_sum_t *grid_array;
    uint32_t tmpVal;
    uint32_t buf_idx;
    uint32_t buf_addr;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_AE_STATUS);
    buf_idx = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_BLAE_BUF_IDX_R);

    buf_addr = (0 == buf_idx) ? ISP_AE_SRAM_BANK1: ISP_AE_SRAM_BANK2;
    grid_array = (isp_ae_sum_t*)buf_addr;
    *data = grid_array[grid_idx].y_sum;
}
#else // not use USE_AEAWB_SRAM
void ISP_Get_AE_Sum(int grid_idx, AE_GRID_DATA_TYPE_E data_type, uint32_t *data)
{
    isp_blae_reg_t *isp_blae_reg = (isp_blae_reg_t*)ISP_BASE;
#if 0
    int grid_idx_offset = isp_blae_reg->blae_read_grid_addr.BF.blae_buf_idx ? (96 << 1) : 0;
#else
    int grid_idx_offset = 0;
#endif
    isp_blae_reg->blae_read_grid_addr.BF.blae_read_grid_addr = (grid_idx << 1) + data_type + grid_idx_offset;
    *data = isp_blae_reg->blae_grid_data.WORD;
}
#endif

void ISP_Set_AE_Hist_State(BL_Fun_Type state)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_AE_GRID_SIZE);
    if (ENABLE == state) {
        tmpVal = BL_SET_REG_BIT(tmpVal, ISP_MIDDLE_AE_HIST_EN_PRE);
    } else {
        tmpVal = BL_CLR_REG_BIT(tmpVal, ISP_MIDDLE_AE_HIST_EN_PRE);
    }
    BL_WR_REG(ISP_BASE, ISP_MIDDLE_AE_GRID_SIZE, tmpVal);
}

int  ISP_Get_AE_Hist_State(BL_Fun_Type *state)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_AE_GRID_SIZE);

    if (BL_IS_REG_BIT_SET(tmpVal, ISP_MIDDLE_AE_HIST_EN_PRE)) {
        *state = ENABLE;
    } else {
        *state = DISABLE;
    }

    return 0;
}

void ISP_Set_AE_Hist_Mode(AE_HIST_MODE_TYPE_E mode)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_AE_GRID_SIZE);
    if (AE_HIST_MODE_Y == mode) {
        tmpVal = BL_SET_REG_BIT(tmpVal, ISP_MIDDLE_AE_HIST_MODE_PRE);
    } else {
        tmpVal = BL_CLR_REG_BIT(tmpVal, ISP_MIDDLE_AE_HIST_MODE_PRE);
    }
    BL_WR_REG(ISP_BASE, ISP_MIDDLE_AE_GRID_SIZE, tmpVal);
}

int  ISP_Get_AE_Hist_Mode(AE_HIST_MODE_TYPE_E *mode)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_AE_GRID_SIZE);

    if (BL_IS_REG_BIT_SET(tmpVal, ISP_MIDDLE_AE_HIST_EN_PRE)) {
        *mode = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AE_HIST_MODE_PRE);
    } else {
        *mode = AE_HIST_MODE_Y;
    }

    return 0;
}

void ISP_Set_AE_Hist_ROI_State(BL_Fun_Type state)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_AE_GRID_SIZE);
    if (ENABLE == state) {
        tmpVal = BL_SET_REG_BIT(tmpVal, ISP_MIDDLE_AE_HIST_ROI_EN_PRE);
    } else {
        tmpVal = BL_CLR_REG_BIT(tmpVal, ISP_MIDDLE_AE_HIST_ROI_EN_PRE);
    }
    BL_WR_REG(ISP_BASE, ISP_MIDDLE_AE_GRID_SIZE, tmpVal);
}

int  ISP_Get_AE_Hist_ROI_State(BL_Fun_Type *state)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_AE_GRID_SIZE);

    if (BL_IS_REG_BIT_SET(tmpVal, ISP_MIDDLE_AE_HIST_ROI_EN_PRE)) {
        *state = ENABLE;
    } else {
        *state = DISABLE;
    }

    return 0;
}

int  ISP_Set_AE_Hist_ROI(BL_Fun_Type state, const isp_ae_hist_roi_t *roi)
{
    uint32_t tmpVal;

    if (DISABLE == state) {
        tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_AE_GRID_SIZE);
        tmpVal = BL_CLR_REG_BIT(tmpVal, ISP_MIDDLE_AE_HIST_ROI_EN_PRE);
        BL_WR_REG(ISP_BASE, ISP_MIDDLE_AE_GRID_SIZE, tmpVal);
        return 0;
    }

    if (NULL == roi) {
        return -1;
    }

    CHECK_PARAM_MAX(roi->ae_hist_x_min, ISP_AE_HIST_ROI_MAX);
    CHECK_PARAM_MAX(roi->ae_hist_x_max, ISP_AE_HIST_ROI_MAX);
    CHECK_PARAM_MAX(roi->ae_hist_y_min, ISP_AE_HIST_ROI_MAX);
    CHECK_PARAM_MAX(roi->ae_hist_y_max, ISP_AE_HIST_ROI_MAX);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_AE2_ROI_X);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AE_HIST_X_MIN_PRE, roi->ae_hist_x_min);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AE_HIST_X_MAX_PRE, roi->ae_hist_x_max);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE_AE2_ROI_X, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_AE2_ROI_Y);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AE_HIST_Y_MIN_PRE, roi->ae_hist_y_min);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AE_HIST_Y_MAX_PRE, roi->ae_hist_y_max);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE_AE2_ROI_Y, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_AE_GRID_SIZE);
    tmpVal = BL_SET_REG_BIT(tmpVal, ISP_MIDDLE_AE_HIST_ROI_EN_PRE);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE_AE_GRID_SIZE, tmpVal);

    return 0;
}

int  ISP_Get_AE_Hist_ROI(BL_Fun_Type state, isp_ae_hist_roi_t *roi)
{
    uint32_t tmpVal;

    if (DISABLE == state) {
        tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_AE_GRID_SIZE);
        tmpVal = BL_CLR_REG_BIT(tmpVal, ISP_MIDDLE_AE_HIST_ROI_EN_PRE);
        BL_WR_REG(ISP_BASE, ISP_MIDDLE_AE_GRID_SIZE, tmpVal);
        return 0;
    }

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_AE2_ROI_X);
    roi->ae_hist_x_min = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AE_HIST_X_MIN_PRE);
    roi->ae_hist_x_max = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AE_HIST_X_MAX_PRE);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_AE2_ROI_Y);
    roi->ae_hist_y_min = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AE_HIST_Y_MIN_PRE);
    roi->ae_hist_y_max = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AE_HIST_Y_MAX_PRE);

    return 0;
}

int  ISP_Get_AE_Hist_SUM_Y(uint32_t *y_sum)
{
    uint32_t tmpVal;
    uint32_t w_cnt_idx_start;
    uint32_t w_cnt_idx_end;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_AE_STATUS);
    w_cnt_idx_start = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AE_HIST_W_CNT_R);

    memcpy(y_sum, (uint32_t*)AE_HIST_SUM_ADDR_Y, ISP_AE_HIST_Y_BIN_NUM);

    w_cnt_idx_end = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AE_HIST_W_CNT_R);
    if (w_cnt_idx_start != w_cnt_idx_end) {
        BL_LOGW("ae hist y sum overwrite!");
        return -1;
    }

    return 0;
}

int  ISP_Get_AE_Hist_SUM_Bayer(uint32_t *r_sum, uint32_t *gr_sum, uint32_t *gb_sum, uint32_t *b_sum)
{
    uint32_t tmpVal;
    uint32_t w_cnt_idx_start;
    uint32_t w_cnt_idx_end;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_AE_STATUS);
    w_cnt_idx_start = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AE_HIST_W_CNT_R);

    memcpy(r_sum,  (uint32_t*)AE_HIST_SUM_ADDR_R,  ISP_AE_HIST_BAYER_BIN_NUM);
    memcpy(gr_sum, (uint32_t*)AE_HIST_SUM_ADDR_GR, ISP_AE_HIST_BAYER_BIN_NUM);
    memcpy(gb_sum, (uint32_t*)AE_HIST_SUM_ADDR_GB, ISP_AE_HIST_BAYER_BIN_NUM);
    memcpy(b_sum,  (uint32_t*)AE_HIST_SUM_ADDR_B,  ISP_AE_HIST_BAYER_BIN_NUM);

    w_cnt_idx_end = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AE_HIST_W_CNT_R);
    if (w_cnt_idx_start != w_cnt_idx_end) {
        BL_LOGW("ae hist bayer sum overwrite!");
        return -1;
    }

    return 0;
}


void ISP_Set_AWB_State(BL_Fun_Type state)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_AWB_SETTING);
    if (ENABLE == state) {
        tmpVal = BL_SET_REG_BIT(tmpVal, ISP_MIDDLE_NEW_AWB_STA_ENABLE_W);
    } else {
        tmpVal = BL_CLR_REG_BIT(tmpVal, ISP_MIDDLE_NEW_AWB_STA_ENABLE_W);
    }

    BL_WR_REG(ISP_BASE, ISP_MIDDLE_AWB_SETTING, tmpVal);
}

void ISP_Set_AWB_Stats_Conf(const isp_awb_stats_conf_t *awb_stats_conf)
{
    isp_middle_reg_t *isp_middle_reg = (isp_middle_reg_t*)ISP_BASE;

    CHECK_PARAM_MAX(awb_stats_conf->roi_start_x, ISP_AWB_ROI_START_X_RANGE_MAX);
    CHECK_PARAM_MAX(awb_stats_conf->roi_start_y, ISP_AWB_ROI_START_Y_RANGE_MAX);
    CHECK_PARAM_MAX(awb_stats_conf->roi_width, ISP_AWB_ROI_WIDTH_RANGE_MAX);
    CHECK_PARAM_MAX(awb_stats_conf->roi_height, ISP_AWB_ROI_HEIGHT_RANGE_MAX);
    CHECK_PARAM_MAX(awb_stats_conf->grid_num_x, ISP_AWB_GRID_NUM_X_RANGE_MAX);
    CHECK_PARAM_MAX(awb_stats_conf->grid_num_y, ISP_AWB_GRID_NUM_Y_RANGE_MAX);
    CHECK_PARAM_MAX(awb_stats_conf->r_max_thr, ISP_AWB_R_MAX_THR_RANGE_MAX);
    CHECK_PARAM_MAX(awb_stats_conf->r_min_thr, ISP_AWB_R_MIN_THR_RANGE_MAX);
    CHECK_PARAM_MAX(awb_stats_conf->g_max_thr, ISP_AWB_G_MAX_THR_RANGE_MAX);
    CHECK_PARAM_MAX(awb_stats_conf->g_min_thr, ISP_AWB_G_MIN_THR_RANGE_MAX);
    CHECK_PARAM_MAX(awb_stats_conf->b_max_thr, ISP_AWB_B_MAX_THR_RANGE_MAX);
    CHECK_PARAM_MAX(awb_stats_conf->b_min_thr, ISP_AWB_B_MIN_THR_RANGE_MAX);

    CHECK_PARAM_MIN(awb_stats_conf->grid_width, ISP_AWB_GRID_WIDTH_RANGE_MIN);
    CHECK_PARAM_MIN(awb_stats_conf->grid_height, ISP_AWB_GRID_HEIGHT_RANGE_MIN);

    /* special check */
    /* AWB ROI must be in the active image region */
    if ((awb_stats_conf->roi_start_x + awb_stats_conf->roi_width > awb_stats_conf->img_width) ||
        (awb_stats_conf->roi_start_y + awb_stats_conf->roi_height > awb_stats_conf->img_height)) {
        BL_LOGE("AWB ROI must be in the active image region!\r\n");
        BL_ABORT();
    }

    /* grid width/height must be even number */
    if ((awb_stats_conf->grid_width & 0x1) & (awb_stats_conf->grid_height & 0x1)) {
        BL_LOGE("Grid width/height must be even number!\r\n");
        BL_ABORT();
    }

    isp_middle_reg->awb_roi_x.BF.awb_roi_start_x_w = awb_stats_conf->roi_start_x;
    isp_middle_reg->awb_roi_x.BF.awb_roi_width_w = awb_stats_conf->roi_width;
    isp_middle_reg->awb_roi_y.BF.awb_roi_start_y_w = awb_stats_conf->roi_start_y;
    isp_middle_reg->awb_roi_y.BF.awb_roi_height_w = awb_stats_conf->roi_height;

    isp_middle_reg->awb_image_size.BF.awb_image_width_w = awb_stats_conf->img_width;
    isp_middle_reg->awb_image_size.BF.awb_image_height_w = awb_stats_conf->img_height;

    isp_middle_reg->awb_grid_size.BF.awb_grid_width_w = awb_stats_conf->grid_width;
    isp_middle_reg->awb_grid_size.BF.awb_grid_height_w = awb_stats_conf->grid_height;

    isp_middle_reg->awb_grid_size.BF.awb_grid_x_max_w = awb_stats_conf->grid_num_x;
    isp_middle_reg->awb_grid_size.BF.awb_grid_y_max_w = awb_stats_conf->grid_num_y;

    isp_middle_reg->awb_r_thre.BF.awb_r_max_thr_w = awb_stats_conf->r_max_thr;
    isp_middle_reg->awb_r_thre.BF.awb_r_min_thr_w = awb_stats_conf->r_min_thr;

    isp_middle_reg->awb_g_thre.BF.awb_g_max_thr_w = awb_stats_conf->g_max_thr;
    isp_middle_reg->awb_g_thre.BF.awb_g_min_thr_w = awb_stats_conf->g_min_thr;

    isp_middle_reg->awb_b_thre.BF.awb_b_max_thr_w = awb_stats_conf->b_max_thr;
    isp_middle_reg->awb_b_thre.BF.awb_b_min_thr_w = awb_stats_conf->b_min_thr;

    isp_middle_reg->awb_setting.BF.new_awb_sta_enable_w = 1;

    /* AWB occupies 2 banks of memory  */
    isp_middle_reg->awb_setting.BF.awb_switch_bank_en_w = 1;
}

void ISP_Get_AWB_Stats_Conf(isp_awb_stats_conf_t *awb_stats_conf)
{
    isp_middle_reg_t *isp_middle_reg = (isp_middle_reg_t*)ISP_BASE;

    awb_stats_conf->roi_start_x = isp_middle_reg->awb_roi_x.BF.awb_roi_start_x_w;
    awb_stats_conf->roi_start_y = isp_middle_reg->awb_roi_y.BF.awb_roi_start_y_w;
    awb_stats_conf->roi_width = isp_middle_reg->awb_roi_x.BF.awb_roi_width_w;
    awb_stats_conf->roi_height = isp_middle_reg->awb_roi_y.BF.awb_roi_height_w;
    awb_stats_conf->img_width = isp_middle_reg->awb_image_size.BF.awb_image_width_w;
    awb_stats_conf->img_height = isp_middle_reg->awb_image_size.BF.awb_image_height_w;

    awb_stats_conf->grid_width = isp_middle_reg->awb_grid_size.BF.awb_grid_width_w;
    awb_stats_conf->grid_height = isp_middle_reg->awb_grid_size.BF.awb_grid_height_w;
    awb_stats_conf->grid_num_x = isp_middle_reg->awb_grid_size.BF.awb_grid_x_max_w;
    awb_stats_conf->grid_num_y = isp_middle_reg->awb_grid_size.BF.awb_grid_y_max_w;

    awb_stats_conf->r_max_thr = isp_middle_reg->awb_r_thre.BF.awb_r_max_thr_w;
    awb_stats_conf->r_min_thr = isp_middle_reg->awb_r_thre.BF.awb_r_min_thr_w;
    awb_stats_conf->g_max_thr = isp_middle_reg->awb_g_thre.BF.awb_g_max_thr_w;
    awb_stats_conf->g_min_thr = isp_middle_reg->awb_g_thre.BF.awb_g_min_thr_w;
    awb_stats_conf->b_max_thr = isp_middle_reg->awb_b_thre.BF.awb_b_max_thr_w;
    awb_stats_conf->b_min_thr = isp_middle_reg->awb_b_thre.BF.awb_b_min_thr_w;
}

#if USE_AEAWB_SRAM

int ISP_Get_AWB_Stats(int grid_num, uint32_t *data)
{
    uint32_t tmpVal;
    uint32_t buf_idx;
    uint32_t buf_addr;
    uint32_t w_cnt_idx_start;
    uint32_t w_cnt_idx_end;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_AWB_STA_DBG1);
    buf_idx = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_BLAWB_BUF_IDX_R);
    w_cnt_idx_start = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_BLAWB_W_CNT_R);

    buf_addr = (0 == buf_idx) ? ISP_AWB_SRAM_BANK1: ISP_AWB_SRAM_BANK2;
    memcpy(data, (uint32_t*)buf_addr, grid_num*2); // two word per grid

    w_cnt_idx_end = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_BLAWB_W_CNT_R);
    if (w_cnt_idx_start != w_cnt_idx_end) {
        BL_LOGW("awb statistics overwrite!");
        return -1;
    }

    return 0;
}

void ISP_Get_AWB_Sum(int grid_idx, RGB_SUM_S *rgb_sum)
{
    uint32_t tmpVal;
    uint32_t buf_idx;
    uint32_t buf_addr;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_AWB_STA_DBG1);
    buf_idx = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_BLAWB_BUF_IDX_R);

    buf_addr = (0 == buf_idx) ? ISP_AWB_SRAM_BANK1: ISP_AWB_SRAM_BANK2;

    // isp_middle_reg_t *isp_middle_reg = (isp_middle_reg_t*)ISP_BASE;
    isp_awb_sum_t *grid_array = (isp_awb_sum_t*)buf_addr;

    rgb_sum->r_sum = grid_array[grid_idx].r_sum;
    rgb_sum->g_sum = grid_array[grid_idx].g_sum;
    rgb_sum->b_sum = grid_array[grid_idx].b_sum;

    // *buf_idx = isp_middle_reg->awb_sta_dbg1.BF.blawb_buf_idx_r;
}
#else
void ISP_Get_AWB_Sum(int grid_idx, RGB_SUM_S *rgb_sum, int *buf_idx)
{
    uint32_t grid_data;
    isp_blawb_reg_t *isp_blawb_reg = (isp_blawb_reg_t*)ISP_BASE;
    *buf_idx = 0;//isp_blawb_reg->blawb_read_grid_addr.BF.blawb_buf_idx;

    /* get R/G sum of the grid */
    isp_blawb_reg->blawb_read_grid_addr.BF.blawb_read_grid_addr =
        (grid_idx << 1) + (*buf_idx ? (1024 << 1) : 0);
    grid_data = isp_blawb_reg->blawb_grid_data.WORD;

    rgb_sum->r_sum = DIV_0_TO_1(grid_data & 0xFFF);
    rgb_sum->g_sum = DIV_0_TO_1((grid_data >> 16) & 0xFFF);

    /* get B sum of the grid */
    isp_blawb_reg->blawb_read_grid_addr.BF.blawb_read_grid_addr =
        (grid_idx << 1) + 1 + (*buf_idx ? (1024 << 1) : 0);
    grid_data = isp_blawb_reg->blawb_grid_data.WORD;

    rgb_sum->b_sum = DIV_0_TO_1(grid_data & 0xFFF);
}
#endif

void ISP_Get_AWB_ValidGridNum(int *valid_grid_num)
{
    isp_middle_reg_t *isp_middle_reg = (isp_middle_reg_t*)ISP_BASE;

    *valid_grid_num = isp_middle_reg->awb_setting.BF.grid_cnt_within_thre_r;
}

void ISP_Set_AWB_Gain(const ISP_RGB_Gain_Type *rgb_gain)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_AWB_RGR_GAIN);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AWB_R_GAIN_PRE, rgb_gain->r_gain);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AWB_GR_GAIN_PRE, rgb_gain->g_gain);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE_AWB_RGR_GAIN, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_AWB_BGB_GAIN);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AWB_B_GAIN_PRE, rgb_gain->b_gain);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AWB_GB_GAIN_PRE, rgb_gain->g_gain);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE_AWB_BGB_GAIN, tmpVal);
}

void ISP_Get_AWB_Gain(ISP_RGB_Gain_Type *rgb_gain)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_AWB_RGR_GAIN);

    rgb_gain->r_gain = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AWB_R_GAIN_PRE);
    rgb_gain->g_gain = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AWB_GR_GAIN_PRE);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_AWB_BGB_GAIN);
    rgb_gain->b_gain = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AWB_B_GAIN_PRE);
}

void ISP_Set_AWB_Offset(const ISP_RGB_Offset_Type *Offset)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_AWB_RGR_OFFSET);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AWB_R_OFFSET_PRE, Offset->r_offset);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AWB_GR_OFFSET_PRE, Offset->g_offset);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE_AWB_RGR_OFFSET, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_AWB_BGB_OFFSET);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AWB_B_OFFSET_PRE, Offset->b_offset);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AWB_GB_OFFSET_PRE, Offset->g_offset);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE_AWB_BGB_OFFSET, tmpVal);
}

void ISP_Get_AWB_Offset(ISP_RGB_Offset_Type *Offset)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_AWB_RGR_OFFSET);

    Offset->r_offset = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AWB_R_OFFSET_PRE);
    Offset->g_offset = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AWB_GR_OFFSET_PRE);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_AWB_BGB_OFFSET);
    Offset->b_offset = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_AWB_B_OFFSET_PRE);
}

void ISP_Set_AWB2_Stats_State(BL_Fun_Type state)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_INT);
    if (ENABLE == state) {
        tmpVal = BL_SET_REG_BIT(tmpVal, ISP_MIDDLE3_AWB2_STAT_EN_PRE);
    } else {
        tmpVal = BL_CLR_REG_BIT(tmpVal, ISP_MIDDLE3_AWB2_STAT_EN_PRE);
    }

    BL_WR_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_INT, tmpVal);
}

void ISP_Set_AWB2_Gain_State(BL_Fun_Type state)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_INT);
    if (ENABLE == state) {
        tmpVal = BL_SET_REG_BIT(tmpVal, ISP_MIDDLE3_AWB2_GAIN_EN_PRE);
    } else {
        tmpVal = BL_CLR_REG_BIT(tmpVal, ISP_MIDDLE3_AWB2_GAIN_EN_PRE);
    }

    BL_WR_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_INT, tmpVal);
}

int ISP_Get_AWB2_Sum(isp_awb2_sum_t *sum)
{
    uint32_t tmpVal;
    uint64_t tmpVal64;
    uint32_t buf_idx;
    uint32_t w_cnt_idx_start;
    uint32_t w_cnt_idx_end;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_INT);
    buf_idx = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE3_AWB2_BUF_IDX_R);
    w_cnt_idx_start = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE3_AWB2_W_CNT_R);

    if (0 == buf_idx) {
        sum->r_avg = BL_GET_REG_BITS_VAL(BL_RD_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_R0), ISP_MIDDLE3_AWB2_R_AVG0_R);
        sum->g_avg = BL_GET_REG_BITS_VAL(BL_RD_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_R0), ISP_MIDDLE3_AWB2_G_AVG0_R);
        sum->b_avg = BL_GET_REG_BITS_VAL(BL_RD_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_R1), ISP_MIDDLE3_AWB2_B_AVG0_R);
        sum->white_ratio = BL_GET_REG_BITS_VAL(BL_RD_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_R1), ISP_MIDDLE3_AWB2_WHITE_RATIO0_R);

        tmpVal64 = BL_RD_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_R4);
        tmpVal   = BL_GET_REG_BITS_VAL(BL_RD_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_R5), ISP_MIDDLE3_AWB2_WHITER_SUML0_R);
        sum->white_r_sum = (tmpVal64 << 9) + tmpVal;

        tmpVal64 = BL_RD_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_R7);
        tmpVal   = BL_GET_REG_BITS_VAL(BL_RD_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_R8), ISP_MIDDLE3_AWB2_WHITEG_SUML0_R);
        sum->white_g_sum = (tmpVal64 << 9) + tmpVal;

        tmpVal64 = BL_RD_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_RA);
        tmpVal   = BL_GET_REG_BITS_VAL(BL_RD_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_RB), ISP_MIDDLE3_AWB2_WHITEB_SUML0_R);
        sum->white_b_sum = (tmpVal64 << 9) + tmpVal;
    }
    else {
        sum->r_avg = BL_GET_REG_BITS_VAL(BL_RD_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_R2), ISP_MIDDLE3_AWB2_R_AVG1_R);
        sum->g_avg = BL_GET_REG_BITS_VAL(BL_RD_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_R2), ISP_MIDDLE3_AWB2_G_AVG1_R);
        sum->b_avg = BL_GET_REG_BITS_VAL(BL_RD_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_R3), ISP_MIDDLE3_AWB2_B_AVG1_R);
        sum->white_ratio = BL_GET_REG_BITS_VAL(BL_RD_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_R3), ISP_MIDDLE3_AWB2_WHITE_RATIO1_R);

         tmpVal64 = BL_RD_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_R6);
         tmpVal   = BL_GET_REG_BITS_VAL(BL_RD_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_R5), ISP_MIDDLE3_AWB2_WHITER_SUML1_R);
         sum->white_r_sum = (tmpVal64 << 9) + tmpVal;

         tmpVal64 = BL_RD_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_R9);
         tmpVal   = BL_GET_REG_BITS_VAL(BL_RD_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_R8), ISP_MIDDLE3_AWB2_WHITEG_SUML1_R);
         sum->white_g_sum = (tmpVal64 << 9) + tmpVal;

         tmpVal64 = BL_RD_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_RC);
         tmpVal   = BL_GET_REG_BITS_VAL(BL_RD_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_RB), ISP_MIDDLE3_AWB2_WHITEB_SUML1_R);
         sum->white_b_sum = (tmpVal64 << 9) + tmpVal;
    }

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_INT);
    w_cnt_idx_end = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE3_AWB2_W_CNT_R);

    // TODO: chang the algo
    if (w_cnt_idx_start != w_cnt_idx_end) {
        // BL_LOGW("w_cnt_idx_start %d, w_cnt_idx_end %d \r\n", w_cnt_idx_start, w_cnt_idx_end);
        // BL_LOGW("awb2 statistics overwrite!");
        return -1;
    }

    return 0;
}

void ISP_Set_AWB2_Gain(const ISP_RGB_Gain_Type *rgb_gain)
{
    ISP_Set_AWB_Gain(rgb_gain);
    // uint32_t tmpVal;

    // tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_GAIN1);
    // tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE3_AWB2_RGAIN_PRE, rgb_gain->r_gain);
    // BL_WR_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_GAIN1, tmpVal);

    // tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_GAIN2);
    // tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE3_AWB2_GGAIN_PRE, rgb_gain->g_gain);
    // tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE3_AWB2_BGAIN_PRE, rgb_gain->b_gain);
    // BL_WR_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_GAIN2, tmpVal);
}

void ISP_Get_AWB2_Gain(ISP_RGB_Gain_Type *rgb_gain)
{
    ISP_Get_AWB_Gain(rgb_gain);
    // uint32_t tmpVal;

    // tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_GAIN1);
    // rgb_gain->r_gain = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE3_AWB2_RGAIN_PRE);

    // tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_GAIN2);
    // rgb_gain->g_gain = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE3_AWB2_GGAIN_PRE);
    // rgb_gain->b_gain = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE3_AWB2_BGAIN_PRE);
}

void ISP_Set_AWB2_Stats_Conf(const isp_awb2_stats_conf_t *awb_stats_conf)
{
    uint32_t tmpVal;

    CHECK_PARAM_MAX(awb_stats_conf->stat_x_min, ISP_AWB_ROI_START_X_RANGE_MAX);
    CHECK_PARAM_MAX(awb_stats_conf->stat_x_max, ISP_AWB_ROI_START_X_RANGE_MAX);
    CHECK_PARAM_MAX(awb_stats_conf->stat_y_min, ISP_AWB_ROI_START_Y_RANGE_MAX);
    CHECK_PARAM_MAX(awb_stats_conf->stat_y_max, ISP_AWB_ROI_START_Y_RANGE_MAX);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_XAXIS);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE3_AWB2_X_MIN_PRE, awb_stats_conf->stat_x_min);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE3_AWB2_X_MAX_PRE, awb_stats_conf->stat_x_max);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_XAXIS, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_YAXIS);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE3_AWB2_Y_MIN_PRE, awb_stats_conf->stat_y_min);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE3_AWB2_Y_MAX_PRE, awb_stats_conf->stat_y_max);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_YAXIS, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_PIXEL_NUM);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE3_AWB2_PIXEL_NUM_PRE, awb_stats_conf->stat_pixel_num);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_PIXEL_NUM, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_GAIN0);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE3_AWB2_STAT_GGAIN_PRE, awb_stats_conf->stat_g_gain);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE3_AWB2_STAT_RGAIN_PRE, awb_stats_conf->stat_r_gain);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_GAIN0, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_GAIN1);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE3_AWB2_STAT_BGAIN_PRE, awb_stats_conf->stat_b_gain);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_GAIN1, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_GAIN3);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE3_AWB2_VGAIN_PRE, awb_stats_conf->stat_v_gain);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE3_AWB2_UGAIN_PRE, awb_stats_conf->stat_u_gain);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_GAIN3, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_L0);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE3_AWB2_YTH1_PRE, awb_stats_conf->y_th[1]);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE3_AWB2_YTH2_PRE, awb_stats_conf->y_th[2]);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE3_AWB2_YTH3_PRE, awb_stats_conf->y_th[3]);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE3_AWB2_YTH4_PRE, awb_stats_conf->y_th[4]);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_L0, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_L1);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE3_AWB2_YTH0_PRE, awb_stats_conf->y_th[0]);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE3_AWB2_YWEIGHT2_PRE, awb_stats_conf->y_weight[2]);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE3_AWB2_YWEIGHT3_PRE, awb_stats_conf->y_weight[3]);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE3_AWB2_YWEIGHT4_PRE, awb_stats_conf->y_weight[4]);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_L1, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_L2);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE3_AWB2_YWEIGHT0_PRE, awb_stats_conf->y_weight[0]);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE3_AWB2_YWEIGHT1_PRE, awb_stats_conf->y_weight[1]);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE3_AWB2_YSLOPE4_PRE, awb_stats_conf->y_slope[4]);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_L2, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_L3);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE3_AWB2_YSLOPE3_PRE, awb_stats_conf->y_slope[3]);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE3_AWB2_YSLOPE2_PRE, awb_stats_conf->y_slope[2]);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_L3, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_L4);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE3_AWB2_YSLOPE1_PRE, awb_stats_conf->y_slope[1]);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE3_AWB2_YSLOPE0_PRE, awb_stats_conf->y_slope[0]);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_L4, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_C0);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE3_AWB2_UREG4_PRE, awb_stats_conf->u[4]);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE3_AWB2_UREG5_PRE, awb_stats_conf->u[5]);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE3_AWB2_UREG6_PRE, awb_stats_conf->u[6]);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE3_AWB2_UREG7_PRE, awb_stats_conf->u[7]);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_C0, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_C1);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE3_AWB2_UREG0_PRE, awb_stats_conf->u[0]);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE3_AWB2_UREG1_PRE, awb_stats_conf->u[1]);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE3_AWB2_UREG2_PRE, awb_stats_conf->u[2]);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE3_AWB2_UREG3_PRE, awb_stats_conf->u[3]);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_C1, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_C2);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE3_AWB2_VREG4_PRE, awb_stats_conf->v[4]);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE3_AWB2_VREG5_PRE, awb_stats_conf->v[5]);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE3_AWB2_VREG6_PRE, awb_stats_conf->v[6]);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE3_AWB2_VREG7_PRE, awb_stats_conf->v[7]);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_C2, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_C3);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE3_AWB2_UREG0_PRE, awb_stats_conf->v[0]);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE3_AWB2_UREG1_PRE, awb_stats_conf->v[1]);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE3_AWB2_UREG2_PRE, awb_stats_conf->v[2]);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE3_AWB2_UREG3_PRE, awb_stats_conf->v[3]);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_C3, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_C4);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE3_AWB2_CTH4_PRE, awb_stats_conf->c_th[4]);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE3_AWB2_CTH5_PRE, awb_stats_conf->c_th[5]);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE3_AWB2_CTH6_PRE, awb_stats_conf->c_th[6]);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE3_AWB2_CTH7_PRE, awb_stats_conf->c_th[7]);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_C4, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_C5);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE3_AWB2_CTH0_PRE, awb_stats_conf->c_th[0]);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE3_AWB2_CTH1_PRE, awb_stats_conf->c_th[1]);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE3_AWB2_CTH2_PRE, awb_stats_conf->c_th[2]);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE3_AWB2_CTH3_PRE, awb_stats_conf->c_th[3]);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_C5, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_C6);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE3_AWB2_CSLOPE4_PRE, awb_stats_conf->c_slope[4]);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE3_AWB2_CSLOPE5_PRE, awb_stats_conf->c_slope[5]);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE3_AWB2_CSLOPE6_PRE, awb_stats_conf->c_slope[6]);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE3_AWB2_CSLOPE7_PRE, awb_stats_conf->c_slope[7]);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_C6, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_C7);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE3_AWB2_CSLOPE0_PRE, awb_stats_conf->c_slope[0]);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE3_AWB2_CSLOPE1_PRE, awb_stats_conf->c_slope[1]);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE3_AWB2_CSLOPE2_PRE, awb_stats_conf->c_slope[2]);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE3_AWB2_CSLOPE3_PRE, awb_stats_conf->c_slope[3]);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE3_AWB2_STA_C7, tmpVal);
}

void ISP_Set_AWB3_Hist_State(BL_Fun_Type state)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_CONFIG_0);
    if (ENABLE == state) {
        tmpVal = BL_SET_REG_BIT(tmpVal, ISP_MIDDLE5_AWB3_STAT_EN_PRE);
    } else {
        tmpVal = BL_CLR_REG_BIT(tmpVal, ISP_MIDDLE5_AWB3_STAT_EN_PRE);
    }

    BL_WR_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_CONFIG_0, tmpVal);
}

void ISP_Get_AWB3_Hist_State(BL_Fun_Type *state)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_CONFIG_0);
    if (BL_IS_REG_BIT_SET(tmpVal, ISP_MIDDLE5_AWB3_STAT_EN_PRE)){
        *state = ENABLE;
    } else {
        *state = DISABLE;
    }
}

void ISP_Set_AWB3_Stats_Conf(const isp_awb3_stats_conf_t *awb_stats_conf)
{
    uint32_t tmpVal;

    CHECK_PARAM_MAX(awb_stats_conf->stat_x_min, ISP_AWB_ROI_START_X_RANGE_MAX);
    CHECK_PARAM_MAX(awb_stats_conf->stat_x_max, ISP_AWB_ROI_START_X_RANGE_MAX);
    CHECK_PARAM_MAX(awb_stats_conf->stat_y_min, ISP_AWB_ROI_START_Y_RANGE_MAX);
    CHECK_PARAM_MAX(awb_stats_conf->stat_y_max, ISP_AWB_ROI_START_Y_RANGE_MAX);

    // awb window
    tmpVal = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_CONFIG_0);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_STAT_X_MIN_PRE, awb_stats_conf->stat_x_min);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_STAT_X_MAX_PRE, awb_stats_conf->stat_x_max);
    BL_WR_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_CONFIG_0, tmpVal);

    tmpVal = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_CONFIG_1);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_STAT_Y_MIN_PRE, awb_stats_conf->stat_y_min);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_STAT_Y_MAX_PRE, awb_stats_conf->stat_y_max);
    BL_WR_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_CONFIG_1, tmpVal);

    // weight_ratio
    tmpVal = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_CONFIG_4);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_WT_RATIO_EN_PRE, awb_stats_conf->weight_ratio_en);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_WT_RATIO_GAIN_PRE, awb_stats_conf->weight_ratio_gain);
    BL_WR_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_CONFIG_4, tmpVal);

    // y coeff
    tmpVal = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_CONFIG_26);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_Y_R_COEFF_PRE, awb_stats_conf->y_r_coeff);
    BL_WR_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_CONFIG_26, tmpVal);

    tmpVal = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_CONFIG_27);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_Y_G_COEFF_PRE, awb_stats_conf->y_g_coeff);
    BL_WR_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_CONFIG_27, tmpVal);

    tmpVal = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_CONFIG_28);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_Y_B_COEFF_PRE, awb_stats_conf->y_b_coeff);
    BL_WR_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_CONFIG_28, tmpVal);

    tmpVal = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_CONFIG_29);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_WT_Y_USE_G_PRE, awb_stats_conf->weight_y_use_g);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_WT_Y_EN_PRE, awb_stats_conf->weight_y_en);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_WT_Y_GAIN_PRE, awb_stats_conf->weight_y_gain);
    BL_WR_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_CONFIG_29, tmpVal);

    memcpy((uint32_t*)(ISP_AWB3_BASE + ISP_MIDDLE5_AWB3_CONFIG_30_OFFSET), \
        (uint32_t*)&(awb_stats_conf->weight_y[0][0]), 21);


    tmpVal = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_CONFIG_51);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_COUNT_ZERO_WT_PRE,   awb_stats_conf->count_zero_weight);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_HIST_Y_USE_G_PRE,    awb_stats_conf->hist_y_use_g);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_BIN_Y_LEVEL_PRE,     awb_stats_conf->bin_y_level);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_BIN_RATIO_LEVEL_PRE, awb_stats_conf->bin_ratio_level);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_BIN_CLIP_EN_PRE,     awb_stats_conf->bin_clip_en);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_BIN_PIXEL_CLIP_PRE,  awb_stats_conf->bin_pixel_clip);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_BIN_MODE_PRE,        awb_stats_conf->bin_mode);
    BL_WR_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_CONFIG_51, tmpVal);

    tmpVal = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_CONFIG_52);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_INK_TYPE_PRE, awb_stats_conf->ink_type);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_INK_GAIN_PRE, awb_stats_conf->ink_gain);
    BL_WR_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_CONFIG_52, tmpVal);

    // calibration
    tmpVal = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_CONFIG_2);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_G_DIVISOR_PRE,    awb_stats_conf->g_divisor);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_R_RATIO_OFST_PRE, awb_stats_conf->r_ratio_ofst);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_B_RATIO_OFST_PRE, awb_stats_conf->b_ratio_ofst);
    BL_WR_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_CONFIG_2, tmpVal);


    tmpVal = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_CONFIG_3);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_R_RATIO_MULT_PRE, awb_stats_conf->r_ratio_mult);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_B_RATIO_MULT_PRE, awb_stats_conf->b_ratio_mult);
    BL_WR_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_CONFIG_3, tmpVal);

    tmpVal = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_CONFIG_4);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_RATIO_SUM_MIN_PRE, awb_stats_conf->ratio_sum_min);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_RATIO_SUM_MAX_PRE, awb_stats_conf->ratio_sum_max);
    BL_WR_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_CONFIG_4, tmpVal);

    memcpy((uint32_t*)(ISP_AWB3_BASE + ISP_MIDDLE5_AWB3_CONFIG_5_OFFSET), \
        (uint32_t*)&(awb_stats_conf->weight_ratio[0][0]), 21);
}

void ISP_Get_AWB3_Stats_Conf(isp_awb3_stats_conf_t *awb_stats_conf)
{
    uint32_t tmpVal;

    // awb window
    tmpVal = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_CONFIG_0);
    awb_stats_conf->stat_x_min = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_STAT_X_MIN_PRE);
    awb_stats_conf->stat_x_max = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_STAT_X_MAX_PRE);

    tmpVal = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_CONFIG_1);
    awb_stats_conf->stat_y_min = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_STAT_Y_MIN_PRE);
    awb_stats_conf->stat_y_max = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_STAT_Y_MAX_PRE);

    // weight_ratio
    tmpVal = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_CONFIG_4);
    awb_stats_conf->weight_ratio_en = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_WT_RATIO_EN_PRE);
    awb_stats_conf->weight_ratio_gain = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_WT_RATIO_GAIN_PRE);

    // y coeff
    tmpVal = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_CONFIG_26);
    awb_stats_conf->y_r_coeff = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_Y_R_COEFF_PRE);

    tmpVal = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_CONFIG_27);
    awb_stats_conf->y_g_coeff = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_Y_G_COEFF_PRE);

    tmpVal = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_CONFIG_28);
    awb_stats_conf->y_b_coeff = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_Y_B_COEFF_PRE);

    tmpVal = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_CONFIG_29);
    awb_stats_conf->weight_y_use_g = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_WT_Y_USE_G_PRE);
    awb_stats_conf->weight_y_en = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_WT_Y_EN_PRE);
    awb_stats_conf->weight_y_gain = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_WT_Y_GAIN_PRE);

    memcpy((uint32_t*)&(awb_stats_conf->weight_y[0][0]), \
        (uint32_t*)(ISP_AWB3_BASE + ISP_MIDDLE5_AWB3_CONFIG_30_OFFSET), 21);

    tmpVal = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_CONFIG_51);
    awb_stats_conf->count_zero_weight = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_COUNT_ZERO_WT_PRE);
    awb_stats_conf->hist_y_use_g = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_HIST_Y_USE_G_PRE);
    awb_stats_conf->bin_y_level = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_BIN_Y_LEVEL_PRE);
    awb_stats_conf->bin_ratio_level = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_BIN_RATIO_LEVEL_PRE);
    awb_stats_conf->bin_clip_en = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_BIN_CLIP_EN_PRE);
    awb_stats_conf->bin_pixel_clip = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_BIN_PIXEL_CLIP_PRE);
    awb_stats_conf->bin_mode = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_BIN_MODE_PRE);

    tmpVal = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_CONFIG_52);
    awb_stats_conf->ink_type = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_INK_TYPE_PRE);
    awb_stats_conf->ink_gain = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_INK_GAIN_PRE);

    // calibration
    tmpVal = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_CONFIG_2);
    awb_stats_conf->g_divisor = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_G_DIVISOR_PRE);
    awb_stats_conf->r_ratio_ofst = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_R_RATIO_OFST_PRE);
    awb_stats_conf->b_ratio_ofst = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_B_RATIO_OFST_PRE);

    tmpVal = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_CONFIG_3);
    awb_stats_conf->r_ratio_mult = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_R_RATIO_MULT_PRE);
    awb_stats_conf->b_ratio_mult = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_B_RATIO_MULT_PRE);

    tmpVal = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_CONFIG_4);
    awb_stats_conf->ratio_sum_min = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_RATIO_SUM_MIN_PRE);
    awb_stats_conf->ratio_sum_max = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_RATIO_SUM_MAX_PRE);

    memcpy((uint32_t*)&(awb_stats_conf->weight_ratio[0][0]), \
        (uint32_t*)(ISP_AWB3_BASE + ISP_MIDDLE5_AWB3_CONFIG_5_OFFSET), 21);
}

int ISP_Get_AWB3_Sum(isp_awb3_sum_t *sum)
{
    uint32_t tmpVal;
    uint64_t tmpVal64;
    uint32_t buf_idx;
    uint32_t w_cnt_idx_start;
    uint32_t w_cnt_idx_end;

    tmpVal = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STATUS_0);
    buf_idx = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_BUF_IDX_R);
    w_cnt_idx_start = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_W_CNT_R);

    if (0 == buf_idx) {
        sum->r_sum = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STA_0);
        sum->g_sum = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STA_1);
        sum->b_sum = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STA_2);

        sum->r_sum_hex = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STA_3);
        sum->g_sum_hex = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STA_4);
        sum->b_sum_hex = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STA_5);

        sum->pixel_hex = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STA_6);

        tmpVal64 = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STA_7);
        tmpVal   = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STA_8);
        sum->r_ratio_hex_y = (tmpVal64 << 32) + tmpVal;

        tmpVal64 = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STA_9);
        tmpVal   = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STA_10);
        sum->b_ratio_hex_y = (tmpVal64 << 32) + tmpVal;

        sum->weight_hex_y = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STA_11);
        sum->pixel_hex_y  = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STA_12);

        tmpVal64 = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STA_13);
        tmpVal   = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STA_14);
        sum->r_ratio_hist = (tmpVal64 << 32) + tmpVal;

        tmpVal64 = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STA_15);
        tmpVal   = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STA_16);
        sum->b_ratio_hist = (tmpVal64 << 32) + tmpVal;

        sum->weight_hist = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STA_17);
        sum->pixel_hist  = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STA_18);

        tmpVal64 = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STA_19);
        tmpVal   = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STA_20);
        sum->r_ratio_hist_clip = (tmpVal64 << 32) + tmpVal;

        tmpVal64 = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STA_21);
        tmpVal   = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STA_22);
        sum->b_ratio_hist_clip = (tmpVal64 << 32) + tmpVal;

        sum->weight_hist_clip = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STA_23);
        sum->pixel_hist_clip  = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STA_24);
    }
    else {
        sum->r_sum = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STA_25);
        sum->g_sum = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STA_26);
        sum->b_sum = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STA_27);

        sum->r_sum_hex = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STA_28);
        sum->g_sum_hex = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STA_29);
        sum->b_sum_hex = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STA_30);

        sum->pixel_hex = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STA_31);

        tmpVal64 = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STA_32);
        tmpVal   = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STA_33);
        sum->r_ratio_hex_y = (tmpVal64 << 32) + tmpVal;

        tmpVal64 = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STA_34);
        tmpVal   = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STA_35);
        sum->b_ratio_hex_y = (tmpVal64 << 32) + tmpVal;

        sum->weight_hex_y = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STA_36);
        sum->pixel_hex_y  = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STA_37);

        tmpVal64 = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STA_38);
        tmpVal   = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STA_39);
        sum->r_ratio_hist = (tmpVal64 << 32) + tmpVal;

        tmpVal64 = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STA_40);
        tmpVal   = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STA_41);
        sum->b_ratio_hist = (tmpVal64 << 32) + tmpVal;

        sum->weight_hist = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STA_42);
        sum->pixel_hist  = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STA_43);

        tmpVal64 = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STA_44);
        tmpVal   = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STA_45);
        sum->r_ratio_hist_clip = (tmpVal64 << 32) + tmpVal;

        tmpVal64 = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STA_46);
        tmpVal   = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STA_47);
        sum->b_ratio_hist_clip = (tmpVal64 << 32) + tmpVal;

        sum->weight_hist_clip = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STA_48);
        sum->pixel_hist_clip  = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STA_49);
    }

    tmpVal = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_STATUS_0);
    w_cnt_idx_end = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_W_CNT_R);

    if (w_cnt_idx_start != w_cnt_idx_end) {
        // BL_LOGW("w_cnt_idx_start %d, w_cnt_idx_end %d \r\n", w_cnt_idx_start, w_cnt_idx_end);
        // BL_LOGW("awb3 statistics overwrite!");
        return -1;
    }

    return 0;
}

int ISP_Get_AWB3_Hist_Bin_Num(uint32_t *num)
{
    uint32_t tmpVal;
    uint32_t bin_y_level;
    uint32_t bin_ratio_level;
    isp_awb3_bin_mode_e mode;

    tmpVal = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_CONFIG_51);
    bin_y_level = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_BIN_Y_LEVEL_PRE);
    bin_ratio_level = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_BIN_RATIO_LEVEL_PRE);

    ISP_Get_AWB3_Bin_Mode(&mode);
    if (mode == AWB3_BIN_MODE_FULL) {
        *num = 1 << (bin_y_level + bin_ratio_level*2);
    } else if (mode == AWB3_BIN_MODE_RATIO) {
        *num = 1 << (bin_ratio_level*2);
    } else if (mode == AWB3_BIN_MODE_Y) {
        *num = 1 << (bin_y_level);
    } else {//AWB3_BIN_MODE_NONE
        *num = 0;
    }

    return 0;
}

int ISP_Get_AWB3_Bin_Mode(isp_awb3_bin_mode_e *mode)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_CONFIG_51);
    *mode = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_BIN_MODE_PRE);

    return 0;
}

void ISP_Set_AWB3_Bin_Mode(isp_awb3_bin_mode_e mode)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_CONFIG_51);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_BIN_MODE_PRE, mode);
    BL_WR_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_CONFIG_51, tmpVal);
}

int ISP_Get_AWB3_Hist(uint32_t *data)
{
    uint32_t bucket_num;

    ISP_Get_AWB3_Hist_Bin_Num(&bucket_num);
    CHECK_PARAM_RANGE(bucket_num, AWB3_HIST_BUCKET_NUM_MIN, AWB3_HIST_BUCKET_NUM_MAX);

    memcpy(data, (uint32_t*)AWB3_HIST_BUCKET_ADDR, sizeof(isp_awb3_bucket_t)*bucket_num);

    return 0;
}

int ISP_Get_AWB3_G_Divisor(int *value)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_CONFIG_2);
    *value = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_G_DIVISOR_PRE);

    return 0;
}

void ISP_Set_AWB3_G_Divisor(int value)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_CONFIG_2);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE5_AWB3_G_DIVISOR_PRE, value);
    BL_WR_REG(ISP_AWB3_BASE, ISP_MIDDLE5_AWB3_CONFIG_2, tmpVal);
}

int ISP_Set_BLC(const ISP_CLAMP_S *clamp)
{
    isp_front_reg_t *isp_front_reg = (isp_front_reg_t*)ISP_BASE;

    CHECK_PARAM_RANGE(clamp->clamp_r, ISP_BLC_MIN, ISP_BLC_MAX);
    CHECK_PARAM_RANGE(clamp->clamp_gr, ISP_BLC_MIN, ISP_BLC_MAX);
    CHECK_PARAM_RANGE(clamp->clamp_gb, ISP_BLC_MIN, ISP_BLC_MAX);
    CHECK_PARAM_RANGE(clamp->clamp_b, ISP_BLC_MIN, ISP_BLC_MAX);

    isp_front_reg->clamp_R_Gr.BF.clp_R_pre = clamp->clamp_r;
    isp_front_reg->clamp_R_Gr.BF.clp_Gr_pre = clamp->clamp_gr;
    isp_front_reg->clamp_B_Gb.BF.clp_Gb_pre = clamp->clamp_gb;
    isp_front_reg->clamp_B_Gb.BF.clp_B_pre = clamp->clamp_b;

    isp_front_reg->gain_R_G.BF.rgain1_pre = 128;
    isp_front_reg->gain_R_G.BF.ggain1_pre = 128;
    isp_front_reg->gain_B.BF.bgain1_pre = 128;

    /* compensate back the lost dynamic range for bayer image */
    isp_front_reg->clp_rgr_gain.BF.clp_gain_en_pre = 1;
    isp_front_reg->clp_rgr_gain.BF.clp_r_gain_pre = ISP_Calc_Clamp_Gain(clamp->clamp_r);
    isp_front_reg->clp_rgr_gain.BF.clp_gr_gain_pre = ISP_Calc_Clamp_Gain(clamp->clamp_gr);
    isp_front_reg->clp_bgb_gain.BF.clp_gb_gain_pre = ISP_Calc_Clamp_Gain(clamp->clamp_gb);
    isp_front_reg->clp_bgb_gain.BF.clp_b_gain_pre = ISP_Calc_Clamp_Gain(clamp->clamp_b);

    return 0;
}

void ISP_Set_RB_Gain(const ISP_RB_Gain_Type *rb_gain)
{
    ISP_RGB_Gain_Type rgb_gain;

    rgb_gain.r_gain = rb_gain->r_gain;
    rgb_gain.g_gain = 4096;
    rgb_gain.b_gain = rb_gain->b_gain;

    ISP_Set_AWB_Gain(&rgb_gain);
}

int ISP_Get_BLC(ISP_CLAMP_S *clamp)
{
    isp_front_reg_t *isp_front_reg = (isp_front_reg_t*)ISP_BASE;

    clamp->clamp_r = isp_front_reg->clamp_R_Gr.BF.clp_R_pre;
    clamp->clamp_gr = isp_front_reg->clamp_R_Gr.BF.clp_Gr_pre;
    clamp->clamp_gb = isp_front_reg->clamp_B_Gb.BF.clp_Gb_pre;
    clamp->clamp_b = isp_front_reg->clamp_B_Gb.BF.clp_B_pre;

    return 0;
}

int ISP_Get_OB_Avg(ISP_OB_S *ob)
{
    isp_front_reg_t *isp_front_reg = (isp_front_reg_t*)ISP_BASE;

    ob->ob_r  = isp_front_reg->Ob_R_Gr.BF.Ob_R_r;
    ob->ob_gr = isp_front_reg->Ob_R_Gr.BF.Ob_Gr_r;
    ob->ob_gb = isp_front_reg->Ob_B_Gb.BF.Ob_Gb_r;
    ob->ob_b  = isp_front_reg->Ob_B_Gb.BF.Ob_B_r;

    return 0;
}

void ISP_Set_DPC_State(BL_Fun_Type state)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_DEFECT_SETTING);
    if (ENABLE == state) {
        tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_DEFECT_CORRECT_MODE_W, ISP_DPC_FLY);
    } else {
        tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_DEFECT_CORRECT_MODE_W, ISP_DPC_OFF);
    }
    BL_WR_REG(ISP_BASE, ISP_MIDDLE_DEFECT_SETTING, tmpVal);
}

void ISP_Get_DPC_State(BL_Fun_Type *state)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_DEFECT_SETTING);
    *state = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_DEFECT_CORRECT_MODE_W);
}

void ISP_Enable_DPC(void)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_DEFECT_SETTING);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_DEFECT_CORRECT_MODE_W, ISP_DPC_FLY);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE_DEFECT_SETTING, tmpVal);
}

void ISP_Disable_DPC(void)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_DEFECT_SETTING);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_DEFECT_CORRECT_MODE_W, ISP_DPC_OFF);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE_DEFECT_SETTING, tmpVal);
}

int ISP_Set_Defect_Correction_Enhance_Group(const ISP_DPC_ENHANCE_GROUP_TYPE *value)
{
    int i;
    isp_middle_reg_t *isp_middle_reg = (isp_middle_reg_t*)ISP_BASE;

    for (i = 0; i < ISP_DPC_Enhance_Num; i++) {
        if ((DISABLE != value->data[i]) && (ENABLE != value->data[i]))  {
            return -1;
        }
    }

    isp_middle_reg->defect_setting.BF.resolution_enhance1_w = value->data[ISP_DPC_Enhance1];
    isp_middle_reg->defect_setting.BF.resolution_enhance2_w = value->data[ISP_DPC_Enhance2];
    isp_middle_reg->defect_setting.BF.resolution_enhance3_w = value->data[ISP_DPC_Enhance3];

    return 0;
}

int ISP_Get_Defect_Correction_Enhance_Group(ISP_DPC_ENHANCE_GROUP_TYPE *value)
{
    isp_middle_reg_t *isp_middle_reg = (isp_middle_reg_t*)ISP_BASE;

    value->data[ISP_DPC_Enhance1] = (BL_Fun_Type)isp_middle_reg->defect_setting.BF.resolution_enhance1_w;
    value->data[ISP_DPC_Enhance2] = (BL_Fun_Type)isp_middle_reg->defect_setting.BF.resolution_enhance2_w;
    value->data[ISP_DPC_Enhance3] = (BL_Fun_Type)isp_middle_reg->defect_setting.BF.resolution_enhance3_w;

    return 0;
}

int ISP_Set_Defect_Correction_Threshold_Group(const ISP_DPC_THRESHOLD_GROUP_TYPE *value)
{
    int i;
    isp_middle_reg_t *isp_middle_reg = (isp_middle_reg_t*)ISP_BASE;

    for (i = 0; i < ISP_DPC_th_Num; i++) {
        CHECK_PARAM_RANGE(value->data[i], ISP_DPC_TH_MIN, ISP_DPC_TH_MAX);
    }

    isp_middle_reg->defect_th_A.BF.defect_th1_w = value->data[ISP_DPC_th1];
    isp_middle_reg->defect_th_A.BF.defect_th2_w = value->data[ISP_DPC_th2];
    isp_middle_reg->defect_th_A.BF.defect_th3_w = value->data[ISP_DPC_th3];
    isp_middle_reg->defect_th_A.BF.defect_th4_w = value->data[ISP_DPC_th4];
    isp_middle_reg->defect_th_B.BF.defect_th5_w = value->data[ISP_DPC_th5];

    return 0;
}

int ISP_Get_Defect_Correction_Threshold_Group(ISP_DPC_THRESHOLD_GROUP_TYPE *value)
{
    isp_middle_reg_t *isp_middle_reg = (isp_middle_reg_t*)ISP_BASE;

    value->data[ISP_DPC_th1] = isp_middle_reg->defect_th_A.BF.defect_th1_w;
    value->data[ISP_DPC_th2] = isp_middle_reg->defect_th_A.BF.defect_th2_w;
    value->data[ISP_DPC_th3] = isp_middle_reg->defect_th_A.BF.defect_th3_w;
    value->data[ISP_DPC_th4] = isp_middle_reg->defect_th_A.BF.defect_th4_w;
    value->data[ISP_DPC_th5] = isp_middle_reg->defect_th_B.BF.defect_th5_w;

    return 0;
}

int ISP_Set_Defect_Postion(const int pos_x, const int pos_y)
{
    isp_middle_reg_t *isp_middle_reg = (isp_middle_reg_t*)ISP_BASE;

    CHECK_PARAM_RANGE(pos_x, ISP_DPC_POS_MIN, ISP_DPC_POS_MAX);
    CHECK_PARAM_RANGE(pos_y, ISP_DPC_POS_MIN, ISP_DPC_POS_MAX);

    isp_middle_reg->defect_position.BF.x_position_w = pos_x;
    isp_middle_reg->defect_position.BF.y_position_w = pos_y;

    return 0;
}

int ISP_Get_Defect_Postion(int *pos_x, int *pos_y)
{
    isp_middle_reg_t *isp_middle_reg = (isp_middle_reg_t*)ISP_BASE;

    *pos_x = isp_middle_reg->defect_position.BF.x_position_w;
    *pos_y = isp_middle_reg->defect_position.BF.y_position_w;

    return 0;
}

void ISP_Set_BNR_State(BL_Fun_Type state)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE4_BNR_CONFIG_0);
    if (ENABLE == state) {
        tmpVal = BL_SET_REG_BIT(tmpVal, ISP_MIDDLE4_BNR_EN_PRE);
    } else {
        tmpVal = BL_CLR_REG_BIT(tmpVal, ISP_MIDDLE4_BNR_EN_PRE);
    }
    BL_WR_REG(ISP_BASE, ISP_MIDDLE4_BNR_CONFIG_0, tmpVal);
}

void ISP_Get_BNR_State(BL_Fun_Type *state)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE4_BNR_CONFIG_0);

    if (BL_IS_REG_BIT_SET(tmpVal, ISP_MIDDLE4_BNR_EN_PRE)) {
        *state = ENABLE;
    } else {
        *state = DISABLE;
    }
}

/*
    Set/Get BNR Weight
    value range: 0~64
*/
void ISP_Set_BNR_Weight(int value)
{
    uint32_t tmpVal;

    CHECK_PARAM_RANGE(value, ISP_BNR_WEIGHT_MIN, ISP_BNR_WEIGHT_MAX);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE4_BNR_CONFIG_0);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE4_BNR_WEIGHT_PRE, value);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE4_BNR_CONFIG_0, tmpVal);
}

void ISP_Get_BNR_Weight(int *value)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE4_BNR_CONFIG_0);
    *value = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE4_BNR_WEIGHT_PRE);
}

/*
    Set/Get BNR LPF Weight
    value range: 0~16
*/
void ISP_Set_BNR_LPF_Weight(int value)
{
    uint32_t tmpVal;

    CHECK_PARAM_RANGE(value, ISP_BNR_LPF_WEIGHT_MIN, ISP_BNR_LPF_WEIGHT_MAX);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE4_BNR_CONFIG_0);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE4_LPF_WEIGHT_PRE, value);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE4_BNR_CONFIG_0, tmpVal);
}

void ISP_Get_BNR_LPF_Weight(int *value)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE4_BNR_CONFIG_0);
    *value = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE4_LPF_WEIGHT_PRE);
}

void ISP_Set_BNR_Threshold(const ISP_BNR_THRESHOLD_TYPE *value)
{
    uint32_t tmpVal;

    CHECK_PARAM_MAX(value->th_base_r, ISP_BNR_BASE_MAX);
    CHECK_PARAM_MAX(value->th_base_g, ISP_BNR_BASE_MAX);
    CHECK_PARAM_MAX(value->th_base_b, ISP_BNR_BASE_MAX);
    CHECK_PARAM_MAX(value->th_slope_r, ISP_BNR_SLOPE_MAX);
    CHECK_PARAM_MAX(value->th_slope_g, ISP_BNR_SLOPE_MAX);
    CHECK_PARAM_MAX(value->th_slope_b, ISP_BNR_SLOPE_MAX);
    CHECK_PARAM_MAX(value->slope_gain, ISP_BNR_SLOPE_GAIN_MAX);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE4_BNR_CONFIG_0);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE4_TH_BASE_B_PRE, value->th_base_b);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE4_BNR_CONFIG_0, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE4_BNR_CONFIG_1);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE4_TH_BASE_G_PRE, value->th_base_g);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE4_TH_BASE_R_PRE, value->th_base_r);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE4_BNR_CONFIG_1, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE4_BNR_CONFIG_2);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE4_TH_SLOPE_B_PRE, value->th_slope_b);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE4_TH_SLOPE_G_PRE, value->th_slope_g);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE4_TH_SLOPE_R_PRE, value->th_slope_r);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE4_SLOPE_GAIN_PRE, value->slope_gain);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE4_BNR_CONFIG_2, tmpVal);
}

void ISP_Get_BNR_Threshold(ISP_BNR_THRESHOLD_TYPE *value)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE4_BNR_CONFIG_0);
    value->th_base_b  = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE4_TH_BASE_B_PRE);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE4_BNR_CONFIG_1);
    value->th_base_g  = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE4_TH_BASE_G_PRE);
    value->th_base_r  = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE4_TH_BASE_R_PRE);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE4_BNR_CONFIG_2);
    value->th_slope_b  = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE4_TH_SLOPE_B_PRE);
    value->th_slope_g  = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE4_TH_SLOPE_G_PRE);
    value->th_slope_r  = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE4_TH_SLOPE_R_PRE);
    value->slope_gain  = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE4_SLOPE_GAIN_PRE);
}

void ISP_Set_BNR_Fallback(const ISP_BNR_FALLBACK_TYPE *value)
{
    uint32_t tmpVal;

    CHECK_PARAM_MAX(value->fb_level, ISP_BNR_FB_LVL_MAX);
    CHECK_PARAM_MAX(value->fb_lpf_weight, ISP_BNR_FB_LPF_WGT_MAX);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE4_BNR_CONFIG_3);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE4_FB_LEVEL_PRE, value->fb_level);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE4_FB_LPF_WEIGHT_PRE, value->fb_lpf_weight);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE4_BNR_CONFIG_3, tmpVal);
}

void ISP_Get_BNR_Fallback(ISP_BNR_FALLBACK_TYPE *value)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE4_BNR_CONFIG_3);
    value->fb_lpf_weight  = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE4_FB_LPF_WEIGHT_PRE);
    value->fb_level  = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE4_FB_LEVEL_PRE);
}

void ISP_Set_LSC_State(BL_Fun_Type state)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_SETTING);
    if (ENABLE == state) {
        tmpVal = BL_SET_REG_BIT(tmpVal, ISP_MIDDLE_LSC_ON_W);
    } else {
        tmpVal = BL_CLR_REG_BIT(tmpVal, ISP_MIDDLE_LSC_ON_W);
    }
    BL_WR_REG(ISP_BASE, ISP_MIDDLE_LSC_SETTING, tmpVal);
}

void ISP_Enable_LSC(void)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_SETTING);
    tmpVal = BL_SET_REG_BIT(tmpVal, ISP_MIDDLE_LSC_ON_W);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE_LSC_SETTING, tmpVal);
}

void ISP_Disable_LSC(void)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_SETTING);
    tmpVal = BL_CLR_REG_BIT(tmpVal, ISP_MIDDLE_LSC_ON_W);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE_LSC_SETTING, tmpVal);
}

void ISP_Get_LSC_State(BL_Fun_Type *state)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_SETTING);

    if (BL_IS_REG_BIT_SET(tmpVal, ISP_MIDDLE_LSC_ON_W)) {
        *state = ENABLE;
    } else {
        *state = DISABLE;
    }
}

/*
    xoffset and yoffset caculated by active and out window
 */
int ISP_Set_LSC_Offset(int xoffset, int yoffset)
{
    isp_middle_reg_t *isp_middle_reg = (isp_middle_reg_t*)ISP_BASE;

    CHECK_PARAM_RANGE(xoffset, ISP_LSC_OFFSET_MIN, ISP_LSC_OFFSET_MAX);
    CHECK_PARAM_RANGE(yoffset, ISP_LSC_OFFSET_MIN, ISP_LSC_OFFSET_MAX);

    isp_middle_reg->LSC_setting.BF.X_offset_w = xoffset;
    isp_middle_reg->LSC_setting.BF.Y_offset_w = yoffset;

    return 0;
}

int ISP_Get_LSC_Offset(int *xoffset, int *yoffset)
{
    isp_middle_reg_t *isp_middle_reg = (isp_middle_reg_t*)ISP_BASE;

    *xoffset = isp_middle_reg->LSC_setting.BF.X_offset_w;
    *yoffset = isp_middle_reg->LSC_setting.BF.Y_offset_w;

    return 0;
}

int ISP_Set_LSC_Coeff(const ISP_LSC_COEF_S *lsc)
{
    uint32_t tmpVal;

    if (!is_LSC_Coeff_Valid(lsc)) {
        return -1;
    }

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_R_A);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE_R_ACOEFF_W, lsc->lsc_coef_r.coef[LSC_COEF_A]);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_R_A, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_R_BE);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE_R_BCOEFF_W, lsc->lsc_coef_r.coef[LSC_COEF_B]);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE_R_ECOEFF_W, lsc->lsc_coef_r.coef[LSC_COEF_E]);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_R_BE, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_R_CD);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE_R_CCOEFF_W, lsc->lsc_coef_r.coef[LSC_COEF_C]);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE_R_DCOEFF_W, lsc->lsc_coef_r.coef[LSC_COEF_D]);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_R_CD, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_R_G);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE_R_GCOEFF_W, lsc->lsc_coef_r.coef[LSC_COEF_G]);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_R_G, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_G_A);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE_G_ACOEFF_W, lsc->lsc_coef_g.coef[LSC_COEF_A]);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_G_A, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_G_BE);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE_G_BCOEFF_W, lsc->lsc_coef_g.coef[LSC_COEF_B]);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE_G_ECOEFF_W, lsc->lsc_coef_g.coef[LSC_COEF_E]);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_G_BE, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_G_CD);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE_G_CCOEFF_W, lsc->lsc_coef_g.coef[LSC_COEF_C]);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE_G_DCOEFF_W, lsc->lsc_coef_g.coef[LSC_COEF_D]);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_G_CD, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_G_G);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE_G_GCOEFF_W, lsc->lsc_coef_g.coef[LSC_COEF_G]);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_G_G, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_B_A);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE_B_ACOEFF_W, lsc->lsc_coef_b.coef[LSC_COEF_A]);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_B_A, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_B_BE);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE_B_BCOEFF_W, lsc->lsc_coef_b.coef[LSC_COEF_B]);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE_B_ECOEFF_W, lsc->lsc_coef_b.coef[LSC_COEF_E]);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_B_BE, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_B_CD);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE_B_CCOEFF_W, lsc->lsc_coef_b.coef[LSC_COEF_C]);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE_B_DCOEFF_W, lsc->lsc_coef_b.coef[LSC_COEF_D]);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_B_CD, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_B_G);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE_B_GCOEFF_W, lsc->lsc_coef_b.coef[LSC_COEF_G]);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_B_G, tmpVal);

    return 0;
}

int ISP_Get_LSC_Coeff(ISP_LSC_COEF_S *lsc)
{
    uint32_t tmpVal;

    /* channel R */
    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_R_A);
    lsc->lsc_coef_r.coef[LSC_COEF_A] = SIGN_EXTEND(BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_R_ACOEFF_W), 23);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_R_BE);
    lsc->lsc_coef_r.coef[LSC_COEF_B] = SIGN_EXTEND(BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_R_BCOEFF_W), 13);
    lsc->lsc_coef_r.coef[LSC_COEF_E] = SIGN_EXTEND(BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_R_ECOEFF_W), 13);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_R_CD);
    lsc->lsc_coef_r.coef[LSC_COEF_C] = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_R_CCOEFF_W);
    lsc->lsc_coef_r.coef[LSC_COEF_D] = SIGN_EXTEND(BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_R_DCOEFF_W), 23);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_R_G);
    lsc->lsc_coef_r.coef[LSC_COEF_G] = SIGN_EXTEND(BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_R_GCOEFF_W), 23);

    /* channel G */
    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_G_A);
    lsc->lsc_coef_g.coef[LSC_COEF_A] = SIGN_EXTEND(BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_G_ACOEFF_W), 23);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_G_BE);
    lsc->lsc_coef_g.coef[LSC_COEF_B] = SIGN_EXTEND(BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_G_BCOEFF_W), 13);
    lsc->lsc_coef_g.coef[LSC_COEF_E] = SIGN_EXTEND(BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_G_ECOEFF_W), 13);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_G_CD);
    lsc->lsc_coef_g.coef[LSC_COEF_C] = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_G_CCOEFF_W);
    lsc->lsc_coef_g.coef[LSC_COEF_D] = SIGN_EXTEND(BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_G_DCOEFF_W), 23);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_G_G);
    lsc->lsc_coef_g.coef[LSC_COEF_G] = SIGN_EXTEND(BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_G_GCOEFF_W), 23);

    /* channel B */
    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_B_A);
    lsc->lsc_coef_b.coef[LSC_COEF_A] = SIGN_EXTEND(BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_B_ACOEFF_W), 23);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_B_BE);
    lsc->lsc_coef_b.coef[LSC_COEF_B] = SIGN_EXTEND(BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_B_BCOEFF_W), 13);
    lsc->lsc_coef_b.coef[LSC_COEF_E] = SIGN_EXTEND(BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_B_ECOEFF_W), 13);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_B_CD);
    lsc->lsc_coef_b.coef[LSC_COEF_C] = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_B_CCOEFF_W);
    lsc->lsc_coef_b.coef[LSC_COEF_D] = SIGN_EXTEND(BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_B_DCOEFF_W), 23);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_B_G);
    lsc->lsc_coef_b.coef[LSC_COEF_G] = SIGN_EXTEND(BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_B_GCOEFF_W), 23);

    return 0;
}

int ISP_Set_LSC_Coeff_Type(const int type_idx, ISP_LSC_COEF_TYPE *lsc)
{
    uint32_t tmpVal;

    if (!is_LSC_Coeff_Type_Valid(lsc)) {
        return -1;
    }

    switch(type_idx)
    {
        case 0: {
            tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_R_A);
            tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE_R_ACOEFF_W, lsc->coef[LSC_COEF_A]);
            BL_WR_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_R_A, tmpVal);

            tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_R_BE);
            tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE_R_BCOEFF_W, lsc->coef[LSC_COEF_B]);
            tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE_R_ECOEFF_W, lsc->coef[LSC_COEF_E]);
            BL_WR_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_R_BE, tmpVal);

            tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_R_CD);
            tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE_R_CCOEFF_W, lsc->coef[LSC_COEF_C]);
            tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE_R_DCOEFF_W, lsc->coef[LSC_COEF_D]);
            BL_WR_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_R_CD, tmpVal);

            tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_R_G);
            tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE_R_GCOEFF_W, lsc->coef[LSC_COEF_G]);
            BL_WR_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_R_G, tmpVal);
            break;
        }
        case 1: {
            tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_G_A);
            tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE_G_ACOEFF_W, lsc->coef[LSC_COEF_A]);
            BL_WR_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_G_A, tmpVal);

            tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_G_BE);
            tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE_G_BCOEFF_W, lsc->coef[LSC_COEF_B]);
            tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE_G_ECOEFF_W, lsc->coef[LSC_COEF_E]);
            BL_WR_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_G_BE, tmpVal);

            tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_G_CD);
            tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE_G_CCOEFF_W, lsc->coef[LSC_COEF_C]);
            tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE_G_DCOEFF_W, lsc->coef[LSC_COEF_D]);
            BL_WR_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_G_CD, tmpVal);

            tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_G_G);
            tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE_G_GCOEFF_W, lsc->coef[LSC_COEF_G]);
            BL_WR_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_G_G, tmpVal);
            break;
        }
        case 2: {
            tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_B_A);
            tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE_B_ACOEFF_W, lsc->coef[LSC_COEF_A]);
            BL_WR_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_B_A, tmpVal);

            tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_B_BE);
            tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE_B_BCOEFF_W, lsc->coef[LSC_COEF_B]);
            tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE_B_ECOEFF_W, lsc->coef[LSC_COEF_E]);
            BL_WR_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_B_BE, tmpVal);

            tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_B_CD);
            tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE_B_CCOEFF_W, lsc->coef[LSC_COEF_C]);
            tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE_B_DCOEFF_W, lsc->coef[LSC_COEF_D]);
            BL_WR_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_B_CD, tmpVal);

            tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_B_G);
            tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_MIDDLE_B_GCOEFF_W, lsc->coef[LSC_COEF_G]);
            BL_WR_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_B_G, tmpVal);
            break;
        }
    }

    return 0;
}

int ISP_Get_LSC_Coeff_Type(const int type_idx, ISP_LSC_COEF_TYPE *lsc)
{
    uint32_t tmpVal;
    switch(type_idx)
    {
        case 0: {  /* channel R */
            tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_R_A);
            lsc->coef[LSC_COEF_A] = SIGN_EXTEND(BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_R_ACOEFF_W), 23);

            tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_R_BE);
            lsc->coef[LSC_COEF_B] = SIGN_EXTEND(BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_R_BCOEFF_W), 13);
            lsc->coef[LSC_COEF_E] = SIGN_EXTEND(BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_R_ECOEFF_W), 13);

            tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_R_CD);
            lsc->coef[LSC_COEF_C] = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_R_CCOEFF_W);
            lsc->coef[LSC_COEF_D] = SIGN_EXTEND(BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_R_DCOEFF_W), 23);

            tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_R_G);
            lsc->coef[LSC_COEF_G] = SIGN_EXTEND(BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_R_GCOEFF_W), 23);
            break;
        }
        case 1: { /* channel G */
            tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_G_A);
            lsc->coef[LSC_COEF_A] = SIGN_EXTEND(BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_G_ACOEFF_W), 23);

            tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_G_BE);
            lsc->coef[LSC_COEF_B] = SIGN_EXTEND(BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_G_BCOEFF_W), 13);
            lsc->coef[LSC_COEF_E] = SIGN_EXTEND(BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_G_ECOEFF_W), 13);

            tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_G_CD);
            lsc->coef[LSC_COEF_C] = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_G_CCOEFF_W);
            lsc->coef[LSC_COEF_D] = SIGN_EXTEND(BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_G_DCOEFF_W), 23);

            tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_G_G);
            lsc->coef[LSC_COEF_G] = SIGN_EXTEND(BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_G_GCOEFF_W), 23);
            break;
        }
        case 2: { /* channel B */
            tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_B_A);
            lsc->coef[LSC_COEF_A] = SIGN_EXTEND(BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_B_ACOEFF_W), 23);

            tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_B_BE);
            lsc->coef[LSC_COEF_B] = SIGN_EXTEND(BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_B_BCOEFF_W), 13);
            lsc->coef[LSC_COEF_E] = SIGN_EXTEND(BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_B_ECOEFF_W), 13);

            tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_B_CD);
            lsc->coef[LSC_COEF_C] = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_B_CCOEFF_W);
            lsc->coef[LSC_COEF_D] = SIGN_EXTEND(BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_B_DCOEFF_W), 23);

            tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_LSC_COEFF_B_G);
            lsc->coef[LSC_COEF_G] = SIGN_EXTEND(BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_B_GCOEFF_W), 23);
            break;
        }
    }

    return 0;
}


int ISP_Set_LSC_Max(const ISP_LSC_MAX_S *lsc_max)
{
    isp_middle_reg_t *isp_middle_reg = (isp_middle_reg_t*)ISP_BASE;

    CHECK_PARAM_RANGE(lsc_max->max_r, ISP_LSC_MAX_MIN, ISP_LSC_MAX_MAX);
    CHECK_PARAM_RANGE(lsc_max->max_g, ISP_LSC_MAX_MIN, ISP_LSC_MAX_MAX);
    CHECK_PARAM_RANGE(lsc_max->max_b, ISP_LSC_MAX_MIN, ISP_LSC_MAX_MAX);

    isp_middle_reg->MAX_R_G.BF.MAXR_w = lsc_max->max_r;
    isp_middle_reg->MAX_R_G.BF.MAXG_w = lsc_max->max_g;
    isp_middle_reg->MAX_B.BF.MAXB_w = lsc_max->max_b;

    return 0;
}

int ISP_Get_LSC_Max(ISP_LSC_MAX_S *lsc_max)
{
    isp_middle_reg_t *isp_middle_reg = (isp_middle_reg_t*)ISP_BASE;

    lsc_max->max_r = isp_middle_reg->MAX_R_G.BF.MAXR_w;
    lsc_max->max_g = isp_middle_reg->MAX_R_G.BF.MAXG_w;
    lsc_max->max_b = isp_middle_reg->MAX_B.BF.MAXB_w;

    return 0;
}

void ISP_Set_Digital_Gain(int r_gain, int gr_gain, int gb_gain, int b_gain)
{
    uint32_t tmpVal;

    CHECK_PARAM_RANGE(r_gain, ISP_DIGITAL_GAIN_MIN, ISP_DIGITAL_GAIN_MAX);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_DIGITAL_RGR_GAIN);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_DIGITAL_R_GAIN_PRE, r_gain);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_DIGITAL_GR_GAIN_PRE, gr_gain);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE_DIGITAL_RGR_GAIN, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_DIGITAL_BGB_GAIN);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_DIGITAL_GB_GAIN_PRE, gb_gain);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_DIGITAL_B_GAIN_PRE, b_gain);
    BL_WR_REG(ISP_BASE, ISP_MIDDLE_DIGITAL_BGB_GAIN, tmpVal);
}

void ISP_Get_Digital_Gain(int *r_gain, int *gr_gain, int *gb_gain, int *b_gain)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_DIGITAL_RGR_GAIN);
    *r_gain = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_DIGITAL_R_GAIN_PRE);
    *gr_gain = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_DIGITAL_GR_GAIN_PRE);
    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_DIGITAL_BGB_GAIN);
    *gb_gain = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_DIGITAL_GB_GAIN_PRE);
    *b_gain = BL_GET_REG_BITS_VAL(tmpVal, ISP_MIDDLE_DIGITAL_B_GAIN_PRE);
}

void ISP_Set_CFA_Int_State(BL_Fun_Type state)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_CFA_INT_1);
    if (ENABLE == state) {
        tmpVal = BL_SET_REG_BIT(tmpVal, ISP_MIDDLE_REG_CFA_FRINGE_ON_W);
    } else {
        tmpVal = BL_CLR_REG_BIT(tmpVal, ISP_MIDDLE_REG_CFA_FRINGE_ON_W);
    }
    BL_WR_REG(ISP_BASE, ISP_MIDDLE_CFA_INT_1, tmpVal);
}

void ISP_Get_CFA_Int_State(BL_Fun_Type *state)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_MIDDLE_CFA_INT_1);

    if (BL_IS_REG_BIT_SET(tmpVal, ISP_MIDDLE_REG_CFA_FRINGE_ON_W)) {
        *state = ENABLE;
    } else {
        *state = DISABLE;
    }
}


void ISP_Set_CCM_State(BL_Fun_Type state)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_CC_OFFSET_0);
    if (ENABLE == state) {
        tmpVal = BL_SET_REG_BIT(tmpVal, ISP_BLBACK_CC_ENABLE_PRE);
    } else {
        tmpVal = BL_CLR_REG_BIT(tmpVal, ISP_BLBACK_CC_ENABLE_PRE);
    }
    BL_WR_REG(ISP_BASE, ISP_BLBACK_CC_OFFSET_0, tmpVal);
}

void ISP_Get_CCM_State(BL_Fun_Type *state)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_CC_OFFSET_0);

    if (BL_IS_REG_BIT_SET(tmpVal, ISP_BLBACK_CC_ENABLE_PRE)) {
        *state = ENABLE;
    } else {
        *state = DISABLE;
    }
}


void ISP_Set_CCM_Coef(const ISP_CCM_COEF_S *ccm)
{
    int i;
    uint32_t tmpVal;

    for (i = 0; i < CCM_COEF_NUM; i++) {
        CHECK_PARAM_RANGE(ccm->ccm_coef[i], ISP_CCM_MIN, ISP_CCM_MAX);
    }

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_CC_MATRIX_0);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_BLBACK_CC_MATRIX00_PRE, ccm->ccm_coef[ISP_CCM_11]);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_BLBACK_CC_MATRIX01_PRE, ccm->ccm_coef[ISP_CCM_12]);
    BL_WR_REG(ISP_BASE, ISP_BLBACK_CC_MATRIX_0, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_CC_MATRIX_1);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_BLBACK_CC_MATRIX02_PRE, ccm->ccm_coef[ISP_CCM_13]);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_BLBACK_CC_MATRIX10_PRE, ccm->ccm_coef[ISP_CCM_21]);
    BL_WR_REG(ISP_BASE, ISP_BLBACK_CC_MATRIX_1, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_CC_MATRIX_2);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_BLBACK_CC_MATRIX11_PRE, ccm->ccm_coef[ISP_CCM_22]);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_BLBACK_CC_MATRIX12_PRE, ccm->ccm_coef[ISP_CCM_23]);
    BL_WR_REG(ISP_BASE, ISP_BLBACK_CC_MATRIX_2, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_CC_MATRIX_3);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_BLBACK_CC_MATRIX20_PRE, ccm->ccm_coef[ISP_CCM_31]);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_BLBACK_CC_MATRIX21_PRE, ccm->ccm_coef[ISP_CCM_32]);
    BL_WR_REG(ISP_BASE, ISP_BLBACK_CC_MATRIX_3, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_CC_CE_MATRIX);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_BLBACK_CC_MATRIX22_PRE, ccm->ccm_coef[ISP_CCM_33]);
    BL_WR_REG(ISP_BASE, ISP_BLBACK_CC_CE_MATRIX, tmpVal);
}

void ISP_Get_CCM_Coef(ISP_CCM_COEF_S *ccm)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_CC_MATRIX_0);
    ccm->ccm_coef[ISP_CCM_11] = SIGN_EXTEND(BL_GET_REG_BITS_VAL(tmpVal, ISP_BLBACK_CC_MATRIX00_PRE), ISP_BLBACK_CC_MATRIX00_PRE_LEN);
    ccm->ccm_coef[ISP_CCM_12] = SIGN_EXTEND(BL_GET_REG_BITS_VAL(tmpVal, ISP_BLBACK_CC_MATRIX01_PRE), ISP_BLBACK_CC_MATRIX01_PRE_LEN);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_CC_MATRIX_1);
    ccm->ccm_coef[ISP_CCM_13] = SIGN_EXTEND(BL_GET_REG_BITS_VAL(tmpVal, ISP_BLBACK_CC_MATRIX02_PRE), ISP_BLBACK_CC_MATRIX02_PRE_LEN);
    ccm->ccm_coef[ISP_CCM_21] = SIGN_EXTEND(BL_GET_REG_BITS_VAL(tmpVal, ISP_BLBACK_CC_MATRIX10_PRE), ISP_BLBACK_CC_MATRIX10_PRE_LEN);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_CC_MATRIX_2);
    ccm->ccm_coef[ISP_CCM_22] = SIGN_EXTEND(BL_GET_REG_BITS_VAL(tmpVal, ISP_BLBACK_CC_MATRIX11_PRE), ISP_BLBACK_CC_MATRIX11_PRE_LEN);
    ccm->ccm_coef[ISP_CCM_23] = SIGN_EXTEND(BL_GET_REG_BITS_VAL(tmpVal, ISP_BLBACK_CC_MATRIX12_PRE), ISP_BLBACK_CC_MATRIX12_PRE_LEN);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_CC_MATRIX_3);
    ccm->ccm_coef[ISP_CCM_31] = SIGN_EXTEND(BL_GET_REG_BITS_VAL(tmpVal, ISP_BLBACK_CC_MATRIX20_PRE), ISP_BLBACK_CC_MATRIX20_PRE_LEN);
    ccm->ccm_coef[ISP_CCM_32] = SIGN_EXTEND(BL_GET_REG_BITS_VAL(tmpVal, ISP_BLBACK_CC_MATRIX21_PRE), ISP_BLBACK_CC_MATRIX21_PRE_LEN);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_CC_CE_MATRIX);
    ccm->ccm_coef[ISP_CCM_33] = SIGN_EXTEND(BL_GET_REG_BITS_VAL(tmpVal, ISP_BLBACK_CC_MATRIX22_PRE), ISP_BLBACK_CC_MATRIX22_PRE_LEN);
}

void ISP_Set_CCM_Pre_Offset(const ISP_CCM_OFFSET_S *ccm)
{
    int i;
    uint32_t tmpVal;

    for (i = 0; i < CCM_OFFSET_NUM; i++) {
        CHECK_PARAM_RANGE(ccm->offset[i], ISP_CCM_OFFSET_MIN, ISP_CCM_OFFSET_MAX);
    }

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_CC_OFFSET_0);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_BLBACK_CC_PRE_OFFSET0_PRE, ccm->offset[ISP_CCM_OFFSET_0]);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_BLBACK_CC_PRE_OFFSET1_PRE, ccm->offset[ISP_CCM_OFFSET_1]);
    BL_WR_REG(ISP_BASE, ISP_BLBACK_CC_OFFSET_0, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_CC_OFFSET_1);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_BLBACK_CC_PRE_OFFSET2_PRE, ccm->offset[ISP_CCM_OFFSET_2]);
    BL_WR_REG(ISP_BASE, ISP_BLBACK_CC_OFFSET_1, tmpVal);

}

void ISP_Get_CCM_Pre_Offset(ISP_CCM_OFFSET_S *ccm)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_CC_OFFSET_0);
    ccm->offset[ISP_CCM_OFFSET_0] = SIGN_EXTEND(BL_GET_REG_BITS_VAL(tmpVal, ISP_BLBACK_CC_PRE_OFFSET0_PRE), ISP_BLBACK_CC_PRE_OFFSET0_PRE_LEN);
    ccm->offset[ISP_CCM_OFFSET_1] = SIGN_EXTEND(BL_GET_REG_BITS_VAL(tmpVal, ISP_BLBACK_CC_PRE_OFFSET1_PRE), ISP_BLBACK_CC_PRE_OFFSET1_PRE_LEN);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_CC_OFFSET_1);
    ccm->offset[ISP_CCM_OFFSET_2] = SIGN_EXTEND(BL_GET_REG_BITS_VAL(tmpVal, ISP_BLBACK_CC_PRE_OFFSET2_PRE), ISP_BLBACK_CC_PRE_OFFSET2_PRE_LEN);
}

void ISP_Set_CCM_Post_Offset(const ISP_CCM_OFFSET_S *ccm)
{
    int i;
    uint32_t tmpVal;

    for (i = 0; i < CCM_OFFSET_NUM; i++) {
        CHECK_PARAM_RANGE(ccm->offset[i], ISP_CCM_OFFSET_MIN, ISP_CCM_OFFSET_MAX);
    }

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_CC_OFFSET_2);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_BLBACK_CC_POST_OFFSET0_PRE, ccm->offset[ISP_CCM_OFFSET_0]);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_BLBACK_CC_POST_OFFSET1_PRE, ccm->offset[ISP_CCM_OFFSET_1]);
    BL_WR_REG(ISP_BASE, ISP_BLBACK_CC_OFFSET_0, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_CC_OFFSET_1);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_BLBACK_CC_POST_OFFSET2_PRE, ccm->offset[ISP_CCM_OFFSET_2]);
    BL_WR_REG(ISP_BASE, ISP_BLBACK_CC_OFFSET_1, tmpVal);

}

void ISP_Get_CCM_Post_Offset(ISP_CCM_OFFSET_S *ccm)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_CC_OFFSET_2);
    ccm->offset[ISP_CCM_OFFSET_0] = SIGN_EXTEND(BL_GET_REG_BITS_VAL(tmpVal, ISP_BLBACK_CC_POST_OFFSET0_PRE), ISP_BLBACK_CC_POST_OFFSET0_PRE_LEN);
    ccm->offset[ISP_CCM_OFFSET_1] = SIGN_EXTEND(BL_GET_REG_BITS_VAL(tmpVal, ISP_BLBACK_CC_POST_OFFSET1_PRE), ISP_BLBACK_CC_POST_OFFSET1_PRE_LEN);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_CC_OFFSET_1);
    ccm->offset[ISP_CCM_OFFSET_2] = SIGN_EXTEND(BL_GET_REG_BITS_VAL(tmpVal, ISP_BLBACK_CC_POST_OFFSET2_PRE), ISP_BLBACK_CC_POST_OFFSET2_PRE_LEN);
}

void ISP_Set_Gamma_State(BL_Fun_Type state)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_GAMMA_A);
    if (ENABLE == state) {
        tmpVal = BL_SET_REG_BIT(tmpVal, ISP_BLBACK_BL_GAMMA_EN_PRE);
    } else {
        tmpVal = BL_CLR_REG_BIT(tmpVal, ISP_BLBACK_BL_GAMMA_EN_PRE);
    }
    BL_WR_REG(ISP_BASE, ISP_BLBACK_GAMMA_A, tmpVal);
}

void ISP_Get_Gamma_State(BL_Fun_Type *state)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_GAMMA_A);

    if (BL_IS_REG_BIT_SET(tmpVal, ISP_BLBACK_BL_GAMMA_EN_PRE)) {
        *state = ENABLE;
    } else {
        *state = DISABLE;
    }
}

void ISP_Set_Gamma_Coef(const ISP_Gamma_Curve_Type *gamma_curve)
{
    memcpy((uint32_t*)(ISP_BASE + ISP_BLBACK_BL_GAMMA_TABLE_1_OFFSET), (uint32_t*)&gamma_curve->curve_coef[0],
        GAMMA_COEF_NUM / 2);
}

void ISP_Get_Gamma_Coef(ISP_Gamma_Curve_Type *gamma_curve)
{
    memcpy((uint32_t*)&gamma_curve->curve_coef[0], (uint32_t*)(ISP_BASE + ISP_BLBACK_BL_GAMMA_TABLE_1_OFFSET),
        GAMMA_COEF_NUM / 2);
}

/*
    should not change, yuv range should change in yuv adjust module
*/
void ISP_Init_Color_Enhancement()
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_CE_MATRIX_0);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_BLBACK_CE_MATRIX00_PRE, 0x99);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_BLBACK_CE_MATRIX01_PRE, 0x12D);
    BL_WR_REG(ISP_BASE, ISP_BLBACK_CE_MATRIX_0, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_CC_CE_MATRIX);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_BLBACK_CE_MATRIX02_PRE, 0x3A);
    BL_WR_REG(ISP_BASE, ISP_BLBACK_CC_CE_MATRIX, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_CE_MATRIX_1);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_BLBACK_CE_MATRIX10_PRE, 0xFA9);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_BLBACK_CE_MATRIX11_PRE, 0xF57);
    BL_WR_REG(ISP_BASE, ISP_BLBACK_CE_MATRIX_1, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_CE_MATRIX_2);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_BLBACK_CE_MATRIX12_PRE, 0x100);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_BLBACK_CE_MATRIX20_PRE, 0x100);
    BL_WR_REG(ISP_BASE, ISP_BLBACK_CE_MATRIX_2, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_CE_MATRIX_3);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_BLBACK_CE_MATRIX21_PRE, 0xF29);
    tmpVal = BL_SET_REG_BITS_VAL_2(tmpVal, ISP_BLBACK_CE_MATRIX22_PRE, 0xFD7);
    BL_WR_REG(ISP_BASE, ISP_BLBACK_CE_MATRIX_3, tmpVal);
}

void ISP_Set_YUV_Conv_Mode(ISP_YUV_CONV_TYPE type)
{
    uint32_t tmpVal;

    /* Check the parameters */
    CHECK_PARAM(IS_ISP_YUV_CONV_TYPE(type));

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_CONV_MODE);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_BLBACK_CONV_MODE_W, type);
    BL_WR_REG(ISP_BASE, ISP_BLBACK_CONV_MODE, tmpVal);
}

void ISP_Get_YUV_Conv_Mode(ISP_YUV_CONV_TYPE *type)
{
    *type = BL_GET_REG_BITS_VAL(BL_RD_REG(ISP_BASE, ISP_BLBACK_CONV_MODE), ISP_BLBACK_CONV_MODE_W);
}

void ISP_Set_Brightness(int value)
{
    uint32_t tmpVal;

    CHECK_PARAM_RANGE(value, ISP_BRIGHTNESS_MIN, ISP_BRIGHTNESS_MAX);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_YUVC_A);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_BLBACK_YBRIGHTNESS_W, value);
    BL_WR_REG(ISP_BASE, ISP_BLBACK_YUVC_A, tmpVal);
}

void ISP_Get_Brightness(int *value)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_YUVC_A);
    *value = SIGN_EXTEND(BL_GET_REG_BITS_VAL(tmpVal, ISP_BLBACK_YBRIGHTNESS_W), ISP_BLBACK_YBRIGHTNESS_W_LEN);
}

void ISP_Set_Contrast(int value)
{
    uint32_t tmpVal;

    CHECK_PARAM_RANGE(value, ISP_CONTRAST_MIN, ISP_CONTRAST_MAX);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_YUVC_A);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_BLBACK_YCONTRAST_W, value);
    BL_WR_REG(ISP_BASE, ISP_BLBACK_YUVC_A, tmpVal);
}

void ISP_Get_Contrast(int *value)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_YUVC_A);
    *value = BL_GET_REG_BITS_VAL(tmpVal, ISP_BLBACK_YCONTRAST_W);
}

void ISP_Set_Saturation(const ISP_SATURATION_S *saturation)
{
    uint32_t tmpVal;

    CHECK_PARAM_MAX(saturation->saturation_cb, ISP_SATURATION_MAX);
    CHECK_PARAM_MAX(saturation->saturation_cr, ISP_SATURATION_MAX);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_YUVC_A);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_BLBACK_CBSATURATION_PRE, saturation->saturation_cb);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_BLBACK_CRSATURATION_PRE, saturation->saturation_cr);
    BL_WR_REG(ISP_BASE, ISP_BLBACK_YUVC_A, tmpVal);
}

void ISP_Get_Saturation(ISP_SATURATION_S *saturation)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_YUVC_A);
    saturation->saturation_cb = BL_GET_REG_BITS_VAL(tmpVal, ISP_BLBACK_CBSATURATION_PRE);
    saturation->saturation_cr = BL_GET_REG_BITS_VAL(tmpVal, ISP_BLBACK_CRSATURATION_PRE);
}

void ISP_Set_Special_Mode(int value)
{
    uint32_t tmpVal;

    CHECK_PARAM_RANGE(value, 0, 4);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_YUVC_B);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_BLBACK_SPECIALMODE_W, value);
    BL_WR_REG(ISP_BASE, ISP_BLBACK_YUVC_B, tmpVal);
}

void ISP_Get_Special_Mode(int *value)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_YUVC_B);
    *value = BL_GET_REG_BITS_VAL(tmpVal, ISP_BLBACK_SPECIALMODE_W);
}

void ISP_Set_LPF_Filting_Mode(int value)
{
    uint32_t tmpVal;

    CHECK_PARAM_RANGE(value, 0, 1);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_YUVC_B);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_BLBACK_C444TO422FILTERINGMODE_W, value);
    BL_WR_REG(ISP_BASE, ISP_BLBACK_YUVC_B, tmpVal);
}

void ISP_Get_LPF_Filting_Mode(int *value)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_YUVC_B);
    *value = BL_GET_REG_BITS_VAL(tmpVal, ISP_BLBACK_C444TO422FILTERINGMODE_W);
}

void ISP_Set_Sepia(int SepiaCr, int SepiaCb)
{
    uint32_t tmpVal;

    CHECK_PARAM_RANGE(SepiaCr, ISP_SOLAR_SEPIA_MIN, ISP_SOLAR_SEPIA_MAX);
    CHECK_PARAM_RANGE(SepiaCb, ISP_SOLAR_SEPIA_MIN, ISP_SOLAR_SEPIA_MAX);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_YUVC_B);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_BLBACK_SEPIACR_W, SepiaCr);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_BLBACK_SEPIACB_W, SepiaCb);
    BL_WR_REG(ISP_BASE, ISP_BLBACK_YUVC_B, tmpVal);
}

void ISP_Get_Sepia(int *SepiaCr, int *SepiaCb)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_YUVC_B);
    *SepiaCr = BL_GET_REG_BITS_VAL(tmpVal, ISP_BLBACK_SEPIACR_W);
    *SepiaCb = BL_GET_REG_BITS_VAL(tmpVal, ISP_BLBACK_SEPIACB_W);
}

void ISP_Set_Solar_Thred(int value)
{
    uint32_t tmpVal;

    CHECK_PARAM_RANGE(value, ISP_SOLAR_THRED_MIN, ISP_SOLAR_THRED_MAX);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_YUVC_B);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_BLBACK_SOLARTHRED_W, value);
    BL_WR_REG(ISP_BASE, ISP_BLBACK_YUVC_B, tmpVal);
}

void ISP_Get_Solar_Thred(int *value)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_YUVC_B);
    *value = BL_GET_REG_BITS_VAL(tmpVal, ISP_BLBACK_SOLARTHRED_W);
}

void ISP_Set_Noise_Level(int value)
{
    uint32_t tmpVal;

    CHECK_PARAM_RANGE(value, ISP_NOISE_LEVEL_MIN, ISP_NOISE_LEVEL_MAX);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_DNR_A);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_BLBACK_NOISE_LEVEL_PRE, value);
    BL_WR_REG(ISP_BASE, ISP_BLBACK_DNR_A, tmpVal);
}

void ISP_Get_Noise_Level(int *value)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_DNR_A);
    *value = BL_GET_REG_BITS_VAL(tmpVal, ISP_BLBACK_NOISE_LEVEL_PRE);
}

void ISP_Set_2D_NR(int value)
{
    uint32_t tmpVal;

    CHECK_PARAM_RANGE(value, ISP_NR_STRENGTH_MIN, ISP_NR_STRENGTH_MAX);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_DNR_A);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_BLBACK_FILT_STRENGTH_2D_PRE, value);
    BL_WR_REG(ISP_BASE, ISP_BLBACK_DNR_A, tmpVal);
}

void ISP_Get_2D_NR(int *value)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_DNR_A);
    *value = BL_GET_REG_BITS_VAL(tmpVal, ISP_BLBACK_FILT_STRENGTH_2D_PRE);
}

void ISP_Set_3D_NR(const int value)
{
    uint32_t tmpVal;

    CHECK_PARAM_RANGE(value, ISP_NR_STRENGTH_MIN, ISP_NR_STRENGTH_MAX);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_DNR_A);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_BLBACK_FILT_STRENGTH_3D_PRE, value);
    BL_WR_REG(ISP_BASE, ISP_BLBACK_DNR_A, tmpVal);
}

void ISP_Get_3D_NR(int *value)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_DNR_A);
    *value = BL_GET_REG_BITS_VAL(tmpVal, ISP_BLBACK_FILT_STRENGTH_3D_PRE);
}

void ISP_Set_NR_HSize(const int value)
{
    uint32_t tmpVal;

    CHECK_PARAM_RANGE(value, ISP_NR_HSIZE_MIN, ISP_NR_HSIZE_MAX);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_DNR_A);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_BLBACK_NR_HSIZE_PRE, value);
    BL_WR_REG(ISP_BASE, ISP_BLBACK_DNR_A, tmpVal);
}

void ISP_Get_NR_HSize(int *value)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_DNR_A);
    *value = BL_GET_REG_BITS_VAL(tmpVal, ISP_BLBACK_NR_HSIZE_PRE);
}

void ISP_Set_EE_State(BL_Fun_Type state)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_BLEE_0);
    if (ENABLE == state) {
        tmpVal = BL_SET_REG_BIT(tmpVal, ISP_BLBACK_EE_EB_PRE);
    } else {
        tmpVal = BL_CLR_REG_BIT(tmpVal, ISP_BLBACK_EE_EB_PRE);
    }
    BL_WR_REG(ISP_BASE, ISP_BLBACK_BLEE_0, tmpVal);
}

void ISP_Get_EE_State(BL_Fun_Type *state)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_BLEE_0);

    if (BL_IS_REG_BIT_SET(tmpVal, ISP_BLBACK_EE_EB_PRE)) {
        *state = ENABLE;
    } else {
        *state = DISABLE;
    }
}

void ISP_Enable_EE(void)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_BLEE_0);
    tmpVal = BL_SET_REG_BIT(tmpVal, ISP_BLBACK_EE_EB_PRE);
    BL_WR_REG(ISP_BASE, ISP_BLBACK_BLEE_0, tmpVal);
}

void ISP_Disable_EE(void)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_BLEE_0);
    tmpVal = BL_CLR_REG_BIT(tmpVal, ISP_BLBACK_EE_EB_PRE);
    BL_WR_REG(ISP_BASE, ISP_BLBACK_BLEE_0, tmpVal);
}

void ISP_EnableEEOverShoot(void)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_BLEE_0);
    tmpVal = BL_SET_REG_BIT(tmpVal, ISP_BLBACK_OVRSHT_EB_PRE);
    BL_WR_REG(ISP_BASE, ISP_BLBACK_BLEE_0, tmpVal);
}

void ISP_DisableEEOverShoot(void)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_BLEE_0);
    tmpVal = BL_CLR_REG_BIT(tmpVal, ISP_BLBACK_OVRSHT_EB_PRE);
    BL_WR_REG(ISP_BASE, ISP_BLBACK_BLEE_0, tmpVal);
}

void ISP_Set_EEOverShoot_State(BL_Fun_Type state)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_BLEE_0);
    if (ENABLE == state) {
        tmpVal = BL_SET_REG_BIT(tmpVal, ISP_BLBACK_OVRSHT_EB_PRE);
    } else {
        tmpVal = BL_CLR_REG_BIT(tmpVal, ISP_BLBACK_OVRSHT_EB_PRE);
    }
    BL_WR_REG(ISP_BASE, ISP_BLBACK_BLEE_0, tmpVal);
}

void ISP_Get_EEOverShoot_State(BL_Fun_Type *state)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_BLEE_0);

    if (BL_IS_REG_BIT_SET(tmpVal, ISP_BLBACK_OVRSHT_EB_PRE)) {
        *state = ENABLE;
    } else {
        *state = DISABLE;
    }
}

void ISP_Set_EE_Thresh(const ISP_EE_Thresh_Type *threshCfg)
{
    uint32_t tmpVal;

   CHECK_PARAM_MAX(threshCfg->dirShpnessThresh, ISP_EE_DIR_SHPNSS_THRESH_MAX);
   CHECK_PARAM_MAX(threshCfg->noDirShpnessThresh, ISP_EE_NO_DIR_SHPNSS_THRESH_MAX);
   CHECK_PARAM_RANGE(threshCfg->textureThresh, (threshCfg->flatThresh + 1), ISP_EE_TEXTURE_REGION_THRESH_MAX);
   CHECK_PARAM_MIN(threshCfg->edgeThresh, (threshCfg->textureThresh + 1));

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_BLEE_0);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_BLBACK_TXT_THR_PRE, threshCfg->textureThresh);
    BL_WR_REG(ISP_BASE, ISP_BLBACK_BLEE_0, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_BLEE_1);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_BLBACK_THRESH1_PRE, threshCfg->dirShpnessThresh);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_BLBACK_THRESH2_PRE, threshCfg->noDirShpnessThresh);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_BLBACK_FLAT_THR_PRE, threshCfg->flatThresh);
    BL_WR_REG(ISP_BASE, ISP_BLBACK_BLEE_1, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_BLEE_3);;
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_BLBACK_EDGE_THR_PRE, threshCfg->edgeThresh);
    BL_WR_REG(ISP_BASE, ISP_BLBACK_BLEE_3, tmpVal);
}

void ISP_Get_EE_Thresh(ISP_EE_Thresh_Type *threshCfg)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_BLEE_0);
    threshCfg->textureThresh = BL_GET_REG_BITS_VAL(tmpVal, ISP_BLBACK_TXT_THR_PRE);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_BLEE_1);
    threshCfg->dirShpnessThresh = BL_GET_REG_BITS_VAL(tmpVal, ISP_BLBACK_THRESH1_PRE);
    threshCfg->noDirShpnessThresh = BL_GET_REG_BITS_VAL(tmpVal, ISP_BLBACK_THRESH2_PRE);
    threshCfg->flatThresh = BL_GET_REG_BITS_VAL(tmpVal, ISP_BLBACK_FLAT_THR_PRE);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_BLEE_3);
    threshCfg->edgeThresh = BL_GET_REG_BITS_VAL(tmpVal, ISP_BLBACK_EDGE_THR_PRE);
}

void ISP_Set_EE_Strength(const ISP_EE_Str_Type *strCfg)
{
    uint32_t tmpVal;

    CHECK_PARAM_MAX(strCfg->posEdgeOvrshtStr, ISP_EE_STRENGTH_MAX);
    CHECK_PARAM_MAX(strCfg->negEdgeOvrshtStr, ISP_EE_STRENGTH_MAX);
    CHECK_PARAM_MAX(strCfg->posEdgeStr, ISP_EE_STRENGTH_MAX);
    CHECK_PARAM_MAX(strCfg->negEdgeStr, ISP_EE_STRENGTH_MAX);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_BLEE_2);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_BLBACK_OVRSHT_POS_PRE, strCfg->posEdgeOvrshtStr);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_BLBACK_OVRSHT_NEG_PRE, strCfg->negEdgeOvrshtStr);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_BLBACK_POSEE_STR_PRE, strCfg->posEdgeStr);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_BLBACK_NEGEE_STR_PRE, strCfg->negEdgeStr);
    BL_WR_REG(ISP_BASE, ISP_BLBACK_BLEE_2, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_BLEE_3);;
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_BLBACK_NR_STR_PRE, strCfg->nrStr);
    BL_WR_REG(ISP_BASE, ISP_BLBACK_BLEE_3, tmpVal);
}

void ISP_Get_EE_Strength(ISP_EE_Str_Type *strCfg)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_BLEE_2);
    strCfg->posEdgeOvrshtStr = BL_GET_REG_BITS_VAL(tmpVal, ISP_BLBACK_OVRSHT_POS_PRE);
    strCfg->negEdgeOvrshtStr = BL_GET_REG_BITS_VAL(tmpVal, ISP_BLBACK_OVRSHT_NEG_PRE);
    strCfg->posEdgeStr = BL_GET_REG_BITS_VAL(tmpVal, ISP_BLBACK_POSEE_STR_PRE);
    strCfg->negEdgeStr = BL_GET_REG_BITS_VAL(tmpVal, ISP_BLBACK_NEGEE_STR_PRE);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_BLEE_3);;
    strCfg->nrStr = BL_GET_REG_BITS_VAL(tmpVal, ISP_BLBACK_NR_STR_PRE);
}

void ISP_Set_EE_LumaWeight(const ISP_EE_Luma_Weight_Type *lumaWgtCfg)
{
    int i;
    uint32_t tmpVal;
    uint32_t tmpAddr = ISP_BASE + ISP_BLBACK_BLEE_4_OFFSET;

    for (i = 0; i < ISP_EE_LUMA_WEIGHT_NUM; i+=2, tmpAddr+=4) {
        CHECK_PARAM_MAX(lumaWgtCfg->lumaWgtTbl[i], ISP_EE_LUMA_WEIGHT_MAX);
        CHECK_PARAM_MAX(lumaWgtCfg->lumaWgtTbl[i+1], ISP_EE_LUMA_WEIGHT_MAX);
        tmpVal = BL_RD_WORD(tmpAddr);
        tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_BLBACK_LUMA_WGT0_PRE, lumaWgtCfg->lumaWgtTbl[i]);
        tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_BLBACK_LUMA_WGT1_PRE, lumaWgtCfg->lumaWgtTbl[i+1]);
        BL_WR_WORD(tmpAddr, tmpVal);
    }
}

void ISP_Get_EE_LumaWeight(ISP_EE_Luma_Weight_Type *lumaWgtCfg)
{
    int i;
    uint32_t tmpVal;
    uint32_t tmpAddr = ISP_BASE + ISP_BLBACK_BLEE_4_OFFSET;

    for (i = 0; i < ISP_EE_LUMA_WEIGHT_NUM; i+=2, tmpAddr+=4) {
        tmpVal = BL_RD_WORD(tmpAddr);
        lumaWgtCfg->lumaWgtTbl[i] = BL_GET_REG_BITS_VAL(tmpVal, ISP_BLBACK_LUMA_WGT0_PRE);
        lumaWgtCfg->lumaWgtTbl[i+1] = BL_GET_REG_BITS_VAL(tmpVal, ISP_BLBACK_LUMA_WGT1_PRE);
    }
}

void ISP_Set_ChromaSuppress_State(BL_Fun_Type state)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_CS2);
    if (ENABLE == state) {
        tmpVal = BL_SET_REG_BIT(tmpVal, ISP_BLBACK_CS2_ENABLE_PRE);
    } else {
        tmpVal = BL_CLR_REG_BIT(tmpVal, ISP_BLBACK_CS2_ENABLE_PRE);
    }
    BL_WR_REG(ISP_BASE, ISP_BLBACK_CS2, tmpVal);
}

void ISP_Get_ChromaSuppress_State(BL_Fun_Type *state)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_CS2);

    if (BL_IS_REG_BIT_SET(tmpVal, ISP_BLBACK_CS2_ENABLE_PRE)) {
        *state = ENABLE;
    } else {
        *state = DISABLE;
    }
}

void ISP_Enable_ChromaSuppress(void)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_CS2);
    tmpVal = BL_SET_REG_BIT(tmpVal, ISP_BLBACK_CS2_ENABLE_PRE);
    BL_WR_REG(ISP_BASE, ISP_BLBACK_CS2, tmpVal);
}

void ISP_Disable_ChromaSuppress(void)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_CS2);
    tmpVal = BL_CLR_REG_BIT(tmpVal, ISP_BLBACK_CS2_ENABLE_PRE);
    BL_WR_REG(ISP_BASE, ISP_BLBACK_CS2, tmpVal);
}

void ISP_Set_ChromaSuppress(const ISP_Chroma_Suppress_Type *chromaSupprCfg)
{
    uint32_t tmpVal;
    CHECK_PARAM_MAX(chromaSupprCfg->gain, ISP_CS_GAIN_MAX);
    CHECK_PARAM_MAX(chromaSupprCfg->weight, ISP_CS_WEIGHT_MAX);
    CHECK_PARAM_MAX(chromaSupprCfg->grey_thr, ISP_CS_GAIN_THR_MAX);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_CS2);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_BLBACK_CS2_GAIN_PRE, chromaSupprCfg->gain);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_BLBACK_CS2_WEIGHT_PRE, chromaSupprCfg->weight);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_BLBACK_CS2_GREY_TH_PRE, chromaSupprCfg->grey_thr);
    BL_WR_REG(ISP_BASE, ISP_BLBACK_CS2, tmpVal);
}

void ISP_Get_ChromaSuppress(ISP_Chroma_Suppress_Type *chromaSupprCfg)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_BLBACK_CS2);
    chromaSupprCfg->gain = BL_GET_REG_BITS_VAL(tmpVal, ISP_BLBACK_CS2_GAIN_PRE);
    chromaSupprCfg->weight = BL_GET_REG_BITS_VAL(tmpVal, ISP_BLBACK_CS2_WEIGHT_PRE);
    chromaSupprCfg->grey_thr = BL_GET_REG_BITS_VAL(tmpVal, ISP_BLBACK_CS2_GREY_TH_PRE);
}

void ISP_Get_AF_Stats(isp_af_stats_conf_t *afStats)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_AUTO_AFWAH);
    afStats->wa_hpf1_sum = BL_GET_REG_BITS_VAL(tmpVal, ISP_AUTO_AFWAH1_R);
    afStats->wa_hpf2_sum = BL_GET_REG_BITS_VAL(tmpVal, ISP_AUTO_AFWAH2_R);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_AUTO_AFWBH);
    afStats->wb_hpf1_sum = BL_GET_REG_BITS_VAL(tmpVal, ISP_AUTO_AFWBH1_R);
    afStats->wb_hpf2_sum = BL_GET_REG_BITS_VAL(tmpVal, ISP_AUTO_AFWBH2_R);
}

void ISP_Set_AF_Stats_Conf(const ISP_AF_CONF_Type *afCfg)
{
    uint32_t tmpVal;
    CHECK_PARAM_MAX(afCfg->gamma_pattern, ISP_AF_GAMMA_PATTERN_MAX);
    CHECK_PARAM_MAX(afCfg->fcore1, ISP_AF_FCORE_MAX);
    CHECK_PARAM_MAX(afCfg->fcore2, ISP_AF_FCORE_MAX);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_AUTO_AFBAE_AFMISC);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_AUTO_AFGAMMA_W, afCfg->gamma_pattern);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_AUTO_FCORE1_W, afCfg->fcore1);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_AUTO_FCORE2_W, afCfg->fcore2);
    BL_WR_REG(ISP_BASE, ISP_AUTO_AFBAE_AFMISC, tmpVal);
}

void ISP_Get_AF_Stats_Conf(ISP_AF_CONF_Type *afCfg)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_AUTO_AFBAE_AFMISC);
    afCfg->gamma_pattern = BL_GET_REG_BITS_VAL(tmpVal, ISP_AUTO_AFGAMMA_W);
    afCfg->fcore1 = BL_GET_REG_BITS_VAL(tmpVal, ISP_AUTO_FCORE1_W);
    afCfg->fcore2 = BL_GET_REG_BITS_VAL(tmpVal, ISP_AUTO_FCORE2_W);
}

void ISP_Set_AF_Window_A(const ISP_AF_WINDOW_Type *afWinCfg)
{
    uint32_t tmpVal;
    CHECK_PARAM_MAX(afWinCfg->start_x, ISP_AF_WIN_START_MAX);
    CHECK_PARAM_MAX(afWinCfg->start_y, ISP_AF_WIN_START_MAX);
    CHECK_PARAM_MAX(afWinCfg->end_x, ISP_AF_WIN_END_MAX);
    CHECK_PARAM_MAX(afWinCfg->end_y, ISP_AF_WIN_END_MAX);
    CHECK_PARAM_MAX(afWinCfg->sum_shift, ISP_AF_SUM_SHIFT_A_MAX);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_AUTO_AFWAS);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_AUTO_AFWASX_W, afWinCfg->start_x);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_AUTO_AFWASY_W, afWinCfg->start_y);
    BL_WR_REG(ISP_BASE, ISP_AUTO_AFWAS, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_AUTO_AFWAE_AFWBS);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_AUTO_AFWAEX_W, afWinCfg->end_x);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_AUTO_AFWAEY_W, afWinCfg->end_y);
    BL_WR_REG(ISP_BASE, ISP_AUTO_AFWAE_AFWBS, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_AUTO_AFBAE_AFMISC);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_AUTO_SUM_SHIFT_A_W, afWinCfg->sum_shift);
    BL_WR_REG(ISP_BASE, ISP_AUTO_AFBAE_AFMISC, tmpVal);
}

void ISP_Get_AF_Window_A(ISP_AF_WINDOW_Type *afWinCfg)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_AUTO_AFWAS);
    afWinCfg->start_x = BL_GET_REG_BITS_VAL(tmpVal, ISP_AUTO_AFWASX_W);
    afWinCfg->start_y = BL_GET_REG_BITS_VAL(tmpVal, ISP_AUTO_AFWASY_W);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_AUTO_AFWAE_AFWBS);
    afWinCfg->end_x = BL_GET_REG_BITS_VAL(tmpVal, ISP_AUTO_AFWAEX_W);
    afWinCfg->end_y = BL_GET_REG_BITS_VAL(tmpVal, ISP_AUTO_AFWAEY_W);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_AUTO_AFBAE_AFMISC);
    afWinCfg->sum_shift = BL_GET_REG_BITS_VAL(tmpVal, ISP_AUTO_SUM_SHIFT_A_W);
}

void ISP_Set_AF_Window_B(const ISP_AF_WINDOW_Type *afWinCfg)
{
    uint32_t tmpVal;
    CHECK_PARAM_MAX(afWinCfg->start_x, ISP_AF_WIN_START_MAX);
    CHECK_PARAM_MAX(afWinCfg->start_y, ISP_AF_WIN_START_MAX);
    CHECK_PARAM_MAX(afWinCfg->end_x, ISP_AF_WIN_END_MAX);
    CHECK_PARAM_MAX(afWinCfg->end_y, ISP_AF_WIN_END_MAX);
    CHECK_PARAM_MAX(afWinCfg->sum_shift, ISP_AF_SUM_SHIFT_B_MAX);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_AUTO_AFWAE_AFWBS);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_AUTO_AFWBSX_W, afWinCfg->start_x);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_AUTO_AFWBSY_W, afWinCfg->start_y);
    BL_WR_REG(ISP_BASE, ISP_AUTO_AFWAE_AFWBS, tmpVal);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_AUTO_AFBAE_AFMISC);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_AUTO_AFWBEX_W, afWinCfg->end_x);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_AUTO_AFWBEY_W, afWinCfg->end_y);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, ISP_AUTO_SUM_SHIFT_B_W, afWinCfg->sum_shift);
    BL_WR_REG(ISP_BASE, ISP_AUTO_AFBAE_AFMISC, tmpVal);
}

void ISP_Get_AF_Window_B(ISP_AF_WINDOW_Type *afWinCfg)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(ISP_BASE, ISP_AUTO_AFWAE_AFWBS);
    afWinCfg->start_x = BL_GET_REG_BITS_VAL(tmpVal, ISP_AUTO_AFWBSX_W);
    afWinCfg->start_y = BL_GET_REG_BITS_VAL(tmpVal, ISP_AUTO_AFWBSY_W);

    tmpVal = BL_RD_REG(ISP_BASE, ISP_AUTO_AFBAE_AFMISC);
    afWinCfg->end_x = BL_GET_REG_BITS_VAL(tmpVal, ISP_AUTO_AFWBEX_W);
    afWinCfg->end_y = BL_GET_REG_BITS_VAL(tmpVal, ISP_AUTO_AFWBEY_W);
    afWinCfg->sum_shift = BL_GET_REG_BITS_VAL(tmpVal, ISP_AUTO_SUM_SHIFT_B_W);
}

void ISP_SW_Reset(void)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(MM_GLB_BASE, MM_GLB_SW_RESET_ISP_SUB);
    tmpVal = BL_SET_REG_BIT(tmpVal, MM_GLB_SWRST_ISP_TSRC);
    BL_WR_REG(MM_GLB_BASE, MM_GLB_SW_RESET_ISP_SUB, tmpVal);

    bflb_platform_delay_ms(1);

    tmpVal = BL_CLR_REG_BIT(tmpVal, MM_GLB_SWRST_ISP_TSRC);
    BL_WR_REG(MM_GLB_BASE, MM_GLB_SW_RESET_ISP_SUB, tmpVal);
}

void ISP_Clock_Sel(ISP_CLK_SEL_E clk_sel, BL_Fun_Type clk_div_en, int clk_div)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(MM_GLB_BASE, MM_GLB_ISP_DP_CLK);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, MM_GLB_REG_ISP_CLK_SEL, clk_sel);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, MM_GLB_REG_ISP_CLK_DIV, clk_div & 0xff);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, MM_GLB_REG_ISP_CLK_DIV_EN, clk_div_en);
    BL_WR_REG(MM_GLB_BASE, MM_GLB_ISP_DP_CLK, tmpVal);
}

void ISP_Enable(void)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(MM_GLB_BASE, MM_GLB_SW_RESET_ISP_SUB);
    tmpVal = BL_CLR_REG_BIT(tmpVal, MM_GLB_SWRST_ISP_MAIN);
    BL_WR_REG(MM_GLB_BASE, MM_GLB_SW_RESET_ISP_SUB, tmpVal);
}

void ISP_Disable(void)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(MM_GLB_BASE, MM_GLB_SW_RESET_ISP_SUB);
    tmpVal = BL_SET_REG_BIT(tmpVal, MM_GLB_SWRST_ISP_MAIN);
    BL_WR_REG(MM_GLB_BASE, MM_GLB_SW_RESET_ISP_SUB, tmpVal);
}



