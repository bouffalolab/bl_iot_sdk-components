#ifndef __IMG_PARAM_GC2053_H__
#define __IMG_PARAM_GC2053_H__

#include <stdint.h>
#include "img_struct.h"

static const img_param_state_container_t state_tbl = {
    .state[ISP_ADJ_AE]                = ENABLE,
    .state[ISP_ADJ_AWB]               = ENABLE,
    .state[ISP_ADJ_BLC]               = ENABLE,
    .state[ISP_ADJ_DPC]               = ENABLE,//DISABLE,
    .state[ISP_ADJ_BNR]               = ENABLE,
    .state[ISP_ADJ_LSC]               = ENABLE,//DISABLE,
    .state[ISP_ADJ_CCM]               = ENABLE,
    .state[ISP_ADJ_GAMMA]             = ENABLE,
    .state[ISP_ADJ_WDR]               = ENABLE,
    .state[ISP_ADJ_SATURATION]        = ENABLE,
    .state[ISP_ADJ_NR]                = ENABLE,
    .state[ISP_ADJ_EE]                = ENABLE,
    .state[ISP_ADJ_CS]                = ENABLE,
};

#define STREAMING 1

static const ae_fw_container_t ae_fw = {
    .header = {
        .module = ISP_ADJ_AE_TARGET,
        .policy = ISP_POL_INTERPL,
        .table  = ISP_TBL_AUTO_ISO,
    },
    /* ae target */
    .ae_fw = {
        {   50,   },      // ISO_100_0DB
        {   50,   },      // ISO_200_6DB
        {   50,   },      // ISO_400_12DB
        {   50,   },      // ISO_800_18DB
        {   50,   },      // ISO_1600_24DB
        {   50,   },      // ISO_3200_30DB
        {   50,   },      // ISO_6400_36DB
        {   50,   },      // ISO_12800_42DB
        {   50,   },      // ISO_25600_48DB
        {   50,   },      // ISO_51200_54DB
    },
    .ae_auto_fw = {5,}
};

static const awb_fw_container_t awb_fw_use = {
    .header = {
        .module = ISP_ADJ_AWB,
        .policy = ISP_POL_INTERPL,
        .table  = ISP_TBL_CUSTOM_COLOR_TMPER,
    },
    .awb_auto_fw = {
        .preference_gain = {
        {{ {4096,4096}, {4096,4096}, {4096,4096} },},  //[0]:-54 db
        {{ {4096,4096}, {4096,4096}, {4096,4096} },},  //[1]:-48 db
        {{ {4096,4096}, {4096,4096}, {4096,4096} },},  //[2]:-42 db
        {{ {4096,4096}, {4096,4096}, {4096,4096} },},  //[3]:-36 db
        {{ {4096,4096}, {4096,4096}, {4096,4096} },},  //[4]:-30 db
        {{ {4096,4096}, {4096,4096}, {4096,4096} },},  //[5]:-24 db
        {{ {4096,4096}, {4096,4096}, {4096,4096} },},  //[6]:-18 db
        {{ {4096,4096}, {4096,4096}, {4096,4096} },},  //[7]:-12 db
        {{ {4096,4096}, {4096,4096}, {4096,4096} },},  //[8]:-6 db
        {{ {4096,4096}, {4096,4096}, {4096,4096} },},  //[9]:0 db
        {{ {4096,4096}, {4096,4096}, {4096,4096} },},  //[10]:6 db
        {{ {4096,4096}, {4096,4096}, {4096,4096} },},  //[11]:12 db
        {{ {4096,4096}, {4096,4096}, {4096,4096} },},  //[12]:18 db
        {{ {4096,4096}, {4096,4096}, {4096,4096} },},  //[13]:24 db
        {{ {4096,4096}, {4096,4096}, {4096,4096} },},  //[14]:30 db
        {{ {4096,4096}, {4096,4096}, {4096,4096} },},  //[15]:36 db
        {{ {4096,4096}, {4096,4096}, {4096,4096} },},  //[16]:42 db
        {{ {4096,4096}, {4096,4096}, {4096,4096} },},  //[17]:48 db
        {{ {4096,4096}, {4096,4096}, {4096,4096} },},  //[18]:54 db
        },
        .speed_level = 4,
    },
};

