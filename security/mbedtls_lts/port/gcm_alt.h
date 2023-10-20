#ifndef MBEDTLS_GCM_ALT_H
#define MBEDTLS_GCM_ALT_H

#include <stdbool.h>
#include <bl_sec.h>

#define MBEDTLS_GCM_ALT_TJ_STEP_DIVISOR 10

struct mbedtls_gcm_alt_tj_ctx;
typedef void (*mbedtls_gcm_alt_submit_step_fn)(struct mbedtls_gcm_alt_tj_ctx *ctx, size_t idx);
typedef bool (*mbedtls_gcm_alt_check_step_fn)(struct mbedtls_gcm_alt_tj_ctx *ctx, size_t idx);

struct mbedtls_gcm_alt_tj_ctx {
    void *ctx[2];
    uint8_t *ctr;
    size_t tot_steps;
    size_t step_len;
    size_t last_step_len;
    const uint8_t *input[2];
    uint8_t *output[2];
    size_t steps[2];
    int state[2];
    size_t ticks[2];
    mbedtls_gcm_alt_submit_step_fn submit[2];
    mbedtls_gcm_alt_check_step_fn check[2];
};

/**
 * \brief          The GCM context structure.
 */
typedef struct mbedtls_gcm_context
{
    uint64_t len;                         /*!< The total length of the encrypted data. */
    uint64_t add_len;                     /*!< The total length of the additional data. */
    unsigned char base_ectr[16];          /*!< The first ECTR for tag. */
    unsigned char y[16];                  /*!< The Y working value. */
    int mode;                             /*!< The operation to perform:
                                               #MBEDTLS_GCM_ENCRYPT or
                                               #MBEDTLS_GCM_DECRYPT. */
    bl_sec_aes_t aes;
    bl_sec_ghash_t ghash;
    struct mbedtls_gcm_alt_tj_ctx tj;
}
mbedtls_gcm_context;

#endif /* gcm_alt.h */
