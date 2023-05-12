#ifndef __IMG_STRUCT_H__
#define __IMG_STRUCT_H__

#include <bl808.h>
#include <bl808_isp.h>
#include "bl_comm_sns.h"

#define SENSOR_NAME_STRING_LENGTH           (16)
#define MAX_ENTRY_NUM_AE_ISO_AUTO           (10)
#define MAX_ENTRY_NUM_AE_SYS_GAIN_AUTO      (3)
#define MAX_ENTRY_NUM_AWB_AUTO_PREF         (19)
#define MAX_ENTRY_NUM_AWB_CT_AUTO           (7)
#define MAX_ENTRY_NUM_LSC                   (2)
#define MAX_ENTRY_NUM_GAMMA_CORR            (2)
#define MAX_ENTRY_NUM_COLOR_CORR            (5)
#define MAX_ENTRY_NUM_AWB_WHITE_REGION      (6)
#define MAX_ENTRY_NUM_EE_LUMA_WEIGHT        (32)
#define VERT_NUM                            (6)

typedef enum {
    ISO_AUTO          = 0,
    ISO_100_0DB       = 0,  // 0 DB
    ISO_200_6DB       = 6,  // 6 DB
    ISO_400_12DB      = 12, // 12DB
    ISO_800_18DB      = 18, // 18DB
    ISO_1600_24DB     = 24, // 24DB
    ISO_3200_30DB     = 30, // 30DB
    ISO_6400_36DB     = 36, // 36DB
    ISO_12800_42DB    = 42, // 42DB
    ISO_25600_48DB    = 48, // 48DB
    ISO_51200_54DB    = 54, // 54DB
    ISO_102400_60DB   = 60, // 60DB
    ISO_204800_66DB   = 66, // 66DB
    ISO_409600_72DB   = 72, // 72DB
    ISO_CUSTOM
}ae_iso_mode_t;

typedef enum {
    WB_AUTO,
    WB_H_2300K     = 2300,
    WB_A_2800K     = 2800,
    WB_TL84_4000K  = 4000,
    WB_CWF_4150K   = 4150,
    WB_D50_5000K   = 5000,
    WB_D65_6500K   = 6500,
    WB_D75_7500K   = 7500,
    WB_CUSTOM
}awb_color_temper_mode_t;

typedef enum {
    ISP_ADJ_AE = 0,
    ISP_ADJ_AE_TARGET,
    ISP_ADJ_AWB,
    ISP_ADJ_AWB_GAIN,
    ISP_ADJ_AWB3_CAL,
    ISP_ADJ_AWB_BIAS,
    ISP_ADJ_AWB_REGION,
    ISP_ADJ_BLC,
    ISP_ADJ_DPC,
    ISP_ADJ_BNR,
    ISP_ADJ_LSC,
    ISP_ADJ_CCM,
    ISP_ADJ_GAMMA,
    ISP_ADJ_WDR,
    ISP_ADJ_SATURATION,
    ISP_ADJ_NR,
    ISP_ADJ_EE,
    ISP_ADJ_CS,
    ISP_ADJ_PARAM_NUM
}img_param_module_t;

typedef enum {
    ISP_POL_INIT = 0,
    ISP_POL_INTERPL,
    ISP_POL_NUM
}img_param_policy_t;

typedef enum {
    ISP_TBL_AUTO_ISO = 0,
    ISP_TBL_AUTO_COLOR_TMPER,
    ISP_TBL_CUSTOM_ISO,
    ISP_TBL_CUSTOM_COLOR_TMPER,
    ISP_TBL_CUSTOM_EV,
    ISP_TBL_INIT,
}img_param_table_t;


// tips for IQ tunner
typedef struct  {
    img_param_module_t module;
    img_param_policy_t policy;
    img_param_table_t  table;
} img_param_header_t;


typedef struct  {
    BL_Fun_Type state[ISP_ADJ_PARAM_NUM];
} img_param_state_container_t;

