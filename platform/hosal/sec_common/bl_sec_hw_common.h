#pragma once

#include <stdbool.h>

#ifdef BL602
#include <bl602_glb.h>
#include <bl602_sec_eng.h>
#elif defined BL616
#include <bl616_glb.h>
#include <bl616_sec_eng.h>
#define SHA512_CAPABLE
#elif defined BL702
#include <bl702_glb.h>
#include <bl702_sec_eng.h>
#elif defined BL702L
#include <bl702l_glb.h>
#include <bl702l_sec_eng.h>
#elif defined BL808
#include <bl808_glb.h>
#include <bl808_sec_eng.h>
#define SHA512_CAPABLE
#define MD5_CAPABLE
#elif defined BL606P
#include <bl606p_glb.h>
#include <bl606p_sec_eng.h>
#define SHA512_CAPABLE
#define MD5_CAPABLE
#else
#warn "Chip not ported"
#endif

bool bl_sec_is_cache_addr(const void *addr);
void *bl_sec_get_no_cache_addr(const void *addr);
