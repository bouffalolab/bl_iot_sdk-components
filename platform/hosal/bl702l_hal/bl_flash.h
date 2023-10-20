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

int bl_flash_erase(uint32_t addr, int len);
int bl_flash_write(uint32_t addr, uint8_t *src, int len);
int bl_flash_read(uint32_t addr, uint8_t *dst, int len);
int bl_flash_erase_need_lock(uint32_t addr, int len);
int bl_flash_write_need_lock(uint32_t addr, uint8_t *src, int len);
int bl_flash_read_need_lock(uint32_t addr, uint8_t *dst, int len);
int bl_flash_config_update(void);
void* bl_flash_get_flashCfg(void);
void bl_flash_get_encryptInfo(encrypt_info_t *info);

int bl_flash_read_byxip(uint32_t addr, uint8_t *dst, int len);
int bl_flash_init(void);
#endif
