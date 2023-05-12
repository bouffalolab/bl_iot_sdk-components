/**
  ******************************************************************************
  * @file    bl702l_ef_cfg.c
  * @version V1.0
  * @date
  * @brief   This file is the standard driver c file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2020 Bouffalo Lab</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of Bouffalo Lab nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

#include "string.h"
#include "bl702l_ef_ctrl.h"
#include "bl702l_ef_cfg.h"
#include "ef_data_0_reg.h"

/** @addtogroup  BL702L_Peripheral_Driver
 *  @{
 */

/** @addtogroup  SEC_EF_CFG
 *  @{
 */

/** @defgroup  SEC_EF_CFG_Private_Macros
 *  @{
 */
#ifndef BOOTROM
#define EF_CTRL_LOAD_BEFORE_READ_R0 EF_Ctrl_Load_Efuse_R0()
#else
#define EF_CTRL_LOAD_BEFORE_READ_R0
#endif

/*@} end of group SEC_EF_CFG_Private_Macros */

/** @defgroup  SEC_EF_CFG_Private_Types
 *  @{
 */

/*@} end of group SEC_EF_CFG_Private_Types */

/** @defgroup  SEC_EF_CFG_Private_Variables
 *  @{
 */
static const Efuse_Common_Trim_Cfg trim_lit[] = {
    {
        .name = "rc32m",
        .en_addr = 0x0C * 8 + 19,
        .parity_addr = 0x0C * 8 + 18,
        .value_addr = 0x0C * 8 + 10,
        .value_len = 8,
    },
    {
        .name = "rc32k",
        .en_addr = 0x0C * 8 + 25,
        .parity_addr = 0x0C * 8 + 24,
        .value_addr = 0x0C * 8 + 20,
        .value_len = 4,
    },
    {
        .name = "ldo14_trim",
        .en_addr = 0x0C * 8 + 31,
        .parity_addr = 0x0C * 8 + 30,
        .value_addr = 0x0C * 8 + 26,
        .value_len = 4,
    },
    {
        .name = "xtal",
        .en_addr = 0x58 * 8 + 9,
        .parity_addr = 0x58 * 8 + 8,
        .value_addr = 0x58 * 8 + 0,
        .value_len = 8,
    },
    {
        .name = "ldo11_trim",
        .en_addr = 0x78 * 8 + 20,
        .parity_addr = 0x78 * 8 + 19,
        .value_addr = 0x78 * 8 + 15,
        .value_len = 4,
    },
    {
        .name = "gpadc_gain",
        .en_addr = 0x78 * 8 + 14,
        .parity_addr = 0x78 * 8 + 13,
        .value_addr = 0x78 * 8 + 1,
        .value_len = 12,
    },
    {
        .name = "tsen",
        .en_addr = 0x78 * 8 + 0,
        .parity_addr = 0x7C * 8 + 12,
        .value_addr = 0x7C * 8 + 0,
        .value_len = 12,
    },
};
/*@} end of group SEC_EF_CFG_Private_Variables */

/** @defgroup  SEC_EF_CFG_Global_Variables
 *  @{
 */

/*@} end of group SEC_EF_CFG_Global_Variables */

/** @defgroup  SEC_EF_CFG_Private_Fun_Declaration
 *  @{
 */

/*@} end of group SEC_EF_CFG_Private_Fun_Declaration */

/** @defgroup  SEC_EF_CFG_Private_Functions
 *  @{
 */

/****************************************************************************/ /**
 * @brief  Analog Trim parity calculate
 *
 * @param  val: Value of efuse trim data
 * @param  len: Length of bit to calculate
 *
 * @return Parity bit value
 *
*******************************************************************************/
#ifndef BFLB_USE_ROM_DRIVER
__WEAK
uint8_t ATTR_CLOCK_SECTION EF_Ctrl_Get_Trim_Parity(uint32_t val, uint8_t len)
{
    uint8_t cnt = 0;
    uint8_t i = 0;

    for (i = 0; i < len; i++) {
        if (val & (1 << i)) {
            cnt++;
        }
    }

    return cnt & 0x01;
}
#endif

/****************************************************************************/ /**
 * @brief  Efuse get trim list
 *
 * @param  trim_list: Trim list pointer
 *
 * @return Trim list count
 *
*******************************************************************************/
uint32_t EF_Ctrl_Get_Common_Trim_List(const Efuse_Common_Trim_Cfg **trim_list)
{
    *trim_list = &trim_lit[0];
    return sizeof(trim_lit) / sizeof(trim_lit[0]);
}

