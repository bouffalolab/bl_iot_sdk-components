#include <FreeRTOS.h>
#include <task.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <cli.h>
#include <aos/kernel.h>
#include <lwip/sockets.h>
#include <netutils/netutils.h>
#include "utils_getopt.h"

#define MAXCLIENTNUM    (2)
#define SERVERPORT      (5150)
#define DATA_BUFSZ      (10 * 1024 + 1)
#define TCP_DEFAULT_TIMEOUT_SEC (3)

static inline void tcps_log(const char *fmt, ...)
{
    va_list va;
    char s[256];

    puts("[TCP-Server] ");
    va_start(va, fmt);
    vsnprintf(s, sizeof(s) - 1, fmt, va);
    va_end(va);
    puts(s);
}

enum {
    TCP_XFER_MODE_UNKNOWN,
    TCP_XFER_MODE_ASCII,
    TCP_XFER_MODE_STREAM,
};

enum {
    TC_ERR_TIMEOUT = 0,
    TC_ERR_CLOSED,
    TC_ERR_READ_RST,
    TC_ERR_WRITE_RST,
};

const char *reason[] = {
    [TC_ERR_TIMEOUT]  = "Timeout",
    [TC_ERR_CLOSED]   = "Peer closed the connection",
    [TC_ERR_READ_RST] = "Reset on read",
    [TC_ERR_WRITE_RST] = "Reset on write",
};

struct tcp_client {
    /* global index */
    int idx;
    /* socket descriptor */
    int sock;
    /* timestamp at which the client is active */
    TickType_t last_active;
    TickType_t last_print;
    /* 0 - dead, 1 - alive */
    char alive;
    /* bytes read from this client */
    uint32_t bytes_xferred;
    uint32_t last_bytes_xferred;
    int xfer_mode;
};

struct tcps_option {
    /* listen port */
    int port;
    /* client connection is closed after this timeout */
    int max_timeout_sec;
};

static struct tcps_option tcps_opt;
static struct tcp_client tcs[MAXCLIENTNUM];
static int n_tc;
static int maxsock;
static uint8_t *g_buf;
static char should_stop;

static void tcp_client_del(struct tcp_client *tc, int err)
{
    if (tc->sock > 0) {
        closesocket(tc->sock);
        tc->sock = -1;
    }
    tc->idx = -1;
    tc->alive = 0;
    tc->last_active = 0;
    tc->last_print = 0;
    tc->bytes_xferred = 0;
    tc->xfer_mode = TCP_XFER_MODE_UNKNOWN;
    n_tc--;
    tcps_log("client%d is deleted, reason %s, current clients number %d\r\n",
             tc->idx, reason[err], n_tc);
}

static void tcp_client_add(struct tcp_client *tc, int sock, int idx)
{
    tc->idx = idx;
    tc->last_active = xTaskGetTickCount();
    tc->last_print = tc->last_active;
    tc->sock = sock;
    tc->bytes_xferred = 0;
    tc->alive = 1;
    tc->xfer_mode = TCP_XFER_MODE_UNKNOWN;
    n_tc++;
    tcps_log("added a new client, current clients number %d\r\n", n_tc);
}

/*
 * @brief: accept a new tcp connection, and try to add a tcp client.
 * @param: lsock - listening socket.
 * @return: 0 on success, nagative number otherwise.
 */
static int tcp_listener_callback(int lsock)
{
    int i, nsock;
    struct sockaddr_in caddr;
    uint32_t addrlen = sizeof(caddr);

    nsock = accept(lsock, (struct sockaddr *)&caddr, &addrlen);
    if (nsock < 0) {
        tcps_log("accept failed, %d %s\r\n", errno, strerror(errno));
        return -1;
    }

    tcps_log("accepted a connection from %s:%d, socket %d\r\n",
             inet_ntoa(caddr.sin_addr), ntohs(caddr.sin_port), nsock);
    if (n_tc >= MAXCLIENTNUM) {
        tcps_log("clients number reached its limit %d, "
                 "refuse to serve the incoming client!\r\n", MAXCLIENTNUM);
        send(nsock, "Bye", 4, 0);
        closesocket(nsock);
        return 0;
    }

    /* register a new client and prepare to monitor its IO events */
    for (i = 0; i < MAXCLIENTNUM; i++) {
        struct tcp_client *tc = &tcs[i];

        if (!tc->alive) {
            tcp_client_add(tc, nsock, i);
            if (nsock > maxsock)
                maxsock = nsock;
            break;
        }
    }

    if (i >= MAXCLIENTNUM) {
        tcps_log("Error: no tcp client slot?!\r\n");
        return -1;
    }
    int yes = 1;
    setsockopt(nsock, IPPROTO_TCP, TCP_NODELAY, (void *)&yes, sizeof(yes));
    return 0;
}

