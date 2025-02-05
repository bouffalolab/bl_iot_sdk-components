#include <utils_bitmap_window.h>

#ifdef CONFIG_CLI_CMD_ENABLE
#include <cli.h>
#endif
#include <stdio.h>

static void cmd_bitmap_window_test(char *buf, int len, int argc, char **argv)
{
    int ret;
    struct utils_bitmap_ctx ctx;

    utils_bitmap_window_init(&ctx, 0, 4095);

    printf("Set Seq 1 2 3 4 5 6 7...\r\n");
    utils_bitmap_window_bit_set(&ctx, 1);
    utils_bitmap_window_bit_set(&ctx, 2);
    utils_bitmap_window_bit_set(&ctx, 3);
    utils_bitmap_window_bit_set(&ctx, 4);
    utils_bitmap_window_bit_set(&ctx, 5);
    utils_bitmap_window_bit_set(&ctx, 6);
    ret = utils_bitmap_window_bit_set(&ctx, 7);
    printf("  Got ssn %d(0) Bitmap %llx(0), ret %x\r\n",
        utils_bitmap_window_ssn_get(&ctx),
        utils_bitmap_window_map_get(&ctx),
        ret
    );

    printf("Set Seq 0 ...\r\n");
    ret = utils_bitmap_window_bit_set(&ctx, 0);
    printf("  Got ssn %d(8) Bitmap %llx(0), ret %x\r\n",
        utils_bitmap_window_ssn_get(&ctx),
        utils_bitmap_window_map_get(&ctx),
        ret
    );

    printf("Set Seq 0 ...\r\n");
    ret = utils_bitmap_window_bit_set(&ctx, 0);
    printf("  Got ssn %d(8) Bitmap %llx(0), ret %x\r\n",
        utils_bitmap_window_ssn_get(&ctx),
        utils_bitmap_window_map_get(&ctx),
        ret
    );

    printf("Set Seq 9 ...\r\n");
    ret = utils_bitmap_window_bit_set(&ctx, 9);
    printf("  Got ssn %d(8) Bitmap %llx(2), ret %x\r\n",
        utils_bitmap_window_ssn_get(&ctx),
        utils_bitmap_window_map_get(&ctx),
        ret
    );

    printf("Set Seq 9 ...\r\n");
    ret = utils_bitmap_window_bit_set(&ctx, 9);
    printf("  Got ssn %d(8) Bitmap %llx(2), ret %d\r\n",
        utils_bitmap_window_ssn_get(&ctx),
        utils_bitmap_window_map_get(&ctx),
        ret
    );

}

#ifdef CONFIG_CLI_CMD_ENABLE
const static struct cli_command cmds_user[] STATIC_CLI_CMD_ATTRIBUTE = {
  {"utils_bitmap_widow_test", "bitmap window test", cmd_bitmap_window_test},
};
#endif

int utils_bitmap_window_cli_init(void)
{
    // static command(s) do NOT need to call aos_cli_register_command(s) to register.
    // However, calling aos_cli_register_command(s) here is OK but is of no effect as cmds_user are included in cmds list.
    // XXX NOTE: Calling this *empty* function is necessary to make cmds_user in this file to be kept in the final link.
    //return aos_cli_register_commands(cmds_user, sizeof(cmds_user) / sizeof(cmds_user[0]));
    return 0;
}
