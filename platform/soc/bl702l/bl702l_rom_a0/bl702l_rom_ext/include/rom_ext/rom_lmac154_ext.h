#ifndef __ROM_LMAC154_EXT_H__
#define __ROM_LMAC154_EXT_H__

#include "lmac154.h"
#include "zb_timer.h"

// Public Functions
void rom_lmac154_hook_init(void);

/****************************************************************************//**
 * @brief  Set tx-rx transition time
 *
 * @param  time: us
 *
 * @return None
 *
*******************************************************************************/
void lmac154_setTxRxTransTime(uint8_t timeInUs);
void lmac154_setFramePendingSourceMatch(bool isEnable);

#endif
