#include "bl_rtc.h"
#include "bl_timer.h"
#include "blog.h"


uint32_t rc32k_cnt_by_xtal(void)
{
    uint32_t cnt;
    uint32_t tmpVal;
    
    tmpVal = BL_RD_REG(GLB_BASE, GLB_XTAL_DEG_32K);
    tmpVal = BL_SET_REG_BIT(tmpVal, GLB_CLR_XTAL_CNT_32K_DONE);
    BL_WR_REG(GLB_BASE, GLB_XTAL_DEG_32K, tmpVal);
    tmpVal = BL_RD_REG(GLB_BASE, GLB_XTAL_DEG_32K);
    tmpVal = BL_SET_REG_BIT(tmpVal, GLB_XTAL_CNT_32K_SW_TRIG_PS);
    BL_WR_REG(GLB_BASE, GLB_XTAL_DEG_32K, tmpVal);
    
    do{
        tmpVal = BL_RD_REG(PDS_BASE, PDS_XTAL_CNT_32K);
    }while(BL_GET_REG_BITS_VAL(tmpVal, PDS_XTAL_CNT_32K_DONE) == 0);
    
    cnt = BL_GET_REG_BITS_VAL(tmpVal, PDS_RO_XTAL_CNT_32K_CNT);
    cnt <<= 6;
    cnt += BL_GET_REG_BITS_VAL(tmpVal, PDS_RO_XTAL_CNT_32K_RES);
    
    return cnt;
}

void rc32k_cal(void)
{
    uint32_t i, j;
    uint32_t cnt;
    uint32_t tmpVal;
    
    if(HBN_Trim_RC32K() != SUCCESS){
        tmpVal = BL_RD_REG(HBN_BASE, HBN_RC32K_CTRL0);
        tmpVal = BL_SET_REG_BITS_VAL(tmpVal, HBN_RC32K_CAP_SEL, 6);
        BL_WR_REG(HBN_BASE, HBN_RC32K_CTRL0, tmpVal);
    }
    
    tmpVal = BL_RD_REG(HBN_BASE, HBN_RC32K_CTRL1);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, HBN_RC32K_CODE_FR_CAL, 0x200 << 3);
    BL_WR_REG(HBN_BASE, HBN_RC32K_CTRL1, tmpVal);
    
    tmpVal = BL_RD_REG(PDS_BASE, PDS_XTAL_CNT_32K);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, PDS_REG_TOTAL_32K_CYCLE, 4);
    BL_WR_REG(PDS_BASE, PDS_XTAL_CNT_32K, tmpVal);
    
    for(i=0; i<10; i++){
        j = 12 - i;
        
        tmpVal = BL_RD_REG(HBN_BASE, HBN_RC32K_CTRL1);
        tmpVal |= 1 << j;
        BL_WR_REG(HBN_BASE, HBN_RC32K_CTRL1, tmpVal);
        
        cnt = rc32k_cnt_by_xtal();
        
        if(cnt > 62500){
            tmpVal = BL_RD_REG(HBN_BASE, HBN_RC32K_CTRL1);
            tmpVal &= ~(1 << j);
            BL_WR_REG(HBN_BASE, HBN_RC32K_CTRL1, tmpVal);
        }
    }
}

void xtal32k_fix(void)
{
    uint32_t tmpVal;
    
    tmpVal = BL_RD_REG(HBN_BASE, HBN_XTAL32K);
    tmpVal = BL_CLR_REG_BIT(tmpVal, HBN_XTAL32K_HIZ_EN);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, HBN_XTAL32K_INV_STRE, 3);
    tmpVal = BL_CLR_REG_BIT(tmpVal, HBN_XTAL32K_AC_CAP_SHORT);
    BL_WR_REG(HBN_BASE, HBN_XTAL32K, tmpVal);
}


void bl_rtc_init(void)
{
    uint32_t tmpVal;
    uint32_t pu32k;
    
#ifdef CFG_USE_XTAL32K
    tmpVal = BL_RD_REG(HBN_BASE, HBN_XTAL32K);
    pu32k = BL_GET_REG_BITS_VAL(tmpVal, HBN_PU_XTAL32K);
    if(!pu32k){
        HBN_Power_On_Xtal_32K();
    }
    
    HBN_32K_Sel(HBN_32K_XTAL);
    xtal32k_fix();
#else
    tmpVal = BL_RD_REG(HBN_BASE, HBN_GLB);
    pu32k = BL_GET_REG_BITS_VAL(tmpVal, HBN_PU_RC32K);
    if(!pu32k){
        HBN_Power_On_RC32K();
    }
    
    HBN_32K_Sel(HBN_32K_RC);
    rc32k_cal();
#endif
    
    HBN_Enable_RTC_Counter();
    
    // if rtc counter does not change, something must be wrong, e.g. CFG_USE_XTAL32K is defined but XTAL32K crystal is not mounted
    uint32_t now = bl_timer_now_us();
    uint32_t timeout = 1000000;
    uint64_t cnt = bl_rtc_get_counter();
    while(bl_rtc_get_counter() - cnt < 3){
        if(bl_timer_now_us() - now >= timeout){
            blog_assert(0);
        }
    }
}

ATTR_TCM_SECTION
uint64_t bl_rtc_get_counter(void)
{
    uint32_t valLow, valHigh;
    
    RomDriver_HBN_Get_RTC_Timer_Val(&valLow, &valHigh);
    
    return ((uint64_t)valHigh << 32) | valLow;
}

ATTR_TCM_SECTION
uint64_t bl_rtc_get_aligned_counter(void)
{
    uint32_t valLow_tmp;
    uint32_t valLow, valHigh;
    uint64_t cnt;
    
    RomDriver_HBN_Get_RTC_Timer_Val(&valLow_tmp, &valHigh);
    
    do {
        RomDriver_HBN_Get_RTC_Timer_Val(&valLow, &valHigh);
    } while(valLow == valLow_tmp);
    
    cnt = ((uint64_t)valHigh << 32) | valLow;
    
    return cnt;
}

ATTR_TCM_SECTION
uint64_t bl_rtc_get_delta_counter(uint64_t ref_cnt)
{
    uint64_t cnt;
    
    cnt = bl_rtc_get_counter();
    ref_cnt &= BL_RTC_MAX_COUNTER;
    
    if(cnt < ref_cnt){
        cnt += BL_RTC_MAX_COUNTER + 1;
    }
    
    return cnt - ref_cnt;
}

uint64_t bl_rtc_get_timestamp_ms(void)
{
    uint64_t cnt;
    
    cnt = bl_rtc_get_counter();
    
    return BL_RTC_COUNTER_TO_MS(cnt);
}

uint64_t bl_rtc_get_delta_time_ms(uint64_t ref_cnt)
{
    uint64_t cnt;
    
    cnt = bl_rtc_get_delta_counter(ref_cnt);
    
    return BL_RTC_COUNTER_TO_MS(cnt);
}
