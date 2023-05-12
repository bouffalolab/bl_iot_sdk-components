/**
 * @file hal_pm.c
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
#include "bl702l_sflash.h"
#include "bl702l_glb.h"
#include "bl702l_pds.h"
#include "bl702l_hbn.h"
#include "hal_pm.h"
#include "hal_clock.h"
#include "hal_rtc.h"
#include "hal_flash.h"
#include "risc-v/Core/Include/clic.h"

/* Cache Way Disable, will get from l1c register */
// uint8_t cacheWayDisable = 0;

/* PSRAM IO Configuration, will get from glb register */
// uint32_t psramIoCfg = 0;

/* Flash offset value, will get from sf_ctrl register */
// uint32_t flash_offset = 0;

SPI_Flash_Cfg_Type *flash_cfg;

#define PM_PDS_FLASH_POWER_OFF   1
#define PM_PDS_DLL_POWER_OFF     1
#define PM_PDS_PLL_POWER_OFF     1
#define PM_PDS_RF_POWER_OFF      1
#define PM_PDS_LDO_LEVEL_DEFAULT HBN_LDO_LEVEL_1P10V
#define PM_HBN_LDO_LEVEL_DEFAULT HBN_LDO_LEVEL_1P10V

static PDS_DEFAULT_LV_CFG_Type pdsCfgLevel31 = {
    .pdsCtl = {
        .pdsStart = 1,
        .sleepForever = 0,
        .xtalForceOff = 0,
        .waitRC32mRdy = 0,
        .avdd14Off = 1,
        .bgSysOff = 1,
        .glbRstProtect = 1,
        .puFlash = 0,
        .clkOff = 1,
        .memStby = 1,
        .swPuFlash = 1,
        .isolation = 1,
        .waitXtalRdy = 0,
        .pdsPwrOff = 1,
        .xtalOff = 0,
        .socEnbForceOn = 0,
        .pdsRstSocEn = 0,
        .pdsRC32mOn = 0,
        .pdsLdoVselEn = 0,
        .xtalCntRC32kEn = 1,
        .cpu0WfiMask = 0,
        .ldo11Off = 1,
        .pdsCtlRfSel = 0,
        .xtalDegEn = 0,
        .bleWakeupReqEn = 0,
        .swGpioIsoMod = 0,
        .pdsClkOff = 0,
        .pdsGpioKeep = 7,
        .pdsGpioIsoMod = 1,

    },
    .pdsCtl2 = {
        .forceCpuPdsRst = 0,
        .forceBzPdsRst = 0,
        .forceCpuGateClk = 0,
        .forceBzGateClk = 0,
    },
    .pdsCtl3 = {
        .forceMiscPwrOff = 0,
        .forceMiscPdsRst = 0,
        .forceMiscGateClk = 0,
        .MiscIsoEn = 1,
    },
    .pdsCtl4 = {
        .cpuRst = 1,
        .cpuGateClk = 1,
        .BzRst = 1,
        .BzGateClk = 1,
        .MiscPwrOff = 1,
        .MiscRst = 1,
        .MiscGateClk = 1,
    }
};

/****************************************************************************/ /**
* @brief  pm power on flash
*
* @param  cfg: PDS app config
*
* @return None
*
*******************************************************************************/
void ATTR_TCM_SECTION pm_power_on_flash(uint32_t *cfg, uint8_t cont)
{
    /* Init flash gpio */
    SF_Cfg_Init_Flash_Gpio(0, 1);

    SF_Ctrl_Set_Owner(SF_CTRL_OWNER_SAHB);

    /* Restore flash */
    SFlash_Restore_From_Powerdown((SPI_Flash_Cfg_Type *)cfg, cont);
}

/****************************************************************************/ /**
 * @brief  pm power on flash pad gpio
 *
 * @param  None
 *
 * @return None
 *
*******************************************************************************/
void ATTR_TCM_SECTION pm_power_on_flash_pad(void)
{
    /* Turn on Flash pad, GPIO23 - GPIO28 */
    SF_Cfg_Init_Internal_Flash_Gpio();
}

