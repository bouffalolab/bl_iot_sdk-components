#include <string.h>
#include <aos/kernel.h>
#include <cli.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <blog.h>
#include "virt_net.h"
#include "ramsync_upper.h"

static virt_net_t g_vnet_net;

static int virt_net_spi_event_cb(virt_net_t obj, enum virt_net_event_code code,
                                 void *opaque)
{
    switch (code) {
        case VIRT_NET_EV_ON_CONNECTED:
            blog_info("AP connect success!\r\n");
            break;

        case VIRT_NET_EV_ON_DISCONNECT:
            blog_info("AP disconnect !\r\n");
            break;

        case VIRT_NET_EV_ON_GOT_IP:
            {
            blog_info("[lwip] netif status callback\r\n");
            blog_info("IP: %s\r\n", ip4addr_ntoa(netif_ip4_addr((struct netif *)opaque)));
            blog_info("MK: %s\r\n", ip4addr_ntoa(netif_ip4_netmask((struct netif *)opaque)));
            blog_info("GW: %s\r\n", ip4addr_ntoa(netif_ip4_gw((struct netif *)opaque)));
            break;
            }
        case VIRT_NET_EV_ON_SCAN_DONE:
            {
            netbus_wifi_mgmr_msg_cmd_t *pkg_data;
            pkg_data = (netbus_wifi_mgmr_msg_cmd_t *)((struct pkg_protocol *)opaque)->payload;

            netbus_fs_scan_ind_cmd_msg_t *msg;
            blog_info("Scan done!\r\n");

            msg = (netbus_fs_scan_ind_cmd_msg_t*)((netbus_fs_scan_ind_cmd_msg_t*)pkg_data);
            blog_info("num = %d \r\n", msg->num);

            for (int i = 0; i < msg->num; i++) {
                blog_info("%s %d %d	%d %d %d\r\n", msg->records[i].ssid, msg->records[i].channel
                        , msg->records[i].rssi, msg->records[i].channel
                        , msg->records[i].auth_mode, msg->records[i].cipher);
            }
            break;
            }
        case VIRT_NET_EV_ON_LINK_STATUS_DONE:
            {
            struct bflbwifi_ap_record* record;
            netbus_fs_link_status_ind_cmd_msg_t* pkg_data;

            pkg_data = (netbus_fs_link_status_ind_cmd_msg_t *)((struct pkg_protocol *)opaque)->payload;
            record = &pkg_data->record;

            if (record->link_status == BF1B_WIFI_LINK_STATUS_UP) {
                blog_info("link status up!\r\n");
            } else if (record->link_status == BF1B_WIFI_LINK_STATUS_DOWN){
                blog_info("link status down!\r\n");
            } else {
                blog_info("link status unknown!\r\n");
            }

            blog_info("ssid:%s\r\n", record->ssid);
            blog_info("bssid: %02x%02x%02x%02x%02x%02x\r\n",
                    record->bssid[0],
                    record->bssid[1],
                    record->bssid[2],
                    record->bssid[3],
                    record->bssid[4],
                    record->bssid[5]);
            break;
            }
        default:
            break;
    }

    return 0;
}

static int inited = 0;
static void __virt_net_init(void)
{
    if (inited) {
       return; 
    }
    inited = 1;

    g_vnet_net = virt_net_create(NULL);
    if (g_vnet_net == NULL) {
        printf("Create vnet_net virtnet failed!!!!\r\n");
        return;
    }

    if (g_vnet_net->init(g_vnet_net)) {
        printf("init spi virtnet failed!!!!\r\n");
        return;
    }

#if 0
    ip4_addr_t ipaddr;
    ip4_addr_t netmask;
    ip4_addr_t gw;

    IP4_ADDR(&ipaddr, 192, 168, 31, 111);
    IP4_ADDR(&netmask, 255, 255, 255, 0);
    IP4_ADDR(&gw, 192, 168, 31, 1);

    netifapi_netif_set_addr((struct netif *)&g_vnet_net->netif, &ipaddr, &netmask,
                            &gw);
#endif
    virt_net_setup_callback(g_vnet_net, virt_net_spi_event_cb, NULL);

    /* set to default netif */
    netifapi_netif_set_default((struct netif *)&g_vnet_net->netif);
}

static void __virt_net_deinit(void)
{
    if (!inited) {
       return; 
    }
    inited = 0;
    
    virt_net_setup_callback(g_vnet_net, NULL, NULL);

    if (g_vnet_net->deinit(g_vnet_net)) {
        printf("Deinit spi virtnet failed!!!!\r\n");
        return;
    }
    virt_net_delete(g_vnet_net);
}

static void cmd_virt_net_connect(char *buf, int len, int argc, char **argv)
{
    __virt_net_init();

    if (argc > 2) {
        virt_net_connect_ap(g_vnet_net, argv[1], argv[2]);
    } else if (argc > 1) {
        /* Connect AP */
        virt_net_connect_ap(g_vnet_net, argv[1], "");
    }else {
        /* Connect AP */
        virt_net_connect_ap(g_vnet_net, "bouffalolab", "bouffalolab2016");
    }
}

static void cmd_virt_net_get_status(char *buf, int len, int argc, char **argv)
{
    virt_net_get_link_status(g_vnet_net);
}

static void cmd_virt_net_scan(char *buf, int len, int argc, char **argv)
{
    __virt_net_init();
    virt_net_scan(g_vnet_net);
}

static void cmd_virt_net_disconnect(char *buf, int len, int argc, char **argv)
{
    __virt_net_init();
    virt_net_disconnect(g_vnet_net);
}

static void cmd_virt_net_enter_hbn(char *buf, int len, int argc, char **argv)
{
    __virt_net_init();
    virt_net_enter_hbn(g_vnet_net);
}

static void cmd_virt_net_version(char *buf, int len, int argc, char **argv)
{
    uint32_t version;
    __virt_net_init();
    if (0 == virt_net_slave_version(g_vnet_net, &version))
    {
        printf("version=0x%08lx\r\n", version);
    }
}

static void cmd_virt_net_destory(char *buf, int len, int argc, char **argv)
{
    __virt_net_deinit();
}

// STATIC_CLI_CMD_ATTRIBUTE makes this(these) command(s) static
const static struct cli_command cmds_user[] STATIC_CLI_CMD_ATTRIBUTE = {
    {"wifi_connect", "wifi_connect <ssid> <psk>", cmd_virt_net_connect},
    {"wifi_disconnect", "wifi_disconnect", cmd_virt_net_disconnect},
    {"wifi_status", "wifi_status", cmd_virt_net_get_status},
    {"wifi_scan", "wifi_scan", cmd_virt_net_scan},
    {"wifi_enter_hbn", "wifi_enter_hbn", cmd_virt_net_enter_hbn},
    {"virt_net_destory", "virt_net destory", cmd_virt_net_destory},
    {"virt_net_slave_version", "virt_net version", cmd_virt_net_version},
};

int virt_net_test_cli_init(void)
{
    // static command(s) do NOT need to call aos_cli_register_command(s) to register.
    // However, calling aos_cli_register_command(s) here is OK but is of no effect as cmds_user are included in cmds list.
    // XXX NOTE: Calling this *empty* function is necessary to make cmds_user in this file to be kept in the final link.
    //return aos_cli_register_commands(cmds_user, sizeof(cmds_user)/sizeof(cmds_user[0]));
    return 0;
}
