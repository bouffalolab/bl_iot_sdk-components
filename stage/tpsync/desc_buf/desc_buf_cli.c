
#include <stdint.h>
#include <string.h>

#include <FreeRTOS.h>
#include <queue.h>

#include <cli.h>

#include <desc_buf.h>
#include <tpdgb.h>

desc_msg_t s_desc;

static void db_test_cmd(char *buf, int len, int argc, char **argv)
{
#define DESC_ITEMS 10
    char msg[30];
    char rmsg[30];
    uint32_t rsize;
    int i;
    int res;

    printf("desc_init test\r\n");
    desc_init(&s_desc, DESC_ITEMS);

    for (i = 0; i < DESC_ITEMS; i++) {
        memset(msg, i, sizeof(msg));
        if (5 == i) {
            res = desc_push_tofront(&s_desc, msg, 10 + i, 0);
            if (0 != res) {
                printf("desc_push_tofront error i = %d, res = %d.\r\n", i, res);
            }
        } else {
            res = desc_push_toback(&s_desc, msg, 10 + i, 0);
            if (0 != res) {
                printf("desc_push_toback error i = %d, res = %d.\r\n", i, res);
            }
        }
    }

    printf("desc_peek test\r\n");
    res = desc_peek(&s_desc, &rsize, 0);
    if (0 != res) {
        printf("desc_pop error res = %d.\r\n", res);
    }
    printf("peek rsize = %ld\r\n", rsize);

    printf("desc_pop test\r\n");
    for (i = 0; i < 11; i++) {
        res = desc_pop(&s_desc, rmsg, &rsize, 0);
        if (0 != res) {
            printf("desc_pop error res = %d.\r\n", res);
            continue;
        }
        log_buf(rmsg, rsize);
    }

    desc_deinit(&s_desc);

    printf("test complete!\r\n");
}

// STATIC_CLI_CMD_ATTRIBUTE makes this(these) command(s) static
const static struct cli_command cmds_user[] STATIC_CLI_CMD_ATTRIBUTE = {
        {"desc_init", "desc buf test", db_test_cmd},
};

int desc_buf_init(void)
{
    // static command(s) do NOT need to call aos_cli_register_command(s) to register.
    // However, calling aos_cli_register_command(s) here is OK but is of no effect as cmds_user are included in cmds list.
    // XXX NOTE: Calling this *empty* function is necessary to make cmds_user in this file to be kept in the final link.
    //return aos_cli_register_commands(cmds_user, sizeof(cmds_user)/sizeof(cmds_user[0]));
    return 0;
}
