/**
 * @file hal_flash.c
 * @brief
 *
 * Copyright (c) 2021 Bouffalolab team
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 */
#include "bl702l_glb.h"
#include "bl702l_xip_sflash.h"
#include "bl702l_sf_cfg.h"
#include "bl702l_ef_ctrl.h"
#include "hal_flash.h"

static uint32_t g_jedec_id = 0;
static SPI_Flash_Cfg_Type g_flash_cfg = {
    .resetCreadCmd = 0xff,
    .resetCreadCmdSize = 3,
    .mid = 0xc8,

    .deBurstWrapCmd = 0x77,
    .deBurstWrapCmdDmyClk = 0x3,
    .deBurstWrapDataMode = SF_CTRL_DATA_4_LINES,
    .deBurstWrapData = 0xF0,

    /* reg */
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
    .cReadMode = 0xa0,

    .burstWrapCmd = 0x77,
    .burstWrapCmdDmyClk = 0x3,
    .burstWrapDataMode = SF_CTRL_DATA_4_LINES,
    .burstWrapData = 0x40,
    /* erase */
    .chipEraseCmd = 0xc7,
    .sectorEraseCmd = 0x20,
    .blk32EraseCmd = 0x52,
    .blk64EraseCmd = 0xd8,
    /* write */
    .pageProgramCmd = 0x02,
    .qpageProgramCmd = 0x32,
    .qppAddrMode = SF_CTRL_ADDR_1_LINE,

    .ioMode = 0x10,
    .clkDelay = 0,
    .clkInvert = 0x03,

    .resetEnCmd = 0x66,
    .resetCmd = 0x99,
    .cRExit = 0xff,
    .wrEnableWriteRegLen = 0x00,

    /* id */
    .jedecIdCmd = 0x9f,
    .jedecIdCmdDmyClk = 0,
    .qpiJedecIdCmd = 0x9f,
    .qpiJedecIdCmdDmyClk = 0x00,
    .sectorSize = 4,
    .pageSize = 256,

    /* read */
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

    /* AC */
    .timeEsector = 500,
    .timeE32k = 2000,
    .timeE64k = 2000,
    .timePagePgm = 5,
    .timeCe = 33 * 1000,
    .pdDelay = 20,
    .qeData = 0,
};

/**
 * @brief flash_get_clock_delay
 *
 * @return int
 */
static BL_Err_Type ATTR_TCM_SECTION flash_get_clock_delay(SPI_Flash_Cfg_Type *cfg)
{
    uint32_t tmpVal = 0;

    tmpVal = BL_RD_REG(SF_CTRL_BASE, SF_CTRL_0);
    /* bit0-3 for clk delay */
    if (BL_IS_REG_BIT_SET(tmpVal, SF_CTRL_SF_IF_READ_DLY_EN)) {
        cfg->clkDelay = BL_GET_REG_BITS_VAL(tmpVal, SF_CTRL_SF_IF_READ_DLY_N) + 1;
    } else {
        cfg->clkDelay = 0;
    }
    cfg->clkInvert = 0;
    /* bit0 for clk invert */
    cfg->clkInvert |= ((BL_GET_REG_BITS_VAL(tmpVal, SF_CTRL_SF_CLK_OUT_INV_SEL) & 1) << 0);
    /* bit1 for rx clk invert */
    cfg->clkInvert |= ((BL_GET_REG_BITS_VAL(tmpVal, SF_CTRL_SF_CLK_SF_RX_INV_SEL) & 1) << 1);

    tmpVal = BL_RD_REG(SF_CTRL_BASE, SF_CTRL_SF_IF_IO_DLY_1);
    /* bit4-6 for do delay */
    cfg->clkDelay |= ((BL_GET_REG_BITS_VAL(tmpVal, SF_CTRL_IO_0_DO_DLY_SEL) & 7) << 4);
    /* bit2-4 for di delay */
    cfg->clkInvert |= ((BL_GET_REG_BITS_VAL(tmpVal, SF_CTRL_IO_0_DI_DLY_SEL) & 7) << 2);
    /* bit5-7 for oe delay */
    cfg->clkInvert |= ((BL_GET_REG_BITS_VAL(tmpVal, SF_CTRL_IO_0_OE_DLY_SEL) & 7) << 5);

    return SUCCESS;
}

