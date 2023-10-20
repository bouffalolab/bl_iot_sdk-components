#include <stdbool.h>

#include <bl_sec.h>
#include "bl_sec_ghash.h"
#include "bl_sec_hw_common.h"

int bl_sec_ghash_init()
{
#ifdef BL616
    Sec_Eng_Group0_Request_GMAC_Access();
#endif
    Sec_Eng_GMAC_Enable_BE();
    Sec_Eng_GMAC_Enable_Link();
    return 0;
}

int bl_ghash_acquire_hw()
{
    bl_sec_enter_critical();
    return 0;
}

int bl_ghash_release_hw()
{
    bl_sec_exit_critical(0);
    return 0;
}

int bl_ghash_init(bl_sec_ghash_t *ctx, const uint8_t key[16])
{
    if (!ctx || !key) {
        return -1;
    }
#ifdef BL616
    if (bl_sec_is_cache_addr(ctx)) {
        L1C_DCache_Clean_Invalid_By_Addr((uintptr_t)ctx, sizeof(*ctx));
        ctx = bl_sec_get_no_cache_addr(ctx);
    }
#endif
    memset(ctx, 0, sizeof(*ctx));
    memcpy(&ctx->link_cfg.gmacKey0, key, 16);
    return 0;
}

int bl_ghash_clear(bl_sec_ghash_t *ctx)
{
    if (!ctx) {
        return -1;
    }
#ifdef BL616
    if (bl_sec_is_cache_addr(ctx)) {
        L1C_DCache_Clean_Invalid_By_Addr((uintptr_t)ctx, sizeof(*ctx));
        ctx = bl_sec_get_no_cache_addr(ctx);
    }
#endif
    memset(&ctx->link_cfg.result, 0, 16);
    return 0;
}

int bl_ghash_update(bl_sec_ghash_t *ctx, const uint8_t *input, size_t input_len)
{
    int ret = 0;
    size_t len, rem;

    if (!ctx) {
        return -1;
    }
    if (input_len >= 16 * 65536) {
        return -1;
    }

    if (input_len == 0) {
        return 0;
    }

#ifdef BL616
    bl_sec_enter_critical();
    if (bl_sec_is_cache_addr(ctx)) {
        L1C_DCache_Clean_Invalid_By_Addr((uintptr_t)ctx, sizeof(*ctx));
        ctx = bl_sec_get_no_cache_addr(ctx);
    }
    if (bl_sec_is_cache_addr(input)) {
        L1C_DCache_Clean_Invalid_By_Addr((uintptr_t)input, input_len);
    }
#endif
    rem = input_len % 16;
    len = input_len - rem;
    if (len) {
        ret = Sec_Eng_GMAC_Link_Work((uint32_t)&ctx->link_cfg, input, len, ctx->tmp_buf);
        if (ret != SUCCESS) {
            goto exit;
        }
    }
    if (rem) {
        memcpy(ctx->tmp_buf, input + len, rem);
        memset(ctx->tmp_buf + rem, 0, 16 - rem);
        ret = Sec_Eng_GMAC_Link_Work((uint32_t)&ctx->link_cfg, ctx->tmp_buf, 16, ctx->tmp_buf);
    }
exit:
#ifdef BL616
    bl_sec_exit_critical(0);
#endif
    return !(ret == SUCCESS);
}

int bl_ghash_finish(bl_sec_ghash_t *ctx, uint8_t result[16])
{
    if (!ctx || !result) {
        return -1;
    }
#ifdef BL616
    if (bl_sec_is_cache_addr(ctx)) {
        L1C_DCache_Clean_Invalid_By_Addr((uintptr_t)ctx, sizeof(*ctx));
        ctx = bl_sec_get_no_cache_addr(ctx);
    }
#endif
    memcpy(result, ctx->link_cfg.result, 16);
    return 0;
}


/*
 * Test cases
 */
#include <stdlib.h>
#include <stdio.h>

#define BL_SEC_INTENTIONALLY_LEAK(x) do{(void)x;}while(0)

#define GHASH_TEST_ITERATIONS 100

bool tc_ghash()
{
    // Test vector from GCM spec, Test Case 2
    const uint8_t H_key[16] = {0x66, 0xe9, 0x4b, 0xd4, 0xef, 0x8a, 0x2c, 0x3b, 0x88, 0x4c, 0xfa, 0x59, 0xca, 0x34, 0x2b, 0x2e};
    const uint8_t C[16] = {0x03, 0x88, 0xda, 0xce, 0x60, 0xb6, 0xa3, 0x92, 0xf3, 0x28, 0xc2, 0xb9, 0x71, 0xb2, 0xfe, 0x78};
    const uint8_t len_A_len_B[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80};
    const uint8_t ghash_H_A_C[16] = {0xf3, 0x8c, 0xbb, 0x1a, 0xd6, 0x92, 0x23, 0xdc, 0xc3, 0x45, 0x7a, 0xe5, 0xb6, 0xb0, 0xf8, 0x85};

    while (1) {
        bl_sec_ghash_t *ctx;
        uint8_t *in, *out;
        ctx = malloc(sizeof(*ctx));
        in = malloc(16);
        out = malloc(16);
        if (!ctx || !in || !out) {
            printf("Test end\r\n");
            return true;
        }

        printf("Test addr %p\r\n", ctx);
        for (size_t i = 0; i < GHASH_TEST_ITERATIONS; ++i) {
            // input from flash
            memset(out, 0, 16);
            bl_ghash_init(ctx, H_key);
            bl_ghash_update(ctx, C, 16);
            bl_ghash_update(ctx, len_A_len_B, 16);
            bl_ghash_finish(ctx, out);

            if (memcmp(ghash_H_A_C, out, 16)) {
                return false;
            }

            // input from ram
            memset(out, 0, 16);
            memcpy(in, H_key, 16);
            bl_ghash_init(ctx, in);
            memcpy(in, C, 16);
            bl_ghash_update(ctx, in, 16);
            memcpy(in, len_A_len_B, 16);
            bl_ghash_update(ctx, in, 16);
            bl_ghash_finish(ctx, out);

            if (memcmp(ghash_H_A_C, out, 16)) {
                return false;
            }
        }
        BL_SEC_INTENTIONALLY_LEAK(ctx);
        BL_SEC_INTENTIONALLY_LEAK(in);
        BL_SEC_INTENTIONALLY_LEAK(out);
    }
    return true;
}

#define MD(tc) {tc, #tc}
static struct {
    bool (*tc_fun)(void);
    const char *desc;
} tc[] = {
    MD(tc_ghash),
};


int bl_sec_ghash_test()
{
    for (int i = 0; i < sizeof(tc) / sizeof(tc[0]); ++i) {
        printf("Running test case %s\r\n", tc[i].desc);
        if (tc[i].tc_fun()) {
            printf("\t\tPassed\r\n");
        } else {
            printf("\t\tFailed\r\n");
        }
    }
    return 0;
}
