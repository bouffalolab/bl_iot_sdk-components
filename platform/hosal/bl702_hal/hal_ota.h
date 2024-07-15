#ifndef __HAL_OTA_H__
#define __HAL_OTA_H__

#include <stdint.h>

#define HAL_OTA_FLASH_WRITE_ON_UPDATE   1  // on each hal_ota_update, write all data to flash
#define HAL_OTA_FLASH_WRITE_BY_PAGE     2  // on each hal_ota_update, write data to flash by page (256 bytes), and cache the remainder
#define HAL_OTA_POLICY                  HAL_OTA_FLASH_WRITE_BY_PAGE

int hal_ota_start(uint8_t *pt, uint32_t len);
int hal_ota_update(uint8_t *data, uint32_t len);
int hal_ota_finish(void);

#endif
