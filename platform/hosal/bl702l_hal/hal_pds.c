#include "hal_pds.h"
#include "hosal_uart.h"


//#define PDS_TEST
//#define GPIO_WAKEUP_TEST
//#define KEY_WAKEUP_TEST

#if defined(PDS_TEST)
#include "FreeRTOS.h"
#include "task.h"
#endif

#if defined(GPIO_WAKEUP_TEST)
static uint8_t test_pin_list[] = {16};
#endif
#if defined(KEY_WAKEUP_TEST)
static uint8_t test_row_pins[] = {25, 24, 23};
static uint8_t test_col_pins[] = {28, 27, 26};
#endif


#define pulTimeHigh                          (volatile uint32_t *)( 0x02000000 + 0xBFFC )
#define pulTimeLow                           (volatile uint32_t *)( 0x02000000 + 0xBFF8 )
#define pullMachineTimerCompareRegister      (volatile uint64_t *)( 0x02000000 + 0x4000 )

#define configCLIC_TIMER_ENABLE_ADDRESS      (0x02800407)

#define MTIMER_TICKS_PER_US                  (2)


extern void vTaskStepTick(uint32_t xTicksToJump);
extern void vPortSetupTimerInterrupt(void);


void hal_pds_init(void)
{
    bl_pds_init();
    
#if defined(GPIO_WAKEUP_TEST)
    bl_pds_gpio_wakeup_cfg(test_pin_list, sizeof(test_pin_list), PDS_GPIO_EDGE_BOTH);
#endif
#if defined(KEY_WAKEUP_TEST)
    bl_pds_key_wakeup_cfg(sizeof(test_row_pins), sizeof(test_col_pins), test_row_pins, test_col_pins);
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
    uint32_t rtcDeltaCnt;
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
    
    rtcDeltaCnt = (uint32_t)bl_rtc_get_delta_counter(rtcRefCnt);
    actualSleepDuration_ms = bl_rtc_counter_to_ms(rtcDeltaCnt);
    
    *pullMachineTimerCompareRegister = -1;
    *(volatile uint8_t *)configCLIC_TIMER_ENABLE_ADDRESS = 0;
    
    mtimerClkCycles = ((uint64_t)ulCurrentTimeHigh << 32) | ulCurrentTimeLow;
    mtimerClkCycles += (uint64_t)actualSleepDuration_ms * 1000 * MTIMER_TICKS_PER_US;
    ulCurrentTimeHigh = mtimerClkCycles >> 32;
    ulCurrentTimeLow = mtimerClkCycles & 0xFFFFFFFF;
    
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
    
#if defined(KEY_WAKEUP_TEST)
    bl_kys_result_t result;
    
    bl_kys_init(sizeof(test_row_pins), sizeof(test_col_pins), test_row_pins, test_col_pins);
    bl_kys_trigger_poll(&result);
    
    if(result.ghost_det){
        printf("ghost key detected!\r\n");
        return;
    }else{
        printf("key_num: %d\r\n", result.key_num);
        if(result.key_num > 0){
            printf("key: ");
            for(int i=0; i<result.key_num; i++){
                printf("(%d, %d) ", result.row_idx[i], result.col_idx[i]);
            }
            printf("\r\n");
        }
        
        bl_pds_set_white_keys(result.key_num, result.row_idx, result.col_idx);
    }
#endif
    
    printf("[%lu] will sleep: %lu ms\r\n", (uint32_t)bl_rtc_get_timestamp_ms(), xExpectedSleepTime);
    arch_delay_us(100);
    
    sleepCycles = bl_rtc_ms_to_counter(xExpectedSleepTime);
    sleepTime = hal_pds_enter_with_time_compensation(31, sleepCycles);
    
    RomDriver_AON_Power_On_XTAL();
    RomDriver_HBN_Set_ROOT_CLK_Sel(HBN_ROOT_CLK_XTAL);
    
    HOSAL_UART_DEV_DECL(uart_stdio, 0, 14, 15, 2000000);
    hosal_uart_init(&uart_stdio);
    
    int source;
    uint32_t gpio;
    uint8_t key_row_idx, key_col_idx;
    int key_event;
    
    source = bl_pds_get_wakeup_source();
    gpio = bl_pds_get_wakeup_gpio();
    key_event = bl_pds_get_wakeup_key_index(&key_row_idx, &key_col_idx);
    
    if(source == PDS_WAKEUP_BY_RTC){
        printf("wakeup source: rtc\r\n");
    }else if(source == PDS_WAKEUP_BY_GPIO){
        printf("wakeup source: gpio -> 0x%08lX\r\n", gpio);
    }else if(source == PDS_WAKEUP_BY_KEY){
        if(key_event == PDS_KEY_EVENT_PRESS){
            printf("wakeup source: key press -> (%d, %d)\r\n", key_row_idx, key_col_idx);
        }else if(key_event == PDS_KEY_EVENT_RELEASE){
            printf("wakeup source: key release -> (%d, %d)\r\n", key_row_idx, key_col_idx);
        }else{
            printf("wakeup source: key error -> (%d, %d)\r\n", key_row_idx, key_col_idx);
        }
    }else{
        printf("wakeup source: unknown\r\n");
    }
    
    printf("[%lu] actually sleep: %lu ms\r\n", (uint32_t)bl_rtc_get_timestamp_ms(), sleepTime);
}
#endif
