/**
 ****************************************************************************************
 *
 * @file bl_wdt.c
 * Copyright (C) Bouffalo Lab 2016-2018
 *
 ****************************************************************************************
 */

#include <bl702l_timer.h>
#include "bl_wdt.h"
#define MAX_MS_WDT (65535)

static uint8_t wdt_enabled = 0;
static int wdt_value = 0;

static void Timer_Watchdog_Case(int ms)
{
    WDT_Disable(WDT0_ID);
    WDT_Set_Clock(WDT0_ID, TIMER_CLKSRC_32K, 31);
    WDT_SetCompValue(WDT0_ID, 1 * ms);

    WDT_ResetCounterValue(WDT0_ID);
    WDT_IntMask(WDT0_ID, WDT_INT, MASK);

    WDT_Enable(WDT0_ID);
}

void bl_wdt_feed(void)
{
    WDT_ResetCounterValue(WDT0_ID);
}

void bl_wdt_disable(void)
{
    WDT_Disable(WDT0_ID);
    wdt_enabled = 0;
}

int bl_wdt_init(int ms)
{
    if (ms >= MAX_MS_WDT) {
        return -1;
    }
    Timer_Watchdog_Case(ms);
    wdt_value = ms;
    wdt_enabled = 1;
    return 0;
}

int bl_wdt_restore(void)
{
    if (!wdt_enabled) {
        return -1;
    }
    Timer_Watchdog_Case(wdt_value);
    return 0;
}
