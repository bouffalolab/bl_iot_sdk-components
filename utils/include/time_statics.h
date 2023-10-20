#ifndef __TIME_STATICS__
#define __TIME_STATICS__

#include <stdint.h>
#include "bl_rtc.h"

#include "rv_hpm.h"

#define TS_STUBS  \
  _X(TS_ALLOW_SLEEP_PDS)  \
  _X(TS_WAKEUP_RESTORE)  \
  _X(TS_WAKEUP_BLE)  \


enum {
#define _X(x) x,
  TS_STUBS
#undef _X
  TS_MAX,
};

extern uint64_t g_ts_record[TS_MAX][5];

/**
 * 0: RTC
 * 1: Cache miss cnt
 * 2: Cache access cnt
 * 3: CPU Cycle
 * 4: Instruction Retired
 */

#if defined (TICKLESS_RECORD)
#define TS_RECORD(t) if (unlikely(g_ts_record[t][0] == 0)) { \
  int irq_state;  \
  __asm__ volatile ("csrrci %0, mstatus, %1" : "=r" (irq_state) : "i" (8)); \
  g_ts_record[t][0] = bl_rtc_get_counter(); \
  RV_HPM_L1_ICache_Miss_Stop_M(&g_ts_record[t][1], &g_ts_record[t][2]); \
  RV_HPM_Cycle_Get_M(&g_ts_record[t][3]); \
  RV_HPM_Instret_Get_M(&g_ts_record[t][4]); \
  __asm__ volatile ("csrw mstatus, %0" : /* no output */ : "r" (irq_state)); \
}
#else
#define TS_RECORD(x)
#endif

void time_static_record_dump(void);
#endif
