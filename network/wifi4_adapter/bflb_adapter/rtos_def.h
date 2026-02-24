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

#ifndef __RTOS_DEF_H__
#define __RTOS_DEF_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * Type by which tasks are referenced.
 */
typedef void *        rtos_task_t;



typedef void *        rtos_task_handle;

/// RTOS priority
typedef uint32_t      rtos_prio;

/// RTOS task function
typedef void *        rtos_task_fct;

/// RTOS queue
typedef void *        rtos_queue;

/// RTOS semaphore
typedef void *        rtos_semaphore;

/// RTOS mutex
typedef void *        rtos_mutex;


#define g_tskIDLE_PRIORITY 0
#define RTOS_TASK_PRIORITY(prio)  (g_tskIDLE_PRIORITY + (prio))


/// Definitions of the different FHOST task priorities

/// Priority of the tcpip task
extern const int fhost_tcpip_priority;
/// Priority of the WiFi task
extern const int fhost_wifi_priority;
/// Priority of the WiFi task (when high priority is set)
extern const int fhost_wifi_priority_high;
/// Priority of the control task
extern const int fhost_cntrl_priority;
/// Priority of the RX task
extern const int fhost_rx_priority;
/// Priority of the TX task
extern const int fhost_tx_priority;
/// Priority of the WPA task
extern const int fhost_wpa_priority;
/// Priority of the IPC task
extern const int fhost_ipc_priority;
/// Priority of the IPERF task
extern const int fhost_iperf_priority;
/// Priority of the CONNECT task
extern const int fhost_connect_priority;

extern const int fhost_tg_priority;

extern const int fhost_ping_priority;

/// Definitions of the different FHOST task stack size requirements
#define WPA_SMALL_STACK_SIZE   1408
#define WPA_NORMAL_STACK_SIZE  1584

#ifdef WPA_SUPPLICANT_USE_NORMAL_STACK
#define WPA_SUPPLICANT_STACK_SIZE   WPA_NORMAL_STACK_SIZE 
#else
#define WPA_SUPPLICANT_STACK_SIZE   WPA_SMALL_STACK_SIZE 
#endif

enum
{
    /// WiFi task stack size
    FHOST_WIFI_STACK_SIZE = 512,
    /// Control task stack size
    FHOST_CNTRL_STACK_SIZE = 512,
    /// TCP/IP task stack size
    FHOST_TCPIP_STACK_SIZE = 1024,
    /// RX task stack size
    FHOST_RX_STACK_SIZE = 768,
    /// TX task stack size
    FHOST_TX_STACK_SIZE = 384,
    /// WPA task stack size
    FHOST_WPA_STACK_SIZE = WPA_SUPPLICANT_STACK_SIZE,
    /// IPC task stack size
    FHOST_IPC_STACK_SIZE = 512,
    /// TG send stack size
    FHOST_TG_SEND_STACK_SIZE = 1024,
    /// Ping send stack size
    FHOST_PING_SEND_STACK_SIZE = 256,
    /// Smartconfig task stack size
    FHOST_SMARTCONF_STACK_SIZE = 512,
    /// IPERF task stack size
    FHOST_IPERF_STACK_SIZE = 1024,
    /// DOORBELL task stack size
    FHOST_DOORBELL_STACK_SIZE = 1024,
};

#define RTOS_portTASK_FUNCTION(vFunction, pvParameters) void vFunction(void* pvParameters)
#define RTOS_TASK_FCT(name)        RTOS_portTASK_FUNCTION(name, env)

#define RTOS_TASK_NULL             NULL

#endif
