/**
  ******************************************************************************
  * @file    bl702l_romapi_patch.c
  * @version V1.0
  * @date
  * @brief   This file is the standard driver c file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2021 Bouffalo Lab</center></h2>
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

#include "bl702l_romapi_patch.h"
#include "bl702l_romdriver.h"
#include "softcrc.h"


typedef struct
{
    uint32_t jedecID;
    char *name;
    const SPI_Flash_Cfg_Type *cfg;
} Flash_Info_t;

static const ATTR_TCM_CONST_SECTION SPI_Flash_Cfg_Type flashCfg_Winb_16JV = {
    .resetCreadCmd = 0xff,
    .resetCreadCmdSize = 3,
    .mid = 0xef,

    .deBurstWrapCmd = 0x77,
    .deBurstWrapCmdDmyClk = 0x3,
    .deBurstWrapDataMode = SF_CTRL_DATA_4_LINES,
    .deBurstWrapData = 0xF0,

    /*reg*/
    .writeEnableCmd = 0x06,
    .wrEnableIndex = 0x00,
    .wrEnableBit = 0x01,
    .wrEnableReadRegLen = 0x01,

    .qeIndex = 1,
    .qeBit = 0x01,
    .qeWriteRegLen = 0x01, /*Q08BV,Q16DV: 0x02.Q32FW,Q32FV: 0x01 */
    .qeReadRegLen = 0x1,

    .busyIndex = 0,
    .busyBit = 0x00,
    .busyReadRegLen = 0x1,
    .releasePowerDown = 0xab,

    .readRegCmd[0] = 0x05,
    .readRegCmd[1] = 0x35,
    .writeRegCmd[0] = 0x01,
    .writeRegCmd[1] = 0x31,

    .fastReadQioCmd = 0xeb,
    .frQioDmyClk = 16 / 8,
    .cReadSupport = 1,
    .cReadMode = 0xa0,

    .burstWrapCmd = 0x77,
    .burstWrapCmdDmyClk = 0x3,
    .burstWrapDataMode = SF_CTRL_DATA_4_LINES,
    .burstWrapData = 0x40,
    /*erase*/
    .chipEraseCmd = 0xc7,
    .sectorEraseCmd = 0x20,
    .blk32EraseCmd = 0x52,
    .blk64EraseCmd = 0xd8,
    /*write*/
    .pageProgramCmd = 0x02,
    .qpageProgramCmd = 0x32,
    .qppAddrMode = SF_CTRL_ADDR_1_LINE,

    .ioMode = SF_CTRL_QIO_MODE,
    .clkDelay = 1,
    .clkInvert = 0x3d,

    .resetEnCmd = 0x66,
    .resetCmd = 0x99,
    .cRExit = 0xff,
    .wrEnableWriteRegLen = 0x00,

    /*id*/
    .jedecIdCmd = 0x9f,
    .jedecIdCmdDmyClk = 0,
    .qpiJedecIdCmd = 0x9f,
    .qpiJedecIdCmdDmyClk = 0x00,
    .sectorSize = 4,
    .pageSize = 256,

    /*read*/
    .fastReadCmd = 0x0b,
    .frDmyClk = 8 / 8,
    .qpiFastReadCmd = 0x0b,
    .qpiFrDmyClk = 8 / 8,
    .fastReadDoCmd = 0x3b,
    .frDoDmyClk = 8 / 8,
    .fastReadDioCmd = 0xbb,
    .frDioDmyClk = 0,
    .fastReadQoCmd = 0x6b,
    .frQoDmyClk = 8 / 8,

    .qpiFastReadQioCmd = 0xeb,
    .qpiFrQioDmyClk = 16 / 8,
    .qpiPageProgramCmd = 0x02,
    .writeVregEnableCmd = 0x50,

    /* qpi mode */
    .enterQpi = 0x38,
    .exitQpi = 0xff,

    /*AC*/
    .timeEsector = 300,
    .timeE32k = 1200,
    .timeE64k = 1200,
    .timePagePgm = 5,
    .timeCe = 33 * 1000,
    .pdDelay = 3,
    .qeData = 0,
};

