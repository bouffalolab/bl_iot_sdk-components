#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#ifdef BL702
#include <bl702.h>
#include <bl_irq.h>
#include <bl_timer.h>
#include <bl_wireless.h>
#include <lmac154.h>
#endif
#ifdef BL702L
#include <bl702l.h>
#include <bl_irq.h>
#include <bl_timer.h>
#include <bl_wireless.h>
#include <rom_lmac154_ext.h>
#endif
#include <utils_list.h>

#include <openthread/config.h>
#include <openthread/thread.h>
#include <platforms/utils/link_metrics.h>
#include <openthread-core-config.h>
#include <openthread_port.h>

#include <FreeRTOS.h>
#include <portmacro.h>
#include <task.h>

#include <ot_internel.h>
#include <ot_utils_ext.h>


__attribute__((section(".bss"))) static otRadio_t                otRadioVar;
__attribute__((section(".bss"))) static uint8_t                  otRadio_buffPool[TOTAL_RX_FRAME_SIZE * (OTRADIO_RX_FRAME_BUFFER_NUM + 1) + (ALIGNED_RX_FRAME_SIZE + MAX_ACK_FRAME_SIZE) * 2];
#ifdef BL702
otRadio_t                       *otRadioVar_ptr = NULL;
#endif
#ifdef BL702L
extern otRadio_t                *otRadioVar_ptr;
#endif

void ot_radioInit(otRadio_opt_t opt) 
{
    otRadio_rxFrame_t *pframe = NULL;
    otRadioVar_ptr = &otRadioVar;

    memset(otRadioVar_ptr, 0, sizeof(otRadio_t));

    otRadioVar_ptr->opt.byte = opt.byte;

    uint32_t tag = otrEnterCrit();
    utils_dlist_init(&otRadioVar_ptr->frameList);
    utils_dlist_init(&otRadioVar_ptr->rxFrameList);

    for (int i = 0; i < OTRADIO_RX_FRAME_BUFFER_NUM; i ++) {
        pframe = (otRadio_rxFrame_t *) (otRadio_buffPool + TOTAL_RX_FRAME_SIZE * (i + 1));
        pframe->frame.mPsdu = ((uint8_t *)pframe) + ALIGNED_RX_FRAME_SIZE;
        utils_dlist_add_tail(&pframe->dlist, &otRadioVar_ptr->frameList);
    }

    otRadioVar_ptr->pTxAckFrame = (otRadioFrame *)(otRadio_buffPool + TOTAL_RX_FRAME_SIZE * (OTRADIO_RX_FRAME_BUFFER_NUM + 1));
    otRadioVar_ptr->pTxAckFrame->mPsdu = ((uint8_t *)otRadioVar_ptr->pTxAckFrame) + ALIGNED_RX_FRAME_SIZE;
    otRadioVar_ptr->pRxAckFrame = (otRadioFrame *)(otRadioVar_ptr->pTxAckFrame->mPsdu + MAX_ACK_FRAME_SIZE);
    otRadioVar_ptr->pRxAckFrame->mPsdu = ((uint8_t *)otRadioVar_ptr->pRxAckFrame) + ALIGNED_RX_FRAME_SIZE;

    if (opt.bf.isLinkMetricEnable) {
        otLinkMetrics_init(LMAC154_RADIO_RECEIVE_SENSITIVITY);
    }

#if (OPENTHREAD_RADIO == 0)
#ifdef BL702
    otRadioVar_ptr->ot_findAddresses_ptr = ot_findAddresses_ftd;
#endif
#ifdef BL702L
#ifdef CFG_PDS_ENABLE
    otRadioVar_ptr->ot_findAddresses_ptr = ot_findAddresses_mtd;
#else
    otRadioVar_ptr->ot_findAddresses_ptr = ot_findAddresses_ftd;
#endif
#endif

#endif
    otrExitCrit(tag);
}

