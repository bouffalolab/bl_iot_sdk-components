#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <utils_list.h>
#include <bl_timer.h>
#ifdef BL702L
#include <bl_pds.h>
#endif
#include <FreeRTOS.h>
#include <timers.h>
#include <openthread/config.h>
#include <openthread/platform/alarm-milli.h>
#include <openthread/platform/alarm-micro.h>
#include <openthread/platform/time.h>
#include <openthread/platform/diag.h>
#include <openthread_port.h>

#define OT_TIMER_MS_TO_TICKS(xTimeInMs) ((xTimeInMs) / 1000 * configTICK_RATE_HZ + ((xTimeInMs) % 1000) * configTICK_RATE_HZ / 1000)

#if defined(BL702)

#define OT_ALARM_TIMER_MARGIN                   10   /** 10 us */
#define OT_ALARM_TIMER_ID                       0

TimerHandle_t otAlarm_timerHandle = NULL;
uint32_t      otAlarm_offset = 0;

static void otPlatALarm_msTimerCallback( TimerHandle_t xTimer ) 
{
    otrNotifyEvent(OT_SYSTEM_EVENT_ALARM_MS_EXPIRED);
}

void ot_alarmInit(void) 
{
    otAlarm_timerHandle = xTimerCreate("ot_timer", 1, pdFALSE, (void *)otAlarm_timerHandle, otPlatALarm_msTimerCallback);

    uint32_t tag = otrEnterCrit();
    bl_timer_restore_time(bl_timer_now_us64());
    otAlarm_offset = (uint32_t)bl_timer_now_us64() - bl_timer_get_current_time();
    otrExitCrit(tag);
}

void otPlatAlarmMilliStartAt(otInstance *aInstance, uint32_t aT0, uint32_t aDt)
{
    BaseType_t ret;

    uint32_t elapseTime = (uint32_t)(bl_timer_now_us64() / 1000) - aT0;

    if (elapseTime < aDt) {

        ret = xTimerChangePeriod( otAlarm_timerHandle, OT_TIMER_MS_TO_TICKS(aDt - elapseTime), 0 );
        configASSERT(ret == pdPASS);

        return;
    }

    otrNotifyEvent(OT_SYSTEM_EVENT_ALARM_MS_EXPIRED);
}

void otPlatAlarmMilliStop(otInstance *aInstance) 
{
    if (xTimerIsTimerActive(otAlarm_timerHandle) == pdTRUE) {
        xTimerStop(otAlarm_timerHandle, 0 );
    }
}

uint32_t otPlatAlarmMilliGetNow(void) 
{
    return (uint32_t)(bl_timer_now_us64() / 1000);
}

uint64_t otPlatTimeGet(void)
{
    return bl_timer_now_us64();
}

uint16_t otPlatTimeGetXtalAccuracy(void)
{
    return OPENTHREAD_CONFIG_PLATFORM_XTAL_ACCURACY;
}

void otAlarm_microTimerCallback(void) 
{
    otrNotifyEvent(OT_SYSTEM_EVENT_ALARM_US_EXPIRED);
}

void otPlatAlarmMicroStartAt(otInstance *aInstance, uint32_t aT0, uint32_t aDt)
{
    uint32_t tag = otrEnterCrit();
    uint32_t now = bl_timer_now_us64();

    if (aDt > OT_ALARM_TIMER_MARGIN &&  now - aT0 < aDt - OT_ALARM_TIMER_MARGIN) {

        bl_timer_start(OT_ALARM_TIMER_ID, aT0 + aDt - otAlarm_offset, otAlarm_microTimerCallback);
        if (bl_timer_get_remaining_time(OT_ALARM_TIMER_ID) <= aDt) {
            otrExitCrit(tag);
            return;
        }

        bl_timer_stop(OT_ALARM_TIMER_ID);
    }

    otrExitCrit(tag);
    otrNotifyEvent(OT_SYSTEM_EVENT_ALARM_US_EXPIRED);
}

void otPlatAlarmMicroStop(otInstance *aInstance)
{
    bl_timer_stop(OT_ALARM_TIMER_ID);
}

uint32_t otPlatAlarmMicroGetNow(void)
{
   return bl_timer_now_us64();
}
#elif defined BL702L
typedef struct {
    uint32_t timerId;
    TimerHandle_t handle;
} otAlarm_t;
extern otAlarm_t otAlarm;

ATTR_PDS_SECTION
void otPlatAlarmMilliStartAt(otInstance *aInstance, uint32_t aT0, uint32_t aDt)
{
    BaseType_t ret;

    uint32_t elapseTime = (uint32_t)(bl_timer_now_us64() / 1000) - aT0;

    if (elapseTime < aDt) {

        ret = xTimerChangePeriod( otAlarm.handle, OT_TIMER_MS_TO_TICKS(aDt - elapseTime), 0 );
        configASSERT(ret == pdPASS);

        return;
    }

    otrNotifyEvent(OT_SYSTEM_EVENT_ALARM_MS_EXPIRED);
}
#endif

void ot_alarmTask(ot_system_event_t sevent) 
{
    if (!(OT_SYSTEM_EVENT_ALARM_ALL_MASK & sevent)) {
        return;
    }

    if (OT_SYSTEM_EVENT_ALARM_MS_EXPIRED & sevent) 
    {
        otPlatAlarmMilliFired(otrGetInstance());
    }

#if OPENTHREAD_CONFIG_PLATFORM_USEC_TIMER_ENABLE
    if (OT_SYSTEM_EVENT_ALARM_US_EXPIRED & sevent) 
    {
        otPlatAlarmMicroFired(otrGetInstance());
    }
#endif
}