static const ATTR_TCM_CONST_SECTION SPI_Flash_Cfg_Type flashCfg_FM_Q80 = {
        .resetCreadCmd = 0xff,
        .resetCreadCmdSize = 3,
        .mid = 0xc8,

        .deBurstWrapCmd = 0x77,
        .deBurstWrapCmdDmyClk = 0x3,
        .deBurstWrapDataMode = SF_CTRL_DATA_4_LINES,
        .deBurstWrapData = 0xF0,

        /*reg*/
        .writeEnableCmd = 0x06,
        .wrEnableIndex = 0x00,
        .wrEnableBit = 0x01,
        .wrEnableReadRegLen = 0x01,

        .qeIndex = 1,
        .qeBit = 0x01,
        .qeWriteRegLen = 0x02,
        .qeReadRegLen = 0x1,

        .busyIndex = 0,
        .busyBit = 0x00,
        .busyReadRegLen = 0x1,
        .releasePowerDown = 0xab,

        .readRegCmd[0] = 0x05,
        .readRegCmd[1] = 0x35,
        .writeRegCmd[0] = 0x01,
        .writeRegCmd[1] = 0x01,

        .fastReadQioCmd = 0xeb,
        .frQioDmyClk = 16 / 8,
        .cReadSupport = 1,
        .cReadMode = 0xA0,

        .burstWrapCmd = 0x77,
        .burstWrapCmdDmyClk = 0x3,
        .burstWrapDataMode = SF_CTRL_DATA_4_LINES,
        .burstWrapData = 0x40,
        /*erase*/
        .chipEraseCmd = 0xc7,
        .sectorEraseCmd = 0x20,
        .blk32EraseCmd = 0x52,
        .blk64EraseCmd = 0xd8,
        /*write*/
        .pageProgramCmd = 0x02,
        .qpageProgramCmd = 0x32,
        .qppAddrMode = SF_CTRL_ADDR_1_LINE,

        .ioMode = SF_CTRL_QIO_MODE,
        .clkDelay = 1,
        .clkInvert = 0x01,

        .resetEnCmd = 0x66,
        .resetCmd = 0x99,
        .cRExit = 0xff,
        .wrEnableWriteRegLen = 0x00,

        /*id*/
        .jedecIdCmd = 0x9f,
        .jedecIdCmdDmyClk = 0,
        .qpiJedecIdCmd = 0x9f,
        .qpiJedecIdCmdDmyClk = 0x00,
        .sectorSize = 4,
        .pageSize = 256,

        /*read*/
        .fastReadCmd = 0x0b,
        .frDmyClk = 8 / 8,
        .qpiFastReadCmd = 0x0b,
        .qpiFrDmyClk = 8 / 8,
        .fastReadDoCmd = 0x3b,
        .frDoDmyClk = 8 / 8,
        .fastReadDioCmd = 0xbb,
        .frDioDmyClk = 0,
        .fastReadQoCmd = 0x6b,
        .frQoDmyClk = 8 / 8,

        .qpiFastReadQioCmd = 0xeb,
        .qpiFrQioDmyClk = 16 / 8,
        .qpiPageProgramCmd = 0x02,
        .writeVregEnableCmd = 0x50,

        /* qpi mode */
        .enterQpi = 0x38,
        .exitQpi = 0xff,

        /*AC*/
        .timeEsector = 300,
        .timeE32k = 1200,
        .timeE64k = 1200,
        .timePagePgm = 5,
        .timeCe = 33000,
        .pdDelay = 20,
        .qeData = 0,
};

static const ATTR_TCM_CONST_SECTION Flash_Info_t flashInfos[] = {
    {
        .jedecID = 0x14650b,
        .name="XTX_25W08F_08_1833",
        .cfg = &flashCfg_Winb_16JV,
    },
    {
        .jedecID = 0x1660c4,
        .name="gt25q32_32_33",
        .cfg = &flashCfg_Winb_16JV,
    },
    {
        .jedecID = 0x152085,
        .name="py25q16hb_16_33",
        .cfg = &flashCfg_FM_Q80,
    },
    {
        .jedecID = 0x162085,
        .name="py25q32hb_32_33",
        .cfg = &flashCfg_FM_Q80,
    },
    {
        .jedecID = 0x166125,
        .name="sk25e032_32_33",
        .cfg = &flashCfg_FM_Q80,
    },
};


