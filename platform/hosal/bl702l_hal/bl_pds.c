/*
 * Copyright (c) 2016-2023 Bouffalolab.
 *
 * This file is part of
 *     *** Bouffalolab Software Dev Kit ***
 *      (see www.bouffalolab.com).
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
 */
#include "bl_pds.h"
#include "bl_flash.h"


/* PDS31 Configuration */
static PDS_DEFAULT_LV_CFG_Type pdsCfgLevel31 = {
    .pdsCtl = {
        .pdsStart = 1,
        .sleepForever = 0,
        .xtalForceOff = 0,
        .waitRC32mRdy = 0,
        .avdd14Off = 1,
        .bgSysOff = 1,
        .glbRstProtect = 0,
        .puFlash = 0,
        .clkOff = 1,
        .memStby = 1,
        .swPuFlash = 1,
        .isolation = 1,
        .waitXtalRdy = 0,
        .pdsPwrOff = 1,
        .xtalOff = 1,
        .socEnbForceOn = 1,
        .pdsRstSocEn = 1,
        .pdsRC32mOn = 0,
        .pdsLdoVselEn = 1,
        .xtalCntRC32kEn = 0,
        .cpu0WfiMask = 0,
        .ldo11Off = 1,
        .pdsCtlRfSel = 1,
        .xtalDegEn = 0,
        .bleWakeupReqEn = 0,
        .swGpioIsoMod = 0,
        .pdsClkOff = 0,
        .pdsGpioKeep = 7,
        .pdsGpioIsoMod = 0,
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

/* Flash Pin Configuration, will get from efuse */
static uint8_t flashPinCfg;

/* Flash Configuration Pointer, will get from bl_flash_get_flashCfg() */
static SPI_Flash_Cfg_Type *flashCfgPtr;


ATTR_PDS_SECTION ATTR_NOINLINE
int bl_pds_pre_process(uint32_t pdsLevel, uint32_t pdsSleepCycles, uint32_t *store, uint32_t arg[])
{
    __disable_irq();
    
    if(pdsLevel < 4){
        *store = 0;
    }
    
    // Power down flash
    RomDriver_HBN_Power_Down_Flash(flashCfgPtr);
    
    // Select RC32M
    RomDriver_HBN_Set_ROOT_CLK_Sel(HBN_ROOT_CLK_RC32M);
    RomDriver_GLB_Power_Off_DLL();
    RomDriver_AON_Power_Off_XTAL();
    
    return 0;
}

ATTR_PDS_SECTION ATTR_NOINLINE
int bl_pds_start(uint32_t pdsLevel, uint32_t pdsSleepCycles)
{
    // Make HBNRAM retention
    BL_WR_REG(HBN_BASE, HBN_SRAM, (0x3<<0)|(0x1<<3)|(0x1<<6)|(0x0<<7));
    
    // Clear HBN_IRQ status
    BL_WR_REG(HBN_BASE, HBN_IRQ_CLR, 0xFFFFFFFF);
    
    // Enter PDS mode
    RomDriver_PDS_Default_Level_Config(&pdsCfgLevel31, pdsSleepCycles);
    __WFI();
    
    return 0;
}

ATTR_PDS_SECTION ATTR_NOINLINE
int bl_pds_post_process(uint32_t pdsLevel, uint32_t pdsSleepCycles, uint32_t reset, uint32_t arg[])
{
    // For pdsLevel >=4, clock and flash will be configured in fast boot entry
    if(pdsLevel < 4){
        // Select XTAL32M
        RomDriver_AON_Power_On_XTAL();
        RomDriver_HBN_Set_ROOT_CLK_Sel(HBN_ROOT_CLK_XTAL);
        
        // Power up flash
        RomDriver_SF_Cfg_Init_Flash_Gpio(flashPinCfg, 1);
        RomDriver_SF_Ctrl_Set_Owner(SF_CTRL_OWNER_SAHB);
        RomDriver_SFlash_Restore_From_Powerdown(flashCfgPtr, flashCfgPtr->cReadSupport);
    }
    
    __enable_irq();
    
    return 0;
}


void bl_pds_init(void)
{
    Efuse_Device_Info_Type devInfo;
    
    // Get flash pin configuration from efuse
    EF_Ctrl_Read_Device_Info(&devInfo);
    if(devInfo.flash_cfg == 0){
        flashPinCfg = SF_CTRL_SEL_EXTERNAL_FLASH;
    }else{
        if(devInfo.sf_reverse == 0){
            flashPinCfg = devInfo.sf_swap_cfg + 1;
        }else{
            flashPinCfg = devInfo.sf_swap_cfg + 5;
        }
    }
    
    // Get flash configuration pointer
    flashCfgPtr = (SPI_Flash_Cfg_Type *)bl_flash_get_flashCfg();
    
    // Configure PDS interrupt
    BL_WR_REG(PDS_BASE, PDS_INT, 0x1<<17);
    
    // Overwrite default soft start delay (default 0, which may cause wakeup failure)
    AON_Set_LDO11_SOC_Sstart_Delay(2);
    
    // Power off XTAL32K if not used (default on, but HBN_XTAL32K_HIZ_EN=1, keep HBN_XTAL32K_HIZ_EN=1)
    if(BL_GET_REG_BITS_VAL(BL_RD_REG(HBN_BASE, HBN_GLB), HBN_F32K_SEL) != HBN_32K_XTAL){
        HBN_Power_Off_Xtal_32K();
    }
    
    // Disable HBN pin pull up/down to reduce PDS/HBN current
    HBN_Hw_Pu_Pd_Cfg(DISABLE);
    
    // Disable HBN pin IE/SMT
    HBN_Aon_Pad_IeSmt_Cfg(0);
    
    // Disable HBN pin wakeup
    HBN_Pin_WakeUp_Mask(0x3FF);
}

void bl_pds_fastboot_cfg(uint32_t addr)
{
    HBN_Set_Wakeup_Addr(addr);
    HBN_Set_Status_Flag(HBN_STATUS_ENTER_FLAG);
}

ATTR_PDS_SECTION ATTR_NOINLINE
void bl_pds_enter_do(uint32_t pdsLevel, uint32_t pdsSleepCycles, uint32_t store, uint32_t *reset)
{
    bl_pds_start(pdsLevel, pdsSleepCycles);
    
    *reset = 0;
}

ATTR_PDS_SECTION
int bl_pds_enter(uint32_t pdsLevel, uint32_t pdsSleepCycles)
{
    uint32_t store = 1;  // set whether cpu registers need store
    uint32_t reset = 1;  // get whether cpu is reset
    uint32_t arg[4];
    int status;
    
    // Pre-process
    status = bl_pds_pre_process(pdsLevel, pdsSleepCycles, &store, arg);
    if(status != 0){
        return status;
    }
    
    // Enter PDS
    bl_pds_enter_do(pdsLevel, pdsSleepCycles, store, &reset);
    
    // Post-process
    status = bl_pds_post_process(pdsLevel, pdsSleepCycles, reset, arg);
    if(status != 0){
        return status;
    }
    
    return 0;
}
