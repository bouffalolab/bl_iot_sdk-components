#include "hal_pds.h"


#define pulTimeHigh                (volatile uint32_t *)( configCLINT_BASE_ADDRESS + 0xBFFC )
#define pulTimeLow                 (volatile uint32_t *)( configCLINT_BASE_ADDRESS + 0xBFF8 )

#define MTIMER_TICKS_PER_US        (4)


extern volatile uint64_t * const pullMachineTimerCompareRegister;
extern void vPortSetupTimerInterrupt(void);


void hal_pds_init(void)
{
    bl_pds_init();
}

void hal_pds_fastboot_cfg(uint32_t addr)
{
    bl_pds_fastboot_cfg(addr);
}

void hal_pds_enter_without_time_compensation(uint32_t pdsLevel, uint32_t pdsSleepCycles)
{
    bl_pds_enter(pdsLevel, pdsSleepCycles);
}

uint32_t hal_pds_enter_with_time_compensation(uint32_t pdsLevel, uint32_t pdsSleepCycles)
{
    uint64_t rtcRefCnt;
    uint32_t actualSleepDuration_ms;
    uint32_t mtimerClkCfg;
    uint32_t mtimerClkCycles;
    uint32_t ulCurrentTimeHigh, ulCurrentTimeLow;
    
    mtimerClkCfg = BL_RD_REG(GLB_BASE, GLB_CPU_CLK_CFG);  // store mtimer clock setting
    
    *pullMachineTimerCompareRegister = -1;  // avoid mtimer interrupt pending
    *(volatile uint8_t *)configCLIC_TIMER_ENABLE_ADDRESS = 0;  // disable mtimer interrrupt
    
    do
    {
        ulCurrentTimeHigh = *pulTimeHigh;
        ulCurrentTimeLow = *pulTimeLow;
    } while( ulCurrentTimeHigh != *pulTimeHigh );
    
    rtcRefCnt = bl_rtc_get_counter();
    
    bl_pds_enter(pdsLevel, pdsSleepCycles);
    
    actualSleepDuration_ms = (uint32_t)bl_rtc_get_delta_time_ms(rtcRefCnt);
    
    *pullMachineTimerCompareRegister = -1;
    *(volatile uint8_t *)configCLIC_TIMER_ENABLE_ADDRESS = 0;
    
    vTaskStepTick(actualSleepDuration_ms);
    
    mtimerClkCycles = actualSleepDuration_ms * 1000 * MTIMER_TICKS_PER_US;
    ulCurrentTimeLow += mtimerClkCycles;
    if(ulCurrentTimeLow < mtimerClkCycles){
        ulCurrentTimeHigh++;
    }
    
    BL_WR_REG(GLB_BASE, GLB_CPU_CLK_CFG, mtimerClkCfg);  // restore mtimer clock setting
    
    *pulTimeLow = 0;
    *pulTimeHigh = ulCurrentTimeHigh;
    *pulTimeLow = ulCurrentTimeLow;
    
    vPortSetupTimerInterrupt();
    *(volatile uint8_t *)configCLIC_TIMER_ENABLE_ADDRESS = 1;
    
    return actualSleepDuration_ms;
}
