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
#if defined(CONFIG_BL_SDK)
#include <bflb_mtimer.h>
#else
#include <bl_timer.h>
#endif
#include <lmac154.h>
#include <zb_timer.h>

#include <openthread_port.h>
#include <openthread/platform/alarm-milli.h>
#include <openthread/platform/alarm-micro.h>

#include <timers.h>
#include <ot_utils_ext.h>

#define OT_TIMER_MS_TO_TICKS(xTimeInMs) ((xTimeInMs) / 1000 * configTICK_RATE_HZ + ((xTimeInMs) % 1000) * configTICK_RATE_HZ / 1000)

#ifdef CFG_OT_USE_ROM_CODE
extern uint32_t * otAlarm[2];
#define otAlarm_timerHandle (TimerHandle_t)(otAlarm[1])
#else
TimerHandle_t otAlarm_timerHandle = NULL;
uint32_t      otAlarm_offset = 0;

void otPlatALarm_msTimerCallback( TimerHandle_t xTimer ) 
{
    otrNotifyEvent(OT_SYSTEM_EVENT_ALARM_MS_EXPIRED);
}

void ot_alarmInit(void) 
{
    otAlarm_timerHandle = xTimerCreate("ot_alarm", 1, pdFALSE, (void *)&otAlarm_timerHandle, otPlatALarm_msTimerCallback);
}
#endif

void otPlatAlarmMilliStartAt(otInstance *aInstance, uint32_t aT0, uint32_t aDt)
{
    BaseType_t ret;
#if defined(CONFIG_BL_SDK)
    uint32_t elapseTime = (uint32_t)(bflb_mtimer_get_time_us() / 1000) - aT0;
#else
    uint32_t elapseTime = (uint32_t)(bl_timer_now_us64() / 1000) - aT0;
#endif

    if (elapseTime < aDt) {
        ret = xTimerChangePeriod( otAlarm_timerHandle, OT_TIMER_MS_TO_TICKS(aDt - elapseTime), 0 );
        configASSERT(ret == pdPASS);
    }
    else {
        otrNotifyEvent(OT_SYSTEM_EVENT_ALARM_MS_EXPIRED);
    }
}

#ifndef CFG_OT_USE_ROM_CODE
void otPlatAlarmMilliStop(otInstance *aInstance) 
{
    if (xTimerIsTimerActive(otAlarm_timerHandle) == pdTRUE) {
        xTimerStop(otAlarm_timerHandle , 0 );
    }
}

uint32_t otPlatAlarmMilliGetNow(void) 
{
#if defined(CONFIG_BL_SDK)
    return (uint32_t)(bflb_mtimer_get_time_us() / 1000);
#else
    return (uint32_t)(bl_timer_now_us64() / 1000);
#endif
}

uint64_t otPlatTimeGet(void)
{
#if defined(CONFIG_BL_SDK)
    return bflb_mtimer_get_time_us();
#else
    return bl_timer_now_us64() ;
#endif
}

static void otPlatALarm_usTimerCallback(void) 
{
    zb_timer_stop(LMAC154_ALARM_MICRO_TIMER_ID);
    otrNotifyEvent(OT_SYSTEM_EVENT_ALARM_US_EXPIRED);
}

void otPlatAlarmMicroStartAt(otInstance *aInstance, uint32_t aT0, uint32_t aDt)
{
    uint32_t tag, now, target;

    tag = otrEnterCrit();

    now = zb_timer_get_current_time();
    if ((now << LMAC154_US_PER_SYMBOL_BITS) - (aT0 & 0xfffffff0) < aDt) {

        /** get target time on zb_timer metric */
        target = aT0 + aDt - (now << LMAC154_US_PER_SYMBOL_BITS) + LMAC154_US_PER_SYMBOL / 2;
        target = (target >> LMAC154_US_PER_SYMBOL_BITS) + now;

        zb_timer_start(LMAC154_ALARM_MICRO_TIMER_ID, target, otPlatALarm_usTimerCallback);

        /** check whether to be expired again */
        now = zb_timer_get_current_time();
        if ((now << LMAC154_US_PER_SYMBOL_BITS) - (aT0 & 0xfffffff0) < aDt) {
            otrExitCrit(tag);
            return;
        }
    }

    zb_timer_stop(LMAC154_ALARM_MICRO_TIMER_ID);
    otrExitCrit(tag);

    otrNotifyEvent(OT_SYSTEM_EVENT_ALARM_US_EXPIRED);
}

void otPlatAlarmMicroStop(otInstance *aInstance)
{
    zb_timer_stop(LMAC154_ALARM_MICRO_TIMER_ID);
}

uint32_t otPlatAlarmMicroGetNow(void)
{
    return zb_timer_get_current_time() << LMAC154_US_PER_SYMBOL_BITS;
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
