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
#ifndef __MATTER_FACTORY_DATA_H
#define __MATTER_FACTORY_DATA_H

#ifdef __cplusplus
extern "C" {
#endif

#define VERSION_MFD_MAJOR 1
#define VERSION_MFD_MINOR 6
#define VERSION_MFD_PATCH 1

bool mfd_init(void);

int mfd_getDacCert(uint8_t *p, uint32_t size);
uint8_t *mfd_getDacCertPtr(uint32_t *psize);
int mfd_getDacPrivateKey(uint8_t *p, uint32_t size);
uint8_t *mfd_getDacPrivateKeyPtr(uint32_t *psize);
int mfd_getPasscode(uint8_t *p, uint32_t size);
int mfd_getPaiCert(uint8_t *p, uint32_t size);
int mfd_getCd(uint8_t *p, uint32_t size);
int mfd_getSerialNumber(char *p, uint32_t size);
int mfd_getDiscriminator(uint8_t *p, uint32_t size);
int mfd_getRotatingDeviceIdUniqueId(uint8_t *p, uint32_t size);
int mfd_getSapke2It(uint8_t *p, uint32_t size);
int mfd_getSapke2Salt(uint8_t *p, uint32_t size);
int mfd_getSapke2Verifier(uint8_t *p, uint32_t size);
/** from device tree */
int mfd_getVendorName (char * buf, uint32_t size);
int mfd_getVendorId(uint8_t *buf, uint32_t size);
int mfd_getProductName(char * buf, uint32_t size);
int mfd_getProductId(uint8_t *buf, uint32_t size);
int mfd_getPartNumber(char *buf, uint32_t size);
int mfd_getProductUrl(char * buf, uint32_t size);
int mfd_getProductLabel(char * buf, uint32_t size);
bool mfd_getManufacturingDate(uint16_t *pYear, uint8_t *pMonth, uint8_t *pDay);
int mfd_getHardwareVersion(uint8_t * buf, uint32_t size);
int mfd_getHardwareVersionString(char * buf, uint32_t size);

int mfd_getElementById(int16_t id, uint8_t * buf, uint32_t size);

#ifdef __cplusplus
}
#endif

#endif