/*********************************************************/
/****************** AE PARAM TYPE ***********************/
typedef struct {
    uint32_t ae_target;
} ae_param_sw_t;

typedef struct {
    ae_param_sw_t container[MAX_ENTRY_NUM_AE_ISO_AUTO];
} ae_container_sw_t;

typedef struct {
    uint8_t speed_level;
} ae_auto_param_fw_t;

typedef struct {
    uint32_t ae_target;
} ae_param_fw_t;

typedef struct {
    img_param_header_t header;
    ae_param_fw_t ae_fw[MAX_ENTRY_NUM_AE_ISO_AUTO];
    ae_auto_param_fw_t ae_auto_fw;
} ae_fw_container_t;

/*********************************************************/
/****************** AWB PARAM TYPE ***********************/
typedef struct {
    ISP_RB_Gain_Type bais[3];
} awb_bais_t;


typedef struct {
    awb_bais_t preference_gain[MAX_ENTRY_NUM_AWB_AUTO_PREF];
    uint8_t speed_level;
} awb_auto_param_fw_t;

typedef struct {
    img_param_header_t header;
    awb_auto_param_fw_t awb_auto_fw;
} awb_fw_container_t;

// AWB Gain
typedef struct blWB_INFO_S {
    ISP_RB_Gain_Type rb_gain;
    BL_COLOR_TEMP color_temp;
} wb_info_t;

typedef struct {
    img_param_header_t header;
    int num_wb_info;
    const wb_info_t *wb_info;
} wb_info_container_t;

typedef struct blWB3_cal_bin_S {
    int32_t bin[ISP_AWB3_CAL_BIN_NUM];
} wb3_cal_bin_t;

typedef struct {
    img_param_header_t header;
    const wb3_cal_bin_t wb3_cal;
} wb3_cal_container_t;

// AWB Bais
typedef struct blWB_BIAS_S {
    ISP_RB_Gain_Type rb_gain;
    int sys_gain_db;
} wb_bais_t;

typedef struct {
    img_param_header_t header;
    int num_wb_bias;
    const wb_bais_t *wb_bias;
} wb_bias_container_t;

// AWB White Region
typedef struct blVERTEX_S {
    int vertx;
    int verty;
} vertex_t;

typedef struct {
    union {
        uint32_t    data[VERT_NUM * 2];
        vertex_t    vert_array[VERT_NUM];
    } hexagon;
    int             weight;
    BL_COLOR_TEMP   color_temp;
} white_region_t;

typedef struct {
    img_param_header_t header;
    int num_white_region;
    const white_region_t *white_region;
} white_region_container_t;

/*********************************************************/
/****************** BLC PARAM TYPE ***********************/

typedef struct {
    uint32_t black_level;
} blc_param_sw_t;

typedef struct {
    blc_param_sw_t container[MAX_ENTRY_NUM_AE_ISO_AUTO];
} blc_container_sw_t;

typedef struct {
    uint32_t black_level;
} black_lvl_corr_t;

typedef struct {
    img_param_header_t header;
    black_lvl_corr_t black_lvl_corr[MAX_ENTRY_NUM_AE_ISO_AUTO];
} black_lvl_corr_container_t;

/*********************************************************/
/****************** DPC PARAM TYPE ***********************/

typedef struct {
    uint8_t strength;
} dpc_param_sw_t;

typedef struct {
    dpc_param_sw_t container[MAX_ENTRY_NUM_AE_ISO_AUTO];
} dpc_container_sw_t;

typedef struct {
    uint8_t defect_THR_1;
    uint8_t defect_THR_2;
} defect_corr_t;

typedef struct {
    img_param_header_t header;
    defect_corr_t defect_corr[MAX_ENTRY_NUM_AE_ISO_AUTO];
} defect_corr_container_t;

typedef struct {
    int strength_tbl[2];
    int thr1_tbl[2];
    int thr2_tbl[2];
} dpc_trans_tbl_t;

/*********************************************************/

