#include <reent.h>
#include <errno.h>
#include <stdio.h>
#include <sys/time.h>

#include "bl_sys_time.h"

static struct timezone tz = {
    .tz_dsttime = 0,
    .tz_minuteswest = 0
};

int _gettimeofday_r(struct _reent *reent, struct timeval *tp, void *tzvp)
{
    struct timezone *tzp = tzvp;
    uint64_t epoch_time = 0;

    if (tp) {
        bl_sys_time_get(&epoch_time);
        tp->tv_sec = epoch_time / 1000;
        tp->tv_usec = (epoch_time % 1000) * 1000;
    }

    if (tzp) {
        tzp->tz_minuteswest = tz.tz_minuteswest;
        tzp->tz_dsttime = tz.tz_dsttime;
    }

    return 0;
}

int _settimeofday_r(struct _reent *reent, const struct timeval *tp, const struct timezone *tzp)
{
    if (tp) {
        bl_sys_time_update((uint64_t)tp->tv_sec * 1000 + (uint64_t)tp->tv_usec/1000);
    }

    if (tzp) {
        tz.tz_dsttime = tzp->tz_dsttime;
        tz.tz_minuteswest = tzp->tz_minuteswest;
    }

    return 0;
}

int settimeofday(const struct timeval *tp, const struct timezone *tzp)
{
    return _settimeofday_r(_REENT, tp, tzp);
}
