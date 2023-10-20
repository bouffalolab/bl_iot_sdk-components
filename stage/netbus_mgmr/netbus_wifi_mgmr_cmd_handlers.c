#include <stdio.h>
#include <stdint.h>
#include <FreeRTOS.h>
#include <task.h>
#include <bl_wifi.h>
#include <wifi_mgmr_ext.h>
#include <blog.h>
#include <netbus_wifi_mgmr_cmd_handlers.h>
#include <supplicant_api.h>

static void stop_ap(void)
{
    wifi_mgmr_ap_stop(NULL);
}

static void stop_sta(void)
{
    int state1 = WIFI_STATE_UNKNOWN;
    int tries = 100;
    bool do_disconnect = false;

    wifi_mgmr_sta_autoconnect_disable();
    extern int wifi_mgmr_state_get_internal(int *state);
    wifi_mgmr_state_get_internal(&state1);

    switch (state1) {
        case WIFI_STATE_IDLE:
            break;
        case WIFI_STATE_CONNECTING:
            while (tries > 0) {
                vTaskDelay(pdMS_TO_TICKS(100));
                wifi_mgmr_state_get_internal(&state1);
                if (state1 != WIFI_STATE_CONNECTING) {
                    break;
                }
                tries--;
            }
            do_disconnect = true;
            break;
        case WIFI_STATE_CONNECTED_IP_GETTING:
            // fallthrough
        case WIFI_STATE_CONNECTED_IP_GOT:
            // fallthrough
        case WIFI_STATE_DISCONNECT:
            do_disconnect = true;
            break;
        // TODO: AP
        default:
            do_disconnect = false;
            break;
    }

    if (do_disconnect) {
        wifi_mgmr_sta_disconnect();
        vTaskDelay(1000);
        wifi_mgmr_sta_disable(NULL);
    }
}

static void handle_reset(netbus_wifi_mode_t *mode)
{
    if (!mode) {
        log_error("handle_reset arg error.\r\n");
        return;
    }

    switch (*mode) {
        case NETBUS_WIFI_MODE_AP:
            stop_ap();
            break;
        case NETBUS_WIFI_MODE_STA:
            stop_sta();
            break;
        case NETBUS_WIFI_MODE_AP_STA:
            stop_ap();
            stop_sta();
            break;
        case NETBUS_WIFI_MODE_SNIFFER:
            wifi_mgmr_sniffer_disable();
            wifi_mgmr_sniffer_unregister(NULL);
            break;
        default:
            break;
    }

    *mode = NETBUS_WIFI_MODE_NONE;
}

int netbus_wifi_mgmr_send_ip_update(netbus_wifi_mgmr_ctx_t *env)
{
    netbus_fs_ip_update_ind_cmd_msg_t msg;

    memset(&msg, 0, sizeof(msg));
    wifi_mgmr_sta_ip_get(&msg.ip4_addr, &msg.ip4_gw, &msg.ip4_mask);
    wifi_mgmr_sta_dns_get(&msg.ip4_dns1, &msg.ip4_dns2);
    wifi_mgmr_sta_mac_get(msg.gw_mac);
    msg.hdr.cmd = BFLB_CMD_STA_IP_UPDATE_IND;

    bflbmsg_send(&env->trcver_ctx, BF1B_MSG_TYPE_CMD, &msg, sizeof(msg));
    return 0;
}

static void handle_hello(netbus_wifi_mgmr_ctx_t *env, netbus_wifi_mgmr_msg_cmd_t *cmd)
{
    // int i;
    blog_info("handle hello\r\n");
    bflb_net_wifi_trcver_set_present(&env->trcver_ctx, true);

    // uint8_t hello[] = {0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a};

    if (env->ip_got) {
        netbus_wifi_mgmr_send_ip_update(env);
    } else {
        // send NULL data
    }
}

static void handle_get_mac(netbus_wifi_mgmr_ctx_t *env, netbus_wifi_mgmr_msg_cmd_t *cmd)
{
    blog_info("handle get_mac\r\n");
    netbus_fs_mac_ind_cmd_msg_t msg;

    msg.hdr.cmd = BFLB_CMD_MAC_IND;
    msg.hdr.msg_id = cmd->msg_id;
    bl_wifi_mac_addr_get(msg.mac);
    blog_info("Responding with MAC %02X:%02X:%02X:%02X:%02X:%02X\r\n",
            msg.mac[0],
            msg.mac[1],
            msg.mac[2],
            msg.mac[3],
            msg.mac[4],
            msg.mac[5]
            );

    bflbmsg_send(&env->trcver_ctx, BF1B_MSG_TYPE_CMD, &msg, sizeof(msg));
}