/**
 * @brief flash_get_jedecid
 *
 * @return BL_Err_Type
 */
uint32_t flash_get_jedecid(void)
{
    uint32_t jid = 0;

    jid = ((g_jedec_id&0xff)<<16) + (g_jedec_id&0xff00) + ((g_jedec_id&0xff0000)>>16);
    return jid;
}

/**
 * @brief flash_get_cfg
 *
 * @return BL_Err_Type
 */
BL_Err_Type flash_get_cfg(uint8_t **cfg_addr, uint32_t *len)
{
    *cfg_addr = (uint8_t *)&g_flash_cfg;
    *len = sizeof(SPI_Flash_Cfg_Type);

    return SUCCESS;
}

/**
 * @brief flash_set_qspi_enable
 *
 * @return BL_Err_Type
 */
static BL_Err_Type ATTR_TCM_SECTION flash_set_qspi_enable(SPI_Flash_Cfg_Type *p_flash_cfg)
{
    if ((p_flash_cfg->ioMode & 0x0f) == SF_CTRL_QO_MODE || (p_flash_cfg->ioMode & 0x0f) == SF_CTRL_QIO_MODE) {
        SFlash_Qspi_Enable(p_flash_cfg);
    }

    return SUCCESS;
}

/**
 * @brief flash_set_l1c_wrap
 *
 * @return BL_Err_Type
 */
static BL_Err_Type ATTR_TCM_SECTION flash_set_l1c_wrap(SPI_Flash_Cfg_Type *p_flash_cfg)
{
    if (((p_flash_cfg->ioMode >> 4) & 0x01) == 1) {
        L1C_Set_Wrap(DISABLE);
    } else {
        L1C_Set_Wrap(ENABLE);
        if ((p_flash_cfg->ioMode & 0x0f) == SF_CTRL_QO_MODE || (p_flash_cfg->ioMode & 0x0f) == SF_CTRL_QIO_MODE) {
            SFlash_SetBurstWrap(p_flash_cfg);
        }
    }

    return SUCCESS;
}

/**
 * @brief get flash size from flash jedec id
 *
 * @return BL_Err_Type
 */
static uint32_t ATTR_TCM_SECTION flash_get_size_from_jedecid(uint32_t jedec_id)
{
    uint8_t flash_size_level = 0;
    uint32_t flash_size = 0;
    uint32_t jid = 0;

    jid = ((jedec_id&0xff)<<16) + (jedec_id&0xff00) + ((jedec_id&0xff0000)>>16);

    if (jid == 0) {
        return 0;
    }

    flash_size_level = (jid & 0x1f);
    flash_size_level -= 0x13;
    flash_size = (1 << flash_size_level) * 512 * 1024;

    return flash_size;
}

/**
 * @brief flash sf2 gpio init for dual bank mode
 *
 * @return BL_Err_Type
 */
// TODO: differ 702l & 702, by zhji, 20220511
// static BL_Err_Type ATTR_TCM_SECTION flash_sf2_gpio_init(void)
// {
//     uint32_t tmpVal;
//     Efuse_Device_Info_Type devInfo;

//     EF_Ctrl_Read_Device_Info(&devInfo);
//     /* flash_cfg != BFLB_FLASH_CFG_SF1_EXT_17_22, flash pad use SF2 */
//     if (devInfo.flash_cfg != BFLB_FLASH_CFG_SF1_EXT_17_22) {
//         tmpVal = BL_RD_REG(GLB_BASE, GLB_GPIO_USE_PSRAM__IO);
//         /* GLB_CFG_GPIO_USE_PSRAM_IO == 0, need init SF2 flash gpio */
//         if (BL_GET_REG_BITS_VAL(tmpVal, GLB_CFG_GPIO_USE_PSRAM_IO) == 0x00) {
//             SF_Cfg_Init_Ext_Flash_Gpio(1);
//         }
//     }

