#ifndef _ZB_SIMPLE_H
#define _ZB_SIMPLE_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "lmac154.h"

enum{
    ZB_SIMPLE_MODE_IDLE = 0x01,
    ZB_SIMPLE_MODE_WAIT_TX_DONE = 0x02,
    ZB_SIMPLE_MODE_WAIT_MAC_ACK = 0x03,
    ZB_SIMPLE_MODE_WAIT_DATA = 0x04,
    ZB_SIMPLE_MODE_WAIT_AUTO_ACK_DONE = 0x05,
    ZB_SIMPLE_MODE_WAIT_RESTORE_FULL_STACK = 0x06,
    ZB_SIMPLE_MODE_TO_SLEEP = 0x07
};

enum{
    ZB_SIMPLE_ADDR_MODE_INDIRECT = 0x00,     // DstAddress and DstEndpoint not present
    ZB_SIMPLE_ADDR_MODE_GROUP    = 0x01,     // 16-bit group address present, DstEndpoint not present
    ZB_SIMPLE_ADDR_MODE_SHORT    = 0x02,     // 16-bit DstAddress and DstEndpoint present
    ZB_SIMPLE_ADDR_MODE_IEEE     = 0x03,     // 64-bit DstAddress and DstEndpoint present
};

enum{
    ZB_SIMPLE_PDS_WAKEUP_BY_SLEEP_CNT = 0x01,
    ZB_SIMPLE_PDS_WAKEUP_BY_GPIO = 0x02,    
};

struct _zbRxPacket{
    uint8_t data[256];
    uint8_t len;
    uint8_t fcs[2];
    uint16_t freqOffset;
    signed char rssi;
    uint8_t sfdCorr;
};

struct _zsedNwkInfo{
    uint8_t currChnl;
    uint8_t macSeqNum;
    uint8_t nwkSeqNum;
    uint8_t apsCounter;
    uint8_t zclTransNum;
    uint16_t panId;
    uint16_t srcShortAddr;
    uint16_t parentShortAddr;
    uint32_t pollIntvlMs;
};

struct _blZbPdsEnv{
    bool parentLost;
    bool wakeupZbFullStack;
    bool macPollInSimpleMode;
    bool csmaEnable;
    uint8_t pdsLevel;
    uint8_t zbStateInSimpleMode;
    uint8_t pollFailureLimitCnt;
    uint8_t pollFailureCnt;
    uint32_t pdsToleranceCycle;
    //in driver, it takes (sleep_cycles-PDS_WARMUP_LATENCY_CNT) as sleep cycles.
    uint32_t pdsMinTimeInCycle;
    uint32_t macMaxFrameTotalWaitTimeUs;
    uint64_t rxBaseTimeUs;
    uint64_t rtcSleepRefCnt;
    uint32_t minIdleDurMs;
    uint32_t totalSleepDurMs;
    lmac154_isr_t triggerEvent;
    struct _zsedNwkInfo zsedNwkInfo;
    struct _zbRxPacket rxPktStoredInRam;
};

extern struct _blZbPdsEnv *zbSimplePdsEnvPtr;
extern uint8_t zbSimpleDataReq[10];

void zb_simple_sleep(void);
void zb_simple_send_data_req(void);
void zb_simple_lmac154_txDoneEvent(lmac154_tx_status_t tx_status);
void zb_simple_lmac154_ackEvent(uint8_t ack_received, uint8_t frame_pending, uint8_t seq_num);
void zb_simple_lmac154_rxDoneEvent(uint8_t *rx_buf, uint8_t rx_len, uint8_t crc_fail);
void zb_simple_lmac154_hwAutoTxAckDoneEvent(void);
bool zb_simple_check_time_expire(void);
void zb_simple_pds_fastboot_done_callback(void);

extern int (*zb_simple_printf_ptr)(const char *fmt, ...);
extern int (*zb_simple_pds_wakeup_by_rtc_ptr)(void);
extern int (*zb_simple_pds_enter_ptr)(uint32_t pdsLevel, uint32_t pdsSleepCycles);
extern uint64_t (*zb_simple_rtc_get_delta_counter_ptr)(uint64_t ref_cnt);
extern uint32_t (*zb_simple_rtc_counter_to_ms_ptr)(uint32_t cnt);
extern void (*zb_simple_irq_enable_ptr)(unsigned int source);
extern void (*zb_simple_irq_disable_ptr)(unsigned int source);
extern void (*zb_simple_irq_pending_clear_ptr)(unsigned int source);
extern uint32_t (*zb_simple_time_ms_to_cycle_ptr)(uint32_t timeMs);
extern void *(*zb_simple_memcpy_ptr)(void *dest, const void *src, size_t n);
extern uint64_t (*zb_simple_get_current_time_us_ptr)(void);
extern uint32_t (*zb_simple_rom_patch_hook)(void *pRet, ...);
#endif
