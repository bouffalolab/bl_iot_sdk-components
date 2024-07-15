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

BL_CHIP_REVISION_E bl_sys_chip_revision_get(void)
{
    return (*(volatile uint32_t *)0x40007074 >> 14) & 0x0F;
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

int bl_sys_run_at_max_speed(void)
{
    unsigned long mstatus_tmp;
    mstatus_tmp = read_csr(mstatus);
    clear_csr(mstatus, MSTATUS_MIE);

    GLB_Set_System_CLK(GLB_DLL_XTAL_32M, GLB_SYS_CLK_DLL128M);
    HBN_Set_XCLK_CLK_Sel(HBN_XCLK_CLK_XTAL);
    GLB_Set_SF_CLK(1, GLB_SFLASH_CLK_42P67M, 0);

    write_csr(mstatus, mstatus_tmp);

    return 0;
}

int bl_sys_run_at_normal_speed(void)
{
    unsigned long mstatus_tmp;
    mstatus_tmp = read_csr(mstatus);
    clear_csr(mstatus, MSTATUS_MIE);

    GLB_Set_System_CLK(GLB_DLL_XTAL_32M, GLB_SYS_CLK_XTAL);
    GLB_Set_SF_CLK(1, GLB_SFLASH_CLK_XCLK, 0);

    write_csr(mstatus, mstatus_tmp);

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
    bl_sys_run_at_max_speed();
#else
    bl_sys_run_at_normal_speed();
#endif

    GLB_Set_MTimer_CLK(1, GLB_MTIMER_CLK_XCLK, 15);

    AON_Set_Xtal_CapCode(118, 118);
    AON_Set_Xtal_CapCode_Extra(1);

    GLB_AHB_MCU_Software_Reset(GLB_AHB_MCU_SW_UART0);

    GLB_GPIO_O_Latch_Mode_Set(1);

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

/*
 * @brief: perform f(data) using stack referred by stacktop,
 *         not its caller's.
 * @param: f - the function to be called.
 *         data - the parameter of f.
 *         stacktop - the top of the new stack.
 * @return: none.
 */
void bl_function_call_with_stack(void (*f)(void *data), void *data, void *stacktop)
{
    /*
     * calling convention
     * register saver
     * x0       -
     * ra       caller
     * sp       callee
     * gp       -
     * tp       -
     * t0       caller
     * t1-2     caller
     * fp       callee
     * s1       callee
     * a0-1     caller
     * a2-7     caller
     * s2-11    callee
     * t3-6     caller
     */
    __asm__ __volatile__ (
        "addi   sp, sp, -12          \n\t"
        "sw     ra, 8(sp)            \n\t"
        "sw     s0, 4(sp)            \n\t"
        "sw     s1, 0(sp)            \n\t"
        "addi   s0, sp, 12           \n\t"

        "mv     s1, sp               \n\t"
        "mv     t0, a0               \n\t"
        "mv     a0, a1               \n\t"
        "mv     sp, a2               \n\t"
        "jalr   t0                   \n\t"
        "mv     sp, s1               \n\t"

        "lw     s1, 0(sp)            \n\t"
        "lw     s0, 4(sp)            \n\t"
        "lw     ra, 8(sp)            \n\t"
        "addi   sp, sp, 12           \n\t"
    );
}