static const wb_info_t wb_gain_map[] = {
    /* r_gain, b_gain, color_temp */
    { {4051, 13177}, 2300 },     // H - 2300K
    { {4797, 11381}, 2856 },     // A - 2856K
    { {5693, 9656},  4000 },     // TL84 - 4000K
    { {6573, 9618},  4150 },     // CWF - 4150K
    { {6666, 7750},  5000 },     // D50 - 5000K
    { {7785, 6482},  6500 },     // D65 - 6500K
    { {8514, 6028},  7500 },     // D75 - 7500K
};

static const wb_info_container_t wb_info_use = {
    .header = {
        .module = ISP_ADJ_AWB_GAIN,
        .policy = ISP_POL_INTERPL,
        .table  = ISP_TBL_CUSTOM_COLOR_TMPER,
    },
    .num_wb_info = sizeof(wb_gain_map)/sizeof(wb_gain_map[0]),
    .wb_info = wb_gain_map,
};

static const wb3_cal_container_t wb3_cal_use = {
    .header = {
        .module = ISP_ADJ_AWB3_CAL,
        .policy = ISP_POL_INIT,
        .table  = ISP_TBL_INIT,
    },
    .wb3_cal = {
        .bin = {
         0, 230, 300, 175, 115, 192, 288,   0,   0,   0,   0,   0,   8,  16,  16,   8,
         0,   0,   0,   0,   8,  48,  96,  48,  16,   0,   0,   0,   8,  96, 192,  64,
        16,   0,  0,   0,   0,  96,  255,  48,   8,   0,   0,   0,   0,  48, 160, 160,
         0,   0,   0,   0,   0,  16,  96,  64,  64,   0,   0,   0,   0,   8,  48,  48,
         0,   0,   0,   0,   0,   0,  16,  48,  16,   0,   0,   0,   0,   0,   0,   8,
        16,   0,   0,   0,   0,   0,   0,   0
        }
    },
};

//AWB_ALGO_1
static const wb_bais_t wb_bias_map[] = {
    /* r_gain, b_gain, sys_gain */
    { {3850, 3900, }, -36 },     //
    { {4096, 4096, }, -3 },     //
};

//AWB_ALGO_1
static const wb_bias_container_t wb_bias = {
    .header = {
        .module = ISP_ADJ_AWB_BIAS,
        .policy = ISP_POL_INTERPL,
        .table  = ISP_TBL_CUSTOM_COLOR_TMPER,
    },
    .num_wb_bias = sizeof(wb_bias_map)/sizeof(wb_bias_map[0]),
    .wb_bias = wb_bias_map,
};

//AWB_ALGO_1
static const white_region_t white_region_map[] = {
    /* H 2300K */
    {
        {{  /* #1,   #2,      #3,       #4,       #5,       #6      */
            833,2401,912,2417,974,2294,1025,2141,923,2138,801,2234
        }},
        .weight             = 1,
        .color_temp         = 2300,
    },
    /* A 2856K */
    {
        {{   /* #1,   #2,      #3,       #4,       #5,       #6      */
            962,2140,1062,2141,1115,1998,1051,1929,993,1932,969,1989
        }},
        .weight             = 1,
        .color_temp         = 2856,
    },
    /* TL84 4000K */
    {
        {{   /* #1,   #2,      #3,       #4,       #5,       #6      */
            1192,1984,1274,2023,1318,1947,1344,1833,1246,1802,1219,1861
        }},
        .weight             = 2,
        .color_temp         = 4000,
    },
    /* CWF 4150K */
    {
        {{   /* #1,   #2,      #3,       #4,       #5,       #6      */
            1328,1969,1390,1997,1405,1935,1433,1859,1370,1856,1349,1911
        }},
        .weight             = 1,
        .color_temp         = 4150,
    },
    /* D50 5000K */
    {
        {{   /* #1,   #2,      #3,       #4,       #5,       #6      */
            1264,1665,1544,1784,1634,1686,1720,1621,1490,1468,1398,1513
        }},
        .weight             = 2,
        .color_temp         = 5000,
    },
    /* D65 6500K */
    {
        {{   /* #1,   #2,      #3,       #4,       #5,       #6      */
            1431,1435,1720,1617,1763,1511,1738,1371,1673,1309,1562,1309
        }},
        .weight             = 8,
        .color_temp         = 6500,
    },
    /* D75 7500K */
    {
        {{   /* #1,   #2,      #3,       #4,       #5,       #6      */
            1627,1327,1709,1400,1838,1358,1834,1296,1688,1253,1640,1292
        }},
        .weight             = 4,
        .color_temp         = 7500,
    },
};

