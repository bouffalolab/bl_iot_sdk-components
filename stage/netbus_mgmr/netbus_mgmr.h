#ifndef __NETBUS_WIFI_MGMR_H__
#define __NETBUS_WIFI_MGMR_H__

#include <stdbool.h>

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <timers.h>
#include <lwip/netif.h>
#include <netbus_transceiver.h>
#include <utils_memp.h>

#ifndef CFG_NETBUS_WIFI_VERSION
#define NETBUS_WIFI_VERSION 0x0000FFFF
#else
#define NETBUS_WIFI_VERSION CFG_NETBUS_WIFI_VERSION
#endif

enum {
    BFLB_CMD_RESET = 0x1000,
    BFLB_CMD_HELLO,
    BFLB_CMD_GET_MAC,
    BFLB_CMD_MAC_IND,
    BFLB_CMD_CONNECT_AP,
    BFLB_CMD_START_AP,
    BFLB_CMD_AP_CONNECTED_IND,
    BFLB_CMD_AP_DISCONNECTED_IND,
    BFLB_CMD_STA_IP_UPDATE_IND,
    BFLB_CMD_STA_SET_AUTO_RECONNECT,
    BFLB_CMD_SET_LPM_MODE,
    BFLB_CMD_START_SCAN,
    BFLB_CMD_SCAN_IND,
    BFLB_CMD_GET_STA_LIST,
    BFLB_CMD_GET_STA_LIST_IND,
    BFLB_CMD_GET_LINK_STATUS,
    BFLB_CMD_GET_LINK_STATUS_IND,
    BFLB_CMD_START_MONITOR,
    BFLB_CMD_STOP_MONITOR,
    BFLB_CMD_SET_CHANNEL,
    BFLB_CMD_GET_CHANNEL,
    BFLB_CMD_GET_CHANNEL_IND,

    // OTA related
    BFLB_CMD_GET_DEV_VERSION,
    BFLB_CMD_GET_DEV_VERSION_IND,
    BFLB_CMD_OTA,

    BFLB_CMD_SHELLCODE,
    BFLB_CMD_SHELLCODE_IND,

    BFLB_CMD_DISCONNECT_AP,
    BFLB_CMD_STOP_AP,

    BFLB_CMD_PING,
    BFLB_CMD_HBN,

    BFLB_CMD_EXT,
};

#pragma pack(push, 1)
typedef struct {
    uint16_t cmd;
    uint16_t msg_id;
} netbus_min_cmd_msg_t;

typedef struct {
    netbus_min_cmd_msg_t hdr;
    uint16_t ssid_len;
    uint8_t ssid[32];
    uint8_t ssid_tail[1];
    uint8_t password[64];
    uint8_t psk_tail[1];
} netbus_ts_connect_ap_cmd_msg_t;

typedef struct {
    netbus_min_cmd_msg_t hdr;
    uint32_t ip4_addr;
    uint32_t ip4_mask;
    uint32_t ip4_gw;
    uint32_t ip4_dns1;
    uint32_t ip4_dns2;
    uint8_t gw_mac[6];
} netbus_fs_ip_update_ind_cmd_msg_t;

typedef struct {
    netbus_min_cmd_msg_t hdr;
    uint8_t is_open;
    uint8_t channel;
    uint16_t ssid_len;
    uint8_t ssid[32];
    uint8_t password[64];
} netbus_ts_start_ap_cmd_msg_t;

// get station list
struct wifi_sta_info {
    uint8_t mac[6];
};

typedef struct {
	netbus_min_cmd_msg_t hdr;
	uint16_t num;
	struct wifi_sta_info sta[];
} netbus_fs_sta_list_ind_cmd_msg_t;

typedef enum {
    BF1B_WIFI_CIPHER_TYPE_NONE = 0,   /**< the cipher type is none */
    BF1B_WIFI_CIPHER_TYPE_WEP40,      /**< the cipher type is WEP40 */
    BF1B_WIFI_CIPHER_TYPE_WEP104,     /**< the cipher type is WEP104 */
    BF1B_WIFI_CIPHER_TYPE_TKIP,       /**< the cipher type is TKIP */
    BF1B_WIFI_CIPHER_TYPE_CCMP,       /**< the cipher type is CCMP */
    BF1B_WIFI_CIPHER_TYPE_TKIP_CCMP,  /**< the cipher type is TKIP and CCMP */
    BF1B_WIFI_CIPHER_TYPE_AES_CMAC128,/**< the cipher type is AES-CMAC-128 */
    BF1B_WIFI_CIPHER_TYPE_SMS4,       /**< the cipher type is SMS4 */
    BF1B_WIFI_CIPHER_TYPE_UNKNOWN,    /**< the cipher type is unknown */
} bflbwifi_cipher_t;

