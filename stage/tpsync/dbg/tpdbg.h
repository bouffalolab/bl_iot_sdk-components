#ifndef __TPDBG_H__
#define __TPDBG_H__ 

#include <stdint.h>
#include "blog.h"

#if 1
#define tpdbg_log(M, ...) blog_info(M, ##__VA_ARGS__)
void tpdbg_buf(const char *str, void *buf, uint32_t len);
#else
#define tpdbg_log(...)
#define tpdbg_buf(...)
#endif

#endif

