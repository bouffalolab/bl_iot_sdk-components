#ifndef __BL_SYS_H__
#define __BL_SYS_H__

#include <stdint.h>
#include "clic.h"
#include "riscv_encoding.h"

typedef enum {
    BL_RST_POR      = 0,
    BL_RST_BOR      = 1,
    BL_RST_WDT      = 2,
    BL_RST_HBN      = 3,
    BL_RST_SOFTWARE = 4,
} BL_RST_REASON_E;

void bl_sys_rstinfo_process(void);
BL_RST_REASON_E bl_sys_rstinfo_get(void);
void bl_sys_rstinfo_clr(void);
int bl_sys_logall_enable(void);
int bl_sys_logall_disable(void);
int bl_sys_reset_por(void);
void bl_sys_reset_system(void);
int bl_sys_isxipaddr(uint32_t addr);
int bl_sys_em_config(void);
int bl_sys_cache_config(void);
int bl_sys_pkg_config(void);
int bl_sys_default_active_config(void);
int bl_sys_early_init(void);
int bl_sys_init(void);
int bl_sys_wdt_rst_count_get();
void bl_function_call_with_stack(void (*f)(void *data), void *data, void *stacktop);

#endif
