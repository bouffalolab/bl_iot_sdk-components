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
#ifndef __BL_FLASH_H__
#define __BL_FLASH_H__
#include <stdint.h>

typedef struct {
    uint8_t encrypt_type;
    uint8_t aes_region_lock;
    uint8_t aes_iv[16];
    uint32_t flash_offset;
    uint32_t img_len;
}encrypt_info_t;

typedef struct {
    uint8_t crc_ignore;
    uint8_t hash_ignore;
    uint8_t boot2_internal;
    uint8_t rollback_enable;
}bootcfg_info_t;

int bl_flash_erase(uint32_t addr, int len);
int bl_flash_write(uint32_t addr, uint8_t *src, int len);
int bl_flash_read(uint32_t addr, uint8_t *dst, int len);
int bl_flash_erase_need_lock(uint32_t addr, int len);
int bl_flash_write_need_lock(uint32_t addr, uint8_t *src, int len);
int bl_flash_read_need_lock(uint32_t addr, uint8_t *dst, int len);
int bl_flash_config_update(void);
int bl_flash_fw_protect_set(uint8_t en, uint32_t fw_addr, uint32_t fw_size, uint32_t pt_addr, uint32_t pt_size);
uint32_t bl_flash_get_size(void);
void* bl_flash_get_flashCfg(void);
void bl_flash_get_encryptInfo(encrypt_info_t *info);
void bl_flash_get_bootcfgInfo(bootcfg_info_t *info);

int bl_flash_read_byxip(uint32_t addr, uint8_t *dst, int len);
int bl_flash_init(void);
#endif