/****************************************************************************/ /**
 * @brief  pm update flash_ctrl setting
 *
 * @param  fastClock: fast clock
 *
 * @return None
 *
*******************************************************************************/
static ATTR_TCM_SECTION void PDS_Update_Flash_Ctrl_Setting(uint8_t fastClock)
{
    if (fastClock) {
        GLB_Set_SF_CLK(1, GLB_SFLASH_CLK_64M, 0);
    } else {
        GLB_Set_SF_CLK(1, GLB_SFLASH_CLK_XCLK, 0);
    }

    SF_Ctrl_Set_Clock_Delay(fastClock);
}

void ATTR_TCM_SECTION pm_pds_enable(uint32_t *cfg)
{
    uint32_t tmpVal = 0;
    PM_PDS_CFG_Type *p = (PM_PDS_CFG_Type *)cfg;
    PDS_DEFAULT_LV_CFG_Type *pPdsCfg = NULL;

    HBN_32K_Sel(HBN_32K_RC);
    HBN_Set_Ldo11_All_Vout(p->ldoLevel);

    /* To make it simple and safe*/
    cpu_global_irq_disable();

    // PDS_WAKEUP_IRQHandler_Install();
    /* CLear HBN RTC INT Status */
    tmpVal = BL_RD_REG(HBN_BASE, HBN_CTL);
    tmpVal &= ~0xE;
    BL_WR_REG(HBN_BASE, HBN_CTL, tmpVal);
    BL_WR_REG(HBN_BASE, HBN_IRQ_CLR, 0xffffffff);
    BL_WR_REG(HBN_BASE, HBN_IRQ_CLR, 0);

    tmpVal = BL_RD_REG(PDS_BASE, PDS_INT);
    tmpVal &= ~(1 << 8); //unmask pds wakeup

    if (BL_GET_REG_BITS_VAL(BL_RD_REG(PDS_BASE, PDS_CFG_PDS_KEY_SCAN), PDS_CR_PDS_GPIO_SET_INT_MASK) != 0xFF) {
        tmpVal |= (1 << 20); //enable pds gpio wakeup for pds
    }

    if (!(BL_RD_REG(GLB_BASE, GLB_GPIO_INT_MASK1) == 0xffffffff)) {
        tmpVal |= (1 << 19); //enable glb gpio wakeup for pds
    }

    if (p->sleepTime) {
        tmpVal |= (1 << 17); //unmask pds sleep time wakeup
    }
    BL_WR_REG(PDS_BASE, PDS_INT, tmpVal);

    PDS_Set_All_GPIO_IntClear();
    PDS_IntClear();

    /* enable PDS interrupt to wakeup CPU (PDS1:CPU not powerdown, CPU __WFI) */
    CPU_Interrupt_Enable(PDS_WAKEUP_IRQn);

    switch (p->pdsLevel) {
        case PM_PDS_LEVEL_31:
            pPdsCfg = &pdsCfgLevel31;
            break;
        default:
            return;
    }

    if (p->powerDownFlash) {
        HBN_Power_Down_Flash((SPI_Flash_Cfg_Type *)*p->flashCfg);
        /* turn_off_ext_flash_pin, GPIO23 - GPIO28 */
        for (uint32_t pin = 23; pin <= 28; pin++) {
            GLB_GPIO_Set_HZ(pin);
        }

        /* SF io select from efuse value */
#if 0 //TODO
        SF_Ctrl_Pin_Select sf_pin_select = 0;
        /* get sw uasge 0 */
        EF_Ctrl_Read_Sw_Usage(0, (uint32_t *)&tmpVal);
        /* get flash type */
        sf_pin_select = (tmpVal >> 14) & 0x3f;
#endif

        pPdsCfg->pdsCtl.puFlash = 1;
    }

    if (p->turnoffDLL) {
        GLB_Set_System_CLK(GLB_DLL_XTAL_NONE, GLB_SYS_CLK_RC32M);
        AON_Power_Off_XTAL();
        GLB_Power_Off_DLL();
        // PDS_Update_Flash_Ctrl_Setting(0);
    }

    if (p->pdsLdoEn) {
        pPdsCfg->pdsCtl.pdsLdoVselEn = 1;
        /* PDS mode LDO level, if cr_pds_ldo_vsel_en =1 */
        tmpVal = BL_RD_REG(HBN_BASE, HBN_CTL);
        tmpVal = BL_SET_REG_BITS_VAL(tmpVal, HBN_LDO11_AON_VOUT_SEL, p->ldoLevel);
        BL_WR_REG(HBN_BASE, HBN_CTL, tmpVal);
    }

    if (p->ocramRetetion) {
        uint32_t pds_ram = 0x10001F;
        PDS_RAM_CFG_Type *ramCfg = (PDS_RAM_CFG_Type *)&pds_ram;
        PDS_RAM_Config(ramCfg);
    }

    if (!(p->turnOffRF)) {
        pPdsCfg->pdsCtl.pdsCtlRfSel = 0;
    }

    /* config  ldo11soc_sstart_delay_aon =2 , cr_pds_pd_ldo11=0 to speedup ldo11soc_rdy_aon */
    AON_Set_LDO11_SOC_Sstart_Delay(0x2);

    PDS_Default_Level_Config(pPdsCfg, p->sleepTime);

    __WFI(); /* if(.wfiMask==0){CPU won't power down until PDS module had seen __wfi} */

    cpu_global_irq_enable();

    if (p->turnoffDLL) {
        GLB_Set_System_CLK(XTAL_TYPE, BSP_ROOT_CLOCK_SOURCE);
        // PDS_Update_Flash_Ctrl_Setting(1);
    }

    if (p->powerDownFlash) {
        pm_power_on_flash(p->flashCfg, p->flashContRead);
    }
}

