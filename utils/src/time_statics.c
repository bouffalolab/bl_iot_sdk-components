#include <stdint.h>
#include <string.h>

#include "time_statics.h"

#if defined (TICKLESS_RECORD)
uint64_t g_ts_record[TS_MAX][5] = {0};
#endif

#if defined (TICKLESS_RECORD)
#define DUMP_RECORD_ITEM(x) printf( "[TSR]: %s:%lld,%lld,%lld,%lld,%lld \r\n", #x, g_ts_record[x][0], g_ts_record[x][1], g_ts_record[x][2], g_ts_record[x][3], g_ts_record[x][4])
#else
#define DUMP_RECORD_ITEM(x)
#endif

void time_static_record_dump(void) {
#define _X(x) DUMP_RECORD_ITEM(x);
  TS_STUBS
#undef _X
#ifdef TICKLESS_RECORD
  memset(g_ts_record, 0, sizeof(g_ts_record));
#endif
}