void ot_radioTask(ot_system_event_t trxEvent) 
{
    otRadio_rxFrame_t   *pframe;
    otRadioFrame        *txframe;
    uint32_t            tag;

    if (!(OT_SYSTEM_EVENT_RADIO_ALL_MASK & trxEvent)) {
        return;
    }

    if (otRadioVar_ptr->pTxFrame) {

        if ((OT_SYSTEM_EVENT_RADIO_TX_ALL_MASK & trxEvent)) {
            txframe = otRadioVar_ptr->pTxFrame;
            otRadioVar_ptr->pTxFrame = NULL;

            if (trxEvent & OT_SYSTEM_EVENT_RADIO_TX_DONE_NO_ACK_REQ) {
                otPlatRadioTxDone(otRadioVar_ptr->aInstance, txframe, NULL, OT_ERROR_NONE);
            }
            else if (trxEvent & OT_SYSTEM_EVENT_RADIO_TX_ACKED) {
                otPlatRadioTxDone(otRadioVar_ptr->aInstance, txframe, otRadioVar_ptr->pRxAckFrame, OT_ERROR_NONE);
            }
            else if (trxEvent & OT_SYSTEM_EVENT_RADIO_TX_NO_ACK) {
                otPlatRadioTxDone(otRadioVar_ptr->aInstance, txframe, NULL, OT_ERROR_NO_ACK);
            }
            else if (trxEvent & OT_SYSTEM_EVENT_RADIO_TX_ABORT) {
                otPlatRadioTxDone(otRadioVar_ptr->aInstance, txframe, NULL, OT_ERROR_ABORT);
            }
            else if (trxEvent & OT_SYSTEM_EVENT_RADIO_TX_ERROR) {
                otPlatRadioTxDone(otRadioVar_ptr->aInstance, txframe, NULL, OT_ERROR_CHANNEL_ACCESS_FAILURE);
            }
        }
    }
    
    if (trxEvent & OT_SYSTEM_EVENT_RADIO_RX_DONE ) {

        pframe = NULL;
        tag = otrEnterCrit();
        if (!utils_dlist_empty(&otRadioVar_ptr->rxFrameList)) {
            pframe = (otRadio_rxFrame_t *)otRadioVar_ptr->rxFrameList.next;
            utils_dlist_del(&pframe->dlist);
        }
        otrExitCrit(tag);

        if (pframe) {
            otPlatRadioReceiveDone(otRadioVar_ptr->aInstance, &pframe->frame, OT_ERROR_NONE);

            tag = otrEnterCrit();
            utils_dlist_add_tail(&pframe->dlist, &otRadioVar_ptr->frameList);
            otrExitCrit(tag);

            if (!utils_dlist_empty(&otRadioVar_ptr->rxFrameList)) {
                /**
                 * [weiyin], issue OT_SYSTEM_EVENT_RADIO_RX_DONE to handle next pending packet
                 */
                otrNotifyEvent(OT_SYSTEM_EVENT_RADIO_RX_DONE);
            }
        }
    }
    else if (trxEvent & OT_SYSTEM_EVENT_RADIO_RX_NO_BUFF) {
        otPlatRadioReceiveDone(otRadioVar_ptr->aInstance, NULL, OT_ERROR_NO_BUFS);
    }
}

/************************************ LMAC 15.4 event function ***********************************/
#if defined(BL702)
void lmac154_txDoneEvent (lmac154_tx_status_t tx_status)
{
    if (otRadioVar_ptr->pTxFrame) {
        if (LMAC154_TX_STATUS_TX_FINISHED == tx_status) {
            if (!(otRadioVar_ptr->pTxFrame->mPsdu[0] & LMAC154_FRAME_ACK_REQUEST_MASK)) {
                otrNotifyEvent(OT_SYSTEM_EVENT_RADIO_TX_DONE_NO_ACK_REQ);
            }
        }
        else {
            otrNotifyEvent(OT_SYSTEM_EVENT_RADIO_TX_NO_ACK);
        }
    }
}

