#ifndef __HAL_ANTI_ROLLBACK_H__
#define __HAL_ANTI_ROLLBACK_H__

#include <stdint.h>

int32_t hal_get_boot2_version_from_efuse(uint8_t *version);
int32_t hal_set_boot2_version_to_efuse(uint8_t version);
int32_t hal_get_app_version_from_efuse(uint8_t *version);
int32_t hal_set_app_version_to_efuse(uint8_t version);

#endif