/*
 * @brief: read data from a tcp client and echo clients in ASCII mode.
 * @param: tc - a tcp client with readable event pending.
 * @return: 0 on success, negative number otherwise.
 */
static int tcp_client_readcb(struct tcp_client *tc)
{
    int ret;

    memset(g_buf, 0, DATA_BUFSZ);
    ret = recv(tc->sock, g_buf, DATA_BUFSZ, 0);
    if (ret < 0) {
        tcps_log("client%d connection shutdown, %s, transferred bytes %d\r\n",
                 tc->idx, strerror(errno), tc->bytes_xferred);
        tcp_client_del(tc, TC_ERR_READ_RST);
        return 0;
    }

    if (!ret) {
        tcps_log("client%d closed its connection, transferred bytes %d\r\n",
                 tc->idx, tc->bytes_xferred);
        tcp_client_del(tc, TC_ERR_CLOSED);
        return 0;
    }

    /* ok, we got something */
    TickType_t tick_now = xTaskGetTickCount();
    tc->last_active = tick_now;
    /*
     * XXX the way by which tcp server finds out what transfer mode
     * a client uses is a little bit of tricky, a reliable solution
     * is to add negotiation message between client and server, which
     * needs more effort.
     */
    if (tc->xfer_mode == TCP_XFER_MODE_UNKNOWN) {
        if (isalpha((unsigned char)g_buf[0])) {
            tc->xfer_mode = TCP_XFER_MODE_ASCII;
            tcps_log("client%d uses ASCII mode\r\n", tc->idx);
        } else {
            tc->xfer_mode = TCP_XFER_MODE_STREAM;
            tcps_log("client%d uses stream mode\r\n", tc->idx);
        }
    }

    // TODO bytes_transfered overflow
    tc->bytes_xferred += ret;
    TickType_t delta = (TickType_t)(int)(tick_now - tc->last_print);
    if (delta >= configTICK_RATE_HZ * 1) {
        uint32_t delta_ms = delta / configTICK_RATE_HZ;
        uint32_t bw_kbps = (tc->bytes_xferred - tc->last_bytes_xferred) * 8;
        bw_kbps = bw_kbps / delta_ms;
        tcps_log("recved %dbytes from client%d so far, bandwidth %ukbps\r\n",
                 tc->bytes_xferred, tc->idx, bw_kbps);
        tc->last_print = tick_now;
        tc->last_bytes_xferred = tc->bytes_xferred;
    }
    /* always echo in ASCII mode */
    if (tc->xfer_mode == TCP_XFER_MODE_ASCII) {
        g_buf[ret] = '\0';
        tcps_log("client%d: %s\r\n", tc->idx, g_buf);

        int sent = 0;
        while (sent < ret) {
            int len;
            len = send(tc->sock, &g_buf[sent], ret - sent, 0);
            if (len < 0) {
                tcps_log("failed to send to client%d, %s\r\n",
                         tc->idx, strerror(errno));
                tcp_client_del(tc, TC_ERR_WRITE_RST);
                return -1;
            }

            sent += len;
            tcps_log("sent %d bytes to client%d\r\n", ret, tc->idx);
        }
        tcps_log("client%d echo done\r\n\r\n", tc->idx);
    }
    return 0;
}

