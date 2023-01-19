#ifndef _OT_INTERNEL_H
#define _OT_INTERNEL_H


#ifndef OTRADIO_RX_FRAME_BUFFER_NUM
#define OTRADIO_RX_FRAME_BUFFER_NUM         8
#endif

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


#ifndef OT_ASSERT
#define OT_ASSERT(x) if ((x) == 0) vAssertCalled()
#endif

typedef enum {
    OTINTERNEL_ADDR_TYPE_NONE       = 0,
    OTINTERNEL_ADDR_TYPE_SHORT      = 1,
    OTINTERNEL_ADDR_TYPE_EXT        = 2,
} otInternel_addrType_t;

typedef struct _otRadio_ieHdr {
    uint16_t    length:7;
    uint16_t    elementId:8;
    uint16_t    type:1;
} __packed otRadio_ieHdr_t;

typedef struct _otRadio_rxFrame_t {
    utils_dlist_t       dlist;
    otRadioFrame        frame;
} otRadio_rxFrame_t;

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

extern otRadio_t                    * otRadioVar_ptr;

void otLinkMetrics_init(int8_t noisefloor);
otError otLinkMetrics_configEnhAckProbing(otShortAddress aShortAddress, const otExtAddress *aExtAddress, otLinkMetrics aLinkMetrics);
uint32_t otLinkMetrics_genEnhAckData(uint32_t addrType, uint8_t *pAddr, int8_t rssi, int8_t lqi, uint8_t * pData);

bool otGetMeshForwarderInfo(otInstance *aInstance, uint8_t *pLinkFailures, uint8_t *pExt, uint16_t *pShort);
bool otSetMeshForwarderInfo(otInstance *aInstance, uint8_t linkFailures);

bool otGetDataPollingInfo(otInstance *aInstance, bool *pRetxMode, uint32_t* pPollPeriodCurrent, uint32_t *pPollPeriodDefault, uint8_t *pPollQuick, 
                                    uint8_t *pPollFastRemaining, uint8_t *pPollFailure);
bool otSetDataPollingInfo(otInstance *aInstance, bool retxMode, uint32_t pollPeriodCurrent, uint8_t pollQuick, uint8_t pollFastRemaining, uint8_t pollFailure);

bool otIsSubMacIdle(otInstance *aInstance);
bool otGetSubMacInfo(otInstance *aInstance, bool *pIsCslActive, uint32_t *pCslLastSync, uint32_t *pCslPeriod, uint8_t *pCslChannel, uint8_t *pCslState, 
    uint8_t *pCslParentAccuracy, uint8_t *pCslParentUncertainty );
bool otSetSubMacInfo(otInstance *aInstance,uint32_t cslLastSync,uint8_t cslState, uint32_t sampleTime);
bool otStartFastPolling(otInstance *aInstance, uint32_t number);

bool otGetMacInfo(otInstance *aInstance, void ** pframe, uint8_t *pSeq, uint8_t *pChannel) ;
bool otSetMacInfo(otInstance *aInstance, uint8_t seq);
uint32_t otGetMacDataPollTimeout(void);

void * otMilliTimerList(otInstance *aInstance);
void * otMicroTimerList(otInstance *aInstance);
#endif