void pm_pds_mode_enter(enum pm_pds_sleep_level pds_level, uint32_t sleep_time)
{
    PM_PDS_CFG_Type cfg = {
        .pdsLevel = pds_level,                    /*!< PDS level */
        .turnOffRF = PM_PDS_RF_POWER_OFF,         /*!< Wheather turn off RF */
        .powerDownFlash = PM_PDS_FLASH_POWER_OFF, /*!< Whether power down flash */
        .ocramRetetion = 1,                       /*!< Whether OCRAM Retention */
        .turnoffPLL = 0,                          /*!< Whether trun off PLL */
        .turnoffDLL = PM_PDS_DLL_POWER_OFF,       /*!< Whether trun off PLL */
        .pdsLdoEn = PM_PDS_PLL_POWER_OFF,         /*!< Whether enable PDS control LDO */
        .flashContRead = 0,                       /*!< Whether enable flash continue read */
        .sleepTime = sleep_time,                  /*!< PDS sleep time */
        .fastRecovery = 0,                        /*!< Whether enable fast recovery */
        .flashCfg = NULL,                         /*!< Flash config pointer, used when power down flash */
        .ldoLevel = PM_PDS_LDO_LEVEL_DEFAULT,     /*!< LDO level */
        .preCbFun = NULL,                         /*!< Pre callback function */
        .postCbFun = NULL,                        /*!< Post callback function */
    };

    uint32_t flash_cfg_len;

    flash_get_cfg((uint8_t **)&flash_cfg, &flash_cfg_len);
    cfg.flashCfg = (uint32_t *)&flash_cfg;

    pm_pds_enable((uint32_t *)&cfg);
}
void pm_hbn_mode_enter(enum pm_hbn_sleep_level hbn_level, uint8_t sleep_time)
{
    HBN_APP_CFG_Type cfg = {
        .useXtal32k = 0,                      /*!< Whether use xtal 32K as 32K clock source,otherwise use rc32k */
        .sleepTime = sleep_time,              /*!< HBN sleep time */
        .hw_pu_pd_en = 0,                     /*!< Always disable HBN pin pull up/down to reduce PDS/HBN current */
        .flashCfg = NULL,                     /*!< Flash config pointer, used when power down flash */
        .hbnLevel = hbn_level,                /*!< HBN level */
        .ldoLevel = PM_HBN_LDO_LEVEL_DEFAULT, /*!< LDO level */
    };

    uint32_t flash_cfg_len;
    flash_get_cfg((uint8_t **)&cfg.flashCfg, &flash_cfg_len);

    HBN_Mode_Enter(&cfg);
}
