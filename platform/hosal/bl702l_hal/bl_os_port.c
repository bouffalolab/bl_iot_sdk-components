/*
 * Copyright (c) 2016-2026 Bouffalolab.
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
#include "bl_os_port.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "semphr.h"
#include "event_groups.h"


void *bl_os_timer_create(const char *name, uint32_t period, void *handler, int repeat)
{
    return xTimerCreate(name, period, repeat, NULL, handler);
}

int bl_os_timer_start(void *xTimer)
{
    BaseType_t xResult = pdFAIL;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    if(!xPortIsInsideInterrupt()){
        xResult = xTimerStart(xTimer, 0);
    }else{
        xResult = xTimerStartFromISR(xTimer, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    
    return xResult;
}

int bl_os_timer_stop(void *xTimer)
{
    BaseType_t xResult = pdFAIL;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    if(!xPortIsInsideInterrupt()){
        xResult = xTimerStop(xTimer, 0);
    }else{
        xResult = xTimerStopFromISR(xTimer, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    
    return xResult;
}

int bl_os_timer_change_period(void *xTimer, uint32_t period)
{
    BaseType_t xResult = pdFAIL;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    if(!xPortIsInsideInterrupt()){
        xResult = xTimerChangePeriod(xTimer, period, 0);
    }else{
        xResult = xTimerChangePeriodFromISR(xTimer, period, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    
    return xResult;
}

int bl_os_timer_reset(void *xTimer)
{
    BaseType_t xResult = pdFAIL;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    if(!xPortIsInsideInterrupt()){
        xResult = xTimerReset(xTimer, 0);
    }else{
        xResult = xTimerResetFromISR(xTimer, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    
    return xResult;
}

int bl_os_timer_delete(void *xTimer)
{
    return xTimerDelete(xTimer, 0);
}


void *bl_os_semphr_create(void)
{
    return xSemaphoreCreateBinary();
}

int bl_os_semphr_take(void *xSemaphore, uint32_t timeout)
{
    BaseType_t xResult = pdFAIL;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    if(!xPortIsInsideInterrupt()){
        xResult = xSemaphoreTake(xSemaphore, timeout);
    }else{
        xResult = xSemaphoreTakeFromISR(xSemaphore, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    
    return xResult;
}

int bl_os_semphr_give(void *xSemaphore)
{
    BaseType_t xResult = pdFAIL;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    if(!xPortIsInsideInterrupt()){
        xResult = xSemaphoreGive(xSemaphore);
    }else{
        xResult = xSemaphoreGiveFromISR(xSemaphore, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    
    return xResult;
}

int bl_os_semphr_delete(void *xSemaphore)
{
    vSemaphoreDelete(xSemaphore);
    return pdPASS;
}


void *bl_os_event_group_create(void)
{
    return xEventGroupCreate();
}

uint32_t bl_os_event_group_wait_bits(void *xEventGroup, uint32_t bits, uint32_t timeout)
{
    return xEventGroupWaitBits(xEventGroup, bits, pdTRUE, pdTRUE, timeout);
}

int bl_os_event_group_clear_bits(void *xEventGroup, uint32_t bits)
{
    BaseType_t xResult = pdPASS;
    
    if(!xPortIsInsideInterrupt()){
        xEventGroupClearBits(xEventGroup, bits);
    }else{
        xResult = xEventGroupClearBitsFromISR(xEventGroup, bits);
    }
    
    return xResult;
}

int bl_os_event_group_set_bits(void *xEventGroup, uint32_t bits)
{
    BaseType_t xResult = pdPASS;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    if(!xPortIsInsideInterrupt()){
        xEventGroupSetBits(xEventGroup, bits);
    }else{
        xResult = xEventGroupSetBitsFromISR(xEventGroup, bits, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    
    return xResult;
}

int bl_os_event_group_delete(void *xEventGroup)
{
    vEventGroupDelete(xEventGroup);
    return pdPASS;
}
