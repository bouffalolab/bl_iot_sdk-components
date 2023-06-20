#ifndef OPENTHREAD_PORT_H
#define OPENTHREAD_PORT_H

#include <openthread/instance.h>
#include <openthread/platform/radio.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef OT_TASK_SIZE
#if defined OPENTHREAD_BORDER_ROUTER
#define OT_TASK_SIZE (1024 + 512)
#elif defined CFG_OPENTHREAD_TESTS_UNIT
#define OT_TASK_SIZE (1024 * 2)
#else
#define OT_TASK_SIZE 1024
#endif
#endif

#ifndef OT_TASK_PRORITY
#define OT_TASK_PRORITY 20
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

#ifdef BL702L
    OT_SYSTEM_EVENT_POLL                                = 0x00010000,
    OT_SYSTEM_EVENT_POLL_DATA_TIMEOUT                   = 0x00020000,
    OT_SYSTEM_EVENT_FULL_STACK                          = 0x00040000,
    OT_SYSTEM_EVENT_RESET_NEXT_POLL                     = 0x00080000,
    OT_SYSTEM_EVENT_MAC_TX_RETRY                        = 0x00100000,
    OT_SYSTEM_EVENT_CSL_TIMER                           = 0x00200000,
#endif
    
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

/****************************************************************************//**
 * @brief  Init openthread tack.
 *
 * @param  None
 *
 * @return None
 *
*******************************************************************************/
void otrStackInit(void);

/****************************************************************************//**
 * @brief  Start OpenThread task.
 *
 * @param  None
 *
 * @return None
 *
*******************************************************************************/
void otrStart(otRadio_opt_t opt);

/****************************************************************************//**
 * @brief  Initializes user code with OpenThread related before OpenThread 
 *          main event loop execution. This function is called after
 *          Openthread instance created and by OpenThread task.
 *
 * @param  None
 *
 * @return None
 *
*******************************************************************************/
void otrInitUser(otInstance * instance);

/****************************************************************************//**
 * @brief  Handle UART task.
 *
 * @param  None
 *
 * @return None
 *
*******************************************************************************/
void ot_uartTask (ot_system_event_t sevent);

/****************************************************************************//**
 * @brief  A wrapper to call otPlatUartReceived.
 *
 * @param  None
 *
 * @return None
 *
*******************************************************************************/
void ot_uartRecieved(uint8_t * rxbuf, uint32_t rxlen);
void ot_uartSetFd(int fd);
void ot_uartLog(const char *fmt, va_list argp);

void ot_alarmInit(void);

/****************************************************************************//**
 * @brief  Handle alarm timer task.
 *
 * @param  None
 *
 * @return None
 *
*******************************************************************************/
void ot_alarmTask(ot_system_event_t sevent);

/****************************************************************************//**
 * @brief  Init radio.
 *
 * @param  opt, radio work optional
 *
 * @return None
 *
*******************************************************************************/
void ot_radioInit(otRadio_opt_t opt);

/****************************************************************************//**
 * @brief  Handle radio task.
 *
 * @param  None
 *
 * @return None
 *
*******************************************************************************/
void ot_radioTask(ot_system_event_t trxEvent);

/****************************************************************************//**
 * @brief  init for openthread cli command.
 *
 * @param  None
 *
 * @return None
 *
*******************************************************************************/
extern void otAppCliInit(otInstance *aInstance);

/****************************************************************************//**
 * @brief  init for openthread ncp command.
 *
 * @param  None
 *
 * @return None
 *
*******************************************************************************/
extern void otAppNcpInit(otInstance *aInstance);

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


uint32_t otrEnterCrit(void);
void otrExitCrit(uint32_t tag);
void otrNotifyEvent(ot_system_event_t sevent);
ot_system_event_t otrGetNotifyEvent(void);

bool ot_radioIdle(void);

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
#define OT_APP_NOTIFY_ISR(ebit)             otrNotifyEvent(ebit & OT_SYSTEM_EVENT_APP)


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
#define OT_APP_NOTIFY(ebit)                 otrNotifyEvent(ebit & OT_SYSTEM_EVENT_APP)

#ifdef __cplusplus
}
#endif

#endif // OPENTHREAD_PORT_H
