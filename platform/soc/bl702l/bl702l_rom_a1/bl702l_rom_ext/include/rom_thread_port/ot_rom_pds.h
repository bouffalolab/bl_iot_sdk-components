
#ifndef __OT_ROM_PDS_H__
#define __OT_ROM_PDS_H__

#ifdef __cplusplus
extern "C" {
#endif

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

typedef void (*otPds_otherEventCb_t)(ot_system_event_t);
typedef void (*otPds_receiveAtFunc_t)(uint8_t, uint32_t, uint32_t);
typedef void (*otPds_lockFunc_t)(bool);
typedef uint32_t (*otPds_sleep_opCb_t)(otPds_op_t pdsOp, uint32_t sleepCycle, int isWakeupFromRtc);

typedef struct _otRadio_otTimer_t {

    uint32_t                        *pHandler;
    uint32_t                        fireTime;
    struct _otRadio_otTimer_t       *next;
} otRadio_otTimer_t;


typedef struct _otRadio_cnt_t {
    uint16_t                        txCnt;
    uint16_t                        txCntAcked;
    uint16_t                        txCntNoAcked;
    uint16_t                        txCntCcaFailured;
    uint16_t                        txCntAbort;
    uint16_t                        txCntRetires;
    uint16_t                        rxCnt;                  /** received frames */
    uint16_t                        rxCntPoll;              /** count to turn on rx for polling */
    uint16_t                        rxCntCslRx;             /** count to turn on rx for csl receiving */
} otRadio_cnt_t;

typedef struct _otRadio_pds_t {
    uint32_t                        isAttaching:1;
    uint32_t                        isRetx:1;
    uint32_t                        isCslEIExist:1;
    uint32_t                        isCslActive:1;
    uint32_t                        isSupervisionEnabled:1;
    uint32_t                        isWakeupForMilliTimer:1;
    uint32_t                        isWakeupForMicroTimer:1;
    uint32_t                        isPollingError:1;
    uint32_t                        isCutoffSleepOpCost:1;
    uint32_t                        isFullStackRequested:1;
    uint32_t                        isAlwaysRestoreFullStack:1;
    uint32_t                        isSlept:1;
    uint32_t                        isHoldLock:1;
    uint32_t                        isDebugSleepEnable:1;
    uint32_t                        isDisableDataPollingCsma:1;
    uint32_t                        randomValue;

    struct otExtAddress             parentExtAddress;
    uint16_t                        parentShortAddress;

    uint16_t                        pdsToleranceCycle;
    uint8_t                         pdsMinCycle;
    uint8_t                         pdsLevel;

    uint16_t                        pdsToleranceCut;
    uint8_t                         pdsPreTaskDelayGuide;
    uint8_t                         pdsPostTaskDelayGuide;
    
    uint8_t                         channel;
    uint8_t                         seqNum;

    uint8_t                         macRetryBackoffPeriod;
    uint8_t                         macRetryBackoffMinExp;
    uint8_t                         macRetryBackoffMaxExp;
    uint8_t                         macRetryBackoffExp;
    uint8_t                         macRetries;

    uint8_t                         failedRouterTransmissions;          /** kFailedRouterTransmissions */
    uint8_t                         pollMaxRetxAttempts;                /** kMaxPollRetxAttempts */

    uint8_t                         linkFailureCnt;
    uint8_t                         pollFailureCnt;

    /** back-to-back polling should not be happened on switch between none-flash and flash pds mode;
     *  back-to-back polling will make system busy till data polled or till reach pollMaxQuick */
    uint8_t                         pollTimeoutCnt;                     /** mPollTimeoutCounter */
    uint8_t                         pollMaxQuick;                       /** kQuickPollsAfterTimeout, back-to-back poll, parent doesn't send data in wait data timeout */

    uint8_t                         pollFastRemaining;                  /** mRemainingFastPolls */
    uint32_t                        pollPeriodFast;                     /** kFastPollPeriod */
    uint32_t                        pollPeriodAttach;                   /** kAttachDataPollPeriod */
    uint32_t                        pollPeriodRetx;                     /** kRetxPollPeriod */
    uint32_t                        pollPeriodMin;                      /** kMinPollPeriod */
    uint32_t                        pollPeriodDefault;                  /** mini between kMaxExternalPeriod and GetDefaultPollPeriod() */
    uint32_t                        pollPeriodCurrent;
    uint32_t                        pollDataTimeout;                    /** kDataPollTimeout */

    uint32_t                        cslLastSync;
    uint32_t                        cslPeriod;                          /** otRadioVar_ptr->cslPeriod only valid if node is attached */
    uint8_t                         cslChannel;
    uint8_t                         cslState;
    uint8_t                         cslParentAccuracy;
    uint8_t                         cslParentUncertainty;
    uint32_t                        cslReceiveAhead;
    uint32_t                        cslMinWindow;
    uint8_t                         cslAccuracy;
    uint8_t                         cslFailedDataPollTransmissions;     /** kFailedCslDataPollTransmissions */
    uint8_t                         cslMaxPollRetxAttempts;             /** kMaxCslPollRetxAttempts */

    int16_t                         cslDiffTimeUs;                      /** diff between sample time and frame received timestamp; negative, received before sample point */
    int16_t                         cslSampleAdjust;
    uint16_t                        cslSampleAdjustValidRegion;
    uint16_t                        cslRxOnTime;
    uint16_t                        supervisionTimeout;

    uint32_t                        *pollingTimerHandlerAddr;
    uint32_t                        *supervisionTimerHandlerAddr;
    uint32_t                        *cslTimerHandlerAddr;
    uint32_t                        *submacTimerHandlerAddr;

    uint32_t                        *pMilliTimerHead;
    uint32_t                        *pMicroTimerHead;

    otRadioFrame                    *pPollFrame;

    otPds_sleep_opCb_t              pdsSleepOpCallback;
    otPds_otherEventCb_t            pdsOtherEventCallback;
    otPds_receiveAtFunc_t           pdsRecvAtFunc;
    otPds_lockFunc_t                pdsLockFunc;
    otRadio_cnt_t                   cnt;

    uint32_t                        expectSleepCycle;
    uint32_t                        *pRegisters;

    uint32_t                        *safeTaskList[5];
} otRadio_pds_t;

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

bool otrIsEventPending(void);

#ifdef __cplusplus
}
#endif

#endif