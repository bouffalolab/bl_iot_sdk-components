#ifndef __BL_WIRELESS_H__
#define __BL_WIRELESS_H__

#include <stdint.h>

#define MAX_CAPCODE_TABLE_SIZE     30

int bl_wireless_mac_addr_set(uint8_t mac[8]);
int bl_wireless_mac_addr_get(uint8_t mac[8]);
int bl_wireless_power_offset_set(int8_t poweroffset_zigbee[16], int8_t poweroffset_ble[4]);
int bl_wireless_power_offset_get(int8_t poweroffset_zigbee[16], int8_t poweroffset_ble[4]);
int bl_wireless_capcode_offset_table_set(int8_t temp[], int8_t offset[], uint8_t size);
int bl_wireless_capcode_offset_table_get(int8_t temp[], int8_t offset[], uint8_t *size);
int8_t bl_wireless_capcode_offset_get(int8_t temp);
void bl_wireless_default_tx_power_set(int8_t power);
int8_t bl_wireless_default_tx_power_get(void);
void bl_wireless_power_tcal_en_set(uint8_t en);
uint8_t bl_wireless_power_tcal_en_get(void);
void bl_wireless_capcode_tcal_en_set(uint8_t en);
uint8_t bl_wireless_capcode_tcal_en_get(void);

#endif