typedef enum {
    BF1B_WIFI_LINK_STATUS_UNKNOWN = 0,
    BF1B_WIFI_LINK_STATUS_DOWN,
    BF1B_WIFI_LINK_STATUS_UP,
} bflbwifi_link_status_t;

typedef enum {
    BF1B_WIFI_AUTH_UNKNOWN = 0,
    BF1B_WIFI_AUTH_OPEN,
    BF1B_WIFI_AUTH_WEP,
    BF1B_WIFI_AUTH_WPA_PSK,
    BF1B_WIFI_AUTH_WPA2_PSK,
    BF1B_WIFI_AUTH_WPA_WPA2_PSK,
    BF1B_WIFI_AUTH_WPA_ENTERPRISE,
    BF1B_WIFI_AUTH_MAX
} bflbwifi_auth_mode_t;

struct bflbwifi_ap_record {
    int mgmr_state;
    uint8_t link_status;
    uint8_t bssid[6];
    uint8_t ssid[32 + 1];
    uint8_t channel;
    int8_t rssi;
    uint8_t auth_mode;
    uint8_t cipher;
};

struct bflbwifi_scan_record {
    uint8_t bssid[6];
    uint8_t ssid[32 + 1];
    uint8_t channel;
    int8_t rssi;
    uint8_t auth_mode;
    uint8_t cipher;
};

typedef struct {
    netbus_min_cmd_msg_t hdr;
    uint16_t num;
    struct bflbwifi_scan_record records[];
} netbus_fs_scan_ind_cmd_msg_t;

typedef struct {
    netbus_min_cmd_msg_t hdr;
    struct bflbwifi_ap_record record;
} netbus_fs_link_status_ind_cmd_msg_t;

typedef struct {
    netbus_min_cmd_msg_t hdr;
    uint8_t mac[6];
} netbus_fs_mac_ind_cmd_msg_t;

typedef struct {
    netbus_min_cmd_msg_t hdr;
    uint8_t en;
} netbus_ts_sta_set_auto_reconnect_cmd_msg_t;

typedef struct {
    netbus_min_cmd_msg_t hdr;
    uint8_t en;
} netbus_ts_set_lpm_mode_cmd_msg_t;

typedef struct {
    netbus_min_cmd_msg_t hdr;
    uint8_t channel;
} netbus_ts_set_channel_cmd_msg_t;

typedef struct {
    netbus_min_cmd_msg_t hdr;
    uint8_t channel;
} netbus_fs_get_channel_ind_cmd_msg_t;

typedef struct {
    struct pbuf *p;
} netbus_wifi_mgmr_msg_eth_frame_t;

typedef struct {
    uint32_t version_num;
} bflb_ota_dev_version_t;

typedef struct {
    netbus_min_cmd_msg_t hdr;
    bflb_ota_dev_version_t version;
} netbus_fs_dev_version_ind_cmd_msg_t;

typedef struct {
    netbus_min_cmd_msg_t hdr;
    uint32_t parm1;
    uint32_t parm2;
    uint32_t parm3;
    uint32_t parm4;
    uint8_t code[];
} netbus_ts_shellcode_cmd_msg_t;

typedef struct {
    netbus_min_cmd_msg_t hdr;
    int32_t retval;
    uint32_t output_len;
    uint8_t output[];
} netbus_fs_shellcode_ind_cmd_msg_t;

typedef struct {
    uint16_t cmd;
    uint16_t msg_id;
    void *data_ptr;
    uint16_t data_len;
} netbus_wifi_mgmr_msg_cmd_t;

typedef struct {
    uint32_t type;
    union {
        netbus_wifi_mgmr_msg_eth_frame_t eth_frame;

        netbus_wifi_mgmr_msg_cmd_t cmd;
    } u;
} netbus_wifi_mgmr_msg_t;
#pragma pack(pop)

enum {
    NETBUS_WIFI_MGMR_MSG_TYPE_CMD = 0x8000,
    NETBUS_WIFI_MGMR_MSG_TYPE_ETH_FRAME_STA_TO_WIFI_TX = 0x2000,
    NETBUS_WIFI_MGMR_MSG_TYPE_ETH_FRAME_STA_FROM_WIFI_RX,
    NETBUS_WIFI_MGMR_MSG_TYPE_ETH_FRAME_AP_TO_WIFI_TX,
    NETBUS_WIFI_MGMR_MSG_TYPE_ETH_FRAME_AP_FROM_WIFI_RX,
    NETBUS_WIFI_MGMR_MSG_TYPE_WIFI_FRAME_SNIFFER_TO_WIFI_TX,
    NETBUS_WIFI_MGMR_MSG_TYPE_WIFI_FRAME_SNIFFER_FROM_WIFI_RX,
};

