#include "bl702_rf_public.h"
#include "bl_wireless.h"
#include "bl_efuse.h"
#include "bl_adc.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "hal_tcal.h"

#define TCAL_PERIOD_MS             (1000*10)

#define printf(...)                (void)0

static int tcal_init = 0;
static void *tcal_timer = NULL;
static int16_t tcal_temperature = 0;

static void hal_tcal_callback(int16_t tsen_val)
{
    if(bl_wireless_power_tcal_en_get() != 0){
        if(tcal_init == 0){
            printf("[tcal] rf702_set_init_tsen_value(%d)\r\n", tsen_val);
            
            rf702_set_init_tsen_value(tsen_val);
            
            tcal_init = 1;
        }else{
            printf("[tcal] rf702_inc_cal_tsen_based(%d)\r\n", tsen_val);
            
            rf702_inc_cal_tsen_based(tsen_val);
        }
    }
    
    tcal_temperature = tsen_val;
}

static void hal_tcal_trigger(void)
{
    bl_adc_tsen_dma_trigger();
}

int hal_tcal_init(void)
{
    bl_adc_tsen_cfg_t tsen_cfg = {
        .tsen_event = hal_tcal_callback,
    };
    
    int status = bl_adc_tsen_dma_init(&tsen_cfg);
    
    printf("[tcal] hal_tcal_init: %d\r\n", status);
    
    return status;
}

int hal_tcal_restart(void)
{
    // Stop periodical tcal trigger
    if(tcal_timer != NULL){
        xTimerDelete(tcal_timer, 0);
        tcal_timer = NULL;
    }
    
    // Perform initial tcal trigger
    tcal_init = 0;
    hal_tcal_trigger();
    
    // Start periodical tcal trigger
    tcal_timer = xTimerCreate("tcal", TCAL_PERIOD_MS, pdTRUE, NULL, (TimerCallbackFunction_t)hal_tcal_trigger);
    if(tcal_timer != NULL){
        xTimerStart(tcal_timer, 0);
    }
    
    printf("[tcal] hal_tcal_restart\r\n");
    
    return 0;
}

int hal_tcal_pause(void)
{
    if(tcal_timer != NULL){
        xTimerDelete(tcal_timer, 0);
        tcal_timer = NULL;
    }
    
    while(bl_adc_tsen_dma_is_busy());
    
    printf("[tcal] hal_tcal_pause\r\n");
    
    return 0;
}

int hal_tcal_resume(void)
{
    if(tcal_timer == NULL){
        tcal_timer = xTimerCreate("tcal", TCAL_PERIOD_MS, pdTRUE, NULL, (TimerCallbackFunction_t)hal_tcal_trigger);
    }
    if(tcal_timer != NULL){
        xTimerStart(tcal_timer, 0);
    }
    
    printf("[tcal] hal_tcal_resume\r\n");
    
    return 0;
}

int16_t hal_tcal_get_temperature(void)
{
    return tcal_temperature;
}
