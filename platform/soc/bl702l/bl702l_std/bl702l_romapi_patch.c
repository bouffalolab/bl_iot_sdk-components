#include "bl702l_ef_cfg.h"
#include "bl702l_ef_ctrl.h"
#include "ef_data_0_reg.h"


#define EF_CTRL_LOAD_BEFORE_READ_R0 EF_Ctrl_Load_Efuse_R0()


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

        tmp1 = BL_RD_REG(EF_DATA_BASE, EF_DATA_0_EF_KEY_SLOT_2_W3);
        tmp2 = BL_RD_REG(EF_DATA_BASE, EF_DATA_0_EF_KEY_SLOT_3_W0);
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
        BL_WR_REG(EF_DATA_BASE, EF_DATA_0_EF_KEY_SLOT_2_W3, BL_RDWD_FRM_BYTEP(maclow));
    }

    /* The high 32 bits */
    if (slot == 0) {
        BL_WR_REG(EF_DATA_BASE, EF_DATA_0_EF_WIFI_MAC_HIGH, BL_RDWD_FRM_BYTEP(machigh));
    } else if (slot == 1) {
        BL_WR_REG(EF_DATA_BASE, EF_DATA_0_EF_KEY_SLOT_5_W1, BL_RDWD_FRM_BYTEP(machigh));
    } else if (slot == 2) {
        BL_WR_REG(EF_DATA_BASE, EF_DATA_0_EF_KEY_SLOT_3_W0, BL_RDWD_FRM_BYTEP(machigh));
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
        tmpVal = BL_RD_REG(EF_DATA_BASE, EF_DATA_0_EF_KEY_SLOT_2_W3);
    }
    BL_WRWD_TO_BYTEP(maclow, tmpVal);

    if (slot == 0) {
        tmpVal = BL_RD_REG(EF_DATA_BASE, EF_DATA_0_EF_WIFI_MAC_HIGH);
    } else if (slot == 1) {
        tmpVal = BL_RD_REG(EF_DATA_BASE, EF_DATA_0_EF_KEY_SLOT_5_W1);
    } else if (slot == 2) {
        tmpVal = BL_RD_REG(EF_DATA_BASE, EF_DATA_0_EF_KEY_SLOT_3_W0);
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
