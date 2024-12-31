#ifndef __BL_FLASH_H__
#define __BL_FLASH_H__
#include <stdint.h>

int bl_flash_erase(uint32_t addr, int len);
int bl_flash_write(uint32_t addr, uint8_t *src, int len);
int bl_flash_read(uint32_t addr, uint8_t *dst, int len);
int bl_flash_erase_need_lock(uint32_t addr, int len);
int bl_flash_write_need_lock(uint32_t addr, uint8_t *src, int len);
int bl_flash_read_need_lock(uint32_t addr, uint8_t *dst, int len);
int bl_flash_config_update(void);
int bl_flash_fw_protect_set(uint8_t en, uint32_t addr, uint32_t size);
uint32_t bl_flash_get_size(void);
void* bl_flash_get_flashCfg(void);

int bl_flash_read_byxip(uint32_t addr, uint8_t *dst, int len);
int bl_flash_init(void);
#endif
