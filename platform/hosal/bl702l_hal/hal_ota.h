#ifndef __HAL_OTA_H__
#define __HAL_OTA_H__

#include <stdint.h>

int hal_ota_start(uint8_t *pt, uint32_t len);
int hal_ota_update(uint8_t *data, uint32_t len);
int hal_ota_finish(void);

#endif
