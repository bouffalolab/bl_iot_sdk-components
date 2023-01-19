/*
 *  Copyright (c) 2016, The OpenThread Authors.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

#include "test_platform.h"

#include <stdio.h>
#include <sys/time.h>

enum
{
    FLASH_SWAP_SIZE = 2048,
    FLASH_SWAP_NUM  = 2,
};

static uint8_t sFlash[FLASH_SWAP_SIZE * FLASH_SWAP_NUM];

ot::Instance *testInitInstance(void)
{
    otInstance *instance = nullptr;

#if OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
    size_t   instanceBufferLength = 0;
    uint8_t *instanceBuffer       = nullptr;

    // Call to query the buffer size
    (void)otInstanceInit(nullptr, &instanceBufferLength);

    // Call to allocate the buffer
    instanceBuffer = (uint8_t *)malloc(instanceBufferLength);
    VerifyOrQuit(instanceBuffer != nullptr, "Failed to allocate otInstance");
    memset(instanceBuffer, 0, instanceBufferLength);

    // Initialize OpenThread with the buffer
    instance = otInstanceInit(instanceBuffer, &instanceBufferLength);
#else
    instance = otInstanceInitSingle();
#endif

    return static_cast<ot::Instance *>(instance);
}

void testFreeInstance(otInstance *aInstance)
{
    otInstanceFinalize(aInstance);

#if OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
    free(aInstance);
#endif
}

bool sDiagMode = false;