//AWB_ALGO_1
static const white_region_container_t white_region_use = {
    .header = {
        .module = ISP_ADJ_AWB_REGION,
        .policy = ISP_POL_INTERPL,
        .table  = ISP_TBL_CUSTOM_COLOR_TMPER,
    },
    .num_white_region = sizeof(white_region_map)/sizeof(white_region_map[0]),
    .white_region = white_region_map,
};

static const black_lvl_corr_container_t black_lvl_corr = {
    .header = {
        .module = ISP_ADJ_BLC,
        .policy = ISP_POL_INTERPL,
        .table  = ISP_TBL_AUTO_ISO,
    },
    .black_lvl_corr = {
        { 64 << 2, },      // ISO_100_0DB
        { 64 << 2, },      // ISO_200_6DB
        { 64 << 2, },      // ISO_400_12DB
        { 64 << 2, },      // ISO_800_18DB
        { 64 << 2, },      // ISO_1600_24DB
        { 64 << 2, },      // ISO_3200_30DB
        { 64 << 2, },      // ISO_6400_36DB
        { 64 << 2, },      // ISO_12800_42DB
        { 64 << 2, },      // ISO_25600_48DB
        { 64 << 2, },      // ISO_51200_54DB
    },
};

static const dpc_trans_tbl_t dpc_trans = {
    .strength_tbl = {    0,  100},
    .thr1_tbl     = {    5,   52},
    .thr2_tbl     = {    1,   17},
};

static const defect_corr_container_t defect_corr = {
    .header = {
        .module = ISP_ADJ_DPC,
        .policy = ISP_POL_INTERPL,
        .table  = ISP_TBL_AUTO_ISO,
    },
    /* thres 1, thres 2 */
    #if STREAMING
    .defect_corr = {
        { 5,    1,    },     // ISO_100_0DB
        { 6,    1,    },     // ISO_200_6DB
        { 8,    2,     },     // ISO_400_12DB
        {11,    4,     },     // ISO_800_18DB
        {18,    7,     },     // ISO_1600_24DB
        {32,    11,    },     // ISO_3200_30DB
        {52,    17,    },     // ISO_6400_36DB
        {52,    17,    },     // ISO_12800_42DB
        {52,    17,    },     // ISO_25600_48DB
        {52,    17,    },     // ISO_51200_54DB
    },
    #else
    .defect_corr = {
        {10,    20,    },     // ISO_100_0DB
        {10,    10,    },     // ISO_200_6DB
        {10,    5,     },     // ISO_400_12DB
        {10,    5,     },     // ISO_800_18DB
        {10,    5,     },     // ISO_1600_24DB
        {30,    20,    },     // ISO_3200_30DB
        {30,    20,    },     // ISO_6400_36DB
        {30,    20,    },     // ISO_12800_42DB
        {30,    20,    },     // ISO_25600_48DB
        {30,    20,    },     // ISO_51200_54DB
    },
    #endif
};

static const bnr_fw_config_container_t bnr_fw_config = {
    .header = {
        .module = ISP_ADJ_BNR,
        .policy = ISP_POL_INIT,
        .table  = ISP_TBL_AUTO_ISO,
    },
    .bnr_fw_config = {
		{    14,    1,   32,   0,     18,  20,	},     // ISO_100_0DB
        {    14,    1,   32,   0,     21,  20,	},     // ISO_200_6DB
        {    14,    1,   32,   0,     27,  20,	},     // ISO_400_12DB
        {    14,    2,   32,   0,     40,  20,	},     // ISO_800_18DB
        {    14,    4,   32,   0,     65,  20,	},     // ISO_1600_24DB
        {    14,    7,   32,   0,     115, 20, 	},     // ISO_3200_30DB
        {    14,    13,	 32,   1,     215, 20,  },     // ISO_6400_36DB
        {    14,    26,	 32,   2,     256, 20,  },     // ISO_12800_42DB
        {    14,    51,	 32,   3,     256, 20,  },     // ISO_25600_48DB
        {    14,    101, 32,   6,     256, 20,  },     // ISO_51200_54DB
    },
};

