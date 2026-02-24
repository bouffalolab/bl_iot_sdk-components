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
typedef struct 
{
    const char * mKey;
    const uint8_t * mData;
    uint32_t mDataSize;
} txt_item_t;

typedef struct {
    char * mInstanceName;
    char * mType;
    char * mProtocol;
    char * mHostName;
    uint16_t mPort;
    uint32_t mTtl;
    int mAddressNum;
    ip_addr_t *mAddress;
    int mTxtItemNum;
    txt_item_t *mTxtItems;
} mdns_discover_result_t;

typedef void (*mdns_result_callback_t)(int aNumResult, mdns_discover_result_t *aResult, struct _mdns_discover_param * aDiscoverParam);

typedef struct _mdns_discover_param {
    char * mInstanceName;
    char * mType;
    char * mProtocol;
    bool  mIsBrowse;
    uint16_t mAddressType;
    mdns_result_callback_t mCallback;
    void * arg1;
    void * arg2;
} mdns_discover_param_t;

otbrError WiFiDnssdPublishService(const char * aHostName, const char *aInstanceName, const char *aServiceType, const char *aProtocol, 
    int aPort, txt_item_t *aTxtItems, int aTxtItemNum, const char **aSubTypes, int aSubTypesNum);

otbrError WiFiDnssdRemoveServices();

otbrError WiFiDnssdBrowse(const char * aType, const char *aProtocol, enum lwip_ip_addr_type aAddrType, mdns_result_callback_t aCallback, 
    uint32_t aTimeout, int aMaxResult, void * arg1, void *arg2);

otbrError WiFiDnssdResolve(const char * aInstanceName, const char *aType, const char * aProtocol, mdns_result_callback_t aCallback, 
    uint32_t aTimeout, int aMaxResult, void * arg1, void * arg2);