/****************************************************************************/ /**
 * @brief  Efuse read common trim value
 *
 * @param  trim: Trim data pointer
 *
 * @return None
 *
*******************************************************************************/
void EF_Ctrl_Read_Common_Trim(char *name, Efuse_Common_Trim_Type *trim)
{
    uint32_t tmpVal;
    uint32_t i = 0;

    /* Trigger read data from efuse */
    EF_CTRL_LOAD_BEFORE_READ_R0;

    trim->en = 0;
    trim->parity = 0;
    trim->empty = 1;
    trim->len = 0;

    for (i = 0; i < sizeof(trim_lit) / sizeof(trim_lit[0]); i++) {
        if (memcmp(name, trim_lit[i].name, strlen(name)) == 0) {
            trim->len = trim_lit[i].value_len;
            tmpVal = BL_RD_WORD(EF_DATA_BASE + (trim_lit[i].en_addr / 32) * 4);
            if (tmpVal & (1 << (trim_lit[i].en_addr % 32))) {
                trim->en = 1;
            }
            tmpVal = BL_RD_WORD(EF_DATA_BASE + (trim_lit[i].parity_addr / 32) * 4);
            if (tmpVal & (1 << (trim_lit[i].parity_addr % 32))) {
                trim->parity = 1;
            }

            if (((trim_lit[i].value_addr % 32) + trim_lit[i].value_len) > 32) {
                uint64_t tmpVal64 = (uint64_t)BL_RD_WORD(EF_DATA_BASE + (trim_lit[i].value_addr / 32) * 4);
                tmpVal64 |= (((uint64_t)BL_RD_WORD(EF_DATA_BASE + (trim_lit[i].value_addr / 32) * 4 + 4)) << 32);
                tmpVal64 = tmpVal64 >> (trim_lit[i].value_addr % 32);
                trim->value = (uint32_t)(tmpVal64 & (((uint64_t)1 << trim_lit[i].value_len) - 1));
            } else {
                tmpVal = BL_RD_WORD(EF_DATA_BASE + (trim_lit[i].value_addr / 32) * 4);
                tmpVal = tmpVal >> (trim_lit[i].value_addr % 32);
                trim->value = tmpVal & (((uint64_t)1 << trim_lit[i].value_len) - 1);
            }

            if (trim->en == 0 && trim->parity == 0 && trim->value == 0) {
                trim->empty = 1;
            } else {
                trim->empty = 0;
            }
        }
    }
}

/****************************************************************************/ /**
 * @brief  Efuse read common trim value
 *
 * @param  trim: Trim data pointer
 *
 * @return None
 *
*******************************************************************************/
void EF_Ctrl_Write_Common_Trim(char *name, uint8_t trim_en, uint32_t trim_value)
{
    uint32_t tmpVal = 0;
    uint32_t i = 0;
    uint8_t parity = 0;
    uint8_t len = 0;

    if (trim_en == 0 && trim_value == 0) {
        return;
    } else {
    }

    /* switch to AHB clock */
    EF_Ctrl_Sw_AHB_Clk_0();

    /* Trigger read data from efuse */
    EF_CTRL_LOAD_BEFORE_READ_R0;

    for (i = 0; i < sizeof(trim_lit) / sizeof(trim_lit[0]); i++) {
        if (memcmp(name, trim_lit[i].name, strlen(name)) == 0) {
            len = trim_lit[i].value_len;

            if (trim_value > (((uint64_t)1 << len) - 1)) {
                return;
            } else {
            }

            if (trim_en == 1) {
                tmpVal = BL_RD_WORD(EF_DATA_BASE + (trim_lit[i].en_addr / 32) * 4);
                tmpVal |= (1 << (trim_lit[i].en_addr % 32));
                BL_WR_WORD((EF_DATA_BASE + (trim_lit[i].en_addr / 32) * 4), tmpVal);
            }

            /* calculate parity of trim_value */
            parity = EF_Ctrl_Get_Trim_Parity(trim_value, len);
            if (parity == 1) {
                tmpVal = BL_RD_WORD(EF_DATA_BASE + (trim_lit[i].parity_addr / 32) * 4);
                tmpVal |= (1 << (trim_lit[i].parity_addr % 32));
                BL_WR_WORD((EF_DATA_BASE + (trim_lit[i].parity_addr / 32) * 4), tmpVal);
            }

            if (trim_value != 0) {
                if (((trim_lit[i].value_addr % 32) + trim_lit[i].value_len) > 32) {
                    tmpVal = BL_RD_WORD(EF_DATA_BASE + (trim_lit[i].value_addr / 32) * 4);
                    tmpVal |= (trim_value << (trim_lit[i].value_addr % 32));
                    BL_WR_WORD((EF_DATA_BASE + (trim_lit[i].value_addr / 32) * 4), tmpVal);

                    tmpVal = BL_RD_WORD(EF_DATA_BASE + (trim_lit[i].value_addr / 32) * 4) + 4;
                    tmpVal |= (trim_value >> (32 - (trim_lit[i].value_addr % 32)));
                    BL_WR_WORD((EF_DATA_BASE + (trim_lit[i].value_addr / 32) * 4 + 4), tmpVal);
                } else {
                    tmpVal = BL_RD_WORD(EF_DATA_BASE + (trim_lit[i].value_addr / 32) * 4);
                    tmpVal |= (trim_value << (trim_lit[i].value_addr % 32));
                    BL_WR_WORD((EF_DATA_BASE + (trim_lit[i].value_addr / 32) * 4), tmpVal);
                }
            }

            EF_Ctrl_Program_Efuse_0();
        }
    }
}

