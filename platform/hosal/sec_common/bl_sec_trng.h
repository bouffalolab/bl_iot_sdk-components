#ifndef BL_SEC_TRNG_H_3VMBGCUF
#define BL_SEC_TRNG_H_3VMBGCUF

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

int bl_sec_trng_init(void);
int bl_rand_stream(uint8_t *buf, int len);
int bl_rand(void);
uint32_t bl_sec_get_random_word(void);

#if defined(__cplusplus)
}
#endif

#endif /* end of include guard: BL_SEC_TRNG_H_3VMBGCUF */
