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

#include <assert.h>

#if defined(CONFIG_BL_SDK)
#include <compiler/compiler_ld.h>
#else
#include <compiler/common.h>
#endif

#include <lmac154.h>
#include <zb_timer.h>

#include <openthread-core-config.h>
#include <openthread_port.h>
#include <openthread/platform/alarm-milli.h>
#include <openthread/platform/alarm-micro.h>

#define OT_ALARM_MAX_MILLI_SEC          (((uint64_t)1 << 36) / 1000)

static int ot_alarm_timer_milli_sec_cnt = 0;

void ot_alarmInit(void) {}

ATTR_TCM_SECTION
static void otPlatALarm_msTimerCallback(void) 
{
    ot_alarm_timer_milli_sec_cnt --;

    if (ot_alarm_timer_milli_sec_cnt <= 0) {
        zb_timer_stop(ZB_TIMER_THREAD_MS);
        otrNotifyEvent(OT_SYSTEM_EVENT_ALARM_MS_EXPIRED);
    }
    else {
        zb_timer_start(ZB_TIMER_THREAD_MS, zb_timer_get_current_time(), otPlatALarm_msTimerCallback);
    }
}

void otPlatAlarmMilliStartAt(otInstance *aInstance, uint32_t aT0, uint32_t aDt)
{
    uint32_t tag, current, delay_cnt;
    uint64_t delta;

    assert(aT0 <= OT_ALARM_MAX_MILLI_SEC);

    zb_timer_stop(ZB_TIMER_THREAD_MS);
    ot_alarm_timer_milli_sec_cnt = 0;

    tag = otrEnterCrit();
    current = ((uint64_t)zb_timer_get_current_time() << ZB_TIMER_BITNUM_PER_TICK) / 1000;
    if (current - aT0 < aDt) {
        delta = (uint64_t)(aDt - (current - aT0));
        delay_cnt = ((delta % OT_ALARM_MAX_MILLI_SEC) * 1000) >> ZB_TIMER_BITNUM_PER_TICK;
        zb_timer_start(ZB_TIMER_THREAD_MS, zb_timer_get_current_time() + delay_cnt, otPlatALarm_msTimerCallback);

        ot_alarm_timer_milli_sec_cnt = (delta + OT_ALARM_MAX_MILLI_SEC - 1) / OT_ALARM_MAX_MILLI_SEC;
    }

    current = ((uint64_t)zb_timer_get_current_time() << ZB_TIMER_BITNUM_PER_TICK) / 1000;
    if (current - aT0 >= aDt) {
        zb_timer_stop(ZB_TIMER_THREAD_MS);
        ot_alarm_timer_milli_sec_cnt = 0;
        otPlatALarm_msTimerCallback();
    }

    otrExitCrit(tag);
}

void otPlatAlarmMilliStop(otInstance *aInstance) 
{
    zb_timer_stop(ZB_TIMER_THREAD_MS);
}

uint32_t otPlatAlarmMilliGetNow(void) 
{
    return (uint32_t)(((uint64_t)zb_timer_get_current_time() << ZB_TIMER_BITNUM_PER_TICK) / 1000);
}

uint64_t otPlatTimeGet(void)
{
    return (uint64_t)zb_timer_get_current_time() << ZB_TIMER_BITNUM_PER_TICK;
}

uint16_t otPlatTimeGetXtalAccuracy(void) { return 0; }

uint64_t otPlatRadioGetNow(otInstance *aInstance)
{
    return (uint64_t)zb_timer_get_current_time() << ZB_TIMER_BITNUM_PER_TICK;
}

#if OPENTHREAD_CONFIG_PLATFORM_USEC_TIMER_ENABLE
ATTR_TCM_SECTION
static void otPlatALarm_usTimerCallback(void)
{
    zb_timer_stop(ZB_TIMER_THREAD_US);
    otrNotifyEvent(OT_SYSTEM_EVENT_ALARM_US_EXPIRED);
}

void otPlatAlarmMicroStartAt(otInstance *aInstance, uint32_t aT0, uint32_t aDt)
{
    uint32_t tag, current;
    uint64_t delta;

    zb_timer_stop(ZB_TIMER_THREAD_US);

    tag = otrEnterCrit();
    current = zb_timer_get_current_time() << ZB_TIMER_BITNUM_PER_TICK;
    if (current - aT0 < aDt) {
        delta = ((uint64_t)(aDt - (current - aT0)) + (1 << ZB_TIMER_BITNUM_PER_TICK) / 2) >> ZB_TIMER_BITNUM_PER_TICK;
        zb_timer_start(ZB_TIMER_THREAD_US, zb_timer_get_current_time() + delta, otPlatALarm_usTimerCallback);
    }

    current = zb_timer_get_current_time() << ZB_TIMER_BITNUM_PER_TICK;
    if (current - aT0 >= aDt) {
        zb_timer_stop(ZB_TIMER_THREAD_US);
        otPlatALarm_usTimerCallback();
    }
    otrExitCrit(tag);
}

void otPlatAlarmMicroStop(otInstance *aInstance)
{
    zb_timer_stop(ZB_TIMER_THREAD_US);
}

uint32_t otPlatAlarmMicroGetNow(void)
{
    return (uint32_t)((uint64_t)zb_timer_get_current_time() << ZB_TIMER_BITNUM_PER_TICK);
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