/****************************************************************************/ /**
 * @brief  Whether MAC address slot is empty
 *
 * @param  slot: MAC address slot
 * @param  reload: whether  reload to check
 *
 * @return 0 for all slots full,1 for others
 *
*******************************************************************************/
uint8_t EF_Ctrl_Is_MAC_Address_Slot_Empty(uint8_t slot, uint8_t reload)
{
    uint32_t tmp1 = 0xffffffff, tmp2 = 0xffffffff;
    uint32_t part1Empty = 0, part2Empty = 0;

    if (slot == 0) {
        /* Switch to AHB clock */
        EF_Ctrl_Sw_AHB_Clk_0();

        if (reload) {
            EF_CTRL_LOAD_BEFORE_READ_R0;
        }

        tmp1 = BL_RD_REG(EF_DATA_BASE, EF_DATA_0_EF_WIFI_MAC_LOW);
        tmp2 = BL_RD_REG(EF_DATA_BASE, EF_DATA_0_EF_WIFI_MAC_HIGH);
    } else if (slot == 1) {
        /* Switch to AHB clock */
        EF_Ctrl_Sw_AHB_Clk_0();

        if (reload) {
            EF_CTRL_LOAD_BEFORE_READ_R0;
        }

        tmp1 = BL_RD_REG(EF_DATA_BASE, EF_DATA_0_EF_KEY_SLOT_5_W0);
        tmp2 = BL_RD_REG(EF_DATA_BASE, EF_DATA_0_EF_KEY_SLOT_5_W1);
    } else if (slot == 2) {
        /* Switch to AHB clock */
        EF_Ctrl_Sw_AHB_Clk_0();

        if (reload) {
            EF_CTRL_LOAD_BEFORE_READ_R0;
        }

        tmp1 = BL_RD_REG(EF_DATA_BASE, EF_DATA_0_EF_DBG_PWD_LOW);
        tmp2 = BL_RD_REG(EF_DATA_BASE, EF_DATA_0_EF_DBG_PWD_HIGH);
    }

    part1Empty = (EF_Ctrl_Is_All_Bits_Zero(tmp1, 0, 32));
    part2Empty = (EF_Ctrl_Is_All_Bits_Zero(tmp2, 0, 22));

    return (part1Empty && part2Empty);
}

