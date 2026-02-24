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
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

int cmd_rf_dump(int argc, char **argv);
int wifi_bcnint_set(int argc, char **argv);
int wifi_capcode_cmd(int argc, char **argv);
int wifi_scan_cmd(int argc, char **argv);
int wifi_scan_filter_cmd(int argc, char **argv);
int wifi_mon_cmd(int argc, char **argv);
int cmd_wifi_raw_send(int argc, char **argv);
int wifi_sta_ip_info(int argc, char **argv);
int wifi_sta_ip_set_cmd(int argc, char **argv);
int wifi_sta_ip_unset_cmd(int argc, char **argv);
int wifi_disconnect_cmd(int argc, char **argv);
int wifi_connect_cmd(int argc, char **argv);
int wifi_sta_get_state_cmd(int argc, char **argv);
int wifi_enable_autoreconnect_cmd(int argc, char **argv);
int wifi_disable_autoreconnect_cmd(int argc, char **argv);
int wifi_rc_fixed_enable(int argc, char **argv);
int wifi_rc_fixed_disable(int argc, char **argv);
int wifi_power_saving_on_cmd(int argc, char **argv);
int wifi_power_saving_off_cmd(int argc, char **argv);
int wifi_power_saving_set(int argc, char **argv);
int wifi_power_saving_get(int argc, char **argv);
int wifi_denoise_enable_cmd(int argc, char **argv);
int wifi_denoise_disable_cmd(int argc, char **argv);
int wifi_sniffer_on_cmd(int argc, char **argv);
int wifi_sniffer_off_cmd(int argc, char **argv);
int cmd_wifi_ap_start(int argc, char **argv);
int cmd_wifi_ap_stop(int argc, char **argv);
int cmd_wifi_ap_chan_switch(int argc, char **argv);
int cmd_wifi_ap_conf_max_sta(int argc, char **argv);
int cmd_wifi_dump(int argc, char **argv);
int cmd_wifi_cfg(int argc, char **argv);
int cmd_dump_reset(int argc, char **argv);
int cmd_wifi_coex_rf_force_on(int argc, char **argv);
int cmd_wifi_coex_rf_force_off(int argc, char **argv);
int cmd_wifi_coex_pti_force_on(int argc, char **argv);
int cmd_wifi_coex_pti_force_off(int argc, char **argv);
int cmd_wifi_coex_pta_set(int argc, char **argv);
int wifi_ap_sta_list_get_cmd(int argc, char **argv);
int wifi_ap_sta_delete_cmd(int argc, char **argv);
int wifi_edca_dump_cmd(int argc, char **argv);
int cmd_wifi_state_get(int argc, char **argv);
int cmd_wifi_power_table_update(int argc, char **argv);

int wifi_mgmr_cli_init(void);

