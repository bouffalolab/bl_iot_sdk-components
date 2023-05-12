#ifndef __OT_INTERNEL_H__
#define __OT_INTERNEL_H__



#define LMAC154_ALARM_MICRO_TIMER_ID        (1)
#define LMAC154_ALAMR_PDS_TIMER_ID          (0)

#define OTRADIO_MAX_PSDU                    (128)

#define ALIGNED_RX_FRAME_SIZE               ((sizeof(otRadio_rxFrame_t) + 3) & 0xfffffffc)
#define TOTAL_RX_FRAME_SIZE                 (ALIGNED_RX_FRAME_SIZE + OTRADIO_MAX_PSDU)
#define MAX_ACK_FRAME_SIZE                  64

#define VENDOR_IE_HEADER_ID                 0
#define VENDOR_IE_THREAD_OUI                0xeab89b
#define VENDOR_IE_THREAD_ENH_ACK_PROBING    0
#define VENDOR_IE_NEST_OUI                  0x18b430
#define VENDOR_IE_NEST_TIME                 1
#define IE_TERINATION2_ID                   0x7f
#define IE_CSL_ID                           0x1a

#define OT_PDS_10_SYMBOL_US             (LMAC154_US_PER_SYMBOL * 10)
#define OT_PDS_CSL_STATE_IDLE           0
#define OT_PDS_CSL_STATE_SAMPLE         1
#define OT_PDS_CSL_STATE_SLEEP          2


#define OT_ASSERT(x) if ((x) == 0) vAssertCalled()

typedef enum {
    OTINTERNEL_ADDR_TYPE_NONE       = 0,
    OTINTERNEL_ADDR_TYPE_SHORT      = 1,
    OTINTERNEL_ADDR_TYPE_EXT        = 2,
} otInternel_addrType_t;

typedef struct _otRadio_rxFrame_t {
    utils_dlist_t       dlist;
    otRadioFrame        frame;
} otRadio_rxFrame_t;

typedef struct _otRadio_frameControl {
    uint16_t frameType:3;
    uint16_t securityEnable:1;
    uint16_t framePending:1;
    uint16_t ackRequest:1;
    uint16_t panidCompression:1;
    uint16_t reserved:1;
    uint16_t seqNumberSup:1;
    uint16_t iePresent:1;
    uint16_t dstAddressMode:2;
    uint16_t frameVersion:2;
    uint16_t srcAddressMode:2;
} otRadio_frameControl_t;

typedef struct _otRadio_auxSecHdr {
    uint8_t  secLvl:3;
    uint8_t  keyIdMode:2;
    uint8_t  frameCounterSup:1;
    uint8_t  asnInNonce:1;
    uint8_t  reserved:1;
    uint32_t frameCounter;
    uint8_t  keyId;
} __packed otRadio_auxSecHdr_t;

typedef struct _otRadioEnhAck_shortAddr {
    otRadio_frameControl_t  frameCtrl;
    uint8_t                 seqNum;
    uint16_t                panid;
    uint16_t                dstShortAddr;
    otRadio_auxSecHdr_t     auxHdr;
} __packed otRadioShortAddr_EnhAck_t;

typedef struct _otRadioEnhAck_extAddr {
    otRadio_frameControl_t  frameCtrl;
    uint8_t                 seqNum;
    uint16_t                panid;
    uint8_t                 dstExtAddr[8];
    otRadio_auxSecHdr_t     auxHdr;
} __packed otRadioExtAddr_EnhAck_t;

typedef struct _otRadio_ieHdr {
    uint16_t    length:7;
    uint16_t    elementId:8;
    uint16_t    type:1;
} __packed otRadio_ieHdr_t;

typedef union {
    struct {
        otRadio_ieHdr_t hdr;
        uint16_t phase;
        uint16_t period;
    } bf;
    uint8_t bytes[6];
} __packed otThreadCslIE_t;

typedef struct _otlm {
    utils_dlist_t                   dlist;
    uint8_t                         ieLinkMetric[11];
    int8_t                          noisefloor;
} otlm_t;
 
