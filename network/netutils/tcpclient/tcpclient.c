#include <FreeRTOS.h>
#include <task.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <cli.h>
#include <aos/kernel.h>
#include <utils_getopt.h>
#include <lwip/sockets.h>
#include <netutils/netutils.h>

#define TCPC_DEFAULT_SVR_IP     "192.168.1.1"
#define TCPC_PORT_CONNECT       (5150)
#define TCPC_BUFFER_SIZE        (1024)
#define TCPC_DEFAULT_BYTES      (512 * 1024)
#define TCPC_DEFAULT_COUNTER    (16)

enum SEND_MODE_E {
    SEND_MODE_ASCII,
    SEND_MODE_STREAM,
};

struct tcpc_opts {
    /* server IP address */
    char host[32];
    /* server listening port */
    int port;
    /* ascii/stream */
    enum SEND_MODE_E mode;
    /* repeat times in ascii mode */
    int counter;
    /* read after write in ascii mode */
    int echo_enable;
    /* bytes to be transferred in stream mode */
    int bytes;
    /* thread index */
    int idx;
};

static inline const char *xfer_mode_string(enum SEND_MODE_E m)
{
    switch (m) {
    case SEND_MODE_ASCII:
        return "ASCII";

    case SEND_MODE_STREAM:
        return "Stream";

    default:
        return "Unknown";
    }
}

static void inline tcpc_log(const char *tag, const char *fmt, ...)
{
    va_list va;
    char s[256];

    puts(tag);
    va_start(va, fmt);
    vsnprintf(s, sizeof(s) - 1, fmt, va);
    va_end(va);
    puts(s);
}

static void tcpc_entry(void *arg)
{
    struct tcpc_opts *opts_ptr = (struct tcpc_opts*)arg;
    const char *host = (char *)opts_ptr->host;
    int sock, ret;
    uint8_t *send_buf;
    struct sockaddr_in addr;
    char tag[32];
    int conn_ok = 0;

    snprintf(tag, sizeof(tag) - 1, "[TCPC-%d] ", opts_ptr->idx);
    tcpc_log(tag, "starting\r\n");

    send_buf = (uint8_t *)pvPortMalloc(TCPC_BUFFER_SIZE);
    if (NULL == send_buf) {
        tcpc_log(tag, "no mem for send buffer\r\n");
        goto err_sndbuf;
    }
    memset(send_buf, 0, TCPC_BUFFER_SIZE);

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
        tcpc_log(tag, "failed to create tcp socket, %s\n", strerror(errno));
        goto err_skt;
    }
    addr.sin_family = PF_INET;
    addr.sin_port = htons(opts_ptr->port);
    addr.sin_addr.s_addr = inet_addr(host);

    ret = connect(sock, (const struct sockaddr*)&addr, sizeof(addr));
    if (ret < 0) {
        tcpc_log(tag, "connect failed, %s\r\n", strerror(errno));
        goto err_connect;
    }

    conn_ok = 1;
    tcpc_log(tag, "connected to %s:%d successfully\r\n", host, opts_ptr->port);
    int on = 1;
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (void *)&on, sizeof(on));

    int nbytes = 0;

    switch (opts_ptr->mode) {
    case SEND_MODE_ASCII:
        for (int r = 0; r < opts_ptr->counter && conn_ok; r++) {
            snprintf((char*)send_buf, TCPC_BUFFER_SIZE - 1, "Data Seq %d\r\n", r);
            /* start this round */
            nbytes = 0;
            while (nbytes < TCPC_BUFFER_SIZE) {
                int rem = TCPC_BUFFER_SIZE - nbytes;
                ret = write(sock, &send_buf[nbytes], rem);
                if (ret < 0) {
                    tcpc_log(tag, "write failed, %s\r\n", strerror(errno));
                    conn_ok = 0;
                    break;
                }

                tcpc_log(tag, "written %d bytes successfully\r\n", ret);
                if (ret < rem) {
                    tcpc_log(tag, "short write, written %d < wanted %d\r\n",
                             ret, rem);
                }
                nbytes += ret;

                if (opts_ptr->echo_enable) {
                    tcpc_log(tag, "reading response from peer...\r\n");
                    ret = read(sock, send_buf, TCPC_BUFFER_SIZE);
                    if (ret < 0) {
                        tcpc_log(tag, "read failed, %s\r\n", strerror(errno));
                        conn_ok = 0;
                        break;
                    }

                    if (!ret) {
                        tcpc_log(tag, "peer closed the connection\r\n");
                        conn_ok = 0;
                        break;
                    }

                    send_buf[TCPC_BUFFER_SIZE - 1] = '\0';
                    tcpc_log(tag, "recved %dbytes from peer, %s\r\n",
                             ret, send_buf);
                }
            }

            if (conn_ok)
                vTaskDelay(1000);
        }
        break;

    case SEND_MODE_STREAM:
        nbytes = opts_ptr->bytes;
        while (nbytes > 0) {
            int bytes = nbytes > TCPC_BUFFER_SIZE ? TCPC_BUFFER_SIZE : nbytes;
            ret = write(sock, send_buf, bytes);
            if (ret < 0) {
                tcpc_log(tag, "write failed, %s\r\n", strerror(errno));
                conn_ok = 0;
                break;
            }
            nbytes -= ret;
        }
        break;

    default:
        tcpc_log(tag, "invalid xfer mode %#hhx\r\n", opts_ptr->mode);
        break;
    }

    if (conn_ok)
        tcpc_log(tag, "tcpc data transfer finished\r\n");
    else
        tcpc_log(tag, "tcpc data transfer aborted due to bad connection\r\n");

