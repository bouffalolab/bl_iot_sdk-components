#include <stdio.h>

#include <bl616.h>
#include <bl616_glb.h>
#include <bflb_sec_trng.h>

#include <openthread_port.h>

#include <openthread/platform/memory.h>
#include <openthread/platform/entropy.h>
#include <openthread/platform/misc.h>

void otPlatReset(otInstance *aInstance) 
{
    __asm volatile( "csrc mstatus, 8" );
    GLB_SW_POR_Reset();
}

otPlatResetReason otPlatGetResetReason(otInstance *aInstance)
{
    return OT_PLAT_RESET_REASON_UNKNOWN;
}

void otPlatAssertFail(const char *aFilename, int aLineNumber)
{
    printf("otPlatAssertFail, %s @ %d\r\n", aFilename, aLineNumber);
    configASSERT(false);
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