/*********************************************************/
/****************** BNR PARAM TYPE ***********************/

typedef struct {
    uint16_t strength;
} bnr_param_sw_t;

typedef struct {
    bnr_param_sw_t container[MAX_ENTRY_NUM_AE_ISO_AUTO];
} bnr_container_sw_t;

typedef struct {
    uint16_t strength;
    uint16_t bnr_weight;
    uint16_t max_bnr_weight;
    uint16_t lpf_weight;
    uint16_t th_base;
    uint16_t th_slope_adj;
} bnr_fw_config_t;

typedef struct {
    img_param_header_t header;
    bnr_fw_config_t bnr_fw_config[MAX_ENTRY_NUM_AE_ISO_AUTO];
} bnr_fw_config_container_t;

/*********************************************************/

/*********************************************************/
/****************** LSC PARAM TYPE ***********************/

typedef struct {
    int16_t coef[LSC_COEF_NUM];
} lsc_coef_t;

typedef struct {
    uint16_t max_r;
    uint16_t max_g;
    uint16_t max_b;
} lsc_max_t;

typedef struct {
    int strength;
    BL_COLOR_TEMP color_temp;
} lsc_param_sw_t;

typedef struct {
    lsc_param_sw_t container[MAX_ENTRY_NUM_AWB_CT_AUTO];
} lsc_container_sw_t;

typedef struct blLSC_S {
    int strength;
    lsc_coef_t lsc_coef_r;
    lsc_coef_t lsc_coef_g;
    lsc_coef_t lsc_coef_b;
    lsc_max_t lsc_max;
    BL_COLOR_TEMP color_temp;
} lens_shade_corr_t;

typedef struct {
    img_param_header_t header;
    int num_lsc;
    const lens_shade_corr_t *lens_shading_corr;
} lens_shade_corr_container_t;

/****************** CCM PARAM TYPE ***********************/

typedef struct {
    int16_t coef[CCM_COEF_NUM];
    BL_COLOR_TEMP color_temp;
} ccm_param_sw_t;

typedef struct {
    ccm_param_sw_t container[MAX_ENTRY_NUM_AWB_CT_AUTO];
} ccm_container_sw_t;

typedef struct {
    int16_t ccm_coef[CCM_COEF_NUM];
} ccm_t;

typedef struct {
    ccm_t ccm;
    BL_COLOR_TEMP color_temp;
} color_corr_t;

typedef struct {
    img_param_header_t header;
    int num_color_corr;
    const color_corr_t *color_corr;
} color_corr_container_t;

/*********************************************************/
/****************** GAMMA PARAM TYPE *********************/

typedef struct {
    int16_t coef[GAMMA_COEF_NUM];
    int sys_gain_db;
} gamma_curve_sw_t;

typedef struct {
    gamma_curve_sw_t container[MAX_ENTRY_NUM_AE_SYS_GAIN_AUTO];
} gamma_container_sw_t;

typedef struct {
    int16_t coef[GAMMA_COEF_NUM];
} gamma_curve_t;

typedef struct {
    gamma_curve_t gamma_curve;
    int sys_gain_db;
} gamma_corr_t;

typedef struct {
    img_param_header_t header;
    int num_gamma_corr;
    const gamma_corr_t *gamma_corr;
} gamma_corr_container_t;

/*********************************************************/
/******************* WDR PARAM TYPE **********************/

typedef struct {
    int cs_strength;
    int curve_weight;
    int enh_weight;
    int sys_gain_db;
} wdr_param_sw_t;

typedef struct {
    wdr_param_sw_t container[MAX_ENTRY_NUM_AE_SYS_GAIN_AUTO];
} wdr_container_sw_t;

typedef struct {
    int cs_weight;
    int cs_gain_thresh;
    int curve_weight;
    int enh_weight;
    int enh_c_gain_extra_slope;
} wdr_param_t;

typedef struct {
    wdr_param_t param;
    int sys_gain_db;
} wdr_fw_config_t;

