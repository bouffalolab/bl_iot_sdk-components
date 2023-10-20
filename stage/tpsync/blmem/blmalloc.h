#ifndef __MALLOC_TEST_H__
#define __MALLOC_TEST_H__

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ram_type {
    DTCM,
    OCRAM,
    WRAM,
} ram_type_t;

void *blmalloc(size_t size, ram_type_t type);
void blfree(void *ptr);

#ifdef __cplusplus
}
#endif

#endif
