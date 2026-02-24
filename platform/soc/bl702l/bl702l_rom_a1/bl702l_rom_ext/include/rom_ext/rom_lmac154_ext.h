#ifndef __ROM_LMAC154_EXT_H__
#define __ROM_LMAC154_EXT_H__

#include "lmac154.h"
#include "zb_timer.h"

// Public Functions
void rom_lmac154_hook_init(void);

lmac154_isr_t lmac154_getPatchedInterruptHandler(void);
void lmac154_setFramePendingSourceMatch(bool isEnable);

#endif