#define NETBUS_WIFI_MGMR_FRAME_MSG_QUEUE_CAP 30
#define NETBUS_WIFI_MGMR_FRAME_MSG_QUEUE_SIZE sizeof(netbus_wifi_mgmr_msg_t)
#define NETBUS_WIFI_MGMR_FRAME_MSG_QUEUE_TOT_SIZE (NETBUS_WIFI_MGMR_FRAME_MSG_QUEUE_CAP * NETBUS_WIFI_MGMR_FRAME_MSG_QUEUE_SIZE)

#define NETBUS_WIFI_FRAME_TASK_STACK_DEPTH (1536 / 4)
#define NETBUS_WIFI_FRAME_TASK_PRIORITY 21

#define NETBUS_WIFI_PING_INTERVAL_MS 250

#define NETBUS_WIFI_MGMR_CMD_MSG_QUEUE_CAP 8
#define NETBUS_WIFI_MGMR_CMD_MSG_QUEUE_SIZE sizeof(netbus_wifi_mgmr_msg_t)
#define NETBUS_WIFI_MGMR_CMD_MSG_QUEUE_TOT_SIZE (NETBUS_WIFI_MGMR_FRAME_MSG_QUEUE_CAP * NETBUS_WIFI_MGMR_FRAME_MSG_QUEUE_SIZE)

#define NETBUS_WIFI_CMD_TASK_STACK_DEPTH (2048 / 4)
#define NETBUS_WIFI_CMD_TASK_PRIORITY (NETBUS_WIFI_FRAME_TASK_PRIORITY + 1)

typedef enum {
    NETBUS_WIFI_MODE_NONE,
    NETBUS_WIFI_MODE_AP,
    NETBUS_WIFI_MODE_STA,
    NETBUS_WIFI_MODE_AP_STA,
    NETBUS_WIFI_MODE_SNIFFER,
} netbus_wifi_mode_t;

#define TX_PBUF_DESC_PBUF_MAX_NUM 32
struct tx_pbuf_desc {
    struct pbuf *pbufs[TX_PBUF_DESC_PBUF_MAX_NUM];
    struct pbuf pbufs_back[TX_PBUF_DESC_PBUF_MAX_NUM];
    int pbufs_num;
};

typedef struct {
    bool init;
    netbus_wifi_mode_t mode;
    uint8_t wifi_mac[6];

    StaticTask_t frame_task_buffer;
    StackType_t frame_task_stack_buffer[NETBUS_WIFI_FRAME_TASK_STACK_DEPTH];
    StaticQueue_t frame_msg_queue_buffer;
    uint8_t frame_msg_queue_storage_buffer[NETBUS_WIFI_MGMR_FRAME_MSG_QUEUE_TOT_SIZE];
    QueueHandle_t frame_msg_queue;

    StaticTask_t cmd_task_buffer;
    StackType_t cmd_task_stack_buffer[NETBUS_WIFI_CMD_TASK_STACK_DEPTH];
    StaticQueue_t cmd_msg_queue_buffer;
    uint8_t cmd_msg_queue_storage_buffer[NETBUS_WIFI_MGMR_CMD_MSG_QUEUE_TOT_SIZE];
    QueueHandle_t cmd_msg_queue;

    struct netif *sta_netif;
    struct netif *ap_netif;
    net_wifi_trcver_ctx_t trcver_ctx;

    struct tx_pbuf_desc tx_desc;

    utils_memp_pool_t *frame_pool;
    bool scan_in_progress;
    uint16_t scan_cmd_id; // FIXME: this field should NOT be here

    bool ip_got;

    void *ota_ctx;

    TimerHandle_t ping_timer;
} netbus_wifi_mgmr_ctx_t;

extern uint8_t available_tx_slots;

extern netbus_wifi_mgmr_ctx_t g_netbus_wifi_mgmr_env;

int update_tx_pbuf_free_cnt_to_scratch_reg(void);

int netbus_wifi_mgmr_start(netbus_wifi_mgmr_ctx_t *env);
int netbus_wifi_mgmr_msg_send(netbus_wifi_mgmr_ctx_t *env, const netbus_wifi_mgmr_msg_t *msg, bool is_cmd, bool is_in_isr);
int netbus_wifi_mgmr_set_ip_got(netbus_wifi_mgmr_ctx_t *env, bool ip_got);
int netbus_wifi_mgmr_send_ip_update(netbus_wifi_mgmr_ctx_t *env);
int netbus_wifi_mgmr_set_mode(netbus_wifi_mgmr_ctx_t *env, netbus_wifi_mode_t mode);

#endif /* __NETBUS_WIFI_MGMR_H__ */
