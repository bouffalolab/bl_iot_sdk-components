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
#ifndef __OPENTHREAD_BR_H__
#define __OPENTHREAD_BR_H__

#include "otbr_err.h"

#define VERSION_OT_BR_MAJOR 1
#define VERSION_OT_BR_MINOR 6
#define VERSION_OT_BR_PATCH 20

// #define VERSION_OT_BR_EXTRA_INFO "customer-1"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*statChangedCallback_t)(otChangedFlags);
typedef void (*otbr_action_cb_t)(uint32_t arg);

struct netif * otbr_getInfraNetif(void);
struct netif * otbr_getThreadNetif(void);

void otbr_instance_init(void * aBackboneNetif);
void otbr_nat64_init(char *nat64Cidr);
void otbr_instance_routing_init(void);
void otbrInstance_addStateChangedCallback(statChangedCallback_t callback);

void otbr_netif_init(void);
err_t otbr_netif_output6_forward(uint8_t *aBuffer, uint32_t aLength);

bool otbr_netif_request_action(otbr_action_cb_t cb, uint32_t arg);
bool otbr_netif_request_frame_handle(otbr_action_cb_t cb, struct pbuf * p);

#ifdef __cplusplus
}
#endif

#endif /* __OPENTHREAD_BR_H__ */
