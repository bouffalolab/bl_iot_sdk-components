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

#include CHIP_HDR
#include CHIP_GLB_HDR
#include CHIP_SYS_HDR
#include <bflb_sec_trng.h>

#include <bl_sys.h>

#include <openthread_port.h>

#include <openthread/platform/memory.h>
#include <openthread/platform/entropy.h>
#include <openthread/platform/misc.h>


void otPlatReset(otInstance *aInstance) 
{
    bl_sys_reset_system();
}

otPlatResetReason otPlatGetResetReason(otInstance *aInstance)
{
    BL_RST_REASON_E rstinfo = bl_sys_rstinfo_get();

    switch (rstinfo) {
        case BL_RST_HARDWARE_WATCHDOG:
        return OT_PLAT_RESET_REASON_WATCHDOG;
        case BL_RST_BOD:
        return OT_PLAT_RESET_REASON_OTHER;
        case BL_RST_HBN:
        return OT_PLAT_RESET_REASON_EXTERNAL;
        case BL_RST_POWER_OFF:
        return OT_PLAT_RESET_REASON_EXTERNAL;
        case BL_RST_SOFTWARE:
        return OT_PLAT_RESET_REASON_SOFTWARE;
        default:
        return OT_PLAT_RESET_REASON_UNKNOWN;
    }

    return OT_PLAT_RESET_REASON_UNKNOWN;
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

void *otPlatCAlloc(size_t aNum, size_t aSize)
{
    return calloc(aNum, aSize);
}

void otPlatFree(void *aPtr)
{
    free(aPtr);
}

otError otPlatEntropyGet(uint8_t *aOutput, uint16_t aOutputLength) 
{
    bflb_trng_readlen(aOutput, aOutputLength);
    return OT_ERROR_NONE;
}