//     return SUCCESS;
// }

/**
 * @brief flash_config_init
 *
 * @return BL_Err_Type
 */
static BL_Err_Type ATTR_TCM_SECTION flash_config_init(SPI_Flash_Cfg_Type *p_flash_cfg, uint8_t *jedec_id)
{
    BL_Err_Type ret = ERROR;
    uint8_t isAesEnable = 0;
    uint32_t jid = 0;
    uint32_t offset = 0;

    cpu_global_irq_disable();
    XIP_SFlash_Opt_Enter(&isAesEnable);
    XIP_SFlash_State_Save(p_flash_cfg, &offset);
// TODO: differ 702l & 702, by zhji
    // flash_sf2_gpio_init();
    SFlash_GetJedecId(p_flash_cfg, (uint8_t *)&jid);
    arch_memcpy(jedec_id, (uint8_t *)&jid, 3);
    jid &= 0xFFFFFF;
    g_jedec_id = jid;
    ret = SF_Cfg_Get_Flash_Cfg_Need_Lock(jid, p_flash_cfg);
    if (ret == SUCCESS) {
        p_flash_cfg->mid = (jid & 0xff);
    }

    /* Set flash controler from p_flash_cfg */
    flash_set_qspi_enable(p_flash_cfg);
    flash_set_l1c_wrap(p_flash_cfg);
    XIP_SFlash_State_Restore(p_flash_cfg, offset);
    XIP_SFlash_Opt_Exit(isAesEnable);
    cpu_global_irq_enable();

    return ret;
}

/**
 * @brief multi flash adapter
 *
 * @return BL_Err_Type
 */
BL_Err_Type ATTR_TCM_SECTION flash_init(void)
{
    int ret = -1;
    uint32_t jedec_id = 0;

    jedec_id = GLB_Get_Flash_Id_Value();
    if (jedec_id != 0) {
        ret = SF_Cfg_Get_Flash_Cfg_Need_Lock(jedec_id, &g_flash_cfg);
        if (ret == 0) {
            g_jedec_id = jedec_id;
            flash_get_clock_delay(&g_flash_cfg);
            return 0;
        }
    }

    ret = flash_config_init(&g_flash_cfg, (uint8_t *)&jedec_id);

    flash_get_clock_delay(&g_flash_cfg);
    GLB_Set_Flash_Id_Value(g_jedec_id);

    return ret;
}

/**
 * @brief get flash size
 *
 * @return BL_Err_Type
 */
uint32_t flash_get_size(void)
{
    return flash_get_size_from_jedecid(g_jedec_id);
}

/**
 * @brief read jedec id
 *
 * @param data
 * @return BL_Err_Type
 */
BL_Err_Type ATTR_TCM_SECTION flash_read_jedec_id(uint8_t *data)
{
    uint32_t jid = 0;
    uint8_t isAesEnable = 0;

    cpu_global_irq_disable();
    XIP_SFlash_Opt_Enter(&isAesEnable);
    XIP_SFlash_GetJedecId_Need_Lock(&g_flash_cfg, (uint8_t *)&jid);
    XIP_SFlash_Opt_Exit(isAesEnable);
    cpu_global_irq_enable();
    jid &= 0xFFFFFF;
    arch_memcpy(data, (void *)&jid, 4);

    return SUCCESS;
}

/**
 * @brief read flash data via xip
 *
 * @param addr
 * @param data
 * @param len
 * @return BL_Err_Type
 */
BL_Err_Type ATTR_TCM_SECTION flash_read_via_xip(uint32_t addr, uint8_t *data, uint32_t len)
{
    cpu_global_irq_disable();
    L1C_Cache_Flush();
    XIP_SFlash_Read_Via_Cache_Need_Lock(addr, data, len);
    L1C_Cache_Flush();
    cpu_global_irq_enable();

    return SUCCESS;
}

/**
 * @brief flash read data
 *
 * @param addr
 * @param data
 * @param len
 * @return BL_Err_Type
 */