static void handle_connect_ap(netbus_wifi_mgmr_ctx_t *env, netbus_wifi_mgmr_msg_cmd_t *cmd)
{
    blog_info("handle connect_ap\r\n");
    wifi_interface_t wifi_interface;
    netbus_ts_connect_ap_cmd_msg_t *ccmd;

    if (!(env->mode == NETBUS_WIFI_MODE_NONE ||
                env->mode == NETBUS_WIFI_MODE_STA)) {
        log_error("mode incorrect, ignore connect ap\r\n");
        goto ret;
    }
    if (!cmd->data_ptr || cmd->data_len != sizeof(netbus_ts_connect_ap_cmd_msg_t)) {
        log_error("msg format error\r\n");
        goto ret;
    }

    ccmd = (netbus_ts_connect_ap_cmd_msg_t *)cmd->data_ptr;
    // FIXME distinguish open AP
    if (strlen((char *)ccmd->ssid) == 0) {
        blog_info("SSID or PASSWORD NULL\r\n");
        return;
    }
    blog_info("cur mode:%d\r\n", env->mode);
    if (env->mode == NETBUS_WIFI_MODE_NONE) {
        env->mode = NETBUS_WIFI_MODE_STA;
    } else if (env->mode == NETBUS_WIFI_MODE_AP) {
        env->mode = NETBUS_WIFI_MODE_AP_STA;
    } else {
        // STA->STA, AP_STA->AP_STA
        stop_sta();
    }
    wifi_mgmr_sta_autoconnect_enable();
    wifi_interface = wifi_mgmr_sta_enable();
    blog_info("connect ssid %s, password %s\r\n", ccmd->ssid, ccmd->password);
    if (ccmd->password && strlen((char *)ccmd->password)) {
        wifi_mgmr_sta_connect(wifi_interface, (char *)ccmd->ssid, (char *)ccmd->password, NULL, NULL, 0, 0);
    }
    else {
        /** for open network */
        wifi_mgmr_sta_connect(wifi_interface, (char *)ccmd->ssid, NULL, NULL, NULL, 0, 0);
    }
ret:
    vPortFree(cmd->data_ptr);
}

static void handle_disconnect_ap(netbus_wifi_mgmr_ctx_t *env, netbus_wifi_mgmr_msg_cmd_t *cmd)
{
    blog_info("handle disconnect_ap\r\n");
    wifi_mgmr_sta_disconnect();
    /*XXX Must make sure sta is already disconnect, otherwise sta disable won't work*/
    vTaskDelay(1000);
    wifi_mgmr_sta_disable(NULL);

    blog_info("cur mode:%d\r\n", env->mode);
    if (env->mode == NETBUS_WIFI_MODE_STA) {
        env->mode = NETBUS_WIFI_MODE_NONE;
    } else if (env->mode == NETBUS_WIFI_MODE_AP_STA) {
        env->mode = NETBUS_WIFI_MODE_AP;
    } else {
       //nothing
    }

    if (cmd->data_ptr != NULL) {
        vPortFree(cmd->data_ptr);
    }
}

static void handle_ap_connected(netbus_wifi_mgmr_ctx_t *env, netbus_wifi_mgmr_msg_cmd_t *cmd)
{
    blog_info("handle ap_connected\r\n");
    netbus_min_cmd_msg_t msg;

    msg.cmd = BFLB_CMD_AP_CONNECTED_IND;

    bflbmsg_send(&env->trcver_ctx, BF1B_MSG_TYPE_CMD, &msg, sizeof(msg));
}

static void handle_ap_disconnected(netbus_wifi_mgmr_ctx_t *env, netbus_wifi_mgmr_msg_cmd_t *cmd)
{
    blog_info("handle ap_disconnect\r\n");
    netbus_min_cmd_msg_t msg;

    msg.cmd = BFLB_CMD_AP_DISCONNECTED_IND;

    bflbmsg_send(&env->trcver_ctx, BF1B_MSG_TYPE_CMD, &msg, sizeof(msg));
}

