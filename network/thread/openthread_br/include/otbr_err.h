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

#ifndef __OTBR_ERR_H__
#define __OTBR_ERR_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    OTBR_ERROR_NONE               = 0, ///< No error.

    OTBR_ERROR_ERRNO              = -1,  ///< Error defined by errno.
    OTBR_ERROR_DBUS               = -2,  ///< DBus error.
    OTBR_ERROR_MDNS               = -3,  ///< mDNS error.
    OTBR_ERROR_OPENTHREAD         = -4,  ///< OpenThread error.
    OTBR_ERROR_REST               = -5,  ///< Rest Server error.
    OTBR_ERROR_SMCROUTE           = -6,  ///< SMCRoute error.
    OTBR_ERROR_NOT_FOUND          = -7,  ///< Not found.
    OTBR_ERROR_PARSE              = -8,  ///< Parse error.
    OTBR_ERROR_NOT_IMPLEMENTED    = -9,  ///< Not implemented error.
    OTBR_ERROR_INVALID_ARGS       = -10, ///< Invalid arguments error.
    OTBR_ERROR_DUPLICATED         = -11, ///< Duplicated operation, resource or name.
    OTBR_ERROR_ABORTED            = -12, ///< The operation is aborted.
    OTBR_ERROR_INVALID_STATE      = -13, ///< The target isn't in a valid state.
    OTBR_ERROR_INFRA_LINK_CHANGED = -14, ///< The infrastructure link is changed.
    OTBR_ERROR_NO_MEM             = -15, ///< No memory available.
} otbrError_t;

#ifdef __cplusplus
}
#endif

#endif /* __OTBR_ERR_H__ */