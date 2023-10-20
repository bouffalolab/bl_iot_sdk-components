#include "bl_sec_hw_common.h"

#ifdef BL616
bool bl_sec_is_cache_addr(const void *addr)
{
    uintptr_t a = (uintptr_t)addr;

    if ((a & 0xFFFF0000UL) >= 0x62FC0000UL) {
        return true;
    }
    return false;
}

void *bl_sec_get_no_cache_addr(const void *addr)
{
    uintptr_t a = (uintptr_t)addr;

    if (!bl_sec_is_cache_addr(addr)) {
        return (void *)addr;
    }

    // OCRAM
    if ((a & 0xF0000000UL) == 0x60000000UL) {
        return (void *)((a & ~0xF0000000UL) | 0x20000000UL);
    }
    // pSRAM
    if ((a & 0xF0000000UL) == 0xA0000000UL) {
        return (void *)((a & ~0xF0000000UL) | 0x10000000UL);
    }

    return NULL;
}
#endif
