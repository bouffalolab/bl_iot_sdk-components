/*
 *  Copyright (c) 2019, The OpenThread Authors.
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

/**
 * @file
 *   This file includes interface definitions for FreeRTOS.
 *
 */

#ifndef OPENTHREAD_PORT_H
#define OPENTHREAD_PORT_H

#include <FreeRTOS.h>
#include <portmacro.h>
#include <task.h>

#include <openthread/instance.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef OT_TASK_SIZE
#define OT_TASK_SIZE 1024
#endif

#ifndef OT_UART_RX_BUFFSIZE
#define OT_UART_RX_BUFFSIZE 256
#endif


typedef union {
    struct {
        uint16_t isCoexEnable:1;
        uint16_t isFtd:1;
        uint16_t isLinkMetricEnable:1;
        uint16_t isCSLReceiverEnable:1;
        uint16_t isTimeSyncEnable:1;
        uint16_t isImmAck4Error:1;
        uint16_t isCslPhaseUpdated:1;
        uint16_t isTxTimestampValid:1;
        uint16_t unused:8;
    } bf;
    uint16_t byte;
} __packed otRadio_opt_t;


typedef enum _ot_system_event {
    OT_SYSTEM_EVENT_NONE                                = 0,

    OT_SYSTEM_EVENT_OT_TASKLET                          = 0x00000001,

    OT_SYSTEM_EVENT_ALARM_MS_EXPIRED                    = 0x00000002,
    OT_SYSTEM_EVENT_ALARM_US_EXPIRED                    = 0x00000004,
    OT_SYSTEM_EVENT_ALARM_ALL_MASK                      = OT_SYSTEM_EVENT_ALARM_MS_EXPIRED | OT_SYSTEM_EVENT_ALARM_US_EXPIRED,

    OT_SYSTEM_EVENT_UART_TXR                            = 0x00000010,
    OT_SYSTEM_EVENT_UART_TXD                            = 0x00000020,
    OT_SYSTEM_EVENT_UART_RXD                            = 0x00000040,
    OT_SYSETM_EVENT_UART_ALL_MASK                       = OT_SYSTEM_EVENT_UART_TXR | OT_SYSTEM_EVENT_UART_TXD | OT_SYSTEM_EVENT_UART_RXD,

    OT_SYSTEM_EVENT_RADIO_TX_DONE_NO_ACK_REQ            = 0x00000100,
    OT_SYSTEM_EVENT_RADIO_TX_ERROR                      = 0x00000200,
    OT_SYSTEM_EVENT_RADIO_TX_ACKED                      = 0x00000400,
    OT_SYSTEM_EVENT_RADIO_TX_NO_ACK                     = 0x00000800,
    OT_SYSTEM_EVENT_RADIO_TX_ABORT                      = 0x00001000,
    OT_SYSTEM_EVENT_RADIO_TX_ALL_MASK                   = OT_SYSTEM_EVENT_RADIO_TX_DONE_NO_ACK_REQ | 
        OT_SYSTEM_EVENT_RADIO_TX_ERROR | OT_SYSTEM_EVENT_RADIO_TX_ACKED | OT_SYSTEM_EVENT_RADIO_TX_NO_ACK | OT_SYSTEM_EVENT_RADIO_TX_ABORT,

    OT_SYSTEM_EVENT_RADIO_RX_DONE                       = 0x00002000,
    OT_SYSTEM_EVENT_RADIO_RX_CRC_FIALED                 = 0x00004000,
    OT_SYSTEM_EVENT_RADIO_RX_NO_BUFF                    = 0x00008000,
    OT_SYSTEM_EVENT_RADIO_RX_ALL_MASK                   = OT_SYSTEM_EVENT_RADIO_RX_NO_BUFF | 
        OT_SYSTEM_EVENT_RADIO_RX_DONE | OT_SYSTEM_EVENT_RADIO_RX_CRC_FIALED,
    OT_SYSTEM_EVENT_RADIO_ALL_MASK                      = OT_SYSTEM_EVENT_RADIO_TX_ALL_MASK | OT_SYSTEM_EVENT_RADIO_RX_ALL_MASK,


    OT_SYSTEM_EVENT_POLL                                = 0x00010000,
    OT_SYSTEM_EVENT_POLL_DATA_TIMEOUT                   = 0x00020000,
    OT_SYSTEM_EVENT_FULL_STACK                          = 0x00040000,
    OT_SYSTEM_EVENT_RESET_NEXT_POLL                     = 0x00080000,
    OT_SYSTEM_EVENT_MAC_TX_RETRY                        = 0x00100000,
    OT_SYSTEM_EVENT_CSL_TIMER                           = 0x00200000,

    OT_SYSTEM_EVENT_APP                                 = 0xff000000,

    OT_SYSTEM_EVENT_ALL                                 = 0xffffffff,
} ot_system_event_t;

extern ot_system_event_t ot_system_event_var;


/****************************************************************************//**
 * @brief  Get current OpenThread instance.
 *
 * @param  None
 *
 * @return otInstance instance
 *
*******************************************************************************/
otInstance *otrGetInstance();

void ot_alarmInit(otInstance *aInstance);

/****************************************************************************//**
 * @brief  Give semphore to OpenThread task
 *
 * @param  None
 *
 * @return None
 *
*******************************************************************************/
void otSysEventSignalPending(void);

