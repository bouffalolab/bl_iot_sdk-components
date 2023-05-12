#ifndef __BL808_COMM_MM_H__
#define __BL808_COMM_MM_H__

#include <bl808_common.h>

/*----------------------------------------------*
 * const defination                             *
 *----------------------------------------------*/

#define BL80X_HW_A0                 1
#define BL80X_HW_B0                 2
#define BL80X_HW_B0_NEW_ARCH        3

#define BL80X_HW_VER                BL80X_HW_B0
#define ENCODE_1080P_SVGA           0
#define USE_AEAWB_SRAM              1
#define INT_DIVIDE_WITH_FPU         0
#define PRINT_FAST_CAP_TIMING       0
#define FAST_CAPTURE_MODE           0
#define BL_LOGD(a,...)              ((void)0)
#define BL_LOGW(a,...)              ((void)0)
#define BL_LOGE(a,...)              ((void)0)
#define BL_LOGI(a,...)              ((void)0)
#define BL_ABORT()                  ((void)0)
#define ISP_AWB3_BASE               ((uint32_t)0x30016000)            /*!< AWB3 base address */
#define AE_SRAM_BASE                ((uint32_t)0x30019000)            /*!< AE stats SRAM base address */
#define AWB_SRAM_BASE               ((uint32_t)0x3001C000)            /*!< AWB stats SRAM base address */

#define BL_SET_REG_BITS_VAL_2(val,bitname,bitval)   \
    ( ((val)&bitname##_UMSK) | (((uint32_t)(bitval)<<bitname##_POS) & bitname##_MSK))

#define CHECK_PARAM_RANGE(value, min, max) \
    CHECK_PARAM((value) >= min && (value) <= max)
#define CHECK_PARAM_MAX(value, max) \
    CHECK_PARAM((value) <= max)
#define CHECK_PARAM_MIN(value, min) \
    CHECK_PARAM((value) >= min)

#define ISP_INVALID_ADDR        (0xDEADBEAF)
#define ENC_END_FRAME_ADDR      (0xDEADBEAF)

#define INTR_MASK(x)        (!x)

#define CCM_COEF_NUM        (9)
#define CCM_OFFSET_NUM      (3)

#define GAMMA_COEF_NUM      (256)
#define RGB_GAIN_1X         (4096)

#ifndef TRUE
#define TRUE  (1)
#endif
#ifndef FALSE
#define FALSE (0)
#endif

/***************************** Value manipulation ************************************/

/* To avoid divide-0 exception in code. */
#define DIV_0_TO_1(a)   ( (0 == (a)) ? 1 : (a) )

/* Divide operation */
#if INT_DIVIDE_WITH_FPU
#define DIV_ROUND(divident, divider)    (int)(((float)(divident) / (divider)) + (float)0.5)
#else
#define DIV_ROUND(divident, divider)    (((divident)+((divider)>>1)) / (divider))
#endif
#define DIV_ROUNDUP(divident, divider)    (((divident)+((divider)-1)) / (divider))
#define DIV_ROUND_SIGNED(divident, divider) \
    ((((divident) < 0) ^ ((divider) < 0)) ? \
    (((divident) - (divider)/2)/(divider)) : (((divident) + (divider)/2)/(divider))) \

#define FLOAT_TO_INT(x)         ((int)((float)(x) + 0.5))

#define SIGN_EXTEND(x, num_bits)    ((((int)(x)) & ((1 << num_bits) - 1)) << (32 - num_bits) >> (32 - num_bits))

#define ALIGN_TO_AXI_BURST_INCR16(x)    ((x) & 0xFFFFFF80)
#define AXI_BURST_INCR16_ALIGNMENT_MASK     ( 0x7F )
#define ROUNDUP_TO_AXI_BURST_INCR16(x)    (((x) + AXI_BURST_INCR16_ALIGNMENT_MASK)  & 0xFFFFFF80)
#define ROUNDUP_TO_16(x)    (((x) + 0xF)  & 0xFFFFFFF0)
#define ROUNDUP_TO_8(x)    (((x) + 0x7)  & 0xFFFFFFF8)
#define BLK_CNT_16X16(x)    (ROUNDUP_TO_16(x) >> 4)
#define BLK_CNT_8X8(x)    (ROUNDUP_TO_8(x) >> 3)
#define IS_MULTIPULE_OF_16(x)    (((x) & 0xF) == 0)
#define IS_MULTIPULE_OF_8(x)    (((x) & 0x7) == 0)
#define IS_MB_ALIGNED(x)    (((x) & 0xF) == 0)

#if (BL80X_HW_VER <= BL80X_HW_A0)
#define ENC_HEIGHT(x)    (ROUNDUP_TO_16(x))
#else
#define ENC_HEIGHT(x)    (x)
#endif

/***************************** Type defines ************************************/
typedef int BL_GAIN_DB;                            /* gain value in dB */

//static const char* bayer_pattern_str_arr[] = {"RG", "GR", "BG", "GB"};

typedef enum {
    BAYER_PATTERN_RG = 0,
    BAYER_PATTERN_GR,
    BAYER_PATTERN_BG,
    BAYER_PATTERN_GB,
} BAYER_PATTERN_E;

typedef enum {
    FLIP_MIRROR_TYPE_NORMAL = 0,
    FLIP_MIRROR_TYPE_MIRROR,
    FLIP_MIRROR_TYPE_FLIP,
    FLIP_MIRROR_TYPE_FLIP_MIRROR,
} FLIP_MIRROR_TYPE_E;

typedef enum {
    LSC_COEF_A = 0,
    LSC_COEF_B = 1,
    LSC_COEF_C = 2,
    LSC_COEF_D = 3,
    LSC_COEF_E = 4,
    LSC_COEF_G = 5,
    LSC_COEF_NUM = 6,
} ISP_LSC_COEF_E;

typedef enum {
    INTR_DISABLE  = 0,
    INTR_ENABLE   = 1,
} ISP_INTR_MASK_E;

typedef enum {
    AXI_XLEN_SINGLE = 0,
    AXI_XLEN_INCR4 = 1,
    AXI_XLEN_INCR8 = 2,
    AXI_XLEN_INCR16 = 3,
    AXI_XLEN_INCR32 = 4,
} AXI_XLEN_E;

typedef struct {
    uint32_t width;
    uint32_t height;
} BL_Img_Size_T;

typedef struct blRGB_SUM_S {
    uint32_t r_sum;
    uint32_t g_sum;
    uint32_t b_sum;
} RGB_SUM_S;


/***************************** Functions ************************************/

int Mem_Size_To_Burst_Cnt(uint32_t mem_size, AXI_XLEN_E xlen);
int Burst_Cnt_To_Mem_Size(uint32_t mem_size, AXI_XLEN_E xlen);

#endif /* __BL808_MM_COMM_H__ */

