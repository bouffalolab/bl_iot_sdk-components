#ifndef OT_PDS_H
#define OT_PDS_H

#include <ot_rom_pds.h>

#ifdef __cplusplus
extern "C" {
#endif

void otPds_restoreOp(void);

uint32_t otPds_getCslRxOnTime(void);

uint32_t otPds_getExpectedMicroTime(void);

uint32_t otPds_getExpectedPollingTime(void);

void otPds_lockOp(bool isLock);

void otPds_vApplicationSleep( TickType_t xExpectedIdleTime );

uint32_t otPds_getMilliTimerRemaining(void);

bool ot_isRadioIdle(void);

bool otIsSubMacIdle(otInstance *aInstance);

uint32_t otGetMacDataPollTimeout(void);
void * otMilliTimerList(otInstance *aInstance);
void * otMicroTimerList(otInstance *aInstance);
bool otGetMacInfo(otInstance *aInstance, void ** pframe, uint8_t *pSeq, uint8_t *pChannel);
bool otSetMacInfo(otInstance *aInstance, uint8_t seq);
bool otIsSubMacIdle(otInstance *aInstance);
bool otGetSubMacInfo(otInstance *aInstance, bool *pIsCslActive, uint32_t *pCslLastSync, uint32_t *pCslPeriod, uint8_t *pCslChannel, uint8_t *pCslState, uint8_t *pCslParentAccuracy, uint8_t *pCslParentUncertainty );
bool otSetSubMacInfo(otInstance *aInstance,uint32_t cslLastSync,uint8_t cslState, uint32_t sampleTime);
bool otGetMeshForwarderInfo(otInstance *aInstance, uint8_t *pLinkFailures, uint8_t *pExt, uint16_t *pShort);
bool otSetMeshForwarderInfo(otInstance *aInstance, uint8_t linkFailures);
bool otGetDataPollingInfo(otInstance *aInstance, bool *pRetxMode, uint32_t* pPollPeriodCurrent, uint32_t *pPollPeriodDefault, uint8_t *pPollQuick, 
                                    uint8_t *pPollFastRemaining, uint8_t *pPollFailure);
bool otSetDataPollingInfo(otInstance *aInstance, bool retxMode, uint32_t pollPeriodCurrent, uint8_t pollQuick, uint8_t pollFastRemaining, uint8_t pollFailure);
void otExternPrintClassSize(void);

#ifdef __cplusplus
}
#endif

#endif
