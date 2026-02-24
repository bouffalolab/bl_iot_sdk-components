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
#include "utils_getopt.h"
#include "utils_hex.h"

#ifdef CFG_IPV6
#include "lwip/tcpip.h"
#include "lwip/ip_addr.h"
#endif
#include "wifi_mgmr_cli.h"
#include "wifi_mgmr_ext.h"

#ifdef BL_IOT_SDK
#include "utils_string.h"
#include "cli.h"
#include "utils_hexdump.h"
#include "bflb_os_system.h"

#define SHELL_CMD_EXPORT_ALIAS(func, name, desc)                                                                \
    static void func##_adapter (char *buf, int len, int argc, char **argv)                                    \
    {                                                                                                           \
        func(argc, argv);                                                                                       \
                                                                                                                \
    }                                                                                                           \
    const static struct cli_command name##cli[] STATIC_CLI_CMD_ATTRIBUTE = {                                    \
                                                                            {#name, #desc , func##_adapter}   \
    }
#else
#include "shell.h"
#endif

SHELL_CMD_EXPORT_ALIAS(cmd_rf_dump, rf_dump, rf dump);

SHELL_CMD_EXPORT_ALIAS(wifi_bcnint_set, wifi_ap_bcnint_set, wifi ap bcnin set);
SHELL_CMD_EXPORT_ALIAS(wifi_capcode_cmd, wifi_capcode, wifi capcode );
SHELL_CMD_EXPORT_ALIAS(wifi_scan_cmd, wifi_scan, wifi scan );
SHELL_CMD_EXPORT_ALIAS(wifi_scan_filter_cmd, wifi_scan_filter, wifi scan );
SHELL_CMD_EXPORT_ALIAS(wifi_mon_cmd, wifi_mon, wifi monitor);
SHELL_CMD_EXPORT_ALIAS(cmd_wifi_raw_send, wifi_raw_send, wifi raw send test );
SHELL_CMD_EXPORT_ALIAS(wifi_sta_ip_info, wifi_sta_info, wifi sta info );
SHELL_CMD_EXPORT_ALIAS(wifi_sta_ip_set_cmd, wifi_sta_ip_set, wifi STA IP config [ip] [mask] [gw] [dns1] [dns2]);
SHELL_CMD_EXPORT_ALIAS(wifi_sta_ip_unset_cmd, wifi_sta_ip_unset, wifi STA IP config unset);
SHELL_CMD_EXPORT_ALIAS(wifi_disconnect_cmd, wifi_sta_disconnect, wifi station disconnect);
SHELL_CMD_EXPORT_ALIAS(wifi_connect_cmd, wifi_sta_connect, wifi station connect);
SHELL_CMD_EXPORT_ALIAS(wifi_sta_get_state_cmd, wifi_sta_get_state, wifi sta get state);
SHELL_CMD_EXPORT_ALIAS(wifi_enable_autoreconnect_cmd, wifi_sta_autoconnect_enable, wifi station enable auto reconnect);
SHELL_CMD_EXPORT_ALIAS(wifi_disable_autoreconnect_cmd, wifi_sta_autoconnect_disable, wifi station disable auto reconnect);
SHELL_CMD_EXPORT_ALIAS(wifi_rc_fixed_enable, rc_fix_en, wifi rate control fixed rate enable);
SHELL_CMD_EXPORT_ALIAS(wifi_rc_fixed_disable, rc_fix_dis, wifi rate control fixed rate diable);
SHELL_CMD_EXPORT_ALIAS(wifi_power_saving_on_cmd, wifi_sta_ps_on, wifi power saving mode ON);
SHELL_CMD_EXPORT_ALIAS(wifi_power_saving_off_cmd, wifi_sta_ps_off, wifi power saving mode OFF);
SHELL_CMD_EXPORT_ALIAS(wifi_power_saving_set, wifi_sta_ps_set, set wifi ps mode active time);
SHELL_CMD_EXPORT_ALIAS(wifi_power_saving_get, wifi_sta_ps_get, get wifi ps mode);
SHELL_CMD_EXPORT_ALIAS(wifi_denoise_enable_cmd, wifi_sta_denoise_enable, wifi denoise);
SHELL_CMD_EXPORT_ALIAS(wifi_denoise_disable_cmd, wifi_sta_denoise_disable, wifi denoise);
SHELL_CMD_EXPORT_ALIAS(wifi_sniffer_on_cmd, wifi_sniffer_on, wifi sniffer mode on);
SHELL_CMD_EXPORT_ALIAS(wifi_sniffer_off_cmd, wifi_sniffer_off, wifi sniffer mode off);
SHELL_CMD_EXPORT_ALIAS(cmd_wifi_ap_start, wifi_ap_start, start Ap mode [channel] [max_sta_supported]);
SHELL_CMD_EXPORT_ALIAS(cmd_wifi_ap_stop, wifi_ap_stop, stop Ap mode);
SHELL_CMD_EXPORT_ALIAS(cmd_wifi_ap_chan_switch, wifi_ap_chan_switch, switch AP channel);
SHELL_CMD_EXPORT_ALIAS(cmd_wifi_ap_conf_max_sta, wifi_ap_conf_max_sta, config Ap max sta);
SHELL_CMD_EXPORT_ALIAS(cmd_wifi_dump, wifi_dump, dump fw statistic);
SHELL_CMD_EXPORT_ALIAS(cmd_wifi_cfg, wifi_cfg, wifi cfg cmd);
SHELL_CMD_EXPORT_ALIAS(cmd_dump_reset, wifi_pkt, wifi dump needed);
SHELL_CMD_EXPORT_ALIAS(cmd_wifi_coex_rf_force_on, wifi_coex_rf_force_on, wifi coex RF forece on);
SHELL_CMD_EXPORT_ALIAS(cmd_wifi_coex_rf_force_off, wifi_coex_rf_force_off, wifi coex RF forece off);
SHELL_CMD_EXPORT_ALIAS(cmd_wifi_coex_pti_force_on, wifi_coex_pti_force_on, wifi coex PTI forece on);
SHELL_CMD_EXPORT_ALIAS(cmd_wifi_coex_pti_force_off, wifi_coex_pti_force_off, wifi coex PTI forece off);
SHELL_CMD_EXPORT_ALIAS(cmd_wifi_coex_pta_set, wifi_coex_pta_set, wifi coex PTA set);
SHELL_CMD_EXPORT_ALIAS(wifi_ap_sta_list_get_cmd, wifi_sta_list, get sta list in AP mode);
SHELL_CMD_EXPORT_ALIAS(wifi_ap_sta_delete_cmd, wifi_sta_del, delete one sta in AP mode);
SHELL_CMD_EXPORT_ALIAS(wifi_edca_dump_cmd, wifi_edca_dump, dump EDCA data);
SHELL_CMD_EXPORT_ALIAS(cmd_wifi_state_get, wifi_state, get wifi_state);
SHELL_CMD_EXPORT_ALIAS(cmd_wifi_power_table_update, wifi_update_power, Power table test command);

int wifi_mgmr_cli_init(void)
{
    // static command(s) do NOT need to call aos_cli_register_command(s) to register.
    // However, calling aos_cli_register_command(s) here is OK but is of no effect as cmds_user are included in cmds list.
    // XXX NOTE: Calling this *empty* function is necessary to make cmds_user in this file to be kept in the final link.
    //return aos_cli_register_commands(cmds_user, sizeof(cmds_user)/sizeof(cmds_user[0]));
    return 0;
}
