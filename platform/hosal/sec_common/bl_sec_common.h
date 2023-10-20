#pragma once

#include <stdint.h>
#include <stddef.h>

#include <FreeRTOS.h>
#include <task.h>

typedef enum {
    BL_SHA1,
    BL_SHA224,
    BL_SHA256,
    BL_MD5,
    BL_SHA384,
    BL_SHA512,
} bl_sha_type_t;

/*
 * SHA1/SHA224/SHA256
 */
/* copied SEC_Eng_SHA256_Ctx from stddrv */
typedef struct {
    uint32_t total[2];
    uint32_t  *shaBuf;
    uint32_t  *shaPadding;
    uint32_t linkAddr;
} bl_SEC_Eng_SHA256_Link_Ctx;

// used for both SHA1/SHA224/SHA256 and SHA512
typedef struct {
    uint32_t shaCfgWord;
    uint32_t shaSrcAddr;                     /*!< Message source address */
    uint32_t result[16];                      /*!< Result of SHA */
} __attribute__ ((aligned(4))) bl_SEC_Eng_SHA_Link_Config_Type;

typedef struct bl_sha_ctx {
    uint32_t guard0_[7];
    bl_sha_type_t type;
    bl_SEC_Eng_SHA256_Link_Ctx ctx;
    bl_SEC_Eng_SHA_Link_Config_Type link_cfg;
    uint32_t tmp[16];
    uint32_t pad[16];
    uint32_t guard1_[7];
} bl_sha_ctx_t;


/*
 * SHA384/512
 */
// copied SEC_Eng_SHA512_Link_Ctx from stddrv */
typedef struct
{
    uint64_t total[2];    /*!< Number of bytes processed */
    uint64_t *shaBuf;     /*!< Data not processed but in this temp buffer */
    uint64_t *shaPadding; /*!< Padding data */
    uint32_t linkAddr;    /*!< Link configure address */
} bl_SEC_Eng_SHA512_Link_Ctx;

typedef struct bl_sha512_ctx {
    uint32_t guard0_[7];
    bl_sha_type_t type;
    bl_SEC_Eng_SHA512_Link_Ctx ctx;
    bl_SEC_Eng_SHA_Link_Config_Type link_cfg;
    uint64_t tmp[16];
    uint64_t pad[16];
    uint32_t guard1_[7];
} bl_sha512_ctx_t;

/*
 * PSK
 */
/// Calculate Wi-Fi PSK
int bl_sec_psk(const char *password, const void *ssid, size_t ssid_len, void *output);
/// Test PSK
int bl_sec_psk_test(void);

static inline unsigned long bl_sec_enter_critical()
{
    taskENTER_CRITICAL();
    return 0;
}

static inline void bl_sec_exit_critical(unsigned long prev_level)
{
    taskEXIT_CRITICAL();
}
