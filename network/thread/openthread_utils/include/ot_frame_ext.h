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
#ifndef __OT_FRAME_EXT_H
#define __OT_FRAME_EXT_H

#define IE_TERINATION2_ID                   0x7f
#define IE_CSL_ID                           0x1a

#define IE_VENDOR_ID                        VENDOR_IE_HEADER_ID
#define IE_THREAD_VENDOR_ID                 VENDOR_IE_THREAD_OUI
#define IE_THREAD_ENH_ACK_PROBING_IE_ID     VENDOR_IE_THREAD_ENH_ACK_PROBING
#define IE_THREAD_HDR_SIZE                  4

typedef enum {
    OTINTERNEL_ADDR_TYPE_NONE       = 0,
    OTINTERNEL_ADDR_TYPE_SHORT      = 1,
    OTINTERNEL_ADDR_TYPE_EXT        = 2,
} otInternel_addrType_t;

typedef union _otRadio_addr {
    uint8_t                     extAddress[8];
    uint16_t                    shortAddress;
} __packed otRadio_addr_t;

typedef struct _otRadio_ieHdr {
    uint16_t    length:7;
    uint16_t    elementId:8;
    uint16_t    type:1;
} __packed otRadio_ieHdr_t;

#endif