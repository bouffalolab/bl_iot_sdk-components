#include "ef_data_0_reg.h"
#include "bl702l_ef_cfg.h"
#include "bl702l_ef_ctrl.h"
#include "bl702l_sf_cfg.h"
#include "bl702l_sf_ctrl.h"


#define EF_CTRL_LOAD_BEFORE_READ_R0 EF_Ctrl_Load_Efuse_R0()


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


/****************************************************************************/ /**
 * @brief  Whether Capcode is empty
 *
 * @param  slot: Cap code slot
 * @param  reload: Whether reload
 *
 * @return 0 for all slots full,1 for others
 *
*******************************************************************************/
uint8_t EF_Ctrl_Is_CapCode_Empty(uint8_t slot, uint8_t reload)
{
    uint32_t tmp;

    /* Switch to AHB clock */
    EF_Ctrl_Sw_AHB_Clk_0();

    if (reload) {
        EF_CTRL_LOAD_BEFORE_READ_R0;
    }

    if (slot == 0) {
        tmp = BL_RD_REG(EF_DATA_BASE, EF_DATA_0_EF_KEY_SLOT_3_W3);
        return (EF_Ctrl_Is_All_Bits_Zero(tmp, 0, 10));
    } else if (slot == 1) {
        tmp = BL_RD_REG(EF_DATA_BASE, EF_DATA_0_EF_KEY_SLOT_3_W3);
        return (EF_Ctrl_Is_All_Bits_Zero(tmp, 10, 10));
    } else if (slot == 2) {
        tmp = BL_RD_REG(EF_DATA_BASE, EF_DATA_0_EF_KEY_SLOT_3_W3);
        return (EF_Ctrl_Is_All_Bits_Zero(tmp, 20, 10));
    } else {
        return 0;
    }
}

/****************************************************************************/ /**
 * @brief  Efuse write Cap code
 *
 * @param  slot: Cap code slot
 * @param  code: Cap code value
 * @param  program: Whether program
 *
 * @return SUCCESS or ERROR
 *
*******************************************************************************/
BL_Err_Type EF_Ctrl_Write_CapCode_Opt(uint8_t slot, uint8_t code, uint8_t program)
{
    uint32_t tmp;
    uint32_t trim;

    if (slot >= 3) {
        return ERROR;
    }

    /* Switch to AHB clock */
    EF_Ctrl_Sw_AHB_Clk_0();

    trim = code;
    trim |= EF_Ctrl_Get_Trim_Parity(code, 8) << 8;
    trim |= 1 << 9;

    if (slot == 0) {
        tmp = BL_RD_REG(EF_DATA_BASE, EF_DATA_0_EF_KEY_SLOT_3_W3);
        BL_WR_REG(EF_DATA_BASE, EF_DATA_0_EF_KEY_SLOT_3_W3, tmp | (trim << 0));
    } else if (slot == 1) {
        tmp = BL_RD_REG(EF_DATA_BASE, EF_DATA_0_EF_KEY_SLOT_3_W3);
        BL_WR_REG(EF_DATA_BASE, EF_DATA_0_EF_KEY_SLOT_3_W3, tmp | (trim << 10));
    } else if (slot == 2) {
        tmp = BL_RD_REG(EF_DATA_BASE, EF_DATA_0_EF_KEY_SLOT_3_W3);
        BL_WR_REG(EF_DATA_BASE, EF_DATA_0_EF_KEY_SLOT_3_W3, tmp | (trim << 20));
    }

    if (program) {
        EF_Ctrl_Program_Efuse_0();
    }

    return SUCCESS;
}