static void handle_set_auto_reconnect(netbus_wifi_mgmr_ctx_t *env, netbus_wifi_mgmr_msg_cmd_t *cmd)
{
    blog_info("handle set_auto_reconnect\r\n");
    netbus_ts_sta_set_auto_reconnect_cmd_msg_t *ccmd;

    if (env->mode != NETBUS_WIFI_MODE_STA) {
        log_error("mode incorrect, ignore set auto reconnect\r\n");
        goto ret;
    }

    if (!cmd->data_ptr || cmd->data_len != sizeof(netbus_ts_sta_set_auto_reconnect_cmd_msg_t)) {
        log_error("msg format error\r\n");
        goto ret;
    }

    ccmd = (netbus_ts_sta_set_auto_reconnect_cmd_msg_t *)cmd->data_ptr;

    if (ccmd->en) {
        blog_info("Enable sta autoconnect\r\n");
        wifi_mgmr_sta_autoconnect_enable();
    } else {
        blog_info("Disable sta autoconnect\r\n");
        wifi_mgmr_sta_autoconnect_disable();
    }
ret:
    vPortFree(cmd->data_ptr);
}

static void handle_start_ap(netbus_wifi_mgmr_ctx_t *env, netbus_wifi_mgmr_msg_cmd_t *cmd)
{
    blog_info("handle start ap\r\n");
    netbus_ts_start_ap_cmd_msg_t *ccmd;
    wifi_interface_t wifi_interface;
    int channel;
    char *passwd;

    if (!(env->mode == NETBUS_WIFI_MODE_NONE ||
                env->mode == NETBUS_WIFI_MODE_STA)) {
        log_error("mode incorrect, ignore start ap\r\n");
        goto ret;
    }
    if (!cmd->data_ptr || cmd->data_len != sizeof(netbus_ts_start_ap_cmd_msg_t)) {
        log_error("msg format error\r\n");
        goto ret;
    }

    ccmd = (netbus_ts_start_ap_cmd_msg_t *)cmd->data_ptr;

    if (strlen((char *)ccmd->ssid) == 0) {
        log_error("SSID null\r\n");
        return;
    }

    channel = ccmd->channel;
    if (ccmd->is_open) {
        passwd = NULL;
    } else {
        passwd = (char *)ccmd->password;
    }
    blog_info("Starting AP with SSID %s, channel %d, password %s\r\n",
            ccmd->ssid,
            channel,
            passwd ? passwd : "[nil]"
            );
    if (env->mode == NETBUS_WIFI_MODE_NONE) {
        env->mode = NETBUS_WIFI_MODE_AP;
    } else {
        env->mode = NETBUS_WIFI_MODE_AP_STA;
    }
    wifi_interface = wifi_mgmr_ap_enable();
    wifi_mgmr_ap_start(wifi_interface, (char *)ccmd->ssid, 0, passwd, channel);
ret:
    vPortFree(cmd->data_ptr);
}

static void handle_stop_ap(netbus_wifi_mgmr_ctx_t *env, netbus_wifi_mgmr_msg_cmd_t *cmd)
{
    blog_info("handle stop ap\r\n");
    wifi_mgmr_ap_stop(NULL);

    blog_info("cur mode:%d\r\n", env->mode);
    if (env->mode == NETBUS_WIFI_MODE_AP) {
        env->mode = NETBUS_WIFI_MODE_NONE;
    } else if (env->mode == NETBUS_WIFI_MODE_AP_STA) {
        env->mode = NETBUS_WIFI_MODE_STA;
    } else {
       //nothing
    }

    if (cmd->data_ptr != NULL) {
        vPortFree(cmd->data_ptr);
    }
}

static void handle_set_lpm_mode(netbus_wifi_mgmr_ctx_t *env, netbus_wifi_mgmr_msg_cmd_t *cmd)
{
    blog_info("handle set_lpm_mode\r\n");
    if (!(env->mode == NETBUS_WIFI_MODE_STA)) {
        log_error("mode incorrect, set lpm mode\r\n");
        goto ret;
    }
    /* log_error("NOT IMPLEMENTED\r\n"); */
    /* return; */
    netbus_ts_set_lpm_mode_cmd_msg_t *ccmd;

    if (!cmd->data_ptr || cmd->data_len != sizeof(netbus_ts_set_lpm_mode_cmd_msg_t)) {
        log_error("msg format error\r\n");
        goto ret;
    }

    ccmd = (netbus_ts_set_lpm_mode_cmd_msg_t *)cmd->data_ptr;

    log_error("Don't support ps\r\n");

    if (ccmd->en) {
        blog_info("Enable sta lpm\r\n");
        //wifi_mgmr_sta_powersaving(2);
    } else {
        blog_info("Disable sta lpm\r\n");
        //wifi_mgmr_sta_powersaving(0);
    }
ret:
    vPortFree(cmd->data_ptr);
}

