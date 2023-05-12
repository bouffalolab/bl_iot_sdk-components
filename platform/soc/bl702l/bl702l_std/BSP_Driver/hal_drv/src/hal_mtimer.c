/**
 * @file hal_mtimer.c
 * @brief
 *
 * Copyright (c) 2021 Bouffalolab team
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 */
#include "hal_mtimer.h"
#include "bl702l_glb.h"
#include "bl702l_clock.h"
#include "risc-v/Core/Include/clic.h"

static void (*systick_callback)(void);
static uint64_t next_compare_tick = 0;
static uint64_t current_set_ticks = 0;

static void Systick_Handler(void)
{
    *(volatile uint64_t *)(CLIC_CTRL_ADDR + CLIC_MTIMECMP) = next_compare_tick;
    systick_callback();
    next_compare_tick += current_set_ticks;
}

/**
 * @brief
 *
 * @param time
 * @param interruptFun
 */
void mtimer_set_alarm_time(uint64_t ticks, void (*interruptfun)(void))
{
    CPU_Interrupt_Disable(MTIME_IRQn);

    uint32_t ulCurrentTimeHigh, ulCurrentTimeLow;
    volatile uint32_t *const pulTimeHigh = (volatile uint32_t *const)(CLIC_CTRL_ADDR + CLIC_MTIME + 4);
    volatile uint32_t *const pulTimeLow = (volatile uint32_t *const)(CLIC_CTRL_ADDR + CLIC_MTIME);
    volatile uint32_t ulHartId = 0;

    current_set_ticks = ticks;
    systick_callback = interruptfun;

    __asm volatile("csrr %0, mhartid"
                   : "=r"(ulHartId));

    do {
        ulCurrentTimeHigh = *pulTimeHigh;
        ulCurrentTimeLow = *pulTimeLow;
    } while (ulCurrentTimeHigh != *pulTimeHigh);

    next_compare_tick = (uint64_t)ulCurrentTimeHigh;
    next_compare_tick <<= 32ULL;
    next_compare_tick |= (uint64_t)ulCurrentTimeLow;
    next_compare_tick += (uint64_t)current_set_ticks;

    *(volatile uint64_t *)(CLIC_CTRL_ADDR + CLIC_MTIMECMP) = next_compare_tick;

    /* Prepare the time to use after the next tick interrupt. */
    next_compare_tick += (uint64_t)current_set_ticks;

    Interrupt_Handler_Register(MTIME_IRQn, Systick_Handler);
    CPU_Interrupt_Enable(MTIME_IRQn);
}

/**
 * @brief
 *
 * @return uint64_t
 */
uint64_t mtimer_get_time_ms()
{
    return CPU_Get_MTimer_MS();
}
/**
 * @brief
 *
 * @return uint64_t
 */
uint64_t mtimer_get_time_us()
{
    return CPU_Get_MTimer_US();
}
/**
 * @brief
 *
 * @param time
 */
void mtimer_delay_ms(uint32_t time)
{
    CPU_MTimer_Delay_MS(time);
}
/**
 * @brief
 *
 * @param time
 */
void mtimer_delay_us(uint32_t time)
{
    CPU_MTimer_Delay_US(time);
}
