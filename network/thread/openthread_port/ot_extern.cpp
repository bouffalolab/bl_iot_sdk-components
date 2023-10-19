
#include "common/instance.hpp"

extern "C" {
#ifdef OT_ASSERT
#undef OT_ASSERT
#endif
    #include <utils_list.h>
    #include <openthread_port.h>
    #include <ot_internel.h>
}

using namespace ot;
using namespace ot::Mac;

extern otRadio_t *                 otRadioVar_ptr;

extern "C" uint32_t otGetMacDataPollTimeout(void) 
{
    return ot::Mac::kDataPollTimeout;
}

/****************************** for milli/micro timer list ***********************/
typedef struct {
    /** struct _timer_scheduler_check_t is used to check whether size of TimerMilli::Scheduler 
     * or TimerMicro::Scheduler is changed */
    char TimerMilli_Scheduler_check[ (sizeof (TimerMilli::Scheduler) == 4) ? 1 :- 1];
    char TimerMicro_Scheduler_check[ (sizeof (TimerMicro::Scheduler) == 4) ? 1 :- 1];
} _timer_scheduler_check_t;

extern "C" void * otMilliTimerList(otInstance *aInstance)
{
    TimerMilli::Scheduler &timerMilli = AsCoreType(aInstance).Get<TimerMilli::Scheduler>();
    return &timerMilli;
}

extern "C" void * otMicroTimerList(otInstance *aInstance)
{
    TimerMicro::Scheduler &timerMicro = AsCoreType(aInstance).Get<TimerMicro::Scheduler>();
    return &timerMicro;
}

/****************************** for mac *****************************************/

typedef struct {
    uint8_t     reserved[5];
    uint8_t     mDataSequence;
    uint8_t     mBroadcastTransmitCount;
    PanId       mPanId;
    uint8_t     mPanChannel;
    uint8_t     mRadioChannel;
    ChannelMask mSupportedChannelMask;
    uint8_t     mScanChannel;
    uint16_t    mScanDuration;
    ChannelMask mScanChannelMask;
    uint8_t     mMaxFrameRetriesDirect;
// #if OPENTHREAD_FTD
//     uint8_t mMaxFrameRetriesIndirect;
// #if OPENTHREAD_CONFIG_MAC_CSL_TRANSMITTER_ENABLE
//     TimeMilli mCslTxFireTime;
// #endif
// #endif
// #if OPENTHREAD_CONFIG_MAC_CSL_RECEIVER_ENABLE
    // When Mac::mCslChannel is 0, it indicates that CSL channel has not been specified by the upper layer.
    uint8_t  mCslChannel;
    uint16_t mCslPeriod;
// #endif
} part_mac_t;

typedef struct {

    /** struct _mac_sub_mac_check_t is used to check whether the following classes are changed */
#if OPENTHREAD_CONFIG_MAC_CSL_RECEIVER_ENABLE
    char Mac_Links_check[ (144 == sizeof(Mac::Links)) ? 1 : -1];
#else
    char Mac_Links_check[ (112 == sizeof(Mac::Links)) ? 1 : -1];
#endif

    char Mac_TxFrames_check[ (4 == sizeof(Mac::TxFrames)) ? 1 : -1];
    char Mac_TxFrame_check[ (32 == sizeof(Mac::TxFrame)) ? 1 : -1];

} _mac_check_t;

extern "C" bool otGetMacInfo(otInstance *aInstance, void ** pframe, uint8_t *pSeq, uint8_t *pChannel) 
{
    if (otInstanceIsInitialized(aInstance)) {
        Mac::Links link(AsCoreType(aInstance));
        TxFrames &txFrames = link.GetTxFrames();

        Mac::Mac &mac = AsCoreType(aInstance).Get<Mac::Mac>();
        part_mac_t *pPartMac = (part_mac_t *) & mac;

        *pSeq = pPartMac->mDataSequence;
        *pChannel = AsCoreType(aInstance).Get<Mac::Mac>().GetPanChannel();

        txFrames.SetChannel(pPartMac->mRadioChannel);
        txFrames.SetMaxCsmaBackoffs(Mac::kMaxCsmaBackoffsDirect);
        txFrames.SetMaxFrameRetries(AsCoreType(aInstance).Get<Mac::Mac>().GetMaxFrameRetriesDirect());

        *pframe = (void*)(AsCoreType(aInstance).Get<DataPollSender>().PrepareDataRequest(txFrames));
        return true;
    }

    return false;
}

extern "C" bool otSetMacInfo(otInstance *aInstance, uint8_t seq) 
{
    if (otInstanceIsInitialized(aInstance)) {
        Mac::Mac &mac = AsCoreType(aInstance).Get<Mac::Mac>();
        part_mac_t *pPartMac = (part_mac_t *) & mac;

        pPartMac->mDataSequence = seq;

        return true;
    }

    return false;
}