static const lens_shade_corr_t lsc_map[] = {
    /*strength*/
    /* R: coefA, coefB, coefC, coefD, coefE, coefG */
    /* G: coefA, coefB, coefC, coefD, coefE, coefG */
    /* B: coefA, coefB, coefC, coefD, coefE, coefG */
    /* max_R, max_G, max_B */
    {
        100,
        {{ -125, 231,  77,-124, 120,   0 }},      // R
        {{ -152, 281, 101,-150, 147,   0 }},      // G
        {{  -50,  93,  35, -49,  47,   0 }},      // B
        {  110, 142,  48 },      // max
        3100,
    },
    {
        100,
        {{  -56, 104,  36, -56,  54,   0 }},      // R
        {{ -111, 205,  74,-111, 108,   0 }},      // G
        {{  -61, 112,  43, -60,  57,   1 }},      // B
        {   51, 105,  59 },      // max
        6500,
    },
};

static const lens_shade_corr_container_t lens_shading_corr = {
    .header = {
        .module = ISP_ADJ_LSC,
        .policy = ISP_POL_INTERPL,
        .table  = ISP_TBL_CUSTOM_COLOR_TMPER,
    },
    .num_lsc = sizeof(lsc_map)/sizeof(lsc_map[0]),
    .lens_shading_corr = lsc_map,
};
//20210519_1456
static const color_corr_t color_corr_map[] = {
    { {{ 816, -270, -34, -249, 703, 57, -74, -827, 1414  }}, 2300 },    // h - 2300K
    { {{ 835, -320, -3, -204, 720, -4, -15, -575, 1103   }}, 2856 },    // A - 2856K
    { {{ 847, -331, -4, -167, 707, -27, -20, -316, 848   }}, 4000 },    // TL84 - 4000K
    { {{ 1072, -583, 22, -184, 701, -4, 11, -362, 863    }}, 4150 },    // CWF - 4150K
    { {{ 962, -438, -12, -145, 770, -112, 1, -336, 846   }}, 5000 },    // D50 - 5000K
    { {{ 915, -431, 27, -103, 705, -89, 13, -246, 745    }}, 6500 },    // D65 - 6500K
    { {{ 1015, -482, -20, -116, 761, -133, 4, -276, 783  }}, 7500 },    // D75 - 7500K
};

static const color_corr_container_t color_corr = {
    .header = {
        .module = ISP_ADJ_CCM,
        .policy = ISP_POL_INTERPL,
        .table  = ISP_TBL_CUSTOM_COLOR_TMPER,
    },
    .num_color_corr = sizeof(color_corr_map)/sizeof(color_corr_map[0]),
    .color_corr = color_corr_map,
};

