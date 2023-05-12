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
#include <bl702l_romdriver.h>
#include <bl702l_glb.h>
#include <bl702l_timer.h>

#include <stdio.h>
#include <stdbool.h>
#include "bl_sys.h"
#include "bl_flash.h"
#include "bl_hbn.h"

volatile bool sys_log_all_enable = true;
static BL_RST_REASON_E sys_rstinfo = BL_RST_POR;
ATTR_HBN_NOINIT_SECTION static int wdt_triger_counter;

void bl_sys_rstinfo_process(void)
{
    uint8_t wdt_rst;
    uint8_t hbn_rst;
    uint8_t pds_rst;

    // get reset status
    wdt_rst = WDT_GetResetStatus(WDT0_ID);
    hbn_rst = BL_GET_REG_BITS_VAL(BL_RD_REG(HBN_BASE,HBN_GLB),HBN_RESET_EVENT);
    pds_rst = BL_GET_REG_BITS_VAL(BL_RD_REG(PDS_BASE,PDS_INT),PDS_RESET_EVENT);

    // clear reset status
    WDT_ClearResetStatus(WDT0_ID);
    HBN_Clear_Reset_Event();
    PDS_Clear_Reset_Event();

    // check reset source
    if(wdt_rst){
        sys_rstinfo = BL_RST_WDT;
    }else if(hbn_rst == 0x0D){
        sys_rstinfo = BL_RST_BOR;
    }else if(hbn_rst == 0x04){
        sys_rstinfo = BL_RST_HBN;
    }else if(pds_rst == 0x07){
        sys_rstinfo = BL_RST_POR;
    }else{
        sys_rstinfo = BL_RST_SOFTWARE;
    }
}

BL_RST_REASON_E bl_sys_rstinfo_get(void)
{
    return sys_rstinfo;
}

int bl_sys_logall_enable(void)
{
    sys_log_all_enable = true;
    return 0;
}

int bl_sys_logall_disable(void)
{
    sys_log_all_enable = false;
    return 0;
}

int bl_sys_reset_por(void)
{
    __disable_irq();
    GLB_SW_POR_Reset();
    while (1) {
        /*empty dead loop*/
    }

    return 0;
}

void bl_sys_reset_system(void)
{
    __disable_irq();
    GLB_SW_System_Reset();
    while (1) {
        /*empty dead loop*/
    }
}

int bl_sys_isxipaddr(uint32_t addr)
{
    if (((addr & 0xFF000000) == 0x23000000) || ((addr & 0xFF000000) == 0x43000000)) {
        return 1;
    }
    return 0;
}

int bl_sys_em_config(void)
{
    extern uint8_t __LD_CONFIG_EM_SEL;
    volatile uint32_t em_size;

    em_size = (uint32_t)&__LD_CONFIG_EM_SEL;

    switch (em_size) {
        case 0 * 1024:
        {
            GLB_Set_EM_Sel(GLB_EM_0KB);
        }
        break;
        case 8 * 1024:
        {
            GLB_Set_EM_Sel(GLB_EM_8KB);
        }
        break;
        case 16 * 1024:
        {
            GLB_Set_EM_Sel(GLB_EM_16KB);
        }
        break;
        default:
        {
            /*nothing here*/
        }
    }

    return 0;
}

int bl_sys_cache_config(void)
{
    extern uint8_t __CACHE_SIZE;
    volatile uint32_t cache_size;

    cache_size = (uint32_t)&__CACHE_SIZE;

    switch (cache_size) {
        case 0 * 1024:
        {
            RomDriver_L1C_Cache_Enable_Set(0x0F);
        }
        break;
        case 4 * 1024:
        {
            RomDriver_L1C_Cache_Enable_Set(0x07);
        }
        break;
        case 8 * 1024:
        {
            RomDriver_L1C_Cache_Enable_Set(0x03);
        }
        break;
        case 12 * 1024:
        {
            RomDriver_L1C_Cache_Enable_Set(0x01);
        }
        break;
        case 16 * 1024:
        {
            RomDriver_L1C_Cache_Enable_Set(0x00);
        }
        break;
        default:
        {
            /*nothing here*/
        }
    }

    return 0;
}

int bl_sys_early_init(void)
{
    bl_sys_rstinfo_process();
    if(BL_RST_WDT == bl_sys_rstinfo_get()){
        wdt_triger_counter++;
    }
    else{
        wdt_triger_counter = 0;
    }

//    bl_flash_init();  // will get flash cfg in start.S

    extern void newlibc_init(void);
    newlibc_init();

    extern void freertos_risc_v_trap_handler(void); //freertos_riscv_ram/portable/GCC/RISC-V/portASM.S
    write_csr(mtvec, &freertos_risc_v_trap_handler);

    HBN_Hw_Pu_Pd_Cfg(DISABLE);

//    HBN_Set_Ldo11_All_Vout(HBN_LDO_LEVEL_1P00V);

#if !(defined(CFG_PDS_ENABLE) || defined(CFG_HBN_ENABLE))
    GLB_Set_System_CLK(GLB_DLL_XTAL_32M, GLB_SYS_CLK_DLL128M);
    HBN_Set_XCLK_CLK_Sel(HBN_XCLK_CLK_XTAL);
    GLB_Set_SF_CLK(1, GLB_SFLASH_CLK_BCLK, 0);
#else
    GLB_Set_System_CLK(GLB_DLL_XTAL_32M, GLB_SYS_CLK_XTAL);
    GLB_Set_SF_CLK(1, GLB_SFLASH_CLK_XCLK, 0);
#endif

    GLB_Set_MTimer_CLK(1, GLB_MTIMER_CLK_XCLK, 15);

    AON_Set_Xtal_CapCode(118, 118);
    AON_Set_Xtal_CapCode_Extra(1);

    GLB_AHB_MCU_Software_Reset(GLB_AHB_MCU_SW_UART0);

    /*debuger may NOT ready don't print anything*/
    return 0;
}

int bl_sys_init(void)
{
//    bl_sys_em_config();  // will be called in setup_heap() in start.S

    return 0;
}

int bl_sys_wdt_rst_count_get()
{
    return wdt_triger_counter;
}

