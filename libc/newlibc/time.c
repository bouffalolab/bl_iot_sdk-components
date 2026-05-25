/*
 * Copyright (c) 2016-2026 Bouffalolab.
 *
 * This file is part of
 *     *** Bouffalolab Software Dev Kit ***
 *      (see www.bouffalolab.com).
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of Bouffalo Lab nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <reent.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <limits.h>
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

        /* CVE-2024-30949 fix: Prevent integer overflow in time conversion */
        if (epoch_time > UINT64_MAX - 1000) {
            reent->_errno = EOVERFLOW;
            return -1;
        }

        tp->tv_sec = epoch_time / 1000;

        /* Safe calculation avoiding overflow */
        uint64_t msec_remainder = epoch_time % 1000;
        if (msec_remainder > UINT64_MAX / 1000) {
            reent->_errno = EOVERFLOW;
            return -1;
        }
        tp->tv_usec = msec_remainder * 1000;
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
        /* CVE-2024-30949 fix: Prevent integer overflow in time conversion */
        uint64_t sec_msec;

        /* Check for overflow in seconds to milliseconds conversion */
        if (tp->tv_sec > UINT64_MAX / 1000) {
            reent->_errno = EOVERFLOW;
            return -1;
        }
        sec_msec = (uint64_t)tp->tv_sec * 1000;

        /* Check for overflow in addition */
        uint64_t usec_msec = (uint64_t)tp->tv_usec / 1000;
        if (sec_msec > UINT64_MAX - usec_msec) {
            reent->_errno = EOVERFLOW;
            return -1;
        }

        bl_sys_time_update(sec_msec + usec_msec);
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