/****************************** for sub mac *************************************/

enum State : uint8_t
{
    kStateDisabled,    // Radio is disabled.
    kStateSleep,       // Radio is in sleep.
    kStateReceive,     // Radio in in receive.
    kStateCsmaBackoff, // CSMA backoff before transmission.
    kStateTransmit,    // Radio is transmitting.
    kStateEnergyScan,  // Energy scan.
// #if OPENTHREAD_CONFIG_MAC_ADD_DELAY_ON_NO_ACK_ERROR_BEFORE_RETRY
    kStateDelayBeforeRetx, // Delay before retx
// #endif
// #if !OPENTHREAD_MTD && OPENTHREAD_CONFIG_MAC_CSL_TRANSMITTER_ENABLE
//     kStateCslTransmit, // CSL transmission.
// #endif
// #if OPENTHREAD_CONFIG_MAC_CSL_RECEIVER_ENABLE
    kStateCslSample, // CSL receive.
// #endif
};

typedef struct {
    otRadioCaps  mRadioCaps;
    State        mState;
    uint8_t      mCsmaBackoffs;
    uint8_t      mTransmitRetries;
    ShortAddress mShortAddress;
    ExtAddress   mExtAddress;
    bool         mRxOnWhenBackoff : 1;
#if OPENTHREAD_CONFIG_MAC_FILTER_ENABLE
    bool mRadioFilterEnabled : 1;
#endif
    int8_t             mEnergyScanMaxRssi;
    TimeMilli          mEnergyScanEndTime;
    TxFrame &          mTransmitFrame;
    SubMac::Callbacks          mCallbacks;
    otLinkPcapCallback mPcapCallback;
    void *             mPcapCallbackContext;
    KeyMaterial        mPrevKey;
    KeyMaterial        mCurrKey;
    KeyMaterial        mNextKey;
    uint32_t           mFrameCounter;
    uint8_t            mKeyId;
// #if OPENTHREAD_CONFIG_MAC_ADD_DELAY_ON_NO_ACK_ERROR_BEFORE_RETRY
    uint8_t mRetxDelayBackOffExponent;
// #endif
// #if OPENTHREAD_CONFIG_PLATFORM_USEC_TIMER_ENABLE
    TimerMicro mTimer;
// #else
//     TimerMilli                mTimer;
// #endif

// #if OPENTHREAD_CONFIG_MAC_CSL_RECEIVER_ENABLE
    uint16_t mCslPeriod;      // The CSL sample period, in units of 10 symbols (160 microseconds).
    uint8_t  mCslChannel : 7; // The CSL sample channel.
    bool mIsCslSampling : 1;  // Indicates that the radio is receiving in CSL state for platforms not supporting delayed
                              // reception.
    uint16_t   mCslPeerShort;      // The CSL peer short address.
    TimeMicro  mCslSampleTime;     // The CSL sample time of the current period.
    TimeMicro  mCslLastSync;       // The timestamp of the last successful CSL synchronization.
    uint8_t    mCslParentAccuracy; // Drift of timer used for scheduling CSL tx by the parent, in ± ppm.
    uint8_t    mCslParentUncert;   // Uncertainty of the scheduling CSL of tx by the parent, in ±10 us units.
    TimerMicro mCslTimer;
// #endif

} sub_mac_t;

typedef struct {
#if OPENTHREAD_CONFIG_MAC_CSL_RECEIVER_ENABLE
    /** struct _mac_sub_mac_check_t is used to check whether size of Mac::SubMac is changed */
    char Mac_SubMac_check[ (136 == sizeof(Mac::SubMac)) ? 1 : -1];
    char __Mac_SubMac_check[ (136 == sizeof(sub_mac_t)) ? 1 : -1];
#else
    /** struct _mac_sub_mac_check_t is used to check whether size of Mac::SubMac is changed */
    char Mac_SubMac_check[ (104 == sizeof(Mac::SubMac)) ? 1 : -1];
#endif
} _mac_sub_mac_check_t;


extern "C" bool otIsSubMacIdle(otInstance *aInstance) 
{
    if (otInstanceIsInitialized(aInstance)) {
        Mac::SubMac &submac = AsCoreType(aInstance).Get<Mac::SubMac>();
        sub_mac_t *pSubmac = (sub_mac_t *) & submac;
        uint8_t state = pSubmac->mState;

        if (otRadioVar_ptr->opt.bf.isCSLReceiverEnable) {
            return kStateDisabled == state || kStateSleep == state || kStateCslSample == state;
        }

        return kStateDisabled == state || kStateSleep == state;
    }

    return false;
}