BL_Err_Type ATTR_TCM_SECTION flash_read(uint32_t addr, uint8_t *data, uint32_t len)
{
    BL_Err_Type ret = ERROR;
    uint8_t isAesEnable = 0;

    cpu_global_irq_disable();
    XIP_SFlash_Opt_Enter(&isAesEnable);
    ret = XIP_SFlash_Read_Need_Lock(&g_flash_cfg, addr, data, len);
    XIP_SFlash_Opt_Exit(isAesEnable);
    cpu_global_irq_enable();

    return ret;
}

/**
 * @brief flash write data
 *
 * @param addr
 * @param data
 * @param len
 * @return BL_Err_Type
 */
BL_Err_Type ATTR_TCM_SECTION flash_write(uint32_t addr, uint8_t *data, uint32_t len)
{
    BL_Err_Type ret = ERROR;
    uint8_t isAesEnable = 0;

    cpu_global_irq_disable();
    XIP_SFlash_Opt_Enter(&isAesEnable);
    ret = XIP_SFlash_Write_Need_Lock(&g_flash_cfg, addr, data, len);
    XIP_SFlash_Opt_Exit(isAesEnable);
    cpu_global_irq_enable();

    return ret;
}

/**
 * @brief flash erase
 *
 * @param startaddr
 * @param endaddr
 * @return BL_Err_Type
 */
BL_Err_Type ATTR_TCM_SECTION flash_erase(uint32_t startaddr, uint32_t len)
{
    BL_Err_Type ret = ERROR;
    uint8_t isAesEnable = 0;

    cpu_global_irq_disable();
    XIP_SFlash_Opt_Enter(&isAesEnable);
    ret = XIP_SFlash_Erase_Need_Lock(&g_flash_cfg, startaddr, startaddr + len - 1);
    XIP_SFlash_Opt_Exit(isAesEnable);
    cpu_global_irq_enable();

    return ret;
}

/**
 * @brief flash clear status register
 *
 * @param None
 * @return BL_Err_Type
 */
BL_Err_Type ATTR_TCM_SECTION flash_clear_status_register(void)
{
    BL_Err_Type ret = ERROR;
    uint8_t isAesEnable = 0;

    cpu_global_irq_disable();
    XIP_SFlash_Opt_Enter(&isAesEnable);
    ret = XIP_SFlash_Clear_Status_Register_Need_Lock(&g_flash_cfg);
    XIP_SFlash_Opt_Exit(isAesEnable);
    cpu_global_irq_enable();

    return ret;
}

/**
 * @brief set flash cache
 *
 * @param cont_read
 * @param cache_enable
 * @param cache_way_disable
 * @param flash_offset
 * @return BL_Err_Type
 */
BL_Err_Type ATTR_TCM_SECTION flash_set_cache(uint8_t cont_read, uint8_t cache_enable, uint8_t cache_way_disable, uint32_t flash_offset)
{
    uint32_t tmp[1];
    uint8_t isAesEnable = 0;
    BL_Err_Type stat;

    SF_Ctrl_Set_Owner(SF_CTRL_OWNER_SAHB);

    XIP_SFlash_Opt_Enter(&isAesEnable);
    /* To make it simple, exit cont read anyway */
    SFlash_Reset_Continue_Read(&g_flash_cfg);

    if (g_flash_cfg.cReadSupport == 0) {
        cont_read = 0;
    }

    if (cont_read == 1) {
        stat = SFlash_Read(&g_flash_cfg, g_flash_cfg.ioMode & 0xf, 1, 0x00000000, (uint8_t *)tmp, sizeof(tmp));

        if (SUCCESS != stat) {
            XIP_SFlash_Opt_Exit(isAesEnable);
            return ERROR;
        }
    }

    /* Set default value */
    L1C_Cache_Enable_Set(0xf);

    if (cache_enable) {
        SF_Ctrl_Set_Flash_Image_Offset(flash_offset);
        SFlash_Cache_Read_Enable(&g_flash_cfg, g_flash_cfg.ioMode & 0xf, cont_read, cache_way_disable);
    }
    XIP_SFlash_Opt_Exit(isAesEnable);

    return SUCCESS;
}