/****************************************************************************/ /**
 * @brief  Efuse read Cap code
 *
 * @param  slot: Cap code slot
 * @param  code: Cap code pointer
 * @param  reload: Whether reload
 *
 * @return SUCCESS or ERROR
 *
*******************************************************************************/
BL_Err_Type EF_Ctrl_Read_CapCode_Opt(uint8_t slot, uint8_t *code, uint8_t reload)
{
    uint32_t tmp;
    uint32_t trim;
    uint8_t en;
    uint8_t parity;

    if (slot >= 3) {
        return ERROR;
    }

    /* Switch to AHB clock */
    EF_Ctrl_Sw_AHB_Clk_0();

    if (reload) {
        EF_CTRL_LOAD_BEFORE_READ_R0;
    }

    if (slot == 0) {
        tmp = BL_RD_REG(EF_DATA_BASE, EF_DATA_0_EF_KEY_SLOT_3_W3);
        trim = (tmp >> 0) & 0x3ff;
    } else if (slot == 1) {
        tmp = BL_RD_REG(EF_DATA_BASE, EF_DATA_0_EF_KEY_SLOT_3_W3);
        trim = (tmp >> 10) & 0x3ff;
    } else if (slot == 2) {
        tmp = BL_RD_REG(EF_DATA_BASE, EF_DATA_0_EF_KEY_SLOT_3_W3);
        trim = (tmp >> 20) & 0x3ff;
    }

    en = trim >> 9;
    parity = (trim >> 8) & 0x01;
    if (en == 1 && parity == EF_Ctrl_Get_Trim_Parity(trim, 8)) {
        *code = trim & 0xff;
        return SUCCESS;
    }

    return ERROR;
}

/****************************************************************************/ /**
 * @brief  Whether power offset slot is empty
 *
 * @param  slot: Power offset code slot
 * @param  reload: Whether reload
 *
 * @return 0 for all slots full,1 for others
 *
*******************************************************************************/
uint8_t EF_Ctrl_Is_PowerOffset_Slot_Empty(uint8_t slot, uint8_t reload)
{
    uint32_t tmp1, tmp2;
    uint32_t part1Empty, part2Empty;

    /* Switch to AHB clock */
    EF_Ctrl_Sw_AHB_Clk_0();

    if (reload) {
        EF_CTRL_LOAD_BEFORE_READ_R0;
    }

    if (slot == 0) {
        tmp1 = BL_RD_REG(EF_DATA_BASE, EF_DATA_0_EF_KEY_SLOT_3_W2);
        part1Empty = (EF_Ctrl_Is_All_Bits_Zero(tmp1, 0, 18));
        part2Empty = 1;
    } else if (slot == 1) {
        tmp1 = BL_RD_REG(EF_DATA_BASE, EF_DATA_0_EF_KEY_SLOT_3_W2);
        tmp2 = BL_RD_REG(EF_DATA_BASE, EF_DATA_0_EF_KEY_SLOT_3_W1);
        part1Empty = (EF_Ctrl_Is_All_Bits_Zero(tmp1, 18, 12));
        part2Empty = (EF_Ctrl_Is_All_Bits_Zero(tmp2, 0, 6));
    } else if (slot == 2) {
        tmp1 = BL_RD_REG(EF_DATA_BASE, EF_DATA_0_EF_KEY_SLOT_3_W1);
        part1Empty = (EF_Ctrl_Is_All_Bits_Zero(tmp1, 6, 18));
        part2Empty = 1;
    } else {
        return 0;
    }

    return (part1Empty && part2Empty);
}

