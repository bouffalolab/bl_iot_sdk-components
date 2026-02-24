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
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <ef_def.h>
#include <easyflash.h>

#include <openthread_port.h>
#include <openthread/platform/settings.h>

#define OT_MAX_KEY_LEN              20
#define OT_MAX_ENTRY_NUM            1024
#define OT_MAX_ENTRY_BITMAP         ((OT_MAX_ENTRY_NUM + 31) / 32) 

void otPlatSettingsInit(otInstance *aInstance, const uint16_t *aSensitiveKeys, uint16_t aSensitiveKeysLength) {}

otError otPlatSettingsGet(otInstance *aInstance, uint16_t aKey, int aIndex, uint8_t *aValue, uint16_t *aValueLength)
{
    char            key[OT_MAX_KEY_LEN];
    uint32_t        bitmapArray[OT_MAX_ENTRY_BITMAP], i, j;
    size_t          valuLength = 0;
    otError         ret = OT_ERROR_NONE;
    struct env_node_obj obj;

    if (aInstance) {
        ef_port_env_lock();

        do {
            if (aIndex < 0) {
                ret = OT_ERROR_NOT_FOUND;
                break;
            }

            snprintf(key, OT_MAX_KEY_LEN, "ot-bm-%x", aKey);
            ef_get_env_blob(key, bitmapArray, OT_MAX_ENTRY_BITMAP * 4, (size_t *)&valuLength);
            if (valuLength != OT_MAX_ENTRY_BITMAP * 4) {
                ret = OT_ERROR_NOT_FOUND;
                break;
            }

            i = aIndex >> 5;
            j = aIndex & 0xfffff;
            if (0 == (bitmapArray[i] & (1 << j))) {
                ret = OT_ERROR_NOT_FOUND;
                break;
            }

            snprintf(key, OT_MAX_KEY_LEN, "ot-%x-%x", aKey, aIndex);
            if (!ef_get_env_obj(key, &obj)) {

                snprintf(key, OT_MAX_KEY_LEN, "ot-bm-%x", aKey);
                bitmapArray[i] = bitmapArray[i] & (~(1 << j));

                ef_set_env_blob(key, bitmapArray, OT_MAX_ENTRY_BITMAP * 4);
                ret = OT_ERROR_NOT_FOUND;
                break;
            }

            if (aValueLength) {
                if (aValue) {
                    ef_get_env_blob(key, aValue, *aValueLength, (size_t *)&valuLength);
                    memcpy(aValueLength, &valuLength, sizeof(uint16_t));
                }
                else {
                    memcpy(aValueLength, &obj.value_len, sizeof(uint16_t));
                }
            }
        } while (0);

        ef_port_env_unlock();
    }
    else {
        snprintf(key, OT_MAX_KEY_LEN, "_nl%04x", aKey);
        ef_get_env_blob(key, aValue, *aValueLength, (size_t *)&valuLength);
    }

    return ret;
}

static bool otPlatSettingsSet_raw(otInstance *aInstance, uint16_t aKey, const uint8_t *aValue, uint16_t aValueLength, 
    int index, char * key, uint32_t * bitmapArray) 
{
    EfErrCode   ret;
    uint32_t i = index >> 5;
    uint32_t j = index & 0xfffff;

    if (aInstance) {
        bitmapArray[i] = bitmapArray[i] | (1 << j);

        ef_port_env_lock();
        do {
            snprintf(key, OT_MAX_KEY_LEN, "ot-%x-%x", aKey, index);
            ret = ef_set_env_blob(key, aValue, aValueLength);

            if (EF_NO_ERR == ret) {
                snprintf(key, OT_MAX_KEY_LEN, "ot-bm-%x", aKey);
                ret = ef_set_env_blob(key, bitmapArray, OT_MAX_ENTRY_BITMAP * 4);
            }
        } while (0);
        ef_port_env_unlock();
    }
    else {
        snprintf(key, OT_MAX_KEY_LEN, "_nl%04x", aKey);
        ret = ef_set_env_blob(key, aValue, aValueLength);
    }

    return EF_NO_ERR == ret? OT_ERROR_NONE : OT_ERROR_FAILED;
}