void lmac154_ackFrameEvent(uint8_t ack_received, uint8_t *rx_buf, uint8_t len)
{
    if (otRadioVar_ptr->pTxFrame) {
        if (rx_buf) {
            memcpy(otRadioVar_ptr->pRxAckFrame->mPsdu, rx_buf, len);
            otRadioVar_ptr->pRxAckFrame->mLength = len;

            otRadioVar_ptr->pRxAckFrame->mChannel = lmac154_getChannel() + OT_RADIO_2P4GHZ_OQPSK_CHANNEL_MIN;
            otRadioVar_ptr->pRxAckFrame->mInfo.mRxInfo.mRssi = lmac154_getRSSI();
            otRadioVar_ptr->pRxAckFrame->mInfo.mRxInfo.mLqi = lmac154_getLQI();
            otRadioVar_ptr->pRxAckFrame->mInfo.mRxInfo.mTimestamp = lmac154_getEventTimeUs(LMAC154_EVENT_TIME_RX_START) + (LMAC154_PREAMBLE_LEN << LMAC154_US_PER_SYMBOL_BITS);

            otrNotifyEvent(OT_SYSTEM_EVENT_RADIO_TX_ACKED);
        }
        else {
            otrNotifyEvent(OT_SYSTEM_EVENT_RADIO_TX_NO_ACK);
        }
    }
}

void lmac154_rxDoneEvent(uint8_t *rx_buf, uint8_t rx_len, uint8_t crc_fail)
{
    if (crc_fail) {
        return;
    }

    if (lmac154_isRxPromiscuousModeEnabled() && LMAC154_FRAME_IS_ACK_REQ(rx_buf[0])) {
        lmac154_enableRx();
    }

    ot_radioEnqueueRecvPacket((uint32_t *)rx_buf, rx_len, true);
}
#endif

/*************************** Openthread radio interface implementation ***************************/

otRadioFrame *otPlatRadioGetTransmitBuffer(otInstance *aInstance) 
{
    otRadioVar_ptr->aInstance = aInstance;

    otRadioFrame * txframe = (otRadioFrame *)otRadio_buffPool;
    txframe->mPsdu = otRadio_buffPool + ALIGNED_RX_FRAME_SIZE;

    txframe->mInfo.mTxInfo.mIeInfo = &otRadioVar_ptr->transmitIeInfo;

    return txframe;
}

otError otPlatRadioTransmit(otInstance *aInstance, otRadioFrame *aFrame) 
{
    int iret = -1;

    if (otRadioVar_ptr->pTxFrame == NULL) {

        iret = ot_radioSend(aFrame);

        if (iret) {
            otrNotifyEvent(OT_SYSTEM_EVENT_RADIO_TX_ERROR);
        }
        else {
            otPlatRadioTxStarted(aInstance, aFrame);
        }

        return OT_ERROR_NONE;
    }

    return OT_ERROR_INVALID_STATE;
}

otRadioCaps otPlatRadioGetCaps(otInstance *aInstance) 
{
    return OT_RADIO_CAPS_ACK_TIMEOUT | OT_RADIO_CAPS_CSMA_BACKOFF | OT_RADIO_CAPS_SLEEP_TO_TX | OT_RADIO_CAPS_TRANSMIT_TIMING | OT_RADIO_CAPS_TRANSMIT_SEC; 
}
const char *otPlatRadioGetVersionString(otInstance *aInstance) 
{
    return lmac154_getLibVersion();
}
int8_t otPlatRadioGetReceiveSensitivity(otInstance *aInstance) 
{
    return LMAC154_RADIO_RECEIVE_SENSITIVITY;
}
void otPlatRadioGetIeeeEui64(otInstance *aInstance, uint8_t *aIeeeEui64) 
{
    uint8_t temp;

    bl_wireless_mac_addr_get(aIeeeEui64);

    for (int i = 0; i < OT_EXT_ADDRESS_SIZE / 2; i ++) {
        temp = aIeeeEui64[OT_EXT_ADDRESS_SIZE - i - 1];
        aIeeeEui64[OT_EXT_ADDRESS_SIZE - i - 1] = aIeeeEui64[i];
        aIeeeEui64[i] = temp;
    }

    if (aIeeeEui64[OT_EXT_ADDRESS_SIZE - 1] == 0 && aIeeeEui64[OT_EXT_ADDRESS_SIZE - 2] == 0) {
        aIeeeEui64[OT_EXT_ADDRESS_SIZE - 1] = aIeeeEui64[OT_EXT_ADDRESS_SIZE - 3];
        aIeeeEui64[OT_EXT_ADDRESS_SIZE - 2] = aIeeeEui64[OT_EXT_ADDRESS_SIZE - 4];
        aIeeeEui64[OT_EXT_ADDRESS_SIZE - 3] = aIeeeEui64[OT_EXT_ADDRESS_SIZE - 5];
    }
}