//static uint8_t cipher_to_sdio(uint8_t cipher)
//{
//    uint8_t ret;
//    return (bflbwifi_cipher_t)cipher;
//}

static uint8_t auth_to_sdio(uint8_t auth)
{
    uint8_t ret;
    switch (auth) {
    default:
        /* fallthrough */
    case WIFI_EVENT_BEACON_IND_AUTH_OPEN:
        ret = BF1B_WIFI_AUTH_OPEN;
        break;
    case WIFI_EVENT_BEACON_IND_AUTH_WEP:
        ret = BF1B_WIFI_AUTH_WEP;
        break;
    case WIFI_EVENT_BEACON_IND_AUTH_WPA_PSK:
        ret = BF1B_WIFI_AUTH_WPA_PSK;
        break;
    case WIFI_EVENT_BEACON_IND_AUTH_WPA2_PSK:
        ret = BF1B_WIFI_AUTH_WPA2_PSK;
        break;
    case WIFI_EVENT_BEACON_IND_AUTH_WPA_WPA2_PSK:
        ret = BF1B_WIFI_AUTH_WPA_WPA2_PSK;
        break;
    case WIFI_EVENT_BEACON_IND_AUTH_WPA_ENT:
        ret = BF1B_WIFI_AUTH_WPA_ENTERPRISE;
        break;
    }
    return ret;
}

static void send_scan_ind(wifi_mgmr_ap_item_t *array, uint32_t cnt)
{
    int i;
    netbus_wifi_mgmr_msg_t swm_msg;
    int ind_msg_len;

    swm_msg.type = NETBUS_WIFI_MGMR_MSG_TYPE_CMD;
    swm_msg.u.cmd.cmd = BFLB_CMD_SCAN_IND;

    ind_msg_len = sizeof(netbus_fs_scan_ind_cmd_msg_t) + cnt * sizeof(struct bflbwifi_ap_record);
    netbus_fs_scan_ind_cmd_msg_t *ind_msg = (netbus_fs_scan_ind_cmd_msg_t *)pvPortMalloc(ind_msg_len);
    if (ind_msg == NULL) {
        printf("malloc failed\r\n");
        return;
    }
    ind_msg->hdr.cmd = BFLB_CMD_SCAN_IND;
    ind_msg->hdr.msg_id = g_netbus_wifi_mgmr_env.scan_cmd_id;
    ind_msg->num = cnt;
    for (i = 0; i < cnt; i++) {
        wifi_mgmr_ap_item_t *si = &array[i];
        struct bflbwifi_scan_record *di = &ind_msg->records[i];
        memset(di, 0, sizeof(*di));
        memcpy(di->bssid, si->bssid, 6);
        memcpy(di->ssid, si->ssid, 32);
        di->channel = si->channel;
        di->rssi = si->rssi;
        di->auth_mode = auth_to_sdio(si->auth);
        //di->cipher = cipher_to_sdio(wifi_mgmr_cipher_to_ext(si->cipher));
    }

    swm_msg.u.cmd.data_ptr = (void *)ind_msg;
    swm_msg.u.cmd.data_len = ind_msg_len;
    if (netbus_wifi_mgmr_msg_send(&g_netbus_wifi_mgmr_env, &swm_msg, true, false)) {
        printf("[scan ind] mgmr queue full, drop\r\n");
        vPortFree(ind_msg);
    }
}

static void do_wifi_scan(void *pvParameters)
{
    netbus_wifi_mgmr_ctx_t *env = (netbus_wifi_mgmr_ctx_t *)pvParameters;
    uint32_t cnt = 0;
    wifi_mgmr_ap_item_t *ap_ary = NULL;
    int ret;

    ret = wifi_mgmr_all_ap_scan(&ap_ary, &cnt);
    if(ret != 0) {
        printf("wifi all ap scan error %d\r\n", ret);
        goto clean;
    }

    send_scan_ind(ap_ary, cnt);

clean:
    vPortFree(ap_ary);
    env->scan_in_progress = false;
    vTaskDelete(NULL);
}