otError otPlatSettingsSet(otInstance *aInstance, uint16_t aKey, const uint8_t *aValue, uint16_t aValueLength)
{
    char            key[OT_MAX_KEY_LEN];
    uint32_t        bitmapArray[OT_MAX_ENTRY_BITMAP];

    memset(bitmapArray, 0, OT_MAX_ENTRY_BITMAP * 4);
    return otPlatSettingsSet_raw(aInstance, aKey, aValue, aValueLength, 0, key, bitmapArray);
}

otError otPlatSettingsAdd(otInstance *aInstance, uint16_t aKey, const uint8_t *aValue, uint16_t aValueLength)
{
    char        key[OT_MAX_KEY_LEN];
    uint32_t    bitmapArray[OT_MAX_ENTRY_BITMAP];
    size_t      len = 0, index = 0, i, j, entryNum;
    otError     ret = OT_ERROR_NONE;

    ef_port_env_lock();

    do {
        snprintf(key, OT_MAX_KEY_LEN, "ot-bm-%x", aKey);
        ef_get_env_blob(key, bitmapArray, OT_MAX_ENTRY_BITMAP * 4, (size_t *)&len);
        if (len != OT_MAX_ENTRY_BITMAP * 4) {
            memset(bitmapArray, 0, OT_MAX_ENTRY_BITMAP * 4);
            ret = otPlatSettingsSet_raw(aInstance, aKey, aValue, aValueLength, 0, key, bitmapArray);
        }
        else {
            /**
             * length value read from should be aligned with 4 bytes (4 times)
             */
            entryNum = len * 8;
            for (index = 0; index < entryNum; index ++) {
                i = index >> 5;
                j = index & 0xfffff;
                if ((bitmapArray[i] & (1 << j)) == 0) {
                    /**
                     * empty index
                     */
                    break;
                }
            }

            if (index == entryNum) {
                ret = OT_ERROR_NO_BUFS;
                break;
            }

            ret = otPlatSettingsSet_raw(aInstance, aKey, aValue, aValueLength, index, key, bitmapArray);
        }
    } while (0);

    ef_port_env_unlock();

    return ret;
}

otError otPlatSettingsDelete(otInstance *aInstance, uint16_t aKey, int aIndex)
{
    char        key[OT_MAX_KEY_LEN];
    uint32_t    bitmapArray[OT_MAX_ENTRY_BITMAP];
    size_t      i, j, len;
    otError     ret = OT_ERROR_NONE;

    ef_port_env_lock();

    snprintf(key, OT_MAX_KEY_LEN, "ot-bm-%x", aKey);
    do {

        if (aIndex < 0) {
            ef_del_env(key);
            break;
        }

        ef_get_env_blob(key, bitmapArray, OT_MAX_ENTRY_BITMAP * 4, (size_t *)&len);
        if (len != OT_MAX_ENTRY_BITMAP * 4) {
            ret = OT_ERROR_NOT_FOUND;
            break;
        }

        i = aIndex >> 5;
        j = aIndex & 0xfffff;
        if (! (bitmapArray[i] & (1 << j))) {
            ret = OT_ERROR_NOT_FOUND;
            break;
        }

        bitmapArray[i] = bitmapArray[i] & (~(1 << j));

        snprintf(key, OT_MAX_KEY_LEN, "ot-bm-%x", aKey);
        ef_set_env_blob(key, bitmapArray, OT_MAX_ENTRY_BITMAP * 4);

        snprintf(key, OT_MAX_KEY_LEN, "ot-%x-%x", aKey, aIndex);
        ef_del_env(key);

    } while (0);

    ef_port_env_unlock();

    return ret;
}

void otPlatSettingsWipe(otInstance *aInstance)
{
    /** reset all */
    ef_env_set_default();
}

void otPlatSettingsDeinit(otInstance *aInstance)
{}