extern "C" bool otGetSubMacInfo(otInstance *aInstance, bool *pIsCslActive, uint32_t *pCslLastSync, uint32_t *pCslPeriod, uint8_t *pCslChannel, uint8_t *pCslState, uint8_t *pCslParentAccuracy, uint8_t *pCslParentUncertainty ) 
{
    if (otInstanceIsInitialized(aInstance)) {
        Mac::SubMac &submac = AsCoreType(aInstance).Get<Mac::SubMac>();
        sub_mac_t *pSubmac = (sub_mac_t *) & submac;

        uint8_t state = pSubmac->mState;
        if (otRadioVar_ptr->opt.bf.isCSLReceiverEnable) {
            if (kStateDisabled == state || kStateSleep == state || kStateCslSample == state) {

                /** radio may be in sleep state when mState is kStateDelayBeforeRetx */
                *pIsCslActive = kStateCslSample == state;
                *pCslPeriod = pSubmac->mCslPeriod;
                if (pSubmac->mCslPeriod) {
                    if (pSubmac->mIsCslSampling) {
                        *pCslState = 1;
                    }
                    else {
                        *pCslState = 2;
                    }
                }
                else {
                    *pCslState = 0;
                }
                *pCslLastSync = pSubmac->mCslLastSync.GetValue();
                *pCslChannel = pSubmac->mCslChannel;

                *pCslParentAccuracy = pSubmac->mCslParentAccuracy;
                *pCslParentUncertainty = pSubmac->mCslParentUncert;

                return true;
            }

            return false;
        }
        else {
            /** radio may be in sleep state when mState is kStateDelayBeforeRetx */
            return kStateDisabled == state || kStateSleep == state;
        }
    }

    return false;
}

extern "C" bool otSetSubMacInfo(otInstance *aInstance,uint32_t cslLastSync,uint8_t cslState, uint32_t sampleTime) 
{
    if (otInstanceIsInitialized(aInstance)) {
        if (otRadioVar_ptr->opt.bf.isCSLReceiverEnable) {
            Mac::SubMac &submac = AsCoreType(aInstance).Get<Mac::SubMac>();
            sub_mac_t *pSubmac = (sub_mac_t *) & submac;

            if (cslState == 1) {
                pSubmac->mIsCslSampling = true;
            }
            else {
                pSubmac->mIsCslSampling = false;
            }

            pSubmac->mCslLastSync = TimeMicro(cslLastSync);
            pSubmac->mCslSampleTime = TimeMicro(sampleTime);
        }
        else {
            return true;
        }
    }

    return false;
}


/****************************** for mesh forwarder ******************************/

extern "C" bool otGetMeshForwarderInfo(otInstance *aInstance, uint8_t *pLinkFailures, uint8_t *pExt, uint16_t *pShort) 
{
    if (otInstanceIsInitialized(aInstance)) {

        Neighbor &parentCandidate = AsCoreType(aInstance).Get<Mle::MleRouter>().GetParentCandidate();
        Neighbor &parent = AsCoreType(aInstance).Get<Mle::MleRouter>().GetParent();

        if (parent.IsStateValid()) {
            memcpy(pExt, parent.GetExtAddress().m8, 8);
            *pShort = parent.GetRloc16();
            *pLinkFailures = parent.GetLinkFailures();
        }
        else if (parentCandidate.IsStateValid()) {
            memcpy(pExt, parentCandidate.GetExtAddress().m8, 8);
            *pShort = parentCandidate.GetRloc16();
            *pLinkFailures = parentCandidate.GetLinkFailures();
        }

        return true;
    }

    return false;
}

extern "C" bool otSetMeshForwarderInfo(otInstance *aInstance, uint8_t linkFailures) 
{
    uint8_t aFailLimit = Mle::kFailedChildTransmissions;

    if (otInstanceIsInitialized(aInstance)) {

        Neighbor &parentCandidate = AsCoreType(aInstance).Get<Mle::MleRouter>().GetParentCandidate();
        Neighbor &parent = AsCoreType(aInstance).Get<Mle::MleRouter>().GetParent();


        if (parent.IsStateValid()) {

            if (linkFailures) {
                while (parent.GetLinkFailures() < linkFailures) {
                    parent.IncrementLinkFailures();
                }
            }
            else {
                parent.ResetLinkFailures();
            }

            if ((Mle::IsActiveRouter(parent.GetRloc16())) && (parent.GetLinkFailures() >= aFailLimit)) {
                AsCoreType(aInstance).Get<Mle::MleRouter>().RemoveRouterLink(static_cast<Router &>(parent));
            }
        }
        else if (parentCandidate.IsStateValid()) {

            if (linkFailures) {
                while (parentCandidate.GetLinkFailures() < linkFailures) {
                    parentCandidate.IncrementLinkFailures();
                }
            }
            else {
                parentCandidate.ResetLinkFailures();
            }

            if ((Mle::IsActiveRouter(parentCandidate.GetRloc16())) && (parentCandidate.GetLinkFailures() >= aFailLimit)) {
                AsCoreType(aInstance).Get<Mle::MleRouter>().RemoveRouterLink(static_cast<Router &>(parentCandidate));
            }
        }  

        return true;
    }

    return false;
}