/****************************************************************************//**
 * @brief  Lock OpenThread task to acquire ciritcal section access
 *
 * @param  None
 *
 * @return None
 *
*******************************************************************************/
void otrLock(void);

/****************************************************************************//**
 * @brief  Unlock OpenThread task to release ciritcal section access
 *
 * @param  None
 *
 * @return None
 *
*******************************************************************************/
void otrUnlock(void);

typedef enum {
    OT_PDS_SLEEP_OP_PRE_APP                 = 0x01,
    OT_PDS_SLEEP_OP_PRE_OT                  = 0x02,
    OT_PDS_SLEEP_OP_PRE_BLE                 = 0x03,

    OT_PDS_SLEEP_OP_APP_END                 = 0x11,
    OT_PDS_SLEEP_OP_OT_END                  = 0x12,
    OT_PDS_SLEEP_OP_BLE_END                 = 0x13,

    OT_PDS_WAKEUP_OT_BLE                    = 0x21,
    OT_PDS_WAKEUP_OP_OT                     = 0x22,
    OT_PDS_WAKEUP_OP_APP                    = 0x23,

} otPds_op_t;

typedef uint32_t (*otPds_sleep_opCb_t)(otPds_op_t pdsOp, uint32_t sleepCycle, int isWakeupFromRtc);


uint32_t otPds_getExpectSleepTime(void);
uint32_t otPds_getSleepCycle(void);

uint32_t otPds_preparePdsStackSleep(void);
bool otPds_sleep(void);
void otPds_restoreLmac154Op(void);
void otPds_setSlept(bool isSlept);
bool otPds_isSlept(void);
void otPds_sleepOp(void);
void otPds_freertosTimerPrvHandle(void *pvTimerID, void * pxCallbackFunction);

uint32_t otPds_getExpectedPollingTime(void);

void otPds_restoreOp(void);

bool otPds_isFullStackRunning();
void otPds_vApplicationSleep( TickType_t xExpectedIdleTime );

uint32_t otPds_getCslRxOnTime(void);


uint32_t otrEnterCrit(void);
void otrExitCrit(uint32_t tag);
void otrNotifyEvent(ot_system_event_t sevent);
ot_system_event_t otrGetNotifyEvent(void);
bool otrIsEventPending(void);

/****************************************************************************//**
 * @brief  Macro OT_THREAD_SAFE provides a method to access OpenThread with 
 * thread-safe in other tasks running context.
 * 
 * ot_initUser and OpenThread callback functions already are thread-safe protected.
 * 
 * Note, do NOT call return in this macro block.
 * 
 * @param ...  OpenThread api call statement
 *
*******************************************************************************/
#define OT_THREAD_SAFE(...)                 \
    otrLock();                              \
    do                                      \
    {                                       \
        __VA_ARGS__;                        \
    } while (0);                            \
    otrUnlock();                            

/****************************************************************************//**
 * @brief  Macro OT_THREAD_SAFE_RET provides a method to access OpenThread with 
 * thread-safe in other tasks running context.
 * 
 * ot_initUser and OpenThread callback functions already are thread-safe protected.
 * 
 * Note, do NOT call return in this macro block.
 *
 * @param ret   return value
 * @param ...   OpenThread api call statement
 *
*******************************************************************************/
#define OT_THREAD_SAFE_RET(ret, ...)        \
    do                                      \
    {                                       \
        otrLock();                          \
        (ret) = __VA_ARGS__;                \
        otrUnlock();                        \
    } while (0)                             


/****************************************************************************//**
 * @brief  An weak function explore to applicaton layer to execute some application code.
 *          Note,
 *              1, this function is running in openthread main task, it is thread safed and
 *              it doesn't need OT_THREAD_SAFE/OT_THREAD_SAFE_RET protection.
 *              2, as it running with main task event let and radio/timer driver,
 *                  this function should be implemented very light without too
 *                  much execution time cost and delay or pending. And also do NOT
 *                  suspend and stop this task.
 *                  heavy execution or any delay or pending for some state transition should be
 *                  executed in another task.
 *
 * @param  sevent, event bit called by OT_APP_NOTIFY_ISR/OT_APP_NOTIFY from application code
 *
 * @return None
 *
*******************************************************************************/
void otrAppProcess(ot_system_event_t sevent);


/****************************************************************************//**
 * @brief  An interface to application to invoke openthread task to execute otrAppProcess, 
 *          which is used to call in application interrupt context.
 *
 * @param  ebit, event bit for otrAppProcess. 
 *              please reference to OT_SYSTEM_EVENT_APP for valid bits.
 *
 * @return None
 *
*******************************************************************************/
#define OT_APP_NOTIFY_ISR(ebit)             (ot_system_event_var |= ((ot_system_event_t)ebit & OT_SYSTEM_EVENT_APP)); otSysEventSignalPending()


/****************************************************************************//**
 * @brief  An interface to application to invoke openthread task to execute otrAppProcess, 
 *          which is used to call in application task context.
 *
 * @param  ebit, event bit for otrAppProcess. 
 *              please reference to OT_SYSTEM_EVENT_APP for valid bits.
 *
 * @return None
 *
*******************************************************************************/
#define OT_APP_NOTIFY(ebit)                 OT_ENTER_CRITICAL(); ot_system_event_var |=  ((ot_system_event_t)ebit & OT_SYSTEM_EVENT_APP); OT_EXIT_CRITICAL(); otSysEventSignalPending()

#ifdef __cplusplus
}
#endif

#endif // OPENTHREAD_PORT_H