typedef struct _otRadio_t {
    otInstance                      *aInstance;

    struct otExtAddress             extAddress;
    struct otMacKeyMaterial         currentKey;
    struct otMacKeyMaterial         previousKey;
    struct otMacKeyMaterial         nextKey;
    uint32_t                        macFrameCounter;
    uint8_t                         macKeyId;
    uint8_t                         macKeyType;

    otRadio_opt_t                   opt;

    uint8_t                         unused[2];
    int8_t                          maxED;
    uint8_t                         ackKeyId;
    uint32_t                        ackFrameCounter;        /** for enh-ack security */

    uint8_t                         immAckFrame[4];
    utils_dlist_t                   rxFrameList;
    utils_dlist_t                   frameList;
    otRadioFrame                    *pTxFrame;
    otRadioFrame                    *pRxAckFrame;
    otRadioFrame                    *pTxAckFrame;           /** used for imm-ack & enh-ack contruction */

    uint32_t                        cslPeriod;
    uint32_t                        cslSampleTime;
    uint32_t                        cslUpdateTime;
    uint32_t                        cslTxRequestTime;
    uint16_t                        cslUpdatePhase;

    uint32_t                        (*ot_findAddresses_ptr)(uint32_t , uint8_t *, uint8_t *);

    otRadioIeInfo                   transmitIeInfo;
    otlm_t                          linkMetrics;

    uint32_t                        aesOutput[64];
} otRadio_t;

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

typedef void (*otPds_otherEventCb_t)(ot_system_event_t);
typedef void (*otPds_receiveAtFunc_t)(uint8_t, uint32_t, uint32_t);
typedef void (*otPds_lockFunc_t)(bool);

typedef struct _otRadio_pds_t {
    uint32_t                        isAttaching:1;
    uint32_t                        isRetx:1;
    uint32_t                        isCslEIExist:1;
    uint16_t                        isCslActive:1;
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

extern otRadio_t                    * otRadioVar_ptr;
extern otRadio_pds_t                * otRadioVar_pds_ptr;
extern void (*otrTask_ptr)(void);


#define ot_printf(...) if (ot_printf_ptr) ot_printf_ptr(__VA_ARGS__)
#define ot_is_print_enabled() (ot_printf_ptr)


void otLinkMetrics_init(int8_t noisefloor);
otError otLinkMetrics_configEnhAckProbing(otShortAddress aShortAddress, const otExtAddress *aExtAddress, otLinkMetrics aLinkMetrics);
uint32_t otLinkMetrics_genEnhAckData(uint32_t addrType, uint8_t *pAddr, int8_t rssi, int8_t lqi, uint8_t * pData);


uint32_t otrGetUserTaskRestTimeToCall(void);


void ot_radioEnable(void);

bool ot_isRadioIdle(void);

uint32_t ot_findAddresses_mtd(uint32_t addrType, uint8_t *extAddr, uint8_t *pShortAddr);

uint8_t * ot_parseMacHeader(uint8_t *rxbuf, uint32_t *pDstAddrType, uint8_t **pDstAddr, uint32_t *pSrcAddrType, uint8_t **pSrcAddr, otRadio_auxSecHdr_t **pAuxHdr);

uint8_t * ot_genAckMacHeader(uint8_t **rxbuf, uint32_t len, uint8_t *ackbuf, uint32_t *pSrcAddrType, uint8_t **pSrcAddr, otRadio_auxSecHdr_t **pAuxHdr);

uint32_t ot_getIElength(uint8_t *buf, uint8_t *buf_end);

uint32_t ot_genCslIE(uint16_t period, uint16_t phase, uint8_t * pData);

uint8_t * ot_findIe(uint8_t *pkt, uint32_t len);

uint8_t * ot_findIeEntry(uint8_t *pkt, uint32_t len2end, uint32_t id);

uint8_t * ot_getPayload(uint8_t *pkt, uint32_t len);
uint32_t ot_getPayloadLen(uint8_t *pkt, uint32_t len2end);

void ot_updateCslIe(otRadioFrame *aFrame);

uint32_t ot_aesOpt(uint32_t isDecrypt, uint8_t *pbuf, uint32_t len, uint32_t addrType, uint8_t *pAddr, 
    otRadio_auxSecHdr_t *pAuxHdr, uint8_t *pld, uint8_t *pkey);

int ot_encrytTxFrame(otRadioFrame *aFrame);

int ot_radioSend(otRadioFrame *aFrame);

void ot_radioUpdateCslTimeByTxDone(void);

uint32_t ot_radioHandleRecv(uint32_t *rx_buf, uint8_t rx_len);

void ot_radioEnqueueRecvPacket(uint32_t * rx_buf, uint32_t rx_len, uint32_t isSecuredAcked);

uint32_t otPds_getExpectedMicroTime(void);

void otPds_lockOp(bool isLock);
#endif


