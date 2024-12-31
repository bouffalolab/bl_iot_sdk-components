#ifndef __BL_EFUSE_H__
#define __BL_EFUSE_H__

#include <stdint.h>
#include <math.h>

int bl_efuse_reload(void);  // reload all data from efuse to efuse registers
int bl_efuse_read(uint32_t offset, uint32_t *data, uint32_t len);  // read data from efuse registers in words
int bl_efuse_write(uint32_t offset, uint32_t *data, uint32_t len);  // write data to efuse registers in words
int bl_efuse_program(void);  // write all data in efuse registers to efuse permanently

int bl_efuse_read_mac(uint8_t mac[8]);
int bl_efuse_read_mac_factory(uint8_t mac[8]);
int bl_efuse_read_capcode(uint8_t *capcode);
int bl_efuse_read_pwroft(int8_t poweroffset[2]);
int bl_efuse_read_pwroft_ex(int8_t poweroffset_zigbee[16], int8_t poweroffset_ble[40]);
int bl_efuse_read_tsen_refcode(int16_t *refcode);
int bl_efuse_read_adc_gain_coe(float *coe);

#endif
