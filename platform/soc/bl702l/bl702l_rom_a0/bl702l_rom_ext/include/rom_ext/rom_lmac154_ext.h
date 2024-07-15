#ifndef __ROM_LMAC154_EXT_H__
#define __ROM_LMAC154_EXT_H__

#include "lmac154.h"
#include "zb_timer.h"

// Public Functions
void rom_lmac154_hook_init(void);

typedef enum {
    LMAC154_FPT_STATUS_SUCCESS          = 0,
    LMAC154_FPT_STATUS_NO_RESOURCE      = -1,
    LMAC154_FPT_STATUS_ADDR_NOT_FOUND   = -2,
    LMAC154_FPT_STATUS_EMPTY            = -3,
    LMAC154_FPT_STATUS_INVALID_PARAM    = -4,
    LMAC154_FPT_STATUS_INVALID_OPT      = -5
}lmac154_fpt_status_t;

// hardware frame pending table (256 bytes)
// shared by short addresses and long addresses (supports up to 128 short addresses or 32 long addresses or mixed)
// used by hardware to set the frame pending bit of hw auto tx ack
// if the address is not found in the table (e.g. not set or removed), the frame pending bit of hw auto tx ack is 1


/****************************************************************************//**
 * @brief  Add (or update) the key-value pair {sadr: pending} to (in) the frame pending table
 *
 * @param  sadr: 16-bit short address
 * @param  pending: value of the corresponding frame pending bit
 *
 * @return LMAC154_FPT_STATUS_SUCCESS or LMAC154_FPT_STATUS_NO_RESOURCE
 *
*******************************************************************************/
lmac154_fpt_status_t lmac154_fptSetShortAddrPending(uint16_t sadr, uint8_t pending);


/****************************************************************************//**
 * @brief  Add (or update) the key-value pair {ladr: pending} to (in) the frame pending table
 *
 * @param  ladr: pointer to 64-bit long address
 * @param  pending: value of the corresponding frame pending bit
 *
 * @return LMAC154_FPT_STATUS_SUCCESS or LMAC154_FPT_STATUS_NO_RESOURCE
 *
*******************************************************************************/
lmac154_fpt_status_t lmac154_fptSetLongAddrPending(uint8_t *ladr, uint8_t pending);


/****************************************************************************//**
 * @brief  Get the corresponding frame pending bit of the short address in the frame pending table
 *
 * @param  sadr: 16-bit short address
 * @param  pending: value of the corresponding frame pending bit
 *
 * @return LMAC154_FPT_STATUS_SUCCESS or LMAC154_FPT_STATUS_ADDR_NOT_FOUND
 *
*******************************************************************************/
lmac154_fpt_status_t lmac154_fptGetShortAddrPending(uint16_t sadr, uint8_t *pending);


/****************************************************************************//**
 * @brief  Get the corresponding frame pending bit of the long address in the frame pending table
 *
 * @param  ladr: pointer to 64-bit long address
 * @param  pending: value of the corresponding frame pending bit
 *
 * @return LMAC154_FPT_STATUS_SUCCESS or LMAC154_FPT_STATUS_ADDR_NOT_FOUND
 *
*******************************************************************************/
lmac154_fpt_status_t lmac154_fptGetLongAddrPending(uint8_t *ladr, uint8_t *pending);


/****************************************************************************//**
 * @brief  Remove the key-value pair {sadr: pending} from the frame pending table
 *
 * @param  sadr: 16-bit short address
 *
 * @return LMAC154_FPT_STATUS_SUCCESS or LMAC154_FPT_STATUS_ADDR_NOT_FOUND
 *
*******************************************************************************/
lmac154_fpt_status_t lmac154_fptRemoveShortAddr(uint16_t sadr);


/****************************************************************************//**
 * @brief  Remove the key-value pair {ladr: pending} from the frame pending table
 *
 * @param  ladr: pointer to 64-bit long address
 *
 * @return LMAC154_FPT_STATUS_SUCCESS or LMAC154_FPT_STATUS_ADDR_NOT_FOUND
 *
*******************************************************************************/
lmac154_fpt_status_t lmac154_fptRemoveLongAddr(uint8_t *ladr);


/****************************************************************************//**
 * @brief  Get short address list in the frame pending table
 *
 * @param  list: pointer to list buffer
 * @param  entry_num: number of entries
 *
 * @return None
 *
*******************************************************************************/
void lmac154_fpt_GetShortAddrList(void *list, uint8_t *entry_num);


/****************************************************************************//**
 * @brief  Get long address list in the frame pending table
 *
 * @param  list: pointer to list buffer
 * @param  entry_num: number of entries
 *
 * @return None
 *
*******************************************************************************/
void lmac154_fptGetLongAddrList(void *list, uint8_t *entry_num);


/****************************************************************************//**
 * @brief  Remove all Short Address from pending table
 *
  * @return None
 *
*******************************************************************************/
void lmac154_fptRemoveAllShortAddr(void);


/****************************************************************************//**
 * @brief  Remove all Extended Address from pending table
 *
  * @return None
 *
*******************************************************************************/
void lmac154_fptRemoveAllLongAddr(void);


/****************************************************************************//**
 * @brief  Set frame pending bit according to frame pending table, or force to set to 1
 *
 * @param  force: 0: set according to frame pending table (default), 1: force to set to 1
 *
 * @return None
 *
*******************************************************************************/
void lmac154_fptForcePending(uint8_t force);


/****************************************************************************//**
 * @brief  Clear the frame pending table
 *
 * @param  None
 *
 * @return None
 *
*******************************************************************************/
void lmac154_fptClear(void);


/****************************************************************************//**
 * @brief  Print the frame pending table using specified print function
 *
 * @param  print_func: user specified print function
 *
 * @return None
 *
*******************************************************************************/
void lmac154_fptDump(int print_func(const char *fmt, ...));


/****************************************************************************//**
 * @brief  Set tx-rx transition time
 *
 * @param  time: us
 *
 * @return None
 *
*******************************************************************************/
void lmac154_setTxRxTransTime(uint8_t timeInUs);


#endif
