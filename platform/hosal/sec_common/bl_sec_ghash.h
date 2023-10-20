#pragma once

#include <stdint.h>
#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct {
    uint32_t :9;
    uint32_t gmacIntClr:1;
    uint32_t gmacIntSet:1;
    uint32_t :5;
    uint32_t gmacMsgLen:16;
    uint32_t gmacSrcAddr;
    uint32_t gmacKey0;
    uint32_t gmacKey1;
    uint32_t gmacKey2;
    uint32_t gmacKey3;
    uint32_t result[4];
} __attribute__((aligned(4))) bl_SEC_Eng_GMAC_Link_Config_Type;

typedef struct {
    uint32_t guard0_[7];
    bl_SEC_Eng_GMAC_Link_Config_Type link_cfg;
    uint32_t dummy_;
    uint8_t tmp_buf[16];
    uint32_t guard1_[7];
} bl_sec_ghash_t;

int bl_sec_ghash_init();

int bl_ghash_init(bl_sec_ghash_t *ctx, const uint8_t key[16]);
int bl_ghash_clear(bl_sec_ghash_t *ctx);
int bl_ghash_acquire_hw();
int bl_ghash_release_hw();
int bl_ghash_update(bl_sec_ghash_t *ctx, const uint8_t *input, size_t input_len);
int bl_ghash_finish(bl_sec_ghash_t *ctx, uint8_t result[16]);
int bl_sec_ghash_test();

#if defined(__cplusplus)
}
#endif
