#ifndef OPENTHREAD_UTILS_H
#define OPENTHREAD_UTILS_H

/****************************************************************************//**
 * @brief  Enable Radio
 *
 * @param  None
 *
 * @return None
 *
*******************************************************************************/
void ot_radioEnable(void);

/****************************************************************************//**
 * @brief  Check whether radio is idle
 *
 * @param  None
 *
 * @return true is idle
 *
*******************************************************************************/
bool ot_isRadioIdle(void);

/****************************************************************************//**
 * @brief  FTD device address from address mapping
 *
 * @param  addrType, address type to input to find another address
 *                   OTINTERNEL_ADDR_TYPE_SHORT, input pShortAddr, output extAddr
 *                   OTINTERNEL_ADDR_TYPE_EXT, input extAddr, output pShortAddr
 * @param  extAddr,  extended address
 * @param  pShortAddr,  short address
 * 
 * @return 1 for entry of address map found
 *
*******************************************************************************/
uint32_t ot_findAddresses_ftd(uint32_t addrType, uint8_t *extAddr, uint8_t *pShortAddr);

#ifdef BL702L
/****************************************************************************//**
 * @brief  MTD device address from address mapping
 *
 * @param  addrType, address type to input to find another address
 *                   OTINTERNEL_ADDR_TYPE_SHORT, input pShortAddr, output extAddr
 *                   OTINTERNEL_ADDR_TYPE_EXT, input extAddr, output pShortAddr
 * @param  extAddr,  extended address
 * @param  pShortAddr,  short address
 * 
 * @return 1 for entry of address map found
 *
*******************************************************************************/
uint32_t ot_findAddresses_mtd(uint32_t addrType, uint8_t *extAddr, uint8_t *pShortAddr);
#endif

/****************************************************************************//**
 * @brief  enqueue a recieved packet
 *
 * @param  rx_buf, received packet buffer
 * @param  rx_len, received packet length
 * @param  isSecuredAcked, wthether a secured enh-ack acked successfully
 *
 * @return None
 *
*******************************************************************************/
void ot_radioEnqueueRecvPacket(uint32_t * rx_buf, uint32_t rx_len, uint32_t isSecuredAcked);

/****************************************************************************//**
 * @brief  frame send
 *
 * @param  aFrame, a frame to send
 *
 * @return None
 *
*******************************************************************************/
int ot_radioSend(otRadioFrame *aFrame);

#ifdef BL702L

typedef enum {
    /** go through application, openthread and ble to prepare sleep */
    OT_PDS_SLEEP_OP_PRE_APP                 = 0x01,
    OT_PDS_SLEEP_OP_PRE_OT                  = 0x02,
    OT_PDS_SLEEP_OP_PRE_BLE                 = 0x03,

    /** go through application, openthread and ble to do sleep */
    OT_PDS_SLEEP_OP_APP_END                 = 0x11,
    OT_PDS_SLEEP_OP_OT_END                  = 0x12,
    OT_PDS_SLEEP_OP_BLE_END                 = 0x13,

    /** go through BLE, openthread and application after wakeup */
    OT_PDS_WAKEUP_OT_BLE                    = 0x21,
    OT_PDS_WAKEUP_OP_OT                     = 0x22,
    OT_PDS_WAKEUP_OP_APP                    = 0x23,
} otPds_op_t;

/****************************************************************************//**
 * @brief  sleep & wakup callback
 *
 * @param  pdsOp, sleep & wakeup action
 * @param  sleepCycle, expect time to sleep at OT_PDS_SLEEP_x actions,
 *                     actual sleep time at OT_PDS_WAKEUP_x actions
 * @param  isWakeupFromRtc, whether to wakeup from RTC at OT_PDS_WAKEUP_x actions
 * 
 * @return None
 *
*******************************************************************************/
typedef uint32_t (*otPds_sleep_opCb_t)(otPds_op_t pdsOp, uint32_t sleepCycle, int isWakeupFromRtc);

/****************************************************************************//**
 * @brief  initialize thread PDS module
 *
 * @param  callback, callback for sleep & wakeup operations
 *
 * @return None
 *
*******************************************************************************/
void otPds_init(otPds_sleep_opCb_t callback);

/****************************************************************************//**
 * @brief  initialize child supervision listener running in Bouffalo Lab PDS mode
 *
 * @param  childSupervisionCheckTimeout,
 *         refer to OPENTHREAD_CONFIG_CHILD_SUPERVISION_CHECK_TIMEOUT
 *
 * @return None
 *
*******************************************************************************/
#if OPENTHREAD_CONFIG_CHILD_SUPERVISION_ENABLE
void otPds_supervisionListener_init(uint32_t childSupervisionCheckTimeout);
#endif