static void handle_start_scan(netbus_wifi_mgmr_ctx_t *env, netbus_wifi_mgmr_msg_cmd_t *cmd)
{
    blog_info("handle start_scan\r\n");
    if (env->scan_in_progress) {
        log_error("scan in progress\r\n");
        return;
    }
    env->scan_cmd_id = cmd->msg_id;
    env->scan_in_progress = true;

    xTaskCreate(do_wifi_scan, (char*)"wifi_scan", 2048, env, 21, NULL);
}

static void handle_scan_ind(netbus_wifi_mgmr_ctx_t *env, netbus_wifi_mgmr_msg_cmd_t *cmd)
{
    blog_info("handle scan_ind\r\n");

    bflbmsg_send(&env->trcver_ctx, BF1B_MSG_TYPE_CMD, cmd->data_ptr, cmd->data_len);
    vPortFree(cmd->data_ptr);
}

static void handle_ap_get_sta_list(netbus_wifi_mgmr_ctx_t *env, netbus_wifi_mgmr_msg_cmd_t *cmd)
{
    blog_info("handle ap get station list\r\n");
    uint8_t sta_cnt = 0, length;
    struct wifi_sta_basic_info sta_info;
    netbus_fs_sta_list_ind_cmd_msg_t *rsp = NULL;
    int filtered_sta_cnt = 0;

    wifi_mgmr_ap_sta_cnt_get(&sta_cnt);
    length = sizeof(*rsp) + sta_cnt * sizeof(struct wifi_sta_info);
    rsp = pvPortMalloc(length);
    if (rsp == NULL) {
        log_error("malloc failed!");
        netbus_fs_sta_list_ind_cmd_msg_t dummy = {
            .hdr = {
                .cmd = BFLB_CMD_GET_STA_LIST_IND,
                .msg_id = cmd->msg_id,
            },
            .num = 0
        };
        bflbmsg_send(&env->trcver_ctx, BF1B_MSG_TYPE_CMD, &dummy, sizeof(dummy));
        return ;
    }
    rsp->hdr.cmd    = BFLB_CMD_GET_STA_LIST_IND;
    rsp->hdr.msg_id = cmd->msg_id;
    rsp->num        = 0; // fixed later
    for (int i = 0; i < sta_cnt; i++) {
        wifi_mgmr_ap_sta_info_get(&sta_info, i);
        if (!memcmp(sta_info.sta_mac, "\x0\x0\x0\x0\x0", 6)) {
            continue;
        }
        blog_info("idx %i MAC %02X:%02X:%02X:%02X:%02X:%02X\r\n",
                filtered_sta_cnt,
                sta_info.sta_mac[0],
                sta_info.sta_mac[1],
                sta_info.sta_mac[2],
                sta_info.sta_mac[3],
                sta_info.sta_mac[4],
                sta_info.sta_mac[5]
                );
        memcpy(rsp->sta[filtered_sta_cnt++].mac, sta_info.sta_mac, 6);
    }

    blog_info("sta_cnt %d vs filtered_sta_cnt %d\r\n", sta_cnt, filtered_sta_cnt);
    rsp->num = filtered_sta_cnt;
    length -= (sta_cnt - filtered_sta_cnt) * sizeof(struct wifi_sta_info);
    bflbmsg_send(&env->trcver_ctx, BF1B_MSG_TYPE_CMD, rsp, length);
    vPortFree(rsp);
}

static void fill_record_ap(struct bflbwifi_ap_record *record)
{
    int value_int, state;
    wifi_mgmr_sta_connect_ind_stat_info_t info;

    /* Link Status*/
    if (0 == wifi_mgmr_state_get(&value_int)) {
        if (WIFI_STATE_CONNECTED_IP_GOT == value_int ||
                WIFI_STATE_CONNECTED_IP_GETTING == value_int) {
            record->link_status = BF1B_WIFI_LINK_STATUS_UP;
        } else {
            record->link_status = BF1B_WIFI_LINK_STATUS_DOWN;
        }
    } else {
        record->link_status = BF1B_WIFI_LINK_STATUS_UNKNOWN;
    }

    /*bssid*/
    //uint8_t bssid[6];

    /*ssid*/
    //uint8_t ssid[32 + 1];

    /*bssid and ssid*/
    wifi_mgmr_sta_connect_ind_stat_get(&info);
    wifi_mgmr_state_get(&state);
    record->mgmr_state = state;

    memcpy(record->ssid, info.ssid, 32);
    record->ssid[32] = '\0';
    memcpy(record->bssid, info.bssid, 6);

    /*channel*/
    if (0 == wifi_mgmr_channel_get(&value_int)) {
        record->channel = value_int;
    } else {
        record->channel = -1;
    }

    /*rssi*/
    if (0 == wifi_mgmr_rssi_get(&value_int)) {
        record->rssi = value_int;
    } else {
        record->rssi = -127;
    }

    /*auth_mode*/
    record->auth_mode = BF1B_WIFI_AUTH_UNKNOWN;
    record->cipher = BF1B_WIFI_CIPHER_TYPE_NONE;
}

static void handle_get_link_status(netbus_wifi_mgmr_ctx_t *env, netbus_wifi_mgmr_msg_cmd_t *cmd)
{
    blog_info("handle get link status\r\n");
    netbus_fs_link_status_ind_cmd_msg_t rsp;

    memset(&rsp, 0, sizeof(rsp));
    rsp.hdr.cmd    = BFLB_CMD_GET_LINK_STATUS_IND;
    rsp.hdr.msg_id = cmd->msg_id;
    fill_record_ap(&rsp.record);

    bflbmsg_send(&env->trcver_ctx, BF1B_MSG_TYPE_CMD, &rsp, sizeof(rsp));
}

// typedef void (*sniffer_cb_t)(void *env, uint8_t *pkt, int len, bl_rx_info_t *info);
static void sniffer_cb(void *env, uint8_t *pkt, int len, bl_rx_info_t *info)
{
    netbus_wifi_mgmr_ctx_t *ctx = env;
    if (!(ctx->mode == NETBUS_WIFI_MODE_SNIFFER)) {
        return;
    }
    if (len > 2000) {
        return;
    }
    struct pbuf p;
    p.tot_len = p.len = len;
    p.next = NULL;
    p.payload = pkt;
}

static void handle_start_monitor(netbus_wifi_mgmr_ctx_t *env, netbus_wifi_mgmr_msg_cmd_t *cmd)
{
    blog_info("handle start monitor\r\n");
    if (!(env->mode == NETBUS_WIFI_MODE_NONE)) {
        log_error("mode incorrect, ignore start monitor\r\n");
        return;
    }
    env->mode = NETBUS_WIFI_MODE_SNIFFER;
    wifi_mgmr_sniffer_enable();
    wifi_mgmr_sniffer_register(env, sniffer_cb);
}

static void handle_stop_monitor(netbus_wifi_mgmr_ctx_t *env, netbus_wifi_mgmr_msg_cmd_t *cmd)
{
    blog_info("handle stop monitor\r\n");
    if (!(env->mode == NETBUS_WIFI_MODE_SNIFFER)) {
        log_error("mode incorrect, ignore stop monitor\r\n");
        return;
    }
    env->mode = NETBUS_WIFI_MODE_NONE;
    wifi_mgmr_sniffer_disable();
    wifi_mgmr_sniffer_unregister(NULL);
}

static void handle_set_channel(netbus_wifi_mgmr_ctx_t *env, netbus_wifi_mgmr_msg_cmd_t *cmd)
{
    blog_info("handle set channel\r\n");
    netbus_ts_set_channel_cmd_msg_t *ccmd;

    if (!cmd->data_ptr || cmd->data_len != sizeof(netbus_ts_set_channel_cmd_msg_t)) {
        log_error("msg format error\r\n");
        goto ret;
    }

    if (!(env->mode == NETBUS_WIFI_MODE_SNIFFER)) {
        log_error("mode incorrect, ignore set channel\r\n");
        goto ret;
    }
    ccmd = (netbus_ts_set_channel_cmd_msg_t *)cmd->data_ptr;
    uint8_t chn = ccmd->channel;
    if (chn >=1 && chn <= 14) {
        wifi_mgmr_channel_set(ccmd->channel, 0);
    }

ret:
    vPortFree(cmd->data_ptr);
}