static const gamma_corr_t gamma_corr_map[] = {
    {
        {{
        0, 58, 115, 172, 228, 284, 339, 393, 447, 500, 553, 605, 656, 707, 757, 807,
        856, 904, 952, 999, 1045, 1091, 1137, 1182, 1226, 1269, 1312, 1355, 1397, 1438, 1479, 1519,
        1559, 1598, 1636, 1674, 1711, 1748, 1784, 1820, 1855, 1889, 1923, 1957, 1990, 2022, 2054, 2085,
        2116, 2146, 2175, 2204, 2233, 2261, 2288, 2315, 2342, 2368, 2393, 2418, 2442, 2466, 2489, 2512,
        2534, 2556, 2577, 2598, 2618, 2638, 2657, 2676, 2695, 2713, 2730, 2748, 2765, 2781, 2797, 2813,
        2829, 2844, 2859, 2874, 2888, 2902, 2916, 2930, 2943, 2957, 2970, 2983, 2995, 3008, 3020, 3032,
        3044, 3056, 3068, 3080, 3092, 3103, 3115, 3126, 3137, 3149, 3160, 3171, 3181, 3192, 3203, 3213,
        3224, 3234, 3244, 3255, 3265, 3275, 3285, 3295, 3305, 3314, 3324, 3333, 3343, 3352, 3362, 3371,
        3380, 3390, 3399, 3408, 3417, 3426, 3435, 3443, 3452, 3461, 3469, 3478, 3486, 3494, 3503, 3511,
        3519, 3527, 3535, 3543, 3550, 3558, 3566, 3573, 3580, 3588, 3595, 3602, 3609, 3616, 3623, 3629,
        3636, 3642, 3649, 3655, 3661, 3667, 3673, 3679, 3685, 3691, 3696, 3702, 3708, 3713, 3719, 3724,
        3729, 3735, 3740, 3745, 3750, 3756, 3761, 3766, 3771, 3776, 3781, 3786, 3791, 3796, 3801, 3806,
        3811, 3816, 3821, 3827, 3832, 3837, 3842, 3847, 3852, 3857, 3862, 3867, 3872, 3877, 3882, 3887,
        3892, 3897, 3902, 3907, 3912, 3916, 3921, 3926, 3931, 3936, 3940, 3945, 3950, 3954, 3959, 3963,
        3968, 3972, 3977, 3981, 3986, 3990, 3994, 3998, 4002, 4006, 4010, 4014, 4018, 4022, 4026, 4030,
        4033, 4037, 4040, 4044, 4047, 4051, 4054, 4057, 4060, 4063, 4066, 4069, 4072, 4074, 4077, 4095,
        }},
        -3,
    },
    {
        {{
        0, 58, 115, 172, 228, 284, 339, 393, 447, 500, 553, 605, 656, 707, 757, 807,
        856, 904, 952, 999, 1045, 1091, 1137, 1182, 1226, 1269, 1312, 1355, 1397, 1438, 1479, 1519,
        1559, 1598, 1636, 1674, 1711, 1748, 1784, 1820, 1855, 1889, 1923, 1957, 1990, 2022, 2054, 2085,
        2116, 2146, 2175, 2204, 2233, 2261, 2288, 2315, 2342, 2368, 2393, 2418, 2442, 2466, 2489, 2512,
        2534, 2556, 2577, 2598, 2618, 2638, 2657, 2676, 2695, 2713, 2730, 2748, 2765, 2781, 2797, 2813,
        2829, 2844, 2859, 2874, 2888, 2902, 2916, 2930, 2943, 2957, 2970, 2983, 2995, 3008, 3020, 3032,
        3044, 3056, 3068, 3080, 3092, 3103, 3115, 3126, 3137, 3149, 3160, 3171, 3181, 3192, 3203, 3213,
        3224, 3234, 3244, 3255, 3265, 3275, 3285, 3295, 3305, 3314, 3324, 3333, 3343, 3352, 3362, 3371,
        3380, 3390, 3399, 3408, 3417, 3426, 3435, 3443, 3452, 3461, 3469, 3478, 3486, 3494, 3503, 3511,
        3519, 3527, 3535, 3543, 3550, 3558, 3566, 3573, 3580, 3588, 3595, 3602, 3609, 3616, 3623, 3629,
        3636, 3642, 3649, 3655, 3661, 3667, 3673, 3679, 3685, 3691, 3696, 3702, 3708, 3713, 3719, 3724,
        3729, 3735, 3740, 3745, 3750, 3756, 3761, 3766, 3771, 3776, 3781, 3786, 3791, 3796, 3801, 3806,
        3811, 3816, 3821, 3827, 3832, 3837, 3842, 3847, 3852, 3857, 3862, 3867, 3872, 3877, 3882, 3887,
        3892, 3897, 3902, 3907, 3912, 3916, 3921, 3926, 3931, 3936, 3940, 3945, 3950, 3954, 3959, 3963,
        3968, 3972, 3977, 3981, 3986, 3990, 3994, 3998, 4002, 4006, 4010, 4014, 4018, 4022, 4026, 4030,
        4033, 4037, 4040, 4044, 4047, 4051, 4054, 4057, 4060, 4063, 4066, 4069, 4072, 4074, 4077, 4095,
        }},
        6,
    },
};

