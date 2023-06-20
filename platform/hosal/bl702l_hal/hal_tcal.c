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
#include "bl702l_glb.h"
#include "bl702l_rf.h"
#include "bl_wireless.h"
#include "bl_efuse.h"
#include "bl_adc.h"
#include "hal_hwtimer.h"
#include "hal_tcal.h"

#define TCAL_PERIOD_MS             (1000*10)

#define printf(...)                (void)0

static int tcal_init = 0;
static hw_timer_t *tcal_timer = NULL;
static int16_t tcal_temperature = 0;

static void hal_tcal_callback(int16_t tsen_val)
{
    uint8_t capcode;
    int8_t offset;
    
    if(bl_wireless_power_tcal_en_get() != 0){
        if(tcal_init == 0){
            printf("[tcal] rf702_set_init_tsen_value(%d)\r\n", tsen_val);
            
            rf_set_init_tsen_value(tsen_val);
            
            tcal_init = 1;
        }else{
            printf("[tcal] rf702_inc_cal_tsen_based(%d)\r\n", tsen_val);
            
            rf_inc_cal_tsen_based(tsen_val);
        }
    }
    
    if(bl_wireless_capcode_tcal_en_get() != 0){
        if(bl_efuse_read_capcode(&capcode) == 0){
            offset = bl_wireless_capcode_offset_get(tsen_val);
            
            printf("[tcal] capcode @ efuse: %d, offset @ %d C: %d\r\n", capcode, tsen_val, offset);
            
            capcode += offset;
            AON_Set_Xtal_CapCode(capcode, capcode);
        }else{
            printf("[tcal] fail to read capcode from efuse\r\n");
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
        hal_hwtimer_delete(tcal_timer);
        tcal_timer = NULL;
    }
    
    // Perform initial tcal trigger
    tcal_init = 0;
    hal_tcal_trigger();
    
    // Start periodical tcal trigger
    tcal_timer = hal_hwtimer_create(TCAL_PERIOD_MS, hal_tcal_trigger, 1);
    
    printf("[tcal] hal_tcal_restart\r\n");
    
    return 0;
}

int hal_tcal_pause(void)
{
    if(tcal_timer != NULL){
        hal_hwtimer_delete(tcal_timer);
        tcal_timer = NULL;
    }
    
    while(bl_adc_tsen_dma_is_busy());
    
    printf("[tcal] hal_tcal_pause\r\n");
    
    return 0;
}

int hal_tcal_resume(void)
{
    if(tcal_timer == NULL){
        tcal_timer = hal_hwtimer_create(TCAL_PERIOD_MS, hal_tcal_trigger, 1);
    }
    
    printf("[tcal] hal_tcal_resume\r\n");
    
    return 0;
}

int16_t hal_tcal_get_temperature(void)
{
    return tcal_temperature;
}