static void handle_get_channel(netbus_wifi_mgmr_ctx_t *env, netbus_wifi_mgmr_msg_cmd_t *cmd)
{
    blog_info("handle get channel\r\n");
    int chn;
    netbus_fs_get_channel_ind_cmd_msg_t msg;

    wifi_mgmr_channel_get(&chn);
    blog_info("Current channel: %d\r\n", chn);

    msg.hdr.cmd = BFLB_CMD_MAC_IND;
    msg.hdr.msg_id = cmd->msg_id;
    msg.channel = chn;

    bflbmsg_send(&env->trcver_ctx, BF1B_MSG_TYPE_CMD, &msg, sizeof(msg));
}

static void handle_get_dev_version(netbus_wifi_mgmr_ctx_t *env, netbus_wifi_mgmr_msg_cmd_t *cmd)
{
    blog_info("handle get dev version\r\n");
    netbus_fs_dev_version_ind_cmd_msg_t msg;

    blog_info("version: %08X\r\n", NETBUS_WIFI_VERSION);

    msg.hdr.cmd = BFLB_CMD_GET_DEV_VERSION_IND;
    msg.hdr.msg_id = cmd->msg_id;
    msg.version.version_num = NETBUS_WIFI_VERSION;

    bflbmsg_send(&env->trcver_ctx, BF1B_MSG_TYPE_CMD, &msg, sizeof(msg));
}

static void handle_ping(netbus_wifi_mgmr_ctx_t *env, netbus_wifi_mgmr_msg_cmd_t *cmd)
{
    netbus_min_cmd_msg_t msg;

    msg.cmd = BFLB_CMD_PING;

    bflbmsg_send(&env->trcver_ctx, BF1B_MSG_TYPE_CMD, &msg, sizeof(msg));
}

__attribute__((weak)) void app_handle_hbn(void *ptr, uint32_t length) {}
static void handle_hbn(netbus_wifi_mgmr_ctx_t *env, netbus_wifi_mgmr_msg_cmd_t *cmd)
{
    app_handle_hbn(cmd->data_ptr, cmd->data_len);
}

static void handle_default(netbus_wifi_mgmr_ctx_t *env, netbus_wifi_mgmr_msg_cmd_t *cmd)
{
    log_error("unknown cmd %u\r\n", cmd->cmd);
    vPortFree(cmd->data_ptr);
}