typedef struct {
    img_param_header_t header;
    int num_wdr_fw_config;
    const wdr_fw_config_t *wdr_fw_config;;
} wdr_fw_config_container_t;

typedef struct {
    int cs_strength_tbl[2];
    int cs_weight_tbl[2];
    int cs_gain_thresh_tbl[2];
} wdr_trans_tbl_t;

/*********************************************************/
/**************** SATURATION PARAM TYPE ******************/

typedef struct {
    uint8_t value;
} sat_param_sw_t;

typedef struct {
    sat_param_sw_t container[MAX_ENTRY_NUM_AE_ISO_AUTO];
} sat_container_sw_t;

typedef struct blSATURATION_S {
    uint8_t value;
} saturation_t;

typedef struct {
    img_param_header_t header;
    saturation_t saturation[MAX_ENTRY_NUM_AE_ISO_AUTO];
} saturation_container_t;

/*********************************************************/
/******************** NR PARAM TYPE **********************/

typedef struct {
    uint16_t strength_level;
    uint8_t  strength_2d;
    uint8_t  strength_3d;
} nr_param_sw_t;

typedef struct {
    nr_param_sw_t container[MAX_ENTRY_NUM_AE_ISO_AUTO];
} nr_container_sw_t;

typedef struct _noise_reduction {
    uint16_t noise_level;
    uint8_t  filt_str_2d;
    uint8_t  filt_str_3d;
} noise_reduction_t;

typedef struct {
    img_param_header_t header;
    noise_reduction_t noise_reduction[MAX_ENTRY_NUM_AE_ISO_AUTO];
} noise_reduct_container_t;

/*********************************************************/
/******************** EE PARAM TYPE **********************/

typedef struct {
    uint8_t pos_edge_strength;
    uint8_t neg_edge_strength;
    uint8_t pos_edge_ovrsht_strength;
    uint8_t neg_edge_ovrsht_strength;
} ee_param_sw_t;

typedef struct {
    ee_param_sw_t container[MAX_ENTRY_NUM_AE_ISO_AUTO];
} ee_container_sw_t;

typedef struct {
    uint8_t posEdgeStr;
    uint8_t negEdgeStr;
    uint8_t posEdgeOvrshtStr;
    uint8_t negEdgeOvrshtStr;
    uint8_t nrStr;
    uint8_t dirShpnessThresh;
    uint8_t noDirShpnessThresh;
    uint8_t flatThresh;
    uint16_t textureThresh;
    uint16_t edgeThresh;
} edge_enhance_t;

typedef struct {
    uint16_t lumaWgtTbl[MAX_ENTRY_NUM_EE_LUMA_WEIGHT];
} ee_luma_wgt_t;

typedef struct {
    img_param_header_t header;
    edge_enhance_t edgeEnhance[MAX_ENTRY_NUM_AE_ISO_AUTO];
    ee_luma_wgt_t lumaWgtTbl;
} edge_enhance_container_t;

/*********************************************************/
/******************** CS PARAM TYPE **********************/
typedef struct {
    uint8_t strength;
} cs_param_sw_t;

typedef struct {
    cs_param_sw_t container[MAX_ENTRY_NUM_AE_ISO_AUTO];
} cs_container_sw_t;

typedef struct {
    int weight;
    int gain_thresh;
} chroma_suppress_t;

typedef struct {
    img_param_header_t header;
    chroma_suppress_t csCfg[MAX_ENTRY_NUM_AE_ISO_AUTO];
} chroma_suppress_container_t;

typedef struct {
    int strength_tbl[2];
    int weight_tbl[2];
    int gain_thresh_tbl[2];
} cs_trans_tbl_t;