/****************************** for data poll ***********************************/

typedef struct {
    TimeMilli mTimerStartTime;
    uint32_t  mPollPeriod;
    uint32_t  mExternalPollPeriod : 26; // In milliseconds.
    uint8_t   mFastPollsUsers : 6;      // Number of callers which request fast polls.

    TimerMilli mTimer;

    bool    mEnabled : 1;              // Indicates whether data polling is enabled/started.
    bool    mAttachMode : 1;           // Indicates whether in attach mode (to use attach poll period).
    bool    mRetxMode : 1;             // Indicates whether last poll tx failed at mac/radio layer (poll retx mode).
    uint8_t mPollTimeoutCounter : 4;   // Poll timeouts counter (0 to `kQuickPollsAfterTimout`).
    uint8_t mPollTxFailureCounter : 4; // Poll tx failure counter (0 to `kMaxPollRetxAttempts`).
    uint8_t mRemainingFastPolls : 4;   // Number of remaining fast polls when in transient fast polling mode.
} data_poll_t;

typedef struct {
    /** struct _data_poll_check_t is used to check whether size of DataPollSender is changed */
    char DataPollSender_check[ (28 == sizeof(DataPollSender)) ? 1 : -1];
    char otRadioFrame_check[(32 == sizeof(otRadioFrame)) ? 1:-1];
} _data_poll_check_t;

extern "C" bool otGetDataPollingInfo(otInstance *aInstance, bool *pRetxMode, uint32_t* pPollPeriodCurrent, uint32_t *pPollPeriodDefault, uint8_t *pPollQuick, 
                                    uint8_t *pPollFastRemaining, uint8_t *pPollFailure)
{

    if (otInstanceIsInitialized(aInstance)) {

        DataPollSender &dataPollSender = AsCoreType(aInstance).Get<DataPollSender>();
        data_poll_t *pDataPoll = (data_poll_t *) & dataPollSender;

        *pRetxMode = pDataPoll->mRetxMode;
        *pPollPeriodCurrent = pDataPoll->mPollPeriod;
        if (pDataPoll->mExternalPollPeriod) {
            *pPollPeriodDefault = OT_MIN(pDataPoll->mExternalPollPeriod, dataPollSender.GetDefaultPollPeriod());
        }
        else {
            *pPollPeriodDefault = dataPollSender.GetDefaultPollPeriod();
        }

        *pPollFastRemaining = pDataPoll->mRemainingFastPolls;

        *pPollQuick = pDataPoll->mPollTimeoutCounter;
        *pPollFastRemaining = pDataPoll->mRemainingFastPolls;
        *pPollFailure = pDataPoll->mPollTxFailureCounter;

        return true;
    }

    return false;
}

extern "C" bool otSetDataPollingInfo(otInstance *aInstance, bool retxMode, uint32_t pollPeriodCurrent, uint8_t pollQuick, uint8_t pollFastRemaining, uint8_t pollFailure) 
{
    if (otInstanceIsInitialized(aInstance)) {
        DataPollSender &dataPollSender = AsCoreType(aInstance).Get<DataPollSender>();
        data_poll_t *pDataPoll = (data_poll_t *) & dataPollSender;

        pDataPoll->mRetxMode = retxMode;
        pDataPoll->mPollPeriod = pollPeriodCurrent;

        pDataPoll->mPollTimeoutCounter = pollQuick;
        pDataPoll->mRemainingFastPolls = pollFastRemaining;
        if (pDataPoll->mRemainingFastPolls == 0) {
            pDataPoll->mFastPollsUsers = 0;
        }
        pDataPoll->mPollTxFailureCounter = pollFailure;

        return true;
    }

    return false;
}


extern "C" void otExternPrintClassSize(void) 
{


    printf ("sizeof(otRadioFrame) = %d\r\n", sizeof(otRadioFrame));

    printf ("sizeof(Mac::Links) = %d\r\n", sizeof(Mac::Links));
    printf ("sizeof(TxFrames) = %d\r\n", sizeof(TxFrames));

    printf ("sizeof(Mac::TxFrame) = %d\r\n", sizeof(Mac::TxFrame));

    printf ("sizeof(Mac::SubMac) = %d\r\n", sizeof(Mac::SubMac));
}