static const wdr_trans_tbl_t wdr_trans_use = {
    .cs_strength_tbl     = {    0,  100},
    .cs_weight_tbl       = {    5,   31},
    .cs_gain_thresh_tbl  = {    0,  128},
};

static const gamma_corr_container_t gamma_corr = {
    .header = {
        .module = ISP_ADJ_GAMMA,
        .policy = ISP_POL_INTERPL,
        .table  = ISP_TBL_CUSTOM_EV,
    },
    .num_gamma_corr = sizeof(gamma_corr_map)/sizeof(gamma_corr_map[0]),
    .gamma_corr = gamma_corr_map,
};

static const wdr_fw_config_t wdr_fw_config_map[] = {
    /*cs_weight,cs_gain_thresh,curve_weight,enh_weight,enh_c_gain_extra_slope,sys_gain_db;*/
        {{8,          1,             16,         16,        48},                     -12},
        {{8,          1,             4,          4,         48},                     -6},
        {{8,          1,             0,          0,         48},                     -3},
};

static const wdr_fw_config_container_t wdr_fw_config = {
    .header = {
        .module = ISP_ADJ_WDR,
        .policy = ISP_POL_INTERPL,
        .table  = ISP_TBL_CUSTOM_EV,
    },
    .num_wdr_fw_config = sizeof(wdr_fw_config_map)/sizeof(wdr_fw_config_map[0]),
    .wdr_fw_config = wdr_fw_config_map
};

static const saturation_container_t saturation = {
    .header = {
        .module = ISP_ADJ_SATURATION,
        .policy = ISP_POL_INTERPL,
        .table  = ISP_TBL_AUTO_ISO,
    },
    /* saturation */
    .saturation = {
        { 128,  },    // ISO_100_0DB
        { 128,  },    // ISO_200_6DB
        { 120,  },    // ISO_400_12DB
        { 110,  },    // ISO_800_18DB
        { 105,  },    // ISO_1600_24DB
        { 100,  },    // ISO_3200_30DB
        { 95,   },    // ISO_6400_36DB
        { 90,   },    // ISO_12800_42DB
        { 80,   },    // ISO_25600_48DB
        { 80,   },    // ISO_51200_54DB
    },
};

static const noise_reduct_container_t noise_reduction = {
    .header = {
        .module = ISP_ADJ_NR,
        .policy = ISP_POL_INTERPL,
        .table  = ISP_TBL_AUTO_ISO,
    },
    /* noise_lvl     2d_str       3d_str  */
#if STREAMING
	    {
        {  4,          4,           2,       },    // ISO_100_0DB
        {  6,          4,           2,       },    // ISO_200_6DB
        {  8,          4,           2,       },    // ISO_400_12DB
        { 10,          6,           3,       },    // ISO_800_18DB
        { 12,          8,           5,       },    // ISO_1600_24DB
        { 16,         10,           6,       },    // ISO_3200_30DB
        { 24,         12,           6,       },    // ISO_6400_36DB
        { 31,         14,           6,       },    // ISO_12800_42DB
        { 31,         14,           6,       },    // ISO_25600_48DB
        { 31,         14,           6,       },    // ISO_51200_54DB
    },
#else
    {
        {  4,          4,           2,       },    // ISO_100_0DB
        {  8,          4,           2,       },    // ISO_200_6DB
        { 14,          5,           3,       },    // ISO_400_12DB
        { 16,          6,           4,       },    // ISO_800_18DB
        { 18,          8,           5,       },    // ISO_1600_24DB
        { 20,         10,           6,       },    // ISO_3200_30DB
        { 28,         12,           6,       },    // ISO_6400_36DB
        { 31,         14,           6,       },    // ISO_12800_42DB
        { 31,         14,           6,       },    // ISO_25600_48DB
        { 31,         14,           6,       },    // ISO_51200_54DB
    },
#endif

};