/****************************************************************************/ /**
 * @brief  Efuse write optional MAC address
 *
 * @param  slot: MAC address slot
 * @param  mac[8]: MAC address buffer
 * @param  program: Whether program
 *
 * @return SUCCESS or ERROR
 *
*******************************************************************************/
BL_Err_Type EF_Ctrl_Write_MAC_Address_Opt(uint8_t slot, uint8_t mac[8], uint8_t program)
{
    uint8_t *maclow = (uint8_t *)mac;
    uint8_t *machigh = (uint8_t *)(mac + 4);
    uint32_t tmpVal = 0;
    uint32_t i = 0;
    uint32_t cnt = 0;

    if (slot >= 3) {
        return ERROR;
    }

    /* Switch to AHB clock */
    EF_Ctrl_Sw_AHB_Clk_0();

    /* The low 32 bits */
    if (slot == 0) {
        BL_WR_REG(EF_DATA_BASE, EF_DATA_0_EF_WIFI_MAC_LOW, BL_RDWD_FRM_BYTEP(maclow));
    } else if (slot == 1) {
        BL_WR_REG(EF_DATA_BASE, EF_DATA_0_EF_KEY_SLOT_5_W0, BL_RDWD_FRM_BYTEP(maclow));
    } else if (slot == 2) {
        BL_WR_REG(EF_DATA_BASE, EF_DATA_0_EF_DBG_PWD_LOW, BL_RDWD_FRM_BYTEP(maclow));
    }

    /* The high 32 bits */
    if (slot == 0) {
        BL_WR_REG(EF_DATA_BASE, EF_DATA_0_EF_WIFI_MAC_HIGH, BL_RDWD_FRM_BYTEP(machigh));
    } else if (slot == 1) {
        BL_WR_REG(EF_DATA_BASE, EF_DATA_0_EF_KEY_SLOT_5_W1, BL_RDWD_FRM_BYTEP(machigh));
    } else if (slot == 2) {
        BL_WR_REG(EF_DATA_BASE, EF_DATA_0_EF_DBG_PWD_HIGH, BL_RDWD_FRM_BYTEP(machigh));
    }

    cnt = 0;
    for (i = 0; i < 8; i++) {
        cnt += EF_Ctrl_Get_Byte_Zero_Cnt(mac[i]);
    }
    cnt &= 0x3f;
    tmpVal = BL_RD_REG(EF_DATA_BASE, EF_DATA_0_EF_KEY_SLOT_5_W2);
    if (slot == 0) {
        tmpVal |= (cnt << 0);
    } else if (slot == 1) {
        tmpVal |= (cnt << 6);
    } else if (slot == 2) {
        tmpVal |= (cnt << 12);
    }
    BL_WR_REG(EF_DATA_BASE, EF_DATA_0_EF_KEY_SLOT_5_W2, tmpVal);

    if (program) {
        if (slot == 2) {
            EF_Ctrl_Program_Efuse_0();
        } else {
            EF_Ctrl_Program_Efuse_0();
        }
    }
    return SUCCESS;
}

/****************************************************************************/ /**
 * @brief  Efuse read optional MAC address
 *
 * @param  slot: MAC address slot
 * @param  mac[8]: MAC address buffer
 * @param  reload: Whether reload
 *
 * @return SUCCESS or ERROR
 *
*******************************************************************************/
BL_Err_Type EF_Ctrl_Read_MAC_Address_Opt(uint8_t slot, uint8_t mac[8], uint8_t reload)
{
    uint8_t *maclow = (uint8_t *)mac;
    uint8_t *machigh = (uint8_t *)(mac + 4);
    uint32_t tmpVal = 0;
    uint32_t i = 0;
    uint32_t cnt = 0;
    uint32_t crc = 0;

    if (slot >= 3) {
        return ERROR;
    }

    /* Trigger read data from efuse */
    if (reload) {
        EF_CTRL_LOAD_BEFORE_READ_R0;
    }

    if (slot == 0) {
        tmpVal = BL_RD_REG(EF_DATA_BASE, EF_DATA_0_EF_WIFI_MAC_LOW);
    } else if (slot == 1) {
        tmpVal = BL_RD_REG(EF_DATA_BASE, EF_DATA_0_EF_KEY_SLOT_5_W0);
    } else if (slot == 2) {
        tmpVal = BL_RD_REG(EF_DATA_BASE, EF_DATA_0_EF_DBG_PWD_LOW);
    }
    BL_WRWD_TO_BYTEP(maclow, tmpVal);

    if (slot == 0) {
        tmpVal = BL_RD_REG(EF_DATA_BASE, EF_DATA_0_EF_WIFI_MAC_HIGH);
    } else if (slot == 1) {
        tmpVal = BL_RD_REG(EF_DATA_BASE, EF_DATA_0_EF_KEY_SLOT_5_W1);
    } else if (slot == 2) {
        tmpVal = BL_RD_REG(EF_DATA_BASE, EF_DATA_0_EF_DBG_PWD_HIGH);
    }
    BL_WRWD_TO_BYTEP(machigh, tmpVal);

    /* Get original parity */
    tmpVal = BL_RD_REG(EF_DATA_BASE, EF_DATA_0_EF_KEY_SLOT_5_W2);
    if (slot == 0) {
        crc = ((tmpVal >> 0) & 0x3f);
    } else if (slot == 1) {
        crc = ((tmpVal >> 6) & 0x3f);
    } else if (slot == 2) {
        crc = ((tmpVal >> 12) & 0x3f);
    }

    /* Check parity */
    for (i = 0; i < 8; i++) {
        cnt += EF_Ctrl_Get_Byte_Zero_Cnt(mac[i]);
    }
    if ((cnt & 0x3f) == crc) {
        return SUCCESS;
    } else {
        return ERROR;
    }
}

/*@} end of group SEC_EF_CFG_Public_Functions */

/*@} end of group SEC_EF_CFG */

/*@} end of group BL702L_Peripheral_Driver */