//FLASH
/****************************************************************************/ /**
 * @brief  Get flash config according to flash ID patch
 *
 * @param  flashID: Flash ID
 * @param  pFlashCfg: Flash config pointer
 *
 * @return SUCCESS or ERROR
 *
*******************************************************************************/
BL_Err_Type ATTR_TCM_SECTION SF_Cfg_Get_Flash_Cfg_Need_Lock_Ext(uint32_t flashID, SPI_Flash_Cfg_Type *pFlashCfg)
{
    uint32_t i;
    uint8_t buf[sizeof(SPI_Flash_Cfg_Type) + 8];
    uint32_t crc, *pCrc;
    uint32_t xipOffset;
    char flashCfgMagic[] = "FCFG";

    if (flashID == 0) {
        xipOffset = SF_Ctrl_Get_Flash_Image_Offset();
        SF_Ctrl_Set_Flash_Image_Offset(0);
        XIP_SFlash_Read_Via_Cache_Need_Lock(8 + BL702L_FLASH_XIP_BASE, buf, sizeof(SPI_Flash_Cfg_Type) + 8);
        SF_Ctrl_Set_Flash_Image_Offset(xipOffset);

        if (ARCH_MemCmp(buf, flashCfgMagic, 4) == 0) {
            crc = BFLB_Soft_CRC32((uint8_t *)buf + 4, sizeof(SPI_Flash_Cfg_Type));
            pCrc = (uint32_t *)(buf + 4 + sizeof(SPI_Flash_Cfg_Type));

            if (*pCrc == crc) {
                ARCH_MemCpy_Fast(pFlashCfg, (uint8_t *)buf + 4, sizeof(SPI_Flash_Cfg_Type));
                return SUCCESS;
            }
        }
    } else {
        if (RomDriver_SF_Cfg_Get_Flash_Cfg_Need_Lock(flashID, pFlashCfg) == SUCCESS) {
            return SUCCESS;
        }
        for (i = 0; i < sizeof(flashInfos) / sizeof(flashInfos[0]); i++) {
            if (flashInfos[i].jedecID == flashID) {
                ARCH_MemCpy_Fast(pFlashCfg, flashInfos[i].cfg, sizeof(SPI_Flash_Cfg_Type));
                return SUCCESS;
            }
        }
    }

    return ERROR;
}

/****************************************************************************/ /**
 * @brief  Identify one flash patch
 *
 * @param  callFromFlash: code run at flash or ram
 * @param  autoScan: autoscan
 * @param  flashPinCfg: flash GPIO config
 * @param  restoreDefault: Wether restore default flash GPIO config
 * @param  pFlashCfg: Flash config pointer
 *
 * @return Flash ID
 *
*******************************************************************************/
uint32_t ATTR_TCM_SECTION SF_Cfg_Flash_Identify_Ext(uint8_t callFromFlash, uint32_t autoScan,
                  uint32_t flashPinCfg, uint8_t restoreDefault, SPI_Flash_Cfg_Type *pFlashCfg)
{
    uint32_t jdecId = 0;
    uint32_t i = 0;
    uint32_t ret = 0;

    ret = SF_Cfg_Flash_Identify(callFromFlash, autoScan, flashPinCfg, restoreDefault, pFlashCfg);
    if (callFromFlash) {
        SFlash_Set_IDbus_Cfg(pFlashCfg, pFlashCfg->ioMode & 0xf, 1, 0, 32);
    }
    if ((ret & BFLB_FLASH_ID_VALID_FLAG) != 0) {
        return ret;
    }

    jdecId = (ret & 0xffffff);
    for (i = 0; i < sizeof(flashInfos) / sizeof(flashInfos[0]); i++) {
        if (flashInfos[i].jedecID == jdecId) {
            ARCH_MemCpy_Fast(pFlashCfg, flashInfos[i].cfg, sizeof(SPI_Flash_Cfg_Type));
            break;
        }
    }
    if (i == sizeof(flashInfos) / sizeof(flashInfos[0])) {
        return jdecId;
    } else {
        return (jdecId | BFLB_FLASH_ID_VALID_FLAG);
    }
}

/****************************************************************************/ /**
 * @brief  Power on XTAL 32K
 *
 * @param  None
 *
 * @return SUCCESS or ERROR
 *
*******************************************************************************/
BL_Err_Type ATTR_CLOCK_SECTION HBN_Power_On_Xtal_32K(void)
{
    uint32_t tmpVal = 0;

    tmpVal = BL_RD_REG(HBN_BASE, HBN_XTAL32K);
    tmpVal = BL_CLR_REG_BIT(tmpVal, HBN_XTAL32K_HIZ_EN);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, HBN_XTAL32K_INV_STRE, 3);
    tmpVal = BL_CLR_REG_BIT(tmpVal, HBN_XTAL32K_AC_CAP_SHORT);
    tmpVal = BL_SET_REG_BIT(tmpVal, HBN_PU_XTAL32K);
    tmpVal = BL_SET_REG_BIT(tmpVal, HBN_PU_XTAL32K_BUF);
    BL_WR_REG(HBN_BASE, HBN_XTAL32K, tmpVal);

    /* Delay >1s, but 1s is too long, so user should delay after this function */
    arch_delay_us(1100);

    return SUCCESS;
}
