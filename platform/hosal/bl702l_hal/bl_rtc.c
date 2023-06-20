/*
 * Copyright (c) 2016-2023 Bouffalolab.
 *
 * This file is part of
 *     *** Bouffalolab Software Dev Kit ***
 *      (see www.bouffalolab.com).
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of Bouffalo Lab nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "bl_rtc.h"


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


void bl_rtc_init(void)
{
#ifdef CFG_USE_XTAL32K
    HBN_32K_Sel(HBN_32K_XTAL);
    *(volatile uint32_t *)0x4000F204 &= ~(1U << 0);
#else
    HBN_32K_Sel(HBN_32K_RC);
    rc32k_cal();
#endif
    
    HBN_Enable_RTC_Counter();
}

uint64_t bl_rtc_get_counter(void)
{
    uint32_t valLow, valHigh;
    
    HBN_Get_RTC_Timer_Val(&valLow, &valHigh);
    
    return ((uint64_t)valHigh << 32) | valLow;
}

ATTR_TCM_SECTION
uint64_t bl_rtc_get_aligned_counter(void)
{
#define RomDriver_HBN_Get_RTC_Timer_Async_Val ((BL_Err_Type (*)(uint32_t *valLow, uint32_t *valHigh))0x210186c2)

    uint32_t valLow, valHigh;
    uint32_t valLow_tmp;
    
    RomDriver_HBN_Get_RTC_Timer_Async_Val(&valLow_tmp, &valHigh);
    
    do {
        RomDriver_HBN_Get_RTC_Timer_Async_Val(&valLow, &valHigh);
    } while(valLow == valLow_tmp);
    
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
