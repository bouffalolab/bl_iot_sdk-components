#ifndef __BL_PDS_H__
#define __BL_PDS_H__


#include "bl702l_glb.h"
#include "bl702l_sflash.h"
#include "bl702l_romdriver.h"


#define ATTR_NOINLINE              __attribute__((noinline))
#define ATTR_PDS_SECTION           __attribute__((section(".pds_code." ATTR_UNI_SYMBOL)))


void bl_pds_init(void);
void bl_pds_fastboot_cfg(uint32_t addr);
int bl_pds_enter(uint32_t pdsLevel, uint32_t pdsSleepCycles);


#endif