void netbus_wifi_mgmr_cmd_entry(netbus_wifi_mgmr_ctx_t *env, netbus_wifi_mgmr_msg_cmd_t *cmd)
{
    switch (cmd->cmd) {
    case BFLB_CMD_RESET:
        blog_info("BFLB_CMD_RESET start\r\n");
        handle_reset(&env->mode);
        blog_info("BFLB_CMD_RESET end\r\n");
        break;
    case BFLB_CMD_HELLO:
        blog_info("BFLB_CMD_HELLO start\r\n");
        handle_hello(env, cmd);
        blog_info("BFLB_CMD_HELLO end\r\n");
        break;
    case BFLB_CMD_GET_MAC:
        blog_info("BFLB_CMD_GET_MAC start\r\n");
        handle_get_mac(env, cmd);
        blog_info("BFLB_CMD_GET_MAC end\r\n");
        break;
    case BFLB_CMD_CONNECT_AP:
        blog_info("BFLB_CMD_CONNECT_AP start\r\n");
        handle_connect_ap(env, cmd);
        blog_info("BFLB_CMD_CONNECT_AP end\r\n");
        break;
    case BFLB_CMD_DISCONNECT_AP:
        blog_info("BFLB_CMD_DISCONNECT_AP start\r\n");
        handle_disconnect_ap(env, cmd);
        blog_info("BFLB_CMD_DISCONNECT_AP end\r\n");
        break;
    case BFLB_CMD_AP_CONNECTED_IND:
        blog_info("BFLB_CMD_AP_CONNECTED_IND start\r\n");
        handle_ap_connected(env, cmd);
        blog_info("BFLB_CMD_AP_CONNECTED_IND end\r\n");
        break;
    case BFLB_CMD_AP_DISCONNECTED_IND:
        blog_info("BFLB_CMD_AP_DISCONNECTED_IND start\r\n");
        handle_ap_disconnected(env, cmd);
        blog_info("BFLB_CMD_AP_DISCONNECTED_IND end\r\n");
        break;
    case BFLB_CMD_STA_SET_AUTO_RECONNECT:
        blog_info("BFLB_CMD_STA_SET_AUTO_RECONNECT start\r\n");
        handle_set_auto_reconnect(env, cmd);
        blog_info("BFLB_CMD_STA_SET_AUTO_RECONNECT end\r\n");
        break;
    case BFLB_CMD_START_AP:
        blog_info("BFLB_CMD_START_AP start\r\n");
        handle_start_ap(env, cmd);
        blog_info("BFLB_CMD_START_AP end\r\n");
        break;
    case BFLB_CMD_STOP_AP:
        blog_info("BFLB_CMD_STOP_AP start\r\n");
        handle_stop_ap(env, cmd);
        blog_info("BFLB_CMD_STOP_AP end\r\n");
        break;
    case BFLB_CMD_SET_LPM_MODE:
        blog_info("BFLB_CMD_SET_LPM_MODE start\r\n");
        handle_set_lpm_mode(env, cmd);
        blog_info("BFLB_CMD_SET_LPM_MODE end\r\n");
        break;
    case BFLB_CMD_START_SCAN:
        blog_info("BFLB_CMD_START_SCAN start\r\n");
        handle_start_scan(env, cmd);
        blog_info("BFLB_CMD_START_SCAN end\r\n");
        break;
    case BFLB_CMD_SCAN_IND:
        blog_info("BFLB_CMD_SCAN_IND start\r\n");
        handle_scan_ind(env, cmd);
        blog_info("BFLB_CMD_SCAN_IND end\r\n");
        break;
    case BFLB_CMD_GET_STA_LIST:
        blog_info("BFLB_CMD_GET_STA_LIST start\r\n");
        handle_ap_get_sta_list(env, cmd);
        blog_info("BFLB_CMD_GET_STA_LIST end\r\n");
        break;
    case BFLB_CMD_GET_LINK_STATUS:
        blog_info("BFLB_CMD_GET_LINK_STATUS start\r\n");
        handle_get_link_status(env, cmd);
        blog_info("BFLB_CMD_GET_LINK_STATUS end\r\n");
        break;
    case BFLB_CMD_START_MONITOR:
        blog_info("BFLB_CMD_START_MONITOR start\r\n");
        handle_start_monitor(env, cmd);
        blog_info("BFLB_CMD_START_MONITOR end\r\n");
        break;
    case BFLB_CMD_STOP_MONITOR:
        blog_info("BFLB_CMD_STOP_MONITOR start\r\n");
        handle_stop_monitor(env, cmd);
        blog_info("BFLB_CMD_STOP_MONITOR end\r\n");
        break;
    case BFLB_CMD_SET_CHANNEL:
        blog_info("BFLB_CMD_SET_CHANNEL start\r\n");
        handle_set_channel(env, cmd);
        blog_info("BFLB_CMD_SET_CHANNEL end\r\n");
        break;
    case BFLB_CMD_GET_CHANNEL:
        blog_info("BFLB_CMD_GET_CHANNEL start\r\n");
        handle_get_channel(env, cmd);
        blog_info("BFLB_CMD_GET_CHANNEL end\r\n");
        break;
    case BFLB_CMD_GET_DEV_VERSION:
        blog_info("BFLB_CMD_GET_DEV_VERSION start\r\n");
        handle_get_dev_version(env, cmd);
        blog_info("BFLB_CMD_GET_DEV_VERSION end\r\n");
        break;
    case BFLB_CMD_OTA:
        blog_info("BFLB_CMD_OTA start\r\n");
        blog_info("BFLB_CMD_OTA end\r\n");
        break;
#if NETBUS_WIFI_SHELLCODE_ENABLE
    case BFLB_CMD_SHELLCODE:
        blog_info("BFLB_CMD_SHELLCODE start\r\n");
        handle_shellcode(env, cmd);
        blog_info("BFLB_CMD_SHELLCODE end\r\n");
        break;
#endif
    case BFLB_CMD_PING:
        blog_info("BFLB_CMD_PING start\r\n");
        handle_ping(env, cmd);
        blog_info("BFLB_CMD_PING end\r\n");
        break;
    case BFLB_CMD_HBN:
        blog_info("BFLB_CMD_HBN start\r\n");
        handle_hbn(env, cmd);
        blog_info("BFLB_CMD_HBN end\r\n");
        break;
    default:
        blog_info("default start\r\n");
        handle_default(env, cmd);
        blog_info("default end\r\n");
        break;
    }
}