void otPlatRadioSetPanId(otInstance *aInstance, otPanId aPanId) 
{
    lmac154_setPanId(aPanId);
}
void otPlatRadioSetExtendedAddress(otInstance *aInstance, const otExtAddress *aExtAddress) 
{
    lmac154_setLongAddr((uint8_t *) aExtAddress->m8);
    memcpy(otRadioVar_ptr->extAddress.m8, aExtAddress->m8, 8);
} 

void otPlatRadioSetShortAddress(otInstance *aInstance, otShortAddress aShortAddress) 
{
    lmac154_setShortAddr(aShortAddress);
}

otError otPlatRadioGetTransmitPower(otInstance *aInstance, int8_t *aPower) 
{
    *aPower = lmac154_getTxPower();
    return OT_ERROR_NONE;
}
otError otPlatRadioSetTransmitPower(otInstance *aInstance, int8_t aPower) 
{
    lmac154_setTxPower(aPower);
    return OT_ERROR_NONE;
}

otError otPlatRadioGetCcaEnergyDetectThreshold(otInstance *aInstance, int8_t *aThreshold) 
{
    *aThreshold = lmac154_getEDThreshold();
    return OT_ERROR_NONE;
}
otError otPlatRadioSetCcaEnergyDetectThreshold(otInstance *aInstance, int8_t aThreshold) 
{
    lmac154_setEDThreshold(aThreshold);
    return OT_ERROR_NONE;
}

bool otPlatRadioGetPromiscuous(otInstance *aInstance) 
{
    return lmac154_isRxPromiscuousModeEnabled() == 1;
}
void otPlatRadioSetPromiscuous(otInstance *aInstance, bool aEnable) 
{
    if (aEnable) {
        lmac154_enableRxPromiscuousMode(true, false);
    }
    else {
        lmac154_disableRxPromiscuousMode();
    }
}

void otPlatRadioSetMacKey(otInstance *            aInstance,
                          uint8_t                 aKeyIdMode,
                          uint8_t                 aKeyId,
                          const otMacKeyMaterial *aPrevKey,
                          const otMacKeyMaterial *aCurrKey,
                          const otMacKeyMaterial *aNextKey,
                          otRadioKeyType          aKeyType)
{
    OT_UNUSED_VARIABLE(aInstance);
    OT_UNUSED_VARIABLE(aKeyIdMode);
    OT_UNUSED_VARIABLE(aKeyType);

    if (aPrevKey && aCurrKey && aNextKey) {
        uint32_t tag = otrEnterCrit();

        otRadioVar_ptr->macKeyType = aKeyType;
        otRadioVar_ptr->macKeyId = aKeyId;

        memcpy(&otRadioVar_ptr->previousKey, aPrevKey, sizeof(otMacKeyMaterial));
        memcpy(&otRadioVar_ptr->currentKey, aCurrKey, sizeof(otMacKeyMaterial));
        memcpy(&otRadioVar_ptr->nextKey, aNextKey, sizeof(otMacKeyMaterial));

        otrExitCrit(tag);
    }
}

void otPlatRadioSetMacFrameCounter(otInstance *aInstance, uint32_t aMacFrameCounter)
{
    OT_UNUSED_VARIABLE(aInstance);

    uint32_t tag = otrEnterCrit();
    otRadioVar_ptr->macFrameCounter = aMacFrameCounter;
    otrExitCrit(tag);
}

uint64_t otPlatRadioGetNow(otInstance *aInstance)
{
    return bl_timer_now_us64();
}