static const edge_enhance_container_t edge_enhance = {
    .header = {
        .module = ISP_ADJ_EE,
        .policy = ISP_POL_INTERPL,
        .table  = ISP_TBL_AUTO_ISO,
    },
    #if STREAMING
            /* posStr:7bit, negStr:7bit, posOvrStr:7bit, negOvrStr:7bit, nrStr:8bit, dirThr:7bit, noDirThr:7bit, flatThr:8bit, txtThr:9bit, edgeThr:16bit */
     .edgeEnhance = {
        { 40,   40,    127,      127,    0,     60,     90,      30,      100,      65535,},    // ISO_100_0DB
        { 40,   40,    127,      127,    0,     60,     90,      30,      100,      65535,},    // ISO_200_6DB
        { 40,   40,    127,      127,    0,     60,     90,      30,      100,      65535,},    // ISO_400_12DB
        { 40,   40,    127,      127,    64,    60,     90,      30,      100,      65535,},    // ISO_800_18DB
        { 40,   40,    127,      127,   128,    60,     90,      40,      100,      65535,},    // ISO_1600_24DB
        { 40,   40,    127,      127,   255,    60,     90,      50,      100,      65535,},    // ISO_3200_30DB
        { 40,   40,    127,      127,   255,    60,     90,      60,      100,      65535,},    // ISO_6400_36DB
        { 40,   40,     64,       64,   255,    60,     90,      60,      100,      65535,},    // ISO_12800_42DB
        { 40,   40,     32,       32,   255,    60,     90,      60,      100,      65535,},    // ISO_25600_48DB
        { 40,   40,     32,       32,   255,    60,     90,      60,      100,      65535,},    // ISO_51200_54DB
        #else
        /* posStr:7bit, negStr:7bit, posOvrStr:7bit, negOvrStr:7bit, nrStr:8bit, dirThr:7bit, noDirThr:7bit, flatThr:8bit, txtThr:9bit, edgeThr:16bit */
     .edgeEnhance = {
        { 40,   40,    127,      127,    0,     60,     90,      50,      400,      65535,},    // ISO_100_0DB
        { 40,   40,    127,      127,    0,     60,     90,      50,      400,      65535,},    // ISO_200_6DB
        { 40,   40,    127,      127,    0,     60,     90,      50,      400,      65535,},    // ISO_400_12DB
        { 30,   30,    127,      127,    64,    60,     90,      50,      400,      65535,},    // ISO_800_18DB
        { 20,   20,    127,      127,   128,    60,     90,      60,      400,      65535,},    // ISO_1600_24DB
        { 15,   15,    127,      127,   255,    60,     90,      70,      400,      65535,},    // ISO_3200_30DB
        { 10,   10,     64,       64,   255,    60,     90,      80,      400,      65535,},    // ISO_6400_36DB
        { 10,   10,     64,       64,   255,    60,     90,      80,      400,      65535,},    // ISO_12800_42DB
        { 10,   10,     32,       32,   255,    60,     90,      80,      400,      65535,},    // ISO_25600_48DB
        { 10,   10,     32,       32,   255,    60,     90,      80,      400,      65535,},    // ISO_51200_54DB
        #endif
},
    /* luma weight table */
    .lumaWgtTbl = {{
        128,144,160,176,192,208,224,240,255,255,255,255,255,255,255,255,
        255,255,255,255,255,255,255,255,240,224,208,192,176,160,144,128,
    }},
};

static const cs_trans_tbl_t cs_trans = {
    .strength_tbl    = {    0,  100},
    .weight_tbl      = {    5,   10},
    .gain_thresh_tbl = {    0,    8},
};

static const chroma_suppress_container_t chroma_suppress = {
    .header = {
        .module = ISP_ADJ_CS,
        .policy = ISP_POL_INTERPL,
        .table  = ISP_TBL_AUTO_ISO,
    },
    .csCfg = {
        {   6,  1,  },// ISO_100_0DB
        {   6,  2,  },// ISO_200_6DB
        {   7,  3,  },// ISO_400_12DB
        {   8,  4,  },// ISO_800_18DB
        {   8,  4,  },// ISO_1600_24DB
        {   8,  5,  },// ISO_3200_30DB
        {   8,  5, },// ISO_6400_36DB
        {   8,  6, },// ISO_12800_42DB
        {   8,  6, },// ISO_25600_48DB
        {   8,  6, },// ISO_51200_54DB
    },
};

#endif
