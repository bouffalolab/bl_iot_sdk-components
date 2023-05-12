#ifndef __FIRMWARE_WDR_H__
#define __FIRMWARE_WDR_H__

#include "bl_comm_sns.h"
#include "bl_comm_aaa.h"
#include "img_struct.h"

typedef struct {
    aaa_stats_cfg_t stats_cfg;
} wdr_config_t;

int  wdr_init(void);

/*** sw ***/
void wdr_get_sw_config(wdr_param_sw_t *config);
void wdr_set_sw_config(wdr_param_sw_t *config);
int wdr_sw_get_fw_register(wdr_param_sw_t *sw, wdr_fw_config_t *fw);

/*** fw ***/
int wdr_fw_set_sw_register(wdr_fw_config_t *fw);
int  wdr_config(wdr_config_t *config, bool is_virtual_sensor);
void wdr_get_fw_config(wdr_fw_config_t *config);
int  wdr_control(const wdr_fw_config_t *config);

/*** hw ***/
void wdr_hw_update(void);

#endif