static void tcps_entry(void *arg)
{
    fd_set fdsr;
    int lsock, i, ret;
    struct timeval tv;
    struct sockaddr_in saddr;
    struct tcp_client *tc = NULL;

    g_buf = pvPortMalloc(DATA_BUFSZ);
    if (!g_buf) {
        tcps_log("no mem for data buffer\r\n");
        vTaskDelete(NULL);
    }
    memset(tcs, 0, sizeof(tcs));

    /* create a tcp listener */
    if ((lsock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        tcps_log("failed to create listen socket, %s\r\n", strerror(errno));
        goto err_lsock;
    }

    int yes = 1;
    if (setsockopt(lsock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        tcps_log("setsockopt failed, %s\r\n", strerror(errno));
        goto err_lsock;
    }

    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(tcps_opt.port);
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    ret = bind(lsock, (struct sockaddr *)&saddr, sizeof(saddr));
    if (ret < 0) {
        tcps_log("bind failed, %s\r\n", strerror(errno));
        goto err_lsock;
    }

    if (listen(lsock, MAXCLIENTNUM) < 0) {
    	tcps_log("listen failed, %s\r\n", strerror(errno));
        goto err_lsock;
    }

    maxsock = lsock;
    tcps_log("Listening on :%d\r\n", tcps_opt.port);
    while (!should_stop) {
        FD_ZERO(&fdsr);
        FD_SET(lsock, &fdsr);
        /*
         * check all the clients, monitor active ones and remove
         * inactive ones.
         */
        TickType_t tick_now = xTaskGetTickCount();
        for (i = 0; i < MAXCLIENTNUM; i++) {
            tc = &tcs[i];
            if (tc->alive) {
                TickType_t delta;
                delta = (TickType_t)(int)(tick_now - tc->last_active);
                if (delta >= tcps_opt.max_timeout_sec * configTICK_RATE_HZ)
                    tcp_client_del(tc, TC_ERR_TIMEOUT);
                else
                    FD_SET(tc->sock, &fdsr);
            }
        }

        tv.tv_sec = 1;
        tv.tv_usec = 0;
        ret = select(maxsock + 1, &fdsr, NULL, NULL, &tv);
        if (ret < 0) {
            tcps_log("select failed, %d %s\r\n", errno, strerror(errno));
            goto err_select;
        }

        if (ret == 0)
            continue;

        /* check if there is any pending data from active clients */
        for (i = 0; i < MAXCLIENTNUM; i++) {
            tc = &tcs[i];
            if (!tc->alive)
                continue;
            if (FD_ISSET(tc->sock, &fdsr))
                tcp_client_readcb(tc);
        }

        /* here comes a new connection */
        if (FD_ISSET(lsock, &fdsr)) {
            int err = tcp_listener_callback(lsock);
            if (err < 0)
                tcps_log("listener encountered a problem\r\n");
        }
    }
    tcps_log("tcp server exiting\r\n");

err_select:
    closesocket(lsock);
err_lsock:
    vPortFree(g_buf);
    vTaskDelete(NULL);
}

static void show_help()
{
    printf("tcps [hptk] - a simple tcp server(no multiple listeners)\r\n"
           "\t-h\t\tshow this help menu and exit\r\n"
           "\t-p\t\tlisten port(default %d)\r\n"
           "\t-t\t\ttimeout in second(default %d)\r\n"
           "\t-k\t\tstop tcp server\r\n",
           SERVERPORT, TCP_DEFAULT_TIMEOUT_SEC);
}

static void cmd_tcp_server(char *buf, int len, int argc, char **argv)
{
    int opt;
    getopt_env_t getopt_env;

    utils_getopt_init(&getopt_env, 0);
    while ((opt = utils_getopt(&getopt_env, argc, argv, "t:p:hk")) != -1) {
        switch (opt) {
        case 't':
            tcps_opt.max_timeout_sec = atoi(getopt_env.optarg);
            break;

        case 'p':
            tcps_opt.port = atoi(getopt_env.optarg);
            break;

        case 'h':
            show_help();
            return;

        case 'k':
            should_stop = 1;
            return;

        default:
            printf("invalid option %c\r\n", getopt_env.optopt);
            break;
        }
    }

    if (!tcps_opt.port)
        tcps_opt.port = SERVERPORT;
    if (!tcps_opt.max_timeout_sec)
        tcps_opt.max_timeout_sec = TCP_DEFAULT_TIMEOUT_SEC;

    printf("listen port\t%d\r\n"
           "timeout\t\t%d seconds\r\n",
           tcps_opt.port, tcps_opt.max_timeout_sec);
    should_stop = 0;
    xTaskCreate(tcps_entry, "tcp-server", 2048, NULL, 10, NULL);
}

const static struct cli_command cmds_user[] STATIC_CLI_CMD_ATTRIBUTE = {
        {"tcps", "create a tcp server in a new task", cmd_tcp_server},
};

int network_netutils_tcpserver_cli_register()
{
    // static command(s) do NOT need to call aos_cli_register_command(s) to register.
    // However, calling aos_cli_register_command(s) here is OK but is of no effect as cmds_user are included in cmds list.
    // XXX NOTE: Calling this *empty* function is necessary to make cmds_user in this file to be kept in the final link.
    //aos_cli_register_commands(cmds_user, sizeof(cmds_user)/sizeof(cmds_user[0]));
    return 0;
}
