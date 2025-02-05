/**
  ******************************************************************************
  * @file    bl702l_sf_cfg.c
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

#include "bl702l_glb.h"
#include "bl702l_glb_gpio.h"
#include "bl702l_sf_cfg.h"
#include "bl702l_sf_ctrl.h"
#include "softcrc.h"
#include "bl702l_xip_sflash.h"

/** @addtogroup  BL702L_Peripheral_Driver
 *  @{
 */

/** @addtogroup  SF_CFG
 *  @{
 */

/** @defgroup  SF_CFG_Private_Macros
 *  @{
 */
#define BFLB_FLASH_CFG_MAGIC "FCFG"

/*@} end of group SF_CFG_Private_Macros */

/** @defgroup  SF_CFG_Private_Types
 *  @{
 */
#ifndef BFLB_USE_ROM_DRIVER
typedef struct
{
    uint32_t jedecID;
    char *name;
    const SPI_Flash_Cfg_Type *cfg;
} Flash_Info_t;
#endif

/*@} end of group SF_CFG_Private_Types */

/** @defgroup  SF_CFG_Private_Variables
 *  @{
 */
#ifndef BFLB_USE_ROM_DRIVER
static const ATTR_TCM_CONST_SECTION SPI_Flash_Cfg_Type flashCfg_Winb_80DV = {
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
    .cReadSupport = 0,
    .cReadMode = 0xFF,

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
    .timeCe = 20 * 1000,
    .pdDelay = 3,
    .qeData = 0,
};

static const ATTR_TCM_CONST_SECTION SPI_Flash_Cfg_Type flashCfg_Gd_Md_40D={
    .resetCreadCmd = 0xff,
    .resetCreadCmdSize = 3,
    .mid = 0x51,

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
    .readRegCmd[1] = 0x00,
    .writeRegCmd[0] = 0x01,
    .writeRegCmd[1] = 0x00,

    .fastReadQioCmd = 0xeb,
    .frQioDmyClk = 16/8,
    .cReadSupport = 0,
    .cReadMode = 0xff,

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

    .ioMode = 0x11,
    .clkDelay = 1,
    .clkInvert = 0x3f,

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
    .frDmyClk = 8/8,
    .qpiFastReadCmd = 0x0b,
    .qpiFrDmyClk = 8/8,
    .fastReadDoCmd = 0x3b,
    .frDoDmyClk = 8/8,
    .fastReadDioCmd = 0xbb,
    .frDioDmyClk = 0,
    .fastReadQoCmd = 0x6b,
    .frQoDmyClk = 8/8,

    .qpiFastReadQioCmd = 0xeb,
    .qpiFrQioDmyClk = 16/8,
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
    .timeCe = 20*1000,
    .pdDelay = 20,
    .qeData = 0,
};