/*********************************************************/
/************ IMG PARAM DESCRIPTION **********************/
typedef struct {
    const char name[16];
    const img_param_state_container_t   state_tbl;
    const ae_container_sw_t             ae_tbl;
    const blc_container_sw_t            blc_tbl;
    const dpc_container_sw_t            dpc_tbl;
    const bnr_container_sw_t            bnr_tbl;
    const lsc_container_sw_t            lsc_tbl;
    const ccm_container_sw_t            ccm_tbl;
    //const gamma_container_sw_t          gamma_tbl;
    const wdr_container_sw_t            wdr_tbl;
    const sat_container_sw_t            sat_tbl;
    const nr_container_sw_t             nr_tbl;
    const ee_container_sw_t             ee_tbl;
    const cs_container_sw_t             cs_tbl;
} BL_IMG_PARAM_SW_BIN_DESC_T;

typedef struct {
    const char *name;
    const img_param_state_container_t   *state_tbl;
    const ae_container_sw_t             *ae_tbl;
    const blc_container_sw_t            *blc_tbl;
    const dpc_container_sw_t            *dpc_tbl;
    const bnr_container_sw_t            *bnr_tbl;
    const lsc_container_sw_t            *lsc_tbl;
    const ccm_container_sw_t            *ccm_tbl;
    const gamma_container_sw_t          *gamma_tbl;
    const wdr_container_sw_t            *wdr_tbl;
    const sat_container_sw_t            *sat_tbl;
    const nr_container_sw_t             *nr_tbl;
    const ee_container_sw_t             *ee_tbl;
    const cs_container_sw_t             *cs_tbl;
} BL_IMG_PARAM_SW_DESC_T;

typedef struct {
    char name[16];
    wb_info_container_t         wb_info_tbl;
    wb3_cal_container_t         wb3_cal_tbl;
    black_lvl_corr_container_t  blc_tbl;
    defect_corr_container_t     defect_corr_tbl;
    lens_shade_corr_container_t lens_shading_corr_tbl;
    color_corr_container_t      color_corr_tbl;
    gamma_corr_container_t      gamma_corr_tbl;
} BL_IMG_PARAM_CALIB_BIN_DESC_T;

typedef struct {
    char name[16];
    ae_fw_container_t           ae_tbl;
    awb_fw_container_t          awb_tbl;
} BL_IMG_PARAM_AUTO_BIN_DESC_T;

typedef struct {
    char name[16];
    ae_auto_param_fw_t           ae_auto_tbl;
    awb_auto_param_fw_t          awb_auto_tbl;
} BL_IMG_PARAM_LOAD_AUTO_BIN_DESC_T;

typedef struct {
    const char* name;
    const img_param_state_container_t *state_tbl;
    const ae_fw_container_t           *ae_tbl;
    const awb_fw_container_t          *awb_tbl;
    const wb_info_container_t         *wb_info_tbl;
    const wb3_cal_container_t         *wb3_cal_tbl;
    const wb_bias_container_t         *wb_bias_tbl;
    const white_region_container_t    *awb_white_region_tbl;
    const black_lvl_corr_container_t  *blc_tbl;
    const dpc_trans_tbl_t             *dpc_trans_tbl;
    const defect_corr_container_t     *defect_corr_tbl;
    const bnr_fw_config_container_t   *bnr_fw_config_tbl;
    const lens_shade_corr_container_t *lens_shading_corr_tbl;
    const color_corr_container_t      *color_corr_tbl;
    const gamma_corr_container_t      *gamma_corr_tbl;
    const wdr_trans_tbl_t             *wdr_trans_tbl;
    const wdr_fw_config_container_t   *wdr_fw_config_tbl;
    const saturation_container_t      *saturation_tbl;
    const noise_reduct_container_t    *noise_reduction_tbl;
    const edge_enhance_container_t    *edge_enhance_tbl;
    const cs_trans_tbl_t              *cs_trans_tbl;
    const chroma_suppress_container_t *cs_tbl;
} BL_IMG_PARAM_DESC_T;


#define ATTR_IMG_PARAM_DESC_TABLE __attribute__((section(".img_param_desc")))  __attribute__((used))


#endif /* __IMG_STRUCT_H__ */

