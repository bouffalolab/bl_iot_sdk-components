#ifndef BL_SEC_SHA
#define BL_SEC_SHA
#include "bl_sec_common.h"

int bl_sec_sha_init();
int bl_sha_init(bl_sha_ctx_t *ctx, const bl_sha_type_t type);
int bl_sha_update(bl_sha_ctx_t *ctx, const uint8_t *input, uint32_t len);
int bl_sha_finish(bl_sha_ctx_t *ctx, uint8_t *hash);
#endif