static const ATTR_TCM_CONST_SECTION SPI_Flash_Cfg_Type flashCfg_Gd_Q80E_Q16E = {
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
    .clkInvert = 0x3f,

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

static const ATTR_TCM_CONST_SECTION SPI_Flash_Cfg_Type flashCfg_Gd_WQ80E_WQ16E = {
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
    .frQioDmyClk = 32 / 8,
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
    .clkInvert = 0x3f,

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
    .frDioDmyClk = 8 / 8,
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
    .timeCe = 20 * 1000,
    .pdDelay = 20,
    .qeData = 0x12,
};

static const ATTR_TCM_CONST_SECTION SPI_Flash_Cfg_Type flashCfg_Mxic = {
    .resetCreadCmd = 0xff,
    .resetCreadCmdSize = 3,
    .mid = 0xC2,

    .deBurstWrapCmd = 0xC0,
    .deBurstWrapCmdDmyClk = 0x00,
    .deBurstWrapDataMode = SF_CTRL_DATA_1_LINE,
    .deBurstWrapData = 0x10,

    /*reg*/
    .writeEnableCmd = 0x06,
    .wrEnableIndex = 0x00,
    .wrEnableBit = 0x01,
    .wrEnableReadRegLen = 0x01,

    .qeIndex = 0,
    .qeBit = 0x06,
    .qeWriteRegLen = 0x02,
    .qeReadRegLen = 0x1,

    .busyIndex = 0,
    .busyBit = 0x00,
    .busyReadRegLen = 0x1,
    .releasePowerDown = 0xab,

    .readRegCmd[0] = 0x05,
    .readRegCmd[1] = 0x00,
    .writeRegCmd[0] = 0x01,
    .writeRegCmd[1] = 0x00,

    .fastReadQioCmd = 0xeb,
    .frQioDmyClk = 16 / 8,
    .cReadSupport = 1,
    .cReadMode = 0xA5,

    .burstWrapCmd = 0xC0,
    .burstWrapCmdDmyClk = 0x00,
    .burstWrapDataMode = SF_CTRL_DATA_1_LINE,
    .burstWrapData = 0x02,
    /*erase*/
    .chipEraseCmd = 0xc7,
    .sectorEraseCmd = 0x20,
    .blk32EraseCmd = 0x52,
    .blk64EraseCmd = 0xd8,
    /*write*/
    .pageProgramCmd = 0x02,
    .qpageProgramCmd = 0x38,
    .qppAddrMode = SF_CTRL_ADDR_4_LINES,

    .ioMode = SF_CTRL_QIO_MODE,
    .clkDelay = 1,
    .clkInvert = 0x3f,

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
    .timeCe = 20 * 1000,
    .pdDelay = 45,
    .qeData = 0,
};

static const ATTR_TCM_CONST_SECTION SPI_Flash_Cfg_Type flashCfg_Mxic_1635F = {
    .resetCreadCmd = 0xff,
    .resetCreadCmdSize = 3,
    .mid = 0xC2,

    .deBurstWrapCmd = 0xC0,
    .deBurstWrapCmdDmyClk = 0x00,
    .deBurstWrapDataMode = SF_CTRL_DATA_1_LINE,
    .deBurstWrapData = 0x10,

    /*reg*/
    .writeEnableCmd = 0x06,
    .wrEnableIndex = 0x00,
    .wrEnableBit = 0x01,
    .wrEnableReadRegLen = 0x01,

    .qeIndex = 0,
    .qeBit = 0x06,
    .qeWriteRegLen = 0x01,
    .qeReadRegLen = 0x1,

    .busyIndex = 0,
    .busyBit = 0x00,
    .busyReadRegLen = 0x1,
    .releasePowerDown = 0xab,

    .readRegCmd[0] = 0x05,
    .readRegCmd[1] = 0x00,
    .writeRegCmd[0] = 0x01,
    .writeRegCmd[1] = 0x00,

    .fastReadQioCmd = 0xeb,
    .frQioDmyClk = 16 / 8,
    .cReadSupport = 1,
    .cReadMode = 0xA5,

    .burstWrapCmd = 0xC0,
    .burstWrapCmdDmyClk = 0x00,
    .burstWrapDataMode = SF_CTRL_DATA_1_LINE,
    .burstWrapData = 0x02,
    /*erase*/
    .chipEraseCmd = 0xc7,
    .sectorEraseCmd = 0x20,
    .blk32EraseCmd = 0x52,
    .blk64EraseCmd = 0xd8,
    /*write*/
    .pageProgramCmd = 0x02,
    .qpageProgramCmd = 0x38,
    .qppAddrMode = SF_CTRL_ADDR_4_LINES,

    .ioMode = SF_CTRL_QIO_MODE,
    .clkDelay = 1,
    .clkInvert = 0x3f,

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
    .timeCe = 20 * 1000,
    .pdDelay = 45,
    .qeData = 0,
};

static const ATTR_TCM_CONST_SECTION SPI_Flash_Cfg_Type flashCfg_MX_KH25 = {
    .resetCreadCmd = 0xff,
    .resetCreadCmdSize = 3,
    .mid = 0xc2,

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
    .qeWriteRegLen = 0x01,
    .qeReadRegLen = 0x1,

    .busyIndex = 0,
    .busyBit = 0x00,
    .busyReadRegLen = 0x1,
    .releasePowerDown = 0xab,

    .readRegCmd[0] = 0x05,
    .readRegCmd[1] = 0x00,
    .writeRegCmd[0] = 0x01,
    .writeRegCmd[1] = 0x00,

    .fastReadQioCmd = 0xeb,
    .frQioDmyClk = 16 / 8,
    .cReadSupport = 0,
    .cReadMode = 0x20,

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

    .ioMode = 0x11,
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
    .qeWriteRegLen = 0x01,
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
    .cReadMode = 0x20,

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
    .timeEsector = 400,
    .timeE32k = 1600,
    .timeE64k = 2000,
    .timePagePgm = 5,
    .timeCe = 33000,
    .pdDelay = 20,
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
        .jedecID = 0x1440ef,
        .name = "Winb_80DV_08_33",
        .cfg = &flashCfg_Winb_80DV,
    },
    {
        .jedecID = 0x1540ef,
        .name = "Winb_16DV_16_33",
        .cfg = &flashCfg_Gd_Q80E_Q16E,
    },
    {
        .jedecID = 0x1640ef,
        .name = "Winb_32FV_32_33",
        .cfg = &flashCfg_Winb_16JV,
    },
    {
        .jedecID = 0x1460ef,
        .name = "Winb_80EW_08_18",
        .cfg = &flashCfg_Winb_16JV,
    },
    {
        .jedecID = 0x1560ef,
        .name = "Winb_16FW_16_18",
        .cfg = &flashCfg_Winb_16JV,
    },
    {
        .jedecID = 0x1660ef,
        .name = "Winb_32FW_32_18",
        .cfg = &flashCfg_Winb_16JV,
    },
    {
        .jedecID = 0x1860ef,
        .name = "Winb_128FW_128_18",
        .cfg = &flashCfg_Winb_16JV,
    },
    {
        .jedecID = 0x1680ef,
        .name = "Winb_32JW_32_18",
        .cfg = &flashCfg_Winb_16JV,
    },
    {
        .jedecID = 0x13605e,
        .name = "Zbit_04_33",
        .cfg = &flashCfg_Winb_16JV,
    },
    {
        .jedecID = 0x14605e,
        .name = "Zbit_08_33",
        .cfg = &flashCfg_Winb_16JV,
    },
    {
        .jedecID = 0x134051,
        .name = "GD_MD04D_04_33",
        .cfg = &flashCfg_Gd_Md_40D,
    },
    {
        .jedecID = 0x1440C8,
        .name = "GD_Q08E_08_33",
        .cfg = &flashCfg_Gd_Q80E_Q16E,
    },
    {
        .jedecID = 0x1540C8,
        .name = "GD_Q16E_16_33",
        .cfg = &flashCfg_Gd_Q80E_Q16E,
    },
    {
        .jedecID = 0x1640C8,
        .name = "GD_Q32C_32_33",
        .cfg = &flashCfg_Winb_16JV,
    },
    {
        .jedecID = 0x1840C8,
        .name = "GD_Q128E_128_33",
        .cfg = &flashCfg_Winb_16JV,
    },
    {
        .jedecID = 0x1460C8,
        .name = "GD_LQ08C_08_18",
        .cfg = &flashCfg_Gd_Q80E_Q16E,
    },
    {
        .jedecID = 0x1560C8,
        .name = "GD_LE16C_16_18",
        .cfg = &flashCfg_Gd_Q80E_Q16E,
    },
    {
        .jedecID = 0x1660C8,
        .name = "GD_LQ32D_32_18",
        .cfg = &flashCfg_Gd_Q80E_Q16E,
    },
    {
        .jedecID = 0x1760C8,
        .name = "GD_LQ64E_64_18",
        .cfg = &flashCfg_Gd_Q80E_Q16E,
    },
    {
        .jedecID = 0x1465C8,
        .name = "GD_WQ80E_80_33",
        .cfg = &flashCfg_Gd_WQ80E_WQ16E,
    },
    {
        .jedecID = 0x1565C8,
        .name = "GD_WQ16E_16_33",
        .cfg = &flashCfg_Gd_WQ80E_WQ16E,
    },
    {
        .jedecID = 0x1665C8,
        .name = "GD_WQ32E_32_33",
        .cfg = &flashCfg_Gd_Q80E_Q16E,
    },
    {
        .jedecID = 0x3425C2,
        .name = "MX_25V80_08_18",
        .cfg = &flashCfg_Mxic,
    },
    {
        .jedecID = 0x3525C2,
        .name = "MX_25U16_35_18",
        .cfg = &flashCfg_Mxic_1635F,
    },
    {
        .jedecID = 0x3625C2,
        .name = "MX_25V32_32_18",
        .cfg = &flashCfg_Mxic,
    },
    {
        .jedecID = 0x13400B,
        .name = "XT_25F04D_04_33",
        .cfg = &flashCfg_Gd_Md_40D,
    },
    {
        .jedecID = 0x15400B,
        .name = "XT_25F16B_16_33",
        .cfg = &flashCfg_Gd_Q80E_Q16E,
    },
    {
        .jedecID = 0x16400B,
        .name = "XT_25F32B_32_33",
        .cfg = &flashCfg_Gd_Q80E_Q16E,
    },
    {
        .jedecID = 0x14600B,
        .name = "XT_25Q80B_08_18",
        .cfg = &flashCfg_Gd_Q80E_Q16E,
    },
    {
        .jedecID = 0x16600B,
        .name = "XT_25Q32B_32_18",
        .cfg = &flashCfg_Gd_Q80E_Q16E,
    },
    {
        .jedecID = 0x134068,
        .name = "Boya_Q04B_04_33",
        .cfg = &flashCfg_Gd_Md_40D,
    },
    {
        .jedecID = 0x144068,
        .name = "Boya_Q08B_08_33",
        .cfg = &flashCfg_Winb_16JV,
    },
    {
        .jedecID = 0x154068,
        .name = "Boya_Q16B_16_33",
        .cfg = &flashCfg_Winb_16JV,
    },
    {
        .jedecID = 0x164068,
        .name = "Boya_Q32B_32_33",
        .cfg = &flashCfg_Winb_16JV,
    },
    {
        .jedecID = 0x174068,
        .name = "Boya_Q64A_64_33",
        .cfg = &flashCfg_Winb_16JV,
    },
    {
        .jedecID = 0x184068,
        .name = "Boya_Q128A_128_33",
        .cfg = &flashCfg_Winb_16JV,
    },
    {
        .jedecID = 0x1320c2,
        .name = "MX_KH40_04_33",
        .cfg = &flashCfg_MX_KH25,
    },
    {
        .jedecID = 0x1420c2,
        .name = "MX_KH80_08_33",
        .cfg = &flashCfg_MX_KH25,
    },
    {
        .jedecID = 0x1520c2,
        .name = "MX_KH16_16_33",
        .cfg = &flashCfg_MX_KH25,
    },
    {
        .jedecID = 0x1440A1,
        .name = "FM_25Q80_80_33",
        .cfg = &flashCfg_Gd_Q80E_Q16E,
    },
    {
        .jedecID = 0x1570EF,
        .name = "Winb_16JV_16_33",
        .cfg = &flashCfg_Winb_16JV,
    },
    {
        .jedecID = 0x1870EF,
        .name = "Winb_128JV_128_33",
        .cfg = &flashCfg_Winb_16JV,
    },
    {
        .jedecID = 0x15605E,
        .name = "ZB_VQ16_16_33",
        .cfg = &flashCfg_Winb_16JV,
    },
    {
        .jedecID = 0x144020,
        .name = "XM_25QH80_80_33",
        .cfg = &flashCfg_Winb_16JV,
    },
    {
        .jedecID = 0x154020,
        .name = "XM_25QH16_16_33",
        .cfg = &flashCfg_Winb_16JV,
    },
    {
        .jedecID = 0x164020,
        .name = "XM_25QH32_32_33",
        .cfg = &flashCfg_Winb_16JV,
    },
    {
        .jedecID = 0x174020,
        .name = "XM_25QH64_64_33",
        .cfg = &flashCfg_Winb_16JV,
    },
    {
        .jedecID = 0x13325E,
        .name = "ZB_D40B_40_33",
        .cfg = &flashCfg_MX_KH25,
    },
    {
        .jedecID = 0x14325E,
        .name = "ZB_D80B_80_33",
        .cfg = &flashCfg_MX_KH25,
    },
    {
        .jedecID = 0x15405E,
        .name = "ZB_25Q16B_15_33",
        .cfg = &flashCfg_Winb_16JV,
    },
    {
        .jedecID = 0x16405E,
        .name = "ZB_25Q32B_16_33",
        .cfg = &flashCfg_Winb_16JV,
    },
    {
        .jedecID = 0x1560EB,
        .name = "TH_25Q16HB_16_33",
        .cfg = &flashCfg_Gd_Q80E_Q16E,
    },
    {
        .jedecID = 0x15345E,
        .name = "ZB_25Q16A_15_33",
        .cfg = &flashCfg_Winb_16JV,
    },
    {
        .jedecID = 0x14650b,
        .name="XTX_25W08F_08_1833",
        .cfg = &flashCfg_Winb_16JV,
    },
    {
        .jedecID = 0x1560c4,
        .name="gt25q16_16_33",
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
#endif

/*@} end of group SF_CFG_Private_Variables */

/** @defgroup  SF_CFG_Global_Variables
 *  @{
 */

/*@} end of group SF_CFG_Global_Variables */

/** @defgroup  SF_CFG_Private_Fun_Declaration
 *  @{
 */

/*@} end of group SF_CFG_Private_Fun_Declaration */

/** @defgroup  SF_CFG_Private_Functions
 *  @{
 */

/****************************************************************************/ /**
 * @brief  Embedded flash set input function enable
 *
 * @param  None
 *
 * @return None
 *
*******************************************************************************/
#ifndef BFLB_USE_ROM_DRIVER
__WEAK
void ATTR_TCM_SECTION SF_Cfg_Init_Internal_Flash_Gpio(void)
{
    uint32_t tmpVal;
    uint32_t gpioPin;
    uint32_t pinOffset;

    for (gpioPin = 32; gpioPin < 38; gpioPin++) {
        pinOffset = (gpioPin >> 1) << 2;
        tmpVal = *(uint32_t *)(GLB_BASE + GLB_GPIO_OFFSET + pinOffset);
        if (gpioPin % 2 == 0) {
            /* [0] is ie */
            tmpVal = BL_SET_REG_BIT(tmpVal, GLB_REG_GPIO_0_IE);
        } else {
            /* [16] is ie */
            tmpVal = BL_SET_REG_BIT(tmpVal, GLB_REG_GPIO_1_IE);
        }
        *(uint32_t *)(GLB_BASE + GLB_GPIO_OFFSET + pinOffset) = tmpVal;
    }
}
#endif

/****************************************************************************/ /**
 * @brief  Init external flash GPIO according to flash GPIO config
 *
 * @param  extFlashPin: Flash GPIO config
 *
 * @return None
 *
*******************************************************************************/
#ifndef BFLB_USE_ROM_DRIVER
__WEAK
/* static */ void ATTR_TCM_SECTION SF_Cfg_Init_Ext_Flash_Gpio(uint8_t extFlashPin)
{
    GLB_GPIO_Cfg_Type cfg;
    uint8_t gpiopins[6];
    uint8_t i = 0;

    cfg.gpioMode = GPIO_MODE_AF;
    cfg.pullType = GPIO_PULL_UP;
    cfg.drive = 1;
    cfg.smtCtrl = 1;
    cfg.gpioFun = GPIO_FUN_FLASH_PSRAM;

    gpiopins[0] = BFLB_EXTFLASH_CLK_GPIO;
    gpiopins[1] = BFLB_EXTFLASH_CS_GPIO;
    gpiopins[2] = BFLB_EXTFLASH_DATA0_GPIO;
    gpiopins[3] = BFLB_EXTFLASH_DATA1_GPIO;
    gpiopins[4] = BFLB_EXTFLASH_DATA2_GPIO;
    gpiopins[5] = BFLB_EXTFLASH_DATA3_GPIO;

    for (i = 0; i < sizeof(gpiopins); i++) {
        cfg.gpioPin = gpiopins[i];
        cfg.gpioMode = GPIO_MODE_AF;

        GLB_GPIO_Init(&cfg);
    }
}
#endif

/****************************************************************************/ /**
 * @brief  Deinit external flash GPIO according to flash GPIO config
 *
 * @param  extFlashPin: Flash GPIO config
 *
 * @return None
 *
*******************************************************************************/
#ifndef BFLB_USE_ROM_DRIVER
__WEAK
/* static */ void ATTR_TCM_SECTION SF_Cfg_Deinit_Ext_Flash_Gpio(uint8_t extFlashPin)
{
    GLB_GPIO_Cfg_Type cfg;
    uint8_t gpiopins[6];
    uint8_t i = 0;

    cfg.gpioMode = GPIO_MODE_INPUT;
    cfg.pullType = GPIO_PULL_UP;
    cfg.drive = 1;
    cfg.smtCtrl = 1;
    cfg.gpioFun = GPIO_FUN_GPIO;

    gpiopins[0] = BFLB_EXTFLASH_CLK_GPIO;
    gpiopins[1] = BFLB_EXTFLASH_CS_GPIO;
    gpiopins[2] = BFLB_EXTFLASH_DATA0_GPIO;
    gpiopins[3] = BFLB_EXTFLASH_DATA1_GPIO;
    gpiopins[4] = BFLB_EXTFLASH_DATA2_GPIO;
    gpiopins[5] = BFLB_EXTFLASH_DATA3_GPIO;

    for (i = 0; i < sizeof(gpiopins); i++) {
        cfg.gpioPin = gpiopins[i];
        GLB_GPIO_Init(&cfg);
    }
}
#endif

/*@} end of group SF_CFG_Private_Functions */

/** @defgroup  SF_CFG_Public_Functions
 *  @{
 */

/****************************************************************************/ /**
 * @brief  Get flash config according to flash ID
 *
 * @param  flashID: Flash ID
 * @param  pFlashCfg: Flash config pointer
 *
 * @return SUCCESS or ERROR
 *
*******************************************************************************/
#ifndef BFLB_USE_ROM_DRIVER
__WEAK
BL_Err_Type ATTR_TCM_SECTION SF_Cfg_Get_Flash_Cfg_Need_Lock(uint32_t flashID, SPI_Flash_Cfg_Type *pFlashCfg)
{
    uint32_t i;
    uint8_t buf[sizeof(SPI_Flash_Cfg_Type) + 8];
    uint32_t crc, *pCrc;
    uint32_t xipOffset;

    if (flashID == 0) {
        xipOffset = SF_Ctrl_Get_Flash_Image_Offset();
        SF_Ctrl_Set_Flash_Image_Offset(0);
        XIP_SFlash_Read_Via_Cache_Need_Lock(8 + BL702L_FLASH_XIP_BASE, buf, sizeof(SPI_Flash_Cfg_Type) + 8);
        SF_Ctrl_Set_Flash_Image_Offset(xipOffset);

        if (BL702L_MemCmp(buf, BFLB_FLASH_CFG_MAGIC, 4) == 0) {
            crc = BFLB_Soft_CRC32((uint8_t *)buf + 4, sizeof(SPI_Flash_Cfg_Type));
            pCrc = (uint32_t *)(buf + 4 + sizeof(SPI_Flash_Cfg_Type));

            if (*pCrc == crc) {
                BL702L_MemCpy_Fast(pFlashCfg, (uint8_t *)buf + 4, sizeof(SPI_Flash_Cfg_Type));
                return SUCCESS;
            }
        }
    } else {
        for (i = 0; i < sizeof(flashInfos) / sizeof(flashInfos[0]); i++) {
            if (flashInfos[i].jedecID == flashID) {
                BL702L_MemCpy_Fast(pFlashCfg, flashInfos[i].cfg, sizeof(SPI_Flash_Cfg_Type));
                return SUCCESS;
            }
        }
    }

    return ERROR;
}
#endif

/****************************************************************************/ /**
 * @brief  Get flash config according to flash ID patch
 *
 * @param  flashID: Flash ID
 * @param  pFlashCfg: Flash config pointer
 *
 * @return SUCCESS or ERROR
 *
*******************************************************************************/
#ifndef BFLB_USE_ROM_DRIVER
__WEAK
BL_Err_Type ATTR_TCM_SECTION SF_Cfg_Get_Flash_Cfg_Need_Lock_Ext(uint32_t flashID, SPI_Flash_Cfg_Type *pFlashCfg)
{
    return SF_Cfg_Get_Flash_Cfg_Need_Lock(flashID, pFlashCfg);
}
#endif

/****************************************************************************/ /**
 * @brief  Init flash GPIO according to flash Pin config
 *
 * @param  flashPinCfg: Specify flash Pin config
 * @param  restoreDefault: Wether to restore default setting
 *
 * @return None
 *
*******************************************************************************/
#ifndef BFLB_USE_ROM_DRIVER
__WEAK
void ATTR_TCM_SECTION SF_Cfg_Init_Flash_Gpio(uint8_t flashPinCfg, uint8_t restoreDefault)
{
    if (restoreDefault) {
        GLB_Set_Embedded_FLash_IO_PARM(0, 0, 0);

        SF_Cfg_Init_Internal_Flash_Gpio();
        SF_Ctrl_Select_Pad(SF_CTRL_PAD_SEL_SF1);

        /* Default is set, so return */
        if (flashPinCfg == SF_CTRL_SEL_INTERNAL_FLASH_SWAP_NONE) {
            return;
        }
    }

    if (flashPinCfg == SF_CTRL_SEL_EXTERNAL_FLASH) {
        SF_Cfg_Init_Ext_Flash_Gpio(0);
        SF_Ctrl_Select_Pad(SF_CTRL_PAD_SEL_SF2);
    } else {
        if (flashPinCfg == SF_CTRL_SEL_INTERNAL_FLASH_SWAP_NONE) {
            GLB_Set_Embedded_FLash_IO_PARM(0, 0, 0);
        } else if (flashPinCfg == SF_CTRL_SEL_INTERNAL_FLASH_SWAP_CSIO2) {
            GLB_Set_Embedded_FLash_IO_PARM(0, 0, 1);
        } else if (flashPinCfg == SF_CTRL_SEL_INTERNAL_FLASH_SWAP_IO0IO3) {
            GLB_Set_Embedded_FLash_IO_PARM(0, 1, 0);
        } else if (flashPinCfg == SF_CTRL_SEL_INTERNAL_FLASH_SWAP_BOTH) {
            GLB_Set_Embedded_FLash_IO_PARM(0, 1, 1);
        } else if (flashPinCfg == SF_CTRL_SEL_INTERNAL_FLASH_REVERSE_SWAP_NONE) {
            GLB_Set_Embedded_FLash_IO_PARM(1, 0, 0);
        } else if (flashPinCfg == SF_CTRL_SEL_INTERNAL_FLASH_REVERSE_SWAP_CSIO2) {
            GLB_Set_Embedded_FLash_IO_PARM(1, 0, 1);
        } else if (flashPinCfg == SF_CTRL_SEL_INTERNAL_FLASH_REVERSE_SWAP_IO0IO3) {
            GLB_Set_Embedded_FLash_IO_PARM(1, 1, 0);
        } else {
            GLB_Set_Embedded_FLash_IO_PARM(1, 1, 1);
        }

        SF_Cfg_Init_Internal_Flash_Gpio();
        SF_Ctrl_Select_Pad(SF_CTRL_PAD_SEL_SF1);
    }
}
#endif

/****************************************************************************/ /**
 * @brief  Identify one flash
 *
 * @param  callFromFlash: code run at flash or ram
 * @param  autoScan: Auto scan all GPIO pin
 * @param  flashPinCfg: Specify flash GPIO config, not auto scan
 * @param  restoreDefault: Wether restore default flash GPIO config
 * @param  pFlashCfg: Flash config pointer
 *
 * @return Flash ID
 *
*******************************************************************************/
#ifndef BFLB_USE_ROM_DRIVER
__WEAK
uint32_t ATTR_TCM_SECTION SF_Cfg_Flash_Identify(uint8_t callFromFlash,
                                                uint32_t autoScan, uint32_t flashPinCfg, uint8_t restoreDefault, SPI_Flash_Cfg_Type *pFlashCfg)
{
    uint32_t jdecId = 0;
    uint32_t i = 0;
    uint32_t offset;
    BL_Err_Type stat;

    BL702L_MemCpy_Fast(pFlashCfg, &flashCfg_Gd_Q80E_Q16E, sizeof(SPI_Flash_Cfg_Type));

    if (callFromFlash == 1) {
        stat = XIP_SFlash_State_Save(pFlashCfg, &offset);

        if (stat != SUCCESS) {
            SF_Ctrl_Set_Owner(SF_CTRL_OWNER_IAHB);
            return 0;
        }
    }

    if (autoScan) {
        flashPinCfg = 0;

        do {
            if (flashPinCfg > SF_CTRL_SEL_INTERNAL_FLASH_REVERSE_SWAP_BOTH) {
                jdecId = 0;
                break;
            }

            SF_Cfg_Init_Flash_Gpio(flashPinCfg, restoreDefault);
            SFlash_Releae_Powerdown(pFlashCfg);
            SFlash_Reset_Continue_Read(pFlashCfg);
            SFlash_DisableBurstWrap(pFlashCfg);
            jdecId = 0;
            SFlash_GetJedecId(pFlashCfg, (uint8_t *)&jdecId);
            SFlash_DisableBurstWrap(pFlashCfg);
            jdecId = jdecId & 0xffffff;
            flashPinCfg++;
        } while ((jdecId & 0x00ffff) == 0 || (jdecId & 0xffff00) == 0 || (jdecId & 0x00ffff) == 0xffff || (jdecId & 0xffff00) == 0xffff00);
    } else {
        /* select media gpio */
        SF_Cfg_Init_Flash_Gpio(flashPinCfg, restoreDefault);
        SFlash_Releae_Powerdown(pFlashCfg);
        SFlash_Reset_Continue_Read(pFlashCfg);
        SFlash_DisableBurstWrap(pFlashCfg);
        SFlash_GetJedecId(pFlashCfg, (uint8_t *)&jdecId);
        SFlash_DisableBurstWrap(pFlashCfg);
        jdecId = jdecId & 0xffffff;
    }

    for (i = 0; i < sizeof(flashInfos) / sizeof(flashInfos[0]); i++) {
        if (flashInfos[i].jedecID == jdecId) {
            BL702L_MemCpy_Fast(pFlashCfg, flashInfos[i].cfg, sizeof(SPI_Flash_Cfg_Type));
            break;
        }
    }

    if (i == sizeof(flashInfos) / sizeof(flashInfos[0])) {
        if (callFromFlash == 1) {
            XIP_SFlash_State_Restore(pFlashCfg, offset);
        }

        return jdecId;
    } else {
        if (callFromFlash == 1) {
            XIP_SFlash_State_Restore(pFlashCfg, offset);
        }

        return (jdecId | BFLB_FLASH_ID_VALID_FLAG);
    }
}
#endif

/****************************************************************************/ /**
 * @brief  Identify one flash patch
 *
 * @param  callFromFlash: code run at flash or ram
 * @param  autoScan: Auto scan all GPIO pin
 * @param  flashPinCfg: Specify flash GPIO config, not auto scan
 * @param  restoreDefault: Wether restore default flash GPIO config
 * @param  pFlashCfg: Flash config pointer
 *
 * @return Flash ID
 *
*******************************************************************************/
#ifndef BFLB_USE_ROM_DRIVER
uint32_t ATTR_TCM_SECTION SF_Cfg_Flash_Identify_Ext(uint8_t callFromFlash, uint32_t autoScan, uint32_t flashPinCfg,
                                                    uint8_t restoreDefault, SPI_Flash_Cfg_Type *pFlashCfg)
{
    return SF_Cfg_Flash_Identify(callFromFlash, autoScan, flashPinCfg, restoreDefault, pFlashCfg);
}
#endif

/*@} end of group SF_CFG_Public_Functions */

/*@} end of group SF_CFG */

/*@} end of group BL702L_Peripheral_Driver */