otRadioState otPlatRadioGetState(otInstance *aInstance) 
{
    otRadioState state = 0;
    lmac154_rf_state_t rfstate = 0;

    if(lmac154_isDisabled()) {
        state = OT_RADIO_STATE_DISABLED;
    }
    else {
        if (otRadioVar_ptr->pTxFrame) {
            /** mac layer retring doesn't get done */
            state = OT_RADIO_STATE_TRANSMIT;
        }
        else {
            rfstate = lmac154_getRFState();
            switch (rfstate)
            {
            case LMAC154_RF_STATE_RX:
            case LMAC154_RF_STATE_RX_DOING:
                state = OT_RADIO_STATE_RECEIVE;
                break;
                case LMAC154_RF_STATE_TX:
                state = OT_RADIO_STATE_TRANSMIT;
                break;
            case LMAC154_RF_STATE_IDLE:
                state = OT_RADIO_STATE_SLEEP;
                break;
            default:
                state = OT_RADIO_STATE_INVALID;
            }
        }
    }

    return state;
}

otError otPlatRadioEnable(otInstance *aInstance) 
{
    ot_radioEnable();

#ifdef CFG_PDS_ENABLE
    otPds_setDataPollCsma(CFG_DATA_POLL_CSMA);
#endif

    bl_irq_enable(M154_IRQn);

    return OT_ERROR_NONE;
}

otError otPlatRadioDisable(otInstance *aInstance) 
{
    bl_irq_disable(M154_IRQn);
    lmac154_disableRx();

    return OT_ERROR_NONE;
}
bool otPlatRadioIsEnabled(otInstance *aInstance) 
{
    return !lmac154_isDisabled();
}

otError otPlatRadioSleep(otInstance *aInstance) 
{
    lmac154_disableRx();

#if !defined(CONFIG_NCP) || CONFIG_NCP == 1
    lmac154_setRxStateWhenIdle(otThreadGetLinkMode(aInstance).mRxOnWhenIdle);
#endif

    return OT_ERROR_NONE;
}
otError otPlatRadioReceive(otInstance *aInstance, uint8_t aChannel) 
{
    uint8_t ch = aChannel - OT_RADIO_2P4GHZ_OQPSK_CHANNEL_MIN;

#ifdef BL702L
    extern bool bz_phy_optimize_tx_channel(uint32_t channel);
    bz_phy_optimize_tx_channel(2405 + 5 * ch);
#endif

    lmac154_setChannel((lmac154_channel_t)ch);
#if !defined(CONFIG_NCP) || CONFIG_NCP == 1
    lmac154_setRxStateWhenIdle(otThreadGetLinkMode(aInstance).mRxOnWhenIdle);
#endif
    lmac154_enableRx();

    return OT_ERROR_NONE;
}