/****************************************************************************//**
 * @brief  initialize CSl receiver module running in Bouffalo Lab PDS mode
 *
 * @param  cslReceiveAhead, ahead time to start receive for CSl receiver
 *         refer to OPENTHREAD_CONFIG_CSL_RECEIVE_TIME_AHEAD
 * @param  cslMinWindow, minimal receiving window
 *         refer to OPENTHREAD_CONFIG_CSL_MIN_RECEIVE_ON
 * @return None
 *
*******************************************************************************/
#if OPENTHREAD_CONFIG_MAC_CSL_RECEIVER_ENABLE
void otPds_cslReceiver_init(uint32_t cslReceiveAhead, uint32_t cslMinWindow);
#endif

/****************************************************************************//**
 * @brief  set delay to retry running in Bouffalo Lab PDS mode
 *
 * @param  macRetryBackoffMinExp, minimal exponent
 *         refer to OPENTHREAD_CONFIG_MAC_RETX_DELAY_MIN_BACKOFF_EXPONENT
 * @param  macRetryBackoffMaxExp, maximal exponent
 *         refer to OPENTHREAD_CONFIG_MAC_RETX_DELAY_MAX_BACKOFF_EXPONENT
 * @return None
 *
*******************************************************************************/
#if OPENTHREAD_CONFIG_MAC_ADD_DELAY_ON_NO_ACK_ERROR_BEFORE_RETRY
void otPds_setDelayBeforeRetry(uint32_t macRetryBackoffMinExp, uint32_t macRetryBackoffMaxExp);
#endif

/****************************************************************************//**
 * @brief  Active state wakeup from Bouffalo Lab Thread PDS has two states
 *         Full stack state, flash is powered on and all code, such as full 
 *         openthread stack and application code, is available access. 
 *         PDS stack state, flash is not powered on, any code on flash is not
 *         available access; this state is responsible for openthread routine
 *         behaviors, such as data polling and CSL receiving.
 * 
 *         otPds_prepareFullStackSleep needs be called when system goes to sleep
 *         from Full stack state.
 * 
 * @param  None
 * 
 * @return time to expect to sleep
 *
*******************************************************************************/
uint32_t otPds_prepareFullStackSleep(void);

/****************************************************************************//**
 * @brief  otPds_prepareFullStackSleep needs be called when system goes to sleep
 *         from PDS stack state.
 * 
 * @param  None
 * 
 * @return time to expect to sleep
 *
*******************************************************************************/
uint32_t otPds_preparePdsStackSleep(void);

/****************************************************************************//**
 * @brief  Check whether full stack state is running
 *
 * @param  None
 * 
 * @return true, full stack is running;
 *
*******************************************************************************/
bool otPds_isFullStackRunning();

/****************************************************************************//**
 * @brief  save PDS task information. Should be called after Tmr Svc and blecontroller
 *          are created
 *
 * @param  None
 * 
 * @return None
 *
*******************************************************************************/
void otPds_savePdsTaskInfo(void);

/****************************************************************************//**
 * @brief  Get sleep cycles
 *
 * @param  None
 * 
 * @return sleep cycles
 *
*******************************************************************************/
uint32_t otPds_getSleepCycle(void);

/****************************************************************************//**
 * @brief  Do sleep
 *
 * @param  None
 * 
 * @return true, it ok to sleep
 *
*******************************************************************************/
bool otPds_sleep(void);

/****************************************************************************//**
 * @brief  restore lmac 154 state and memory
 *
 * @param  None
 * 
 * @return None
 *
*******************************************************************************/
void otPds_restoreLmac154Op(void);

/****************************************************************************//**
 * @brief  pass openthread PDS module whether system is slept OT_PDS_WAKEUP_x actions 
 *
 * @param  isSlept, it slept before
 * 
 * @return None
 *
*******************************************************************************/
void otPds_setSlept(bool isSlept);

/****************************************************************************//**
 * @brief  Return whether system slept before which saved by otPds_setSlept
 *
 * @param  None
 * 
 * @return True if it is slept before
 *
*******************************************************************************/
bool otPds_isSlept(void);

/****************************************************************************//**
 * @brief  Openthread sleep operation at OT_PDS_SLEEP_OP_OT_END
 *
 * @param  None
 * 
 * @return None
 *
*******************************************************************************/
void otPds_sleepOp(void);

/****************************************************************************//**
 * @brief  Enable do CSMA when sending data poll command during PDS stack state 
 *
 * @param  isEnableCsma, true to do CSMA to send data polling; otherwise, not
 * 
 * @return None
 *
*******************************************************************************/
void otPds_setDataPollCsma(bool isEnableCsma);

void otPds_handleWakeup(bool isWakeupFromRtc);

/****************************************************************************//**
 * @brief  Do utils to initialize 
 *
 * @param  None
 * 
 * @return None
 *
*******************************************************************************/
void ot_utils_init(void);
#endif
#endif