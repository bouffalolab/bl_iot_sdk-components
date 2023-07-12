
#include <assert.h>
#include <stdio.h>
#include <errno.h>

#ifdef BL702L
#include <rom_hal_ext.h>
#endif

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <openthread-core-config.h>
#include <openthread/cli.h>
#include <openthread/diag.h>
#include <openthread/tasklet.h>

#include "openthread_port.h"

#ifdef OPENTHREAD_BORDER_ROUTER
#include <lwip/tcpip.h>
#endif /* OPENTHREAD_BORDER_ROUTER */

#if defined(CFG_USE_PSRAM) || defined(OPENTHREAD_BORDER_ROUTER)
#include <mbedtls/platform.h>
#endif
#include <ot_utils_ext.h>

#ifdef BL702
__attribute__((section(".bss"))) ot_system_event_t                   ot_system_event_var = OT_SYSTEM_EVENT_NONE;
__attribute__((section(".bss"))) static SemaphoreHandle_t            ot_extLock          = NULL;
__attribute__((section(".bss"))) static otInstance *                 ot_instance         = NULL;
__attribute__((section(".bss"))) static TaskHandle_t                 ot_taskHandle       = NULL;
#endif

#ifdef BL702L
extern SemaphoreHandle_t             ot_extLock;
extern otInstance *                  ot_instance;
extern TaskHandle_t                  ot_taskHandle;
extern void (*otrTask_ptr)(void);
#endif

#ifdef BL702

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
    if (ot_extLock) {
        xSemaphoreTakeRecursive(ot_extLock, portMAX_DELAY);
    }
}

void otrUnlock(void)
{
    if (ot_extLock) {
        xSemaphoreGiveRecursive(ot_extLock);
    }
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

__attribute__((weak)) void otrAppProcess(ot_system_event_t sevent) 
{
}

__attribute__((weak)) void ot_uartTask (ot_system_event_t sevent) 
{
}

__attribute__((weak)) void otrInitUser(otInstance * instance)
{

}


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
    ot_uartTask(sevent);
    ot_radioTask(sevent);
    otrAppProcess(sevent);
}


void otrStackInit(void) 
{
    ot_instance = otInstanceInitSingle();
    assert(ot_instance);
}

OT_TOOL_WEAK void otbr_netif_process(otInstance *aInstance) {}

#ifdef BL702L
ATTR_PDS_SECTION
#endif
void otrTaskLoop(void) 
{   
    /** need put on RAM */
    while (true)
    {
#ifdef BL702L
        if (otrTask_ptr) {
            otrTask_ptr();
        }
        else
#endif 
        {
            OT_THREAD_SAFE (
                otTaskletsProcess(ot_instance);
                otSysProcessDrivers(ot_instance);
                otbr_netif_process(ot_instance);
            );
        }
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    }
}

static void otrStackTask(void *p_arg)
{
    /** This task is an example to handle both main event loop of openthread task lets and 
     * hardware drivers for openthread, such as radio, alarm timer and also uart shell.
     * Customer can implement own task for both of two these missions with other privoded APIs.  */
    otRadio_opt_t opt;

    otrLock();
    otrUnlock();
    
    (void)errno;

    opt.byte = (uint8_t)((uint32_t)p_arg);
    ot_system_event_var = OT_SYSTEM_EVENT_NONE;

#ifdef CFG_OPENTHREAD_TESTS_UNIT

    ot_alarmInit();
    ot_radioInit(opt);

extern void test_main(void);
    test_main();
    printf ("==============================end==========================\r\n");

#endif
    
    OT_THREAD_SAFE (
        ot_alarmInit();
        ot_radioInit(opt);
        otrStackInit();
#if defined(CFG_USE_PSRAM) || defined(OPENTHREAD_BORDER_ROUTER)
        mbedtls_platform_set_calloc_free(pvPortCalloc, vPortFree);
#endif /* CFG_USE_PSRAM */
#if OPENTHREAD_ENABLE_DIAG
        otDiagInit(ot_instance);
#endif
        otrInitUser(ot_instance);
    );

#ifdef CFG_PDS_ENABLE
    otPds_savePdsTaskInfo();
#endif

    otrTaskLoop();

    otInstanceFinalize(ot_instance);
    ot_instance = NULL;

    vTaskDelete(NULL);
}

void otrStart(otRadio_opt_t opt)
{
    ot_extLock = xSemaphoreCreateMutex();
    assert(ot_extLock != NULL);

    otrLock();

    xTaskCreate(otrStackTask, "threadTask", OT_TASK_SIZE, (void *)((uint32_t)opt.byte), 15, &ot_taskHandle);

    otrUnlock();
}
