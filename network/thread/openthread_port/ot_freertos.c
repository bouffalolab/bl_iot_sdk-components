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
#include <openthread_port.h>
#include <openthread/tasklet.h>

#include <semphr.h>
#include <ot_radio_trx.h>
#include <ot_utils_ext.h>

#ifdef CFG_OT_USE_ROM_CODE
extern ot_system_event_t            ot_system_event_var;
extern SemaphoreHandle_t            ot_extLock;
extern otInstance *                 ot_instance;
extern TaskHandle_t                 ot_taskHandle;

#else
ot_system_event_t                   ot_system_event_var = OT_SYSTEM_EVENT_NONE;
static SemaphoreHandle_t            ot_extLock          = NULL;
static otInstance *                 ot_instance         = NULL;
static TaskHandle_t                 ot_taskHandle       = NULL;

uint32_t otrEnterCrit(void) 
{
    if (xPortIsInsideInterrupt()) {
        return taskENTER_CRITICAL_FROM_ISR();
    }
    else {
        taskENTER_CRITICAL();
        return 0;
    }
}

void otrExitCrit(uint32_t tag) 
{
    if (xPortIsInsideInterrupt()) {
        taskEXIT_CRITICAL_FROM_ISR(tag);
    }
    else {
        taskEXIT_CRITICAL();
    }
}

ot_system_event_t otrGetNotifyEvent(void) 
{
    ot_system_event_t sevent = 0;

    taskENTER_CRITICAL();
    sevent = ot_system_event_var;
    ot_system_event_var = 0;
    taskEXIT_CRITICAL();

    return sevent;
}

void otrLock(void)
{
    xSemaphoreTakeRecursive(ot_extLock, portMAX_DELAY);
}

void otrUnlock(void)
{
    xSemaphoreGiveRecursive(ot_extLock);
}

bool otrIsThreadTask(void) 
{
    return ot_taskHandle == xTaskGetCurrentTaskHandle();
}

void otSysEventSignalPending(void)
{
    if (xPortIsInsideInterrupt())
    {
        BaseType_t pxHigherPriorityTaskWoken = pdTRUE;
        vTaskNotifyGiveFromISR( ot_taskHandle, &pxHigherPriorityTaskWoken);
    }
    else
    {
        xTaskNotifyGive(ot_taskHandle);
    }
}

void otrNotifyEvent(ot_system_event_t sevent) 
{
    if (xPortIsInsideInterrupt()) {
        ot_system_event_var |= sevent;
        BaseType_t pxHigherPriorityTaskWoken = pdTRUE;
        vTaskNotifyGiveFromISR( ot_taskHandle, &pxHigherPriorityTaskWoken);
    }
    else {
        taskENTER_CRITICAL();
        ot_system_event_var |= sevent;
        taskEXIT_CRITICAL();
        xTaskNotifyGive(ot_taskHandle);
    }
}
#endif

__attribute__((weak)) void ot_serialProcess(ot_system_event_t sevent) {}
__attribute__((weak)) void otrAppProcess(ot_system_event_t sevent) {}
__attribute__((weak)) void otrInitUser(otInstance * instance) {}
__attribute__((weak)) void otbr_netif_process(otInstance *aInstance) {}
__attribute__((weak)) void otbr_event_process(ot_system_event_t sevent) {}

void otTaskletsSignalPending(otInstance *aInstance)
{
    if (aInstance) {
        otrNotifyEvent(OT_SYSTEM_EVENT_OT_TASKLET);
    }
}

otInstance *otrGetInstance()
{
    return ot_instance;
}

void otSysProcessDrivers(otInstance *aInstance) 
{
    ot_system_event_t sevent = otrGetNotifyEvent();

    ot_alarmTask(sevent);
    ot_radioTask(sevent);
    ot_serialProcess(sevent);
    otbr_event_process(sevent);
    otrAppProcess(sevent);
}

void otrStackInit(void)
{
    ot_instance = otInstanceInitSingle();
    configASSERT(ot_instance);
}

#if defined(CFG_PDS_ENABLE)
extern void otrStackTask(void *p_arg);
#else
static void otrStackTask(void *p_arg)
{
    /** This task is an example to handle both main event loop of openthread task lets and 
     * hardware drivers for openthread, such as radio, alarm timer and also uart shell.
     * Customer can implement own task for both of two these missions with other privoded APIs.  */
    otRadio_opt_t opt;

    otrLock();
    otrUnlock();
    
    opt.byte = (uint8_t)((uint32_t)p_arg);
    ot_system_event_var = OT_SYSTEM_EVENT_NONE;

    OT_THREAD_SAFE (
        ot_alarmInit();
        ot_radioInit(opt);
        otrStackInit();

        otrInitUser(ot_instance);
    );

    while (true)
    {
        OT_THREAD_SAFE (
            otTaskletsProcess(ot_instance);
            otSysProcessDrivers(ot_instance);
            otbr_netif_process(ot_instance);
        );
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    }

    otInstanceFinalize(ot_instance);
    ot_instance = NULL;

    vTaskDelete(NULL);
}
#endif

void otrStart(otRadio_opt_t opt)
{
    ot_extLock = xSemaphoreCreateMutex();
    configASSERT(ot_extLock != NULL);

    xTaskCreate(otrStackTask, "threadTask", OT_TASK_SIZE, (void *)((uint32_t)opt.byte), 15, &ot_taskHandle);
}