err_connect:
    closesocket(sock);
err_skt:
    vPortFree(send_buf);
err_sndbuf:
    vPortFree(arg);
    vTaskDelete(NULL);
}

static void show_help(void)
{
    printf("tcpc [bchmpt] - A simple tcp client\r\n"
           "\t-b\t\tbytes that is expected to be transferred, %d by default(stream mode only)\r\n"
           "\t-c\t\tserver IP address, %s by default\r\n"
           "\t-p\t\tserver port, %d by default\r\n"
           "\t-t\t\txfer count/round in ASCII mode\r\n"
           "\t-m\t\ttransfer mode, 0 - ASCII(default), 1 - stream\r\n"
           "\t-e\t\tenable echo if it's NOT zero(ASCII mode only, enabled by dedfault)\r\n"
           "\t-h\t\tshow this help menu and exit\r\n",
           TCPC_DEFAULT_BYTES, TCPC_DEFAULT_SVR_IP, TCPC_PORT_CONNECT);
}

void tcpclient_cmd(char *buf, int len, int argc, char **argv)
{
    static uint8_t tcpc_counter = 0;
    int opt;
    getopt_env_t getopt_env;
    struct tcpc_opts *opts_ptr;
    char name[32];

    opts_ptr = (struct tcpc_opts*)pvPortMalloc(sizeof(struct tcpc_opts));
    if (NULL == opts_ptr) {
        printf("[TCPC] no mem for struct tcpc_opts\r\n");
        return;
    }

    memset(opts_ptr, 0, sizeof(struct tcpc_opts));
    opts_ptr->echo_enable = 1;
    utils_getopt_init(&getopt_env, 0);

    while ((opt = utils_getopt(&getopt_env, argc, argv, "m:c:b:t:p:e:h")) != -1) {
        switch (opt) {
        case 'm':
            opts_ptr->mode = atoi(getopt_env.optarg);
            break;

        case 'c':
            strncpy(opts_ptr->host, getopt_env.optarg, sizeof(opts_ptr->host) - 1);
            opts_ptr->host[sizeof(opts_ptr->host) - 1] = '\0';
            break;

        case 'p':
            opts_ptr->port = atoi(getopt_env.optarg);
            break;

        case 'b':
            opts_ptr->bytes = atoi(getopt_env.optarg);
            break;

        case 't':
            opts_ptr->counter = atoi(getopt_env.optarg);
            break;

        case 'e':
            opts_ptr->echo_enable = !!atoi(getopt_env.optarg);
            break;

        case 'h':
            show_help();
            vPortFree(opts_ptr);
            return;

        default:
            printf("unknow option: %c\r\n", getopt_env.optopt);
            break;
        }
    }

    /* sanity check */
    if ('\0' == opts_ptr->host[0])
        strcpy(opts_ptr->host, TCPC_DEFAULT_SVR_IP);
    if (!opts_ptr->counter)
        opts_ptr->counter = TCPC_DEFAULT_COUNTER;
    if (!opts_ptr->port)
        opts_ptr->port = TCPC_PORT_CONNECT;
    if (!opts_ptr->bytes)
        opts_ptr->bytes = TCPC_DEFAULT_BYTES;
    if (opts_ptr->mode != SEND_MODE_ASCII && opts_ptr->mode != SEND_MODE_STREAM)
        opts_ptr->mode = SEND_MODE_ASCII;

    printf("[TCPCLIENT]\r\n"
           "\thost endpoint\t%s:%d\r\n"
           "\tmode\t\t%s\r\n"
           "\tbytes\t\t%d\r\n"
           "\tcounter\t\t%d\r\n"
           "\techo\t\t%d\r\n",
           opts_ptr->host, opts_ptr->port,
           xfer_mode_string(opts_ptr->mode),
           opts_ptr->bytes,
           opts_ptr->counter,
           opts_ptr->echo_enable);

    int idx = tcpc_counter++;
    snprintf(name, sizeof(name), "tcpc-%u", idx);
    opts_ptr->idx = idx;
    xTaskCreate(tcpc_entry, name, 2048, opts_ptr, 10, NULL);
}

// STATIC_CLI_CMD_ATTRIBUTE makes this(these) command(s) static
const static struct cli_command cmds_user[] STATIC_CLI_CMD_ATTRIBUTE = {
        {"tcpc", "create a tcp client in a new task", tcpclient_cmd},
};

int network_netutils_tcpclinet_cli_register()
{
    // static command(s) do NOT need to call aos_cli_register_command(s) to register.
    // However, calling aos_cli_register_command(s) here is OK but is of no effect as cmds_user are included in cmds list.
    // XXX NOTE: Calling this *empty* function is necessary to make cmds_user in this file to be kept in the final link.
    //aos_cli_register_commands(cmds_user, sizeof(cmds_user)/sizeof(cmds_user[0]));
    return 0;
}
