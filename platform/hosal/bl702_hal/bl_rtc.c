#include "bl_rtc.h"
#include "bl_timer.h"
#include "blog.h"


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
#else
    tmpVal = BL_RD_REG(HBN_BASE, HBN_GLB);
    pu32k = BL_GET_REG_BITS_VAL(tmpVal, HBN_PU_RC32K);
    if(!pu32k){
        HBN_Power_On_RC32K();
    }
    
    HBN_32K_Sel(HBN_32K_RC);
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

uint64_t bl_rtc_get_counter(void)
{
    uint32_t valLow, valHigh;
    
    HBN_Get_RTC_Timer_Val(&valLow, &valHigh);
    
    return ((uint64_t)valHigh << 32) | valLow;
}

uint64_t bl_rtc_get_timestamp_ms(void)
{
    uint64_t cnt;
    
    cnt = bl_rtc_get_counter();
    
    return BL_RTC_COUNTER_TO_MS(cnt);
}

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

uint64_t bl_rtc_get_delta_time_ms(uint64_t ref_cnt)
{
    uint64_t cnt;
    
    cnt = bl_rtc_get_delta_counter(ref_cnt);
    
    return BL_RTC_COUNTER_TO_MS(cnt);
}