/****************************************************************************/ /**
 * @brief  Efuse write power offset
 *
 * @param  slot: Power offset slot
 * @param  pwrOffset[4]: Power offset value array
 * @param  program: Whether program
 *
 * @return SUCCESS or ERROR
 *
*******************************************************************************/
BL_Err_Type EF_Ctrl_Write_PowerOffset_Opt(uint8_t slot, int8_t pwrOffset[4], uint8_t program)
{
    uint32_t k = 0;
    uint32_t Value = 0;
    uint32_t tmp;
    uint32_t trim;

    if (slot >= 3) {
        return ERROR;
    }

    for (k = 0; k < 4; k++) {
        /* Use 4 bits as signed value */
        if (pwrOffset[k] > 7) {
            pwrOffset[k] = 7;
        }
        if (pwrOffset[k] < -8) {
            pwrOffset[k] = -8;
        }
        Value <<= 4;
        Value |= (uint32_t)(pwrOffset[k] & 0x0f);
    }

    trim = Value;
    trim |= EF_Ctrl_Get_Trim_Parity(Value, 16) << 16;
    trim |= 1 << 17;

    if (slot == 0) {
        tmp = BL_RD_REG(EF_DATA_BASE, EF_DATA_0_EF_KEY_SLOT_3_W2);
        tmp |= trim << 0;
        BL_WR_REG(EF_DATA_BASE, EF_DATA_0_EF_KEY_SLOT_3_W2, tmp);
    } else if (slot == 1) {
        tmp = BL_RD_REG(EF_DATA_BASE, EF_DATA_0_EF_KEY_SLOT_3_W2);
        tmp |= (trim & 0x0fff) << 18;
        BL_WR_REG(EF_DATA_BASE, EF_DATA_0_EF_KEY_SLOT_3_W2, tmp);

        tmp = BL_RD_REG(EF_DATA_BASE, EF_DATA_0_EF_KEY_SLOT_3_W1);
        tmp |= ((trim >> 12) & 0x003f) << 0;
        BL_WR_REG(EF_DATA_BASE, EF_DATA_0_EF_KEY_SLOT_3_W1, tmp);
    } else if (slot == 2) {
        tmp = BL_RD_REG(EF_DATA_BASE, EF_DATA_0_EF_KEY_SLOT_3_W1);
        tmp |= trim << 6;
        BL_WR_REG(EF_DATA_BASE, EF_DATA_0_EF_KEY_SLOT_3_W1, tmp);
    }

    if (program) {
        EF_Ctrl_Program_Efuse_0();
    }

    return SUCCESS;
}

/****************************************************************************/ /**
 * @brief  Efuse read poweroffset value
 *
 * @param  slot: Power offset slot
 * @param  pwrOffset[4]: Power offset array
 * @param  reload: Whether reload
 *
 * @return SUCCESS or ERROR
 *
*******************************************************************************/
BL_Err_Type EF_Ctrl_Read_PowerOffset_Opt(uint8_t slot, int8_t pwrOffset[4], uint8_t reload)
{
    uint32_t k = 0;
    uint32_t Value = 0;
    uint32_t tmp;
    uint32_t trim;
    uint8_t en;
    uint8_t parity;

    if (slot >= 3) {
        return ERROR;
    }

    /* Switch to AHB clock */
    EF_Ctrl_Sw_AHB_Clk_0();

    if (reload) {
        EF_CTRL_LOAD_BEFORE_READ_R0;
    }

    if (slot == 0) {
        tmp = BL_RD_REG(EF_DATA_BASE, EF_DATA_0_EF_KEY_SLOT_3_W2);
        trim = tmp & 0x3ffff;
    } else if (slot == 1) {
        tmp = BL_RD_REG(EF_DATA_BASE, EF_DATA_0_EF_KEY_SLOT_3_W2);
        trim = (tmp >> 18) & 0x0fff;

        tmp = BL_RD_REG(EF_DATA_BASE, EF_DATA_0_EF_KEY_SLOT_3_W1);
        trim |= (tmp & 0x003f) << 12;
    } else if (slot == 2) {
        tmp = BL_RD_REG(EF_DATA_BASE, EF_DATA_0_EF_KEY_SLOT_3_W1);
        trim = (tmp >> 6) & 0x3ffff;
    }

    en = trim >> 17;
    parity = (trim >> 16) & 0x01;
    if (en == 1 && parity == EF_Ctrl_Get_Trim_Parity(trim, 16)) {
        Value = trim & 0xffff;
        for (k = 0; k < 4; k++) {
            tmp = (Value >> 12) & 0x0f;
            if (tmp >= 8) {
                pwrOffset[k] = tmp - 16;
            } else {
                pwrOffset[k] = tmp;
            }
            Value <<= 4;
        }
        return SUCCESS;
    }

    return ERROR;
}

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

    if (flashID != 0) {
        for (i = 0; i < sizeof(flashInfos) / sizeof(flashInfos[0]); i++) {
            if (flashInfos[i].jedecID == flashID) {
                ARCH_MemCpy_Fast(pFlashCfg, flashInfos[i].cfg, sizeof(SPI_Flash_Cfg_Type));
                return SUCCESS;
            }
        }
    }

    return SF_Cfg_Get_Flash_Cfg_Need_Lock(flashID, pFlashCfg);
}
