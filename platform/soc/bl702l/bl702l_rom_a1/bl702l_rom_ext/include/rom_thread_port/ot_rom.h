
#ifndef __OT_ROM_H__
#define __OT_ROM_H__

extern void otRadio_lmac154_txDoneEvent (lmac154_tx_status_t tx_status);
extern void otRadio_lmac154_ackFrameEvent(uint8_t ack_received, uint8_t *rx_buf, uint8_t len);
extern void otRadio_lmac154_rxDoneEvent(uint8_t *rx_buf, uint8_t rx_len, uint8_t crc_fail);
extern void otRadio_lmac154_hwAutoTxAckDoneEvent(void);

extern void (*xTimerCallbackFunctionPrvHook)(void *callback);

extern int (*ot_printf_ptr)(const char *fmt, ...);

extern uint64_t (*ot_uint64_div16_ptr)(uint64_t dividend, uint16_t divisor);
extern uint64_t (*ot_uint64_mul16_ptr)(uint64_t a, uint16_t b);
extern uint64_t (*ot_uint64_mod16_ptr)(uint64_t dividend, uint16_t divisor);

extern void (*ot_irq_enable_ptr)(unsigned int source);
extern void (*ot_irq_disable_ptr)(unsigned int source);
extern void (*ot_irq_pending_clear_ptr)(unsigned int source);

extern uint64_t (*ot_timer_now_us64_ptr)(void);
extern uint32_t (*ot_hal_pds_enter_with_time_compensation_ptr)(uint32_t pdsLevel, uint32_t pdsSleepCycles);

extern uint64_t (*ot_rtc_get_counter_ptr)(void);
extern uint32_t (*ot_rtc_counter_to_ms_ptr)(uint32_t);
extern uint32_t (*ot_rtc_ms_to_counter_ptr)(uint32_t);
extern uint64_t (*ot_rtc_get_delta_counter_ptr)(uint64_t);

extern void (*ot_flash_restore_ptr)(void);

extern void (*otPds_restoreFullStack_ptr)(void);
extern int (*ot_pds_wakeup_by_rtc_ptr)(void);

extern uint32_t (*ot_otrEnterCrit_ptr)(void);
extern void (*ot_otrExitCrit_ptr)(uint32_t tag);
extern void (*ot_otrNotifyEvent_ptr)(ot_system_event_t sevent);
extern ot_system_event_t (*ot_otrGetNotifyEvent_ptr)(void);
extern bool (*ot_otrIsEventPending_ptr)(void);

extern int (*ot_lmac154_triggerTxDelay_ptr)(uint8_t *, uint32_t , uint32_t , 
    uint64_t , uint32_t );
extern lmac154_aes_status_t (*ot_lmac154_runAESCCM_ptr)(lmac154_aes_mode_t , 
                                       const uint8_t *, 
                                       uint8_t , 
                                       const uint8_t *, 
                                       const uint32_t *, 
                                       lmac154_aes_mic_len_t , 
                                       uint32_t *, 
                                       const uint32_t *, 
                                       uint32_t *, 
                                       uint8_t );
extern uint32_t (*ot_lmac154_postFillMPDU_ptr)(uint8_t *, uint32_t );
extern uint32_t (*ot_lmac154_isTriggerTimeSelected_ptr)(void);
extern lmac154_rf_state_t (*ot_lmac154_getRFState_ptr)(void);
extern uint64_t (*ot_lmac154_getEventTimeUs_ptr)(lmac154_eventTimeType_t );

extern void (*otPlatAlarmMilliStartAt_ptr)(otInstance *, uint32_t , uint32_t );
extern uint32_t (*ot_zb_timer_get_current_time_ptr)(void);
extern void* (*ot_zb_timer_stop_ptr)(uint8_t);
extern void (*ot_zb_timer_start_ptr)(uint8_t, uint32_t, zb_timer_cb_t);

extern bl_romhook_t ot_misc_patchHook_ptr;
extern bl_romhook_t ot_pds_patchHook_ptr;
extern bl_romhook_t ot_pds_time_patchHook_ptr;
extern bl_romhook_t ot_utils_patchHook_ptr;

#endif