/*
 *  Description: GCM hardware acceleration
 *  Copyright (C) Bouffalo Lab 2016-2023
 *  SPDX-License-Identifier: Apache-2.0
 *  File Name:   gcm_alt.c
 *  Author:      Chien Wong(qwang@bouffalolab.com)
 *  Start Date:  Jun 20, 2023
 *  Last Update: Jul 11, 2023
 *
 *  Change log:
 *  Jun 25, 2023: qwang
 *      First version.
 *  Jul 7, 2023: qwang
 *      Optimize further.
 *  Aug 14, 202: qwang
 *      Add data protection.
 */
#include "common.h"

#if defined(MBEDTLS_GCM_C)

#include "mbedtls/gcm.h"
#include "mbedtls/platform.h"
#include "mbedtls/platform_util.h"
#include "mbedtls/error.h"

#include <string.h>

#include <bl_sec.h>
#include <bl_sec_hw_common.h>

#define AES_ID SEC_ENG_AES_ID0
#define SEC_ENG_BUSY_TIMEOUT_COUNT (100 * 160 * 1000)

/* Parameter validation macros */
#define GCM_VALIDATE_RET(cond) \
    MBEDTLS_INTERNAL_VALIDATE_RET(cond, MBEDTLS_ERR_GCM_BAD_INPUT)
#define GCM_VALIDATE(cond) \
    MBEDTLS_INTERNAL_VALIDATE(cond)

#define AES_CHECK_OP(op)                                 \
    do {                                                 \
        bl_aes_acquire_hw();                             \
        if (op) {                                        \
            bl_aes_release_hw();                         \
            ret = MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED;  \
            goto cleanup;                                \
        }                                                \
        bl_aes_release_hw();                             \
    } while (0)
#define GHASH_CHECK_OP(op)                               \
    do {                                                 \
        bl_ghash_acquire_hw();                           \
        if (op) {                                        \
            bl_ghash_release_hw();                       \
            ret = MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED;  \
            goto cleanup;                                \
        }                                                \
        bl_ghash_release_hw();                           \
    } while (0)

/*
 * Initialize a context
 */
void mbedtls_gcm_init(mbedtls_gcm_context *ctx)
{
    GCM_VALIDATE(ctx != NULL);
    memset(ctx, 0, sizeof(mbedtls_gcm_context));
}

int mbedtls_gcm_setkey(mbedtls_gcm_context *ctx,
                       mbedtls_cipher_id_t cipher,
                       const unsigned char *key,
                       unsigned int keybits)
{
    unsigned char work_buf[16];
    int ret = 0;

    GCM_VALIDATE_RET(ctx != NULL);
    GCM_VALIDATE_RET(key != NULL);
    GCM_VALIDATE_RET(keybits == 128 || keybits == 192 || keybits == 256);

    if (cipher != MBEDTLS_CIPHER_ID_AES) {
        return MBEDTLS_ERR_PLATFORM_FEATURE_UNSUPPORTED;
    }

    bl_aes_init(&ctx->aes);
    bl_aes_set_key(&ctx->aes, BL_AES_ENCRYPT, key, keybits / 8);
    memset(work_buf, 0x00, sizeof(work_buf));
    bl_aes_set_mode(&ctx->aes, BL_AES_ECB, NULL);
    AES_CHECK_OP(bl_aes_transform(&ctx->aes, BL_AES_ENCRYPT, work_buf, work_buf));
    bl_ghash_init(&ctx->ghash, work_buf);

cleanup:
    return ret;
}

static inline void ctr_inc(uint8_t ctr[16], uint32_t value)
{
    uint32_t carry = 0;
    for (int i = 15; i >= 0; i--) {
        uint32_t sum = ctr[i] + (value & 0xFF) + carry;
        ctr[i] = sum;
        carry = sum >> 8;
        value >>= 8;
        if (value == 0 && carry == 0) {
            break;
        }
    }
}

int mbedtls_gcm_starts(mbedtls_gcm_context *ctx,
                       int mode,
                       const unsigned char *iv,
                       size_t iv_len,
                       const unsigned char *add,
                       size_t add_len)
{
    int ret = 0;
    GCM_VALIDATE_RET(ctx != NULL);
    GCM_VALIDATE_RET(iv != NULL);
    GCM_VALIDATE_RET(add_len == 0 || add != NULL);

    /* IV and AD are limited to 2^64 bits, so 2^61 bytes */
    /* IV is not allowed to be zero length */
    if (iv_len == 0 ||
        ((uint64_t)iv_len) >> 61 != 0 ||
        ((uint64_t)add_len) >> 61 != 0) {
        return MBEDTLS_ERR_GCM_BAD_INPUT;
    }

    memset(ctx->y, 0x00, sizeof(ctx->y));

    ctx->mode = mode;
    ctx->len = 0;
    ctx->add_len = 0;

    if (iv_len == 12) {
        memcpy(ctx->y, iv, iv_len);
        ctx->y[15] = 1;
    } else {
        return MBEDTLS_ERR_PLATFORM_FEATURE_UNSUPPORTED;
    }

    bl_aes_set_mode(&ctx->aes, BL_AES_ECB, NULL);
    AES_CHECK_OP(bl_aes_transform(&ctx->aes, BL_AES_ENCRYPT, ctx->y, ctx->base_ectr));

    bl_ghash_clear(&ctx->ghash);
    ctx->add_len = add_len;
    if (add_len > 0) {
        GHASH_CHECK_OP(bl_ghash_update(&ctx->ghash, add, add_len));
    }

cleanup:
    return ret;
}

enum {
    TJ_READY,
    TJ_PENDING,
    TJ_ERROR,
};

typedef int (*task_poll_fn)(void *arg, bool run_first);

static int task_join(task_poll_fn poll, void *arg)
{
    int r1, r2;
    while (1) {
        r1 = poll(arg, true);
        r2 = poll(arg, false);
        if (r1 == TJ_ERROR || r2 == TJ_ERROR) {
            return TJ_ERROR;
        }
        if (r1 == TJ_READY && r2 == TJ_READY) {
            break;
        }
    }
    return TJ_READY;
}

enum {
    TASK_INIT,
    TASK_DOING,
    TASK_BLOCKED,
    TASK_DONE,
    TASK_ERROR,
};

static int gcm_task(void *arg, bool run_first)
{
    size_t idx = !run_first;
    size_t idx_peer = !idx;
    struct mbedtls_gcm_alt_tj_ctx *ctx = arg;
    int *state = &ctx->state[idx];
    int ret;

    switch (*state) {
    case TASK_INIT:
        if (run_first) {
            *state = TASK_DOING;
            ctx->submit[idx](ctx, idx);
            ctx->ticks[idx] = 0;
        } else {
            *state = TASK_BLOCKED;
        }

        break;
    case TASK_DOING:
        if (ctx->check[idx](ctx, idx)) {
            ctx->steps[idx]++;

            if (ctx->steps[idx] == ctx->tot_steps) {
                *state = TASK_DONE;
                break;
            }
            if (run_first || ctx->steps[idx_peer] > ctx->steps[idx]) {
                ctx->submit[idx](ctx, idx);
                ctx->ticks[idx] = 0;
            } else {
                *state = TASK_BLOCKED;
            }
        } else if (++ctx->ticks[idx] >= SEC_ENG_BUSY_TIMEOUT_COUNT) {
            *state = TASK_ERROR;
            break;
        }

        break;
    case TASK_BLOCKED:
        if (ctx->steps[idx_peer] > ctx->steps[idx]) {
            *state = TASK_DOING;
            ctx->submit[idx](ctx, idx);
            ctx->ticks[idx] = 0;
        }

        break;
    case TASK_DONE:
        break;
    }

    ret = TJ_PENDING;
    if (*state == TASK_DONE) {
        ret = TJ_READY;
    } else if (*state == TASK_ERROR) {
        ret = TJ_ERROR;
    }
    return ret;
}

static void ctr_submit(struct mbedtls_gcm_alt_tj_ctx *ctx, size_t idx)
{
    const uint32_t AESx = SEC_ENG_BASE;
    uint32_t tmpVal;
    bl_sec_aes_t *aes = ctx->ctx[idx];
    bl_SEC_Eng_AES_Link_Config_Type *link = &aes->link_cfg;
    uint16_t blks = ctx->step_len;

#ifdef BL616
    link = bl_sec_get_no_cache_addr(link);
#endif
    if (ctx->steps[idx] + 1 == ctx->tot_steps) {
        blks = ctx->last_step_len;
    }
    ctr_inc(ctx->ctr, 1);
    bl_aes_set_mode(aes, BL_AES_CTR, ctx->ctr);
    link->aesSrcAddr = (uint32_t)ctx->input[idx];
    link->aesDstAddr = (uint32_t)ctx->output[idx];
    link->aesMsgLen = blks;
    BL_WR_REG(AESx, SEC_ENG_SE_AES_0_LINK, (uint32_t)link);
    tmpVal = BL_RD_REG(AESx, SEC_ENG_SE_AES_0_CTRL);
    BL_WR_REG(AESx, SEC_ENG_SE_AES_0_CTRL, BL_SET_REG_BIT(tmpVal, SEC_ENG_SE_AES_0_EN));
    tmpVal = BL_RD_REG(AESx, SEC_ENG_SE_AES_0_CTRL);
    BL_WR_REG(AESx, SEC_ENG_SE_AES_0_CTRL, BL_SET_REG_BIT(tmpVal, SEC_ENG_SE_AES_0_TRIG_1T));
    ctx->input[idx] += blks * 16;
    ctx->output[idx] += blks * 16;
    ctr_inc(ctx->ctr, blks - 1);
}

static bool ctr_check(struct mbedtls_gcm_alt_tj_ctx *ctx, size_t idx)
{
    const uint32_t AESx = SEC_ENG_BASE;
    uint32_t tmpVal;
    tmpVal = BL_RD_REG(AESx, SEC_ENG_SE_AES_0_CTRL);
    return !BL_IS_REG_BIT_SET(tmpVal, SEC_ENG_SE_AES_0_BUSY);
}

static void ghash_submit(struct mbedtls_gcm_alt_tj_ctx *ctx, size_t idx)
{
    const uint32_t GMACx = SEC_ENG_BASE;
    uint32_t tmpVal;
    bl_sec_ghash_t *ghash = ctx->ctx[idx];
    bl_SEC_Eng_GMAC_Link_Config_Type *link = &ghash->link_cfg;
    uint16_t blks = ctx->step_len;
    if (ctx->steps[idx] + 1 == ctx->tot_steps) {
        blks = ctx->last_step_len;
    }

#ifdef BL616
    link = bl_sec_get_no_cache_addr(link);
#endif
    link->gmacSrcAddr = (uint32_t)ctx->input[idx];
    link->gmacMsgLen = blks;
    BL_WR_REG(GMACx, SEC_ENG_SE_GMAC_0_LCA, (uint32_t)link);
    tmpVal = BL_RD_REG(GMACx, SEC_ENG_SE_GMAC_0_CTRL_0);
    BL_WR_REG(GMACx, SEC_ENG_SE_GMAC_0_CTRL_0, BL_SET_REG_BIT(tmpVal, SEC_ENG_SE_GMAC_0_TRIG_1T));

    ctx->input[idx] += blks * 16;
}

static bool ghash_check(struct mbedtls_gcm_alt_tj_ctx *ctx, size_t idx)
{
    const uint32_t GMACx = SEC_ENG_BASE;
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(GMACx, SEC_ENG_SE_GMAC_0_CTRL_0);
    return !BL_IS_REG_BIT_SET(tmpVal, SEC_ENG_SE_GMAC_0_BUSY);
}

static void gcm_hw_init(mbedtls_gcm_context *ctx)
{
#ifdef BL616
    if (bl_sec_is_cache_addr(ctx)) {
        L1C_DCache_Clean_Invalid_By_Addr((uintptr_t)ctx, sizeof(*ctx));
        ctx = bl_sec_get_no_cache_addr(ctx);
    }
#endif
    bl_aes_acquire_hw();
    bl_ghash_acquire_hw();

    Sec_Eng_AES_Enable_Link(AES_ID);
    ctx->aes.link_cfg.aesDecEn = SEC_ENG_AES_ENCRYPTION;
}

static void gcm_hw_cleanup(void)
{
    const uint32_t AESx = SEC_ENG_BASE;
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(AESx, SEC_ENG_SE_AES_0_CTRL);
    BL_WR_REG(AESx, SEC_ENG_SE_AES_0_CTRL, BL_CLR_REG_BIT(tmpVal, SEC_ENG_SE_AES_0_EN));

    Sec_Eng_AES_Disable_Link(AES_ID);

    bl_ghash_release_hw();
    bl_aes_release_hw();
}

int mbedtls_gcm_update(mbedtls_gcm_context *ctx,
                       size_t length,
                       const unsigned char *input,
                       unsigned char *output)
{
    unsigned char ectr[16];
    size_t i;
    const unsigned char *p;
    unsigned char *out_p = output;
    size_t blocks, rem;
    int ret = 0;

    GCM_VALIDATE_RET(ctx != NULL);
    GCM_VALIDATE_RET(length == 0 || input != NULL);
    GCM_VALIDATE_RET(length == 0 || output != NULL);

    if (output > input && (size_t)(output - input) < length) {
        return MBEDTLS_ERR_GCM_BAD_INPUT;
    }

    /* Total length is restricted to 2^39 - 256 bits, ie 2^36 - 2^5 bytes
     * Also check for possible overflow */
    if (ctx->len + length < ctx->len ||
        (uint64_t)ctx->len + length > 0xFFFFFFFE0ull) {
        return MBEDTLS_ERR_GCM_BAD_INPUT;
    }

    if (length > 16 * 65535) {
        return MBEDTLS_ERR_PLATFORM_FEATURE_UNSUPPORTED;
    }

    ctx->len += length;

    blocks = length / 16;
    rem = length % 16;

    p = input;
#ifdef BL616
    unsigned long crit_flag = bl_sec_enter_critical();
    if (bl_sec_is_cache_addr(input)) {
        L1C_DCache_Clean_Invalid_By_Addr((uintptr_t)input, length);
    }
    if (bl_sec_is_cache_addr(output) && input != output) {
        L1C_DCache_Clean_Invalid_By_Addr((uintptr_t)output, length);
    }
#endif
    if (blocks > 0) {
        size_t step_len;
        size_t tot_steps;
        size_t last_step_len;
        struct mbedtls_gcm_alt_tj_ctx *tj = &ctx->tj;
        size_t idx1, idx2;
        int tj_ret;

        memset(tj, 0, sizeof(*tj));

        step_len = blocks / MBEDTLS_GCM_ALT_TJ_STEP_DIVISOR;
        if (step_len == 0) {
            step_len = 1;
        }
        tot_steps = blocks / step_len;
        last_step_len = step_len;
        if (blocks % step_len) {
            tot_steps++;
            last_step_len = blocks % step_len;
        }

        tj->ctr = ctx->y;
        tj->last_step_len = last_step_len;
        tj->tot_steps = tot_steps;
        tj->step_len = step_len;

        if (ctx->mode == MBEDTLS_GCM_DECRYPT) {
            idx1 = 0;
            idx2 = 1;

            tj->input[0] = input;
            tj->input[1] = input;
            tj->output[0] = NULL;
            tj->output[1] = output;
        } else {
            idx1 = 1;
            idx2 = 0;

            tj->input[0] = input;
            tj->input[1] = output;
            tj->output[0] = output;
            tj->output[1] = NULL;
        }
        tj->ctx[idx1] = &ctx->ghash;
        tj->ctx[idx2] = &ctx->aes;
        tj->submit[idx1] = ghash_submit;
        tj->submit[idx2] = ctr_submit;
        tj->check[idx1] = ghash_check;
        tj->check[idx2] = ctr_check;

        gcm_hw_init(ctx);
        tj_ret = task_join(gcm_task, tj);
        gcm_hw_cleanup();

        if (tj_ret == TJ_ERROR) {
            ret = MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED;
            goto cleanup;
        }
    }
    if (rem > 0) {
        p += blocks * 16;
        out_p += blocks * 16;

        if (ctx->mode == MBEDTLS_GCM_DECRYPT) {
            GHASH_CHECK_OP(bl_ghash_update(&ctx->ghash, p, rem));
        }

        ctr_inc(ctx->y, 1);
        bl_aes_set_mode(&ctx->aes, BL_AES_ECB, NULL);
        AES_CHECK_OP(bl_aes_transform(&ctx->aes, BL_AES_ENCRYPT, ctx->y, ectr));
        for (i = 0; i < rem; i++) {
            out_p[i] = ectr[i] ^ p[i];
        }

        if (ctx->mode == MBEDTLS_GCM_ENCRYPT) {
            GHASH_CHECK_OP(bl_ghash_update(&ctx->ghash, out_p, rem));
        }
    }

cleanup:
#ifdef BL616
    bl_sec_exit_critical(crit_flag);
#endif
    return ret;
}

int mbedtls_gcm_finish(mbedtls_gcm_context *ctx,
                       unsigned char *tag,
                       size_t tag_len)
{
    unsigned char work_buf[16];
    size_t i;
    uint64_t orig_len;
    uint64_t orig_add_len;
    int ret = 0;

    GCM_VALIDATE_RET(ctx != NULL);
    GCM_VALIDATE_RET(tag != NULL);

    orig_len = ctx->len * 8;
    orig_add_len = ctx->add_len * 8;

    if (tag_len > 16 || tag_len < 4) {
        return MBEDTLS_ERR_GCM_BAD_INPUT;
    }

    bl_aes_set_mode(&ctx->aes, BL_AES_ECB, NULL);

    memcpy(tag, ctx->base_ectr, tag_len);

    if (orig_len || orig_add_len) {
        memset(work_buf, 0x00, 16);

        MBEDTLS_PUT_UINT32_BE((orig_add_len >> 32), work_buf, 0);
        MBEDTLS_PUT_UINT32_BE((orig_add_len), work_buf, 4);
        MBEDTLS_PUT_UINT32_BE((orig_len >> 32), work_buf, 8);
        MBEDTLS_PUT_UINT32_BE((orig_len), work_buf, 12);

        GHASH_CHECK_OP(bl_ghash_update(&ctx->ghash, work_buf, 16));
        bl_ghash_finish(&ctx->ghash, work_buf);

        for (i = 0; i < tag_len; i++) {
            tag[i] ^= work_buf[i];
        }
    }

cleanup:
    return ret;
}

int mbedtls_gcm_crypt_and_tag(mbedtls_gcm_context *ctx,
                              int mode,
                              size_t length,
                              const unsigned char *iv,
                              size_t iv_len,
                              const unsigned char *add,
                              size_t add_len,
                              const unsigned char *input,
                              unsigned char *output,
                              size_t tag_len,
                              unsigned char *tag)
{
    int ret = MBEDTLS_ERR_ERROR_CORRUPTION_DETECTED;

    GCM_VALIDATE_RET(ctx != NULL);
    GCM_VALIDATE_RET(iv != NULL);
    GCM_VALIDATE_RET(add_len == 0 || add != NULL);
    GCM_VALIDATE_RET(length == 0 || input != NULL);
    GCM_VALIDATE_RET(length == 0 || output != NULL);
    GCM_VALIDATE_RET(tag != NULL);

    if ((ret = mbedtls_gcm_starts(ctx, mode, iv, iv_len, add, add_len)) != 0) {
        return ret;
    }

    if ((ret = mbedtls_gcm_update(ctx, length, input, output)) != 0) {
        return ret;
    }

    if ((ret = mbedtls_gcm_finish(ctx, tag, tag_len)) != 0) {
        return ret;
    }

    return 0;
}

int mbedtls_gcm_auth_decrypt(mbedtls_gcm_context *ctx,
                             size_t length,
                             const unsigned char *iv,
                             size_t iv_len,
                             const unsigned char *add,
                             size_t add_len,
                             const unsigned char *tag,
                             size_t tag_len,
                             const unsigned char *input,
                             unsigned char *output)
{
    int ret = MBEDTLS_ERR_ERROR_CORRUPTION_DETECTED;
    unsigned char check_tag[16];
    size_t i;
    int diff;

    GCM_VALIDATE_RET(ctx != NULL);
    GCM_VALIDATE_RET(iv != NULL);
    GCM_VALIDATE_RET(add_len == 0 || add != NULL);
    GCM_VALIDATE_RET(tag != NULL);
    GCM_VALIDATE_RET(length == 0 || input != NULL);
    GCM_VALIDATE_RET(length == 0 || output != NULL);

    if ((ret = mbedtls_gcm_crypt_and_tag(ctx, MBEDTLS_GCM_DECRYPT, length,
                                         iv, iv_len, add, add_len,
                                         input, output, tag_len, check_tag)) != 0) {
        return ret;
    }

    /* Check tag in "constant-time" */
    for (diff = 0, i = 0; i < tag_len; i++) {
        diff |= tag[i] ^ check_tag[i];
    }

    if (diff != 0) {
        mbedtls_platform_zeroize(output, length);
        return MBEDTLS_ERR_GCM_AUTH_FAILED;
    }

    return 0;
}

void mbedtls_gcm_free(mbedtls_gcm_context *ctx)
{
    if (ctx == NULL) {
        return;
    }
    mbedtls_platform_zeroize(ctx, sizeof(mbedtls_gcm_context));
}

#endif /* MBEDTLS_GCM_C */
