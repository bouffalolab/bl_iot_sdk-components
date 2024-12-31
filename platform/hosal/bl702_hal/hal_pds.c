#include "hal_pds.h"
#include "hosal_uart.h"


//#define PDS_TEST
//#define GPIO_WAKEUP_TEST

#if defined(PDS_TEST)
#include "FreeRTOS.h"
#include "task.h"
#endif

#if defined(GPIO_WAKEUP_TEST)
static uint8_t test_pin_list[] = {9};
#endif


#define pulTimeHigh                          (volatile uint32_t *)( 0x02000000 + 0xBFFC )
#define pulTimeLow                           (volatile uint32_t *)( 0x02000000 + 0xBFF8 )
#define pullMachineTimerCompareRegister      (volatile uint64_t *)( 0x02000000 + 0x4000 )

#define configCLIC_TIMER_ENABLE_ADDRESS      (0x02800407)

#define MTIMER_TICKS_PER_US                  (4)


extern void vTaskStepTick(uint32_t xTicksToJump);
extern void vPortSetupTimerInterrupt(void);


void hal_pds_init(void)
{
    bl_pds_init();
    
#if defined(GPIO_WAKEUP_TEST)
    bl_pds_gpio_wakeup_cfg(test_pin_list, sizeof(test_pin_list));
#endif
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
    uint64_t mtimerClkCycles;
    uint32_t ulCurrentTimeHigh, ulCurrentTimeLow;
    
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
    
    mtimerClkCycles = ((uint64_t)ulCurrentTimeHigh << 32) | ulCurrentTimeLow;
    mtimerClkCycles += (uint64_t)actualSleepDuration_ms * 1000 * MTIMER_TICKS_PER_US;
    ulCurrentTimeHigh = mtimerClkCycles >> 32;
    ulCurrentTimeLow = mtimerClkCycles & 0xFFFFFFFF;
    
    GLB_Set_MTimer_CLK(1, GLB_MTIMER_CLK_BCLK, SystemCoreClockGet()/(GLB_Get_BCLK_Div()+1)/4000000 - 1);
    
    *pulTimeLow = 0;
    *pulTimeHigh = ulCurrentTimeHigh;
    *pulTimeLow = ulCurrentTimeLow;
    
    vTaskStepTick(actualSleepDuration_ms);
    
    vPortSetupTimerInterrupt();
    *(volatile uint8_t *)configCLIC_TIMER_ENABLE_ADDRESS = 1;
    
    return actualSleepDuration_ms;
}


#if defined(PDS_TEST)
void vApplicationSleep( TickType_t xExpectedIdleTime )
{
    eSleepModeStatus eSleepStatus;
    uint32_t xExpectedSleepTime;
    uint32_t sleepCycles;
    uint32_t sleepTime;
    
    if(xTaskGetTickCount() < 1000){
        return;
    }
    
    eSleepStatus = eTaskConfirmSleepModeStatus();
    if(eSleepStatus == eAbortSleep){
        return;
    }
    
    if(xExpectedIdleTime < 5500){
        return;
    }
    
    xExpectedSleepTime = 5000;
    
    printf("[%lu] will sleep: %lu ms\r\n", (uint32_t)bl_rtc_get_timestamp_ms(), xExpectedSleepTime);
    arch_delay_us(100);
    
    sleepCycles = xExpectedSleepTime * 32768 / 1000;
    sleepTime = hal_pds_enter_with_time_compensation(31, sleepCycles);
    
    HOSAL_UART_DEV_DECL(uart_stdio, 0, 14, 15, 2000000);
    hosal_uart_init(&uart_stdio);
    
    int source = bl_pds_get_wakeup_source();
    uint32_t gpio = bl_pds_get_wakeup_gpio();
    
    if(source == PDS_WAKEUP_BY_SLEEP_CNT){
        printf("wakeup source: sleep cnt\r\n");
    }else if(source == PDS_WAKEUP_BY_GPIO){
        printf("wakeup source: gpio -> 0x%08lX\r\n", gpio);
    }else{
        printf("wakeup source: unknown\r\n");
    }
    
    printf("[%lu] actually sleep: %lu ms\r\n", (uint32_t)bl_rtc_get_timestamp_ms(), sleepTime);
}
#endif