int8_t otPlatRadioGetRssi(otInstance *aInstance) 
{
    int rssi = 127;

    if (lmac154_runCCA(&rssi)) {
        return 127;
    }

    return (int8_t)rssi;
}
otError otPlatRadioEnergyScan(otInstance *aInstance, uint8_t aScanChannel, uint16_t aScanDuration)
{
    return OT_ERROR_NONE;
}
void otPlatRadioEnableSrcMatch(otInstance *aInstance, bool aEnable) 
{
    lmac154_fptForcePending(!aEnable);
}
otError otPlatRadioAddSrcMatchShortEntry(otInstance *aInstance, otShortAddress aShortAddress) 
{
    return lmac154_fptSetShortAddrPending(aShortAddress, 1) == LMAC154_FPT_STATUS_SUCCESS ? OT_ERROR_NONE:OT_ERROR_NO_ADDRESS;
}
otError otPlatRadioAddSrcMatchExtEntry(otInstance *aInstance, const otExtAddress *aExtAddress) 
{
    return lmac154_fptSetLongAddrPending((uint8_t *)aExtAddress->m8, 1) == LMAC154_FPT_STATUS_SUCCESS ? OT_ERROR_NONE:OT_ERROR_NO_ADDRESS;
}
otError otPlatRadioClearSrcMatchShortEntry(otInstance *aInstance, otShortAddress aShortAddress) 
{
    return lmac154_fptSetShortAddrPending(aShortAddress, 0) == LMAC154_FPT_STATUS_SUCCESS ? OT_ERROR_NONE:OT_ERROR_NO_ADDRESS;
}
otError otPlatRadioClearSrcMatchExtEntry(otInstance *aInstance, const otExtAddress *aExtAddress) 
{
    return lmac154_fptSetLongAddrPending((uint8_t *)aExtAddress->m8, 0) == LMAC154_FPT_STATUS_SUCCESS ? OT_ERROR_NONE:OT_ERROR_NO_ADDRESS;
}
void otPlatRadioClearSrcMatchShortEntries(otInstance *aInstance) 
{
    uint8_t num = 128;
    uint16_t * plist = (uint16_t *)pvPortMalloc(sizeof(uint16_t) * num);

    lmac154_fpt_GetShortAddrList(plist, &num);
    for (uint32_t i = 0; i < num; i ++) 
    {
        lmac154_fptRemoveShortAddr(plist[i]);
    }
    vPortFree(plist);
}
void otPlatRadioClearSrcMatchExtEntries(otInstance *aInstance) 
{
    uint8_t num = 32;
    uint8_t * plist = (uint8_t *)pvPortMalloc(sizeof(uint8_t) * 8 * num);

    lmac154_fptGetLongAddrList(plist, &num);
    for (uint32_t i = 0; i < num; i ++) 
    {
        lmac154_fptRemoveLongAddr(plist + i * 8);
    }
    vPortFree(plist);
}
uint32_t otPlatRadioGetSupportedChannelMask(otInstance *aInstance) 
{
    return OT_RADIO_2P4GHZ_OQPSK_CHANNEL_MASK;
}
uint32_t otPlatRadioGetPreferredChannelMask(otInstance *aInstance) 
{
    return OT_RADIO_2P4GHZ_OQPSK_CHANNEL_MASK;
}

otError otPlatRadioSetCoexEnabled(otInstance *aInstance, bool aEnabled) 
{
    if (aEnabled) {
        lmac154_enableCoex();
    }
    else {
        lmac154_disableCoex();
    }

    otRadioVar_ptr->opt.bf.isCoexEnable = aEnabled;
    return OT_ERROR_NONE;
}

bool otPlatRadioIsCoexEnabled(otInstance *aInstance) 
{
    return otRadioVar_ptr->opt.bf.isCoexEnable;
}

otError otPlatRadioReceiveAt(otInstance *aInstance, uint8_t aChannel, uint32_t aStart, uint32_t aDuration)
{
    return OT_ERROR_NOT_IMPLEMENTED;
}

#if OPENTHREAD_CONFIG_MAC_CSL_RECEIVER_ENABLE
otError otPlatRadioEnableCsl(otInstance *        aInstance,
                             uint32_t            aCslPeriod,
                             otShortAddress      aShortAddr,
                             const otExtAddress *aExtAddr)
{
    OT_UNUSED_VARIABLE(aInstance);
    OT_UNUSED_VARIABLE(aShortAddr);
    OT_UNUSED_VARIABLE(aExtAddr);

    otRadioVar_ptr->cslPeriod = aCslPeriod;

    return OT_ERROR_NONE;
}

void otPlatRadioUpdateCslSampleTime(otInstance *aInstance, uint32_t aCslSampleTime)
{
    OT_UNUSED_VARIABLE(aInstance);

    otRadioVar_ptr->cslSampleTime = aCslSampleTime;
}

OT_TOOL_WEAK uint8_t otPlatRadioGetCslAccuracy(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);

    return 20;
}

OT_TOOL_WEAK uint8_t otPlatRadioGetCslClockUncertainty(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);

    return 20;
}
#endif

#if OPENTHREAD_CONFIG_MLE_LINK_METRICS_SUBJECT_ENABLE
otError otPlatRadioConfigureEnhAckProbing(otInstance *         aInstance,
                                          otLinkMetrics        aLinkMetrics,
                                          const otShortAddress aShortAddress,
                                          const otExtAddress * aExtAddress)
{
    OT_UNUSED_VARIABLE(aInstance);

    return otLinkMetrics_configEnhAckProbing(aShortAddress, aExtAddress, aLinkMetrics);
}
#endif