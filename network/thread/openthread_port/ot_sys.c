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
#include <stdio.h>
#include <string.h>
#include <bl_sys.h>
#include <bl_sec.h>

#include <openthread_port.h>
#include <openthread/platform/memory.h>
#include <openthread/platform/misc.h>
#include <openthread/platform/entropy.h>

void *otPlatCAlloc(size_t aNum, size_t aSize)
{
    return calloc(aNum, aSize);
}

void otPlatFree(void *aPtr)
{
    free(aPtr);
}

void otPlatReset(otInstance *aInstance) 
{
#if CFG_USE_PSRAM

extern uint8_t _heap_size; // @suppress("Type cannot be resolved")
extern uint8_t _heap2_size; // @suppress("Type cannot be resolved")
extern uint8_t _heap3_size; // @suppress("Type cannot be resolved")
extern unsigned int xPortGetMinimumEverFreeHeapSize( void );
extern unsigned int xPortGetMinimumEverFreeHeapSizePsram( void );

    printf ("otPlatReset sram = %u/%u, psram = %u/%u\r\n", 
        (unsigned int) &_heap_size + (unsigned int) &_heap2_size - xPortGetMinimumEverFreeHeapSize(), 
        (unsigned int) &_heap_size + (unsigned int) &_heap2_size,
        (unsigned int) &_heap3_size - xPortGetMinimumEverFreeHeapSizePsram(),
        (unsigned int) &_heap3_size);

    vTaskDelay(500);
#else
extern uint8_t _heap_size; // @suppress("Type cannot be resolved")
extern uint8_t _heap2_size; // @suppress("Type cannot be resolved")
extern unsigned int xPortGetMinimumEverFreeHeapSize( void );

    printf ("otPlatReset sram = %u/%u\r\n", 
        (unsigned int) &_heap_size + (unsigned int) &_heap2_size - xPortGetMinimumEverFreeHeapSize(), 
        (unsigned int) &_heap_size + (unsigned int) &_heap2_size);
    
    vTaskDelay(500);
#endif
    bl_sys_reset_por();
}

otPlatResetReason otPlatGetResetReason(otInstance *aInstance)
{

    BL_RST_REASON_E rstinfo = bl_sys_rstinfo_get();

    switch (rstinfo) {
        case BL_RST_WDT:
        return OT_PLAT_RESET_REASON_WATCHDOG;
        case BL_RST_BOR:
        return OT_PLAT_RESET_REASON_OTHER;
        case BL_RST_HBN:
        return OT_PLAT_RESET_REASON_EXTERNAL;
        case BL_RST_POR:
        return OT_PLAT_RESET_REASON_EXTERNAL;
        case BL_RST_SOFTWARE:
        return OT_PLAT_RESET_REASON_SOFTWARE;
        default:
        return OT_PLAT_RESET_REASON_UNKNOWN;
    }
}

void otPlatAssertFail(const char *aFilename, int aLineNumber)
{
    printf("otPlatAssertFail, %s @ %d\r\n", aFilename, aLineNumber);
    abort();
}

void otPlatWakeHost(void)
{}

otError otPlatSetMcuPowerState(otInstance *aInstance, otPlatMcuPowerState aState)
{
    return OT_ERROR_NONE;
}
otPlatMcuPowerState otPlatGetMcuPowerState(otInstance *aInstance)
{
    return OT_PLAT_MCU_POWER_STATE_ON;
}

otError otPlatEntropyGet(uint8_t *aOutput, uint16_t aOutputLength) 
{
    bl_rand_stream(aOutput, aOutputLength);
    return OT_ERROR_NONE;
}