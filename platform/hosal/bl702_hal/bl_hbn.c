#include "bl_hbn.h"
#include "bl_flash.h"
#include "bl_rtc.h"
#include "hosal_uart.h"


//#define HBN_TEST
//#define GPIO_WAKEUP_TEST
//#define ACOMP0_WAKEUP_TEST
//#define ACOMP1_WAKEUP_TEST

#if defined(GPIO_WAKEUP_TEST)
static uint8_t test_pin_list[] = {9};
#endif
#if defined(ACOMP0_WAKEUP_TEST)
static uint8_t test_acomp0_ch = 0;  // GPIO8
#endif
#if defined(ACOMP1_WAKEUP_TEST)
static uint8_t test_acomp1_ch = 6;  // GPIO7
#endif


#define CGEN_CFG0                  (~(uint8_t)((1<<1)|(1<<2)|(1<<4)))  // do not gate DMA[3], which affects M154_AES
#define CGEN_CFG1                  (~(uint32_t)((1<<2)|(1<<3)|(1<<4)|(1<<5)|(1<<7)|(1<<12)|(1<<13)|(0x7FDE<<16)))  // do not gate sf_ctrl[11], uart0[16], timer[21]


/* Device Information, will get from efuse */
ATTR_HBN_NOINIT_SECTION static Efuse_Device_Info_Type devInfo;

/* PSRAM IO Configuration, will get according to device information */
ATTR_HBN_NOINIT_SECTION static uint8_t psramIoCfg;

/* Cache Way Disable, will get from l1c register */
ATTR_HBN_NOINIT_SECTION static uint8_t cacheWayDisable;

/* EM Select, will get from glb register */
ATTR_HBN_NOINIT_SECTION static uint8_t emSel;

/* Flash Configuration, will get from bl_flash_get_flashCfg() */
ATTR_HBN_NOINIT_SECTION static SPI_Flash_Cfg_Type flashCfg;

/* Flash Image Offset, will get from SF_Ctrl_Get_Flash_Image_Offset() */
ATTR_HBN_NOINIT_SECTION static uint32_t flashImageOffset;

/* Flash Continuous Read, will get based on flash configuration */
ATTR_HBN_NOINIT_SECTION static uint8_t flashContRead;

/* SF Control Configuration, will get based on flash configuration */
ATTR_HBN_NOINIT_SECTION static SF_Ctrl_Cfg_Type sfCtrlCfg;

/* HBN IRQ Status, will get from hbn register after wakeup */
ATTR_HBN_NOINIT_SECTION static uint32_t hbnIrqStatus;

/* HBN Wakeup Time, will get in rtc cycles after wakeup */
ATTR_HBN_NOINIT_SECTION static uint64_t hbnWakeupTime;


/* Private Functions */
void bl_hbn_get_sf_ctrl_cfg(SPI_Flash_Cfg_Type *pFlashCfg, SF_Ctrl_Cfg_Type *pSfCtrlCfg)
{
    const uint8_t delay[8] = {0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe};
    uint8_t index;
    
    pSfCtrlCfg->owner = SF_CTRL_OWNER_SAHB;
    
    /* bit0-3 for clk delay */
    pSfCtrlCfg->clkDelay = pFlashCfg->clkDelay & 0x0f;
    
    /* bit4-6 for do delay */
    index = (pFlashCfg->clkDelay >> 4) & 0x07;
    pSfCtrlCfg->doDelay = delay[index];
    
    /* bit0 for clk invert */
    pSfCtrlCfg->clkInvert = pFlashCfg->clkInvert & 0x01;
    
    /* bit1 for rx clk invert */
    pSfCtrlCfg->rxClkInvert=(pFlashCfg->clkInvert >> 1) & 0x01;
    
    /* bit2-4 for di delay */
    index = (pFlashCfg->clkInvert >> 2) & 0x07;
    pSfCtrlCfg->diDelay = delay[index];
    
    /* bit5-7 for oe delay */
    index = (pFlashCfg->clkInvert >> 5) & 0x07;
    pSfCtrlCfg->oeDelay = delay[index];
}

ATTR_HBN_CODE_SECTION
void bl_hbn_restore_sf(void)
{
    // Clear flash pads pull configuration
    *(volatile uint32_t *)0x4000F02C &= ~(uint32_t)((0x3F<<16)|(0x3F<<24));
    
    // Initialize flash gpio
    RomDriver_SF_Cfg_Init_Flash_Gpio((devInfo.flash_cfg<<2)|devInfo.sf_swap_cfg, 0);
    
    // Patch: BL706C-22
    *(volatile uint32_t *)0x40000130 |= (1U << 16);  // enable GPIO25 input
    *(volatile uint32_t *)0x40000134 |= (1U << 16);  // enable GPIO27 input
    
    // Patch: restore psram io configuration
    BL_WR_REG(GLB_BASE, GLB_GPIO_USE_PSRAM__IO, psramIoCfg);
}

ATTR_HBN_CODE_SECTION
void bl_hbn_restore_flash(SF_Ctrl_Cfg_Type *pSfCtrlCfg, SPI_Flash_Cfg_Type *pFlashCfg, uint32_t flashImageOffset, uint8_t flashContRead, uint8_t cacheWayDisable)
{
    uint32_t tmp[1];
    
    RomDriver_SFlash_Init(pSfCtrlCfg);
    
    RomDriver_SFlash_Releae_Powerdown(pFlashCfg);
    RomDriver_BL702_Delay_US(pFlashCfg->pdDelay);
    
    RomDriver_SFlash_Reset_Continue_Read(pFlashCfg);
    
    RomDriver_SFlash_Software_Reset(pFlashCfg);
    
    RomDriver_SFlash_Write_Enable(pFlashCfg);
    
    RomDriver_SFlash_DisableBurstWrap(pFlashCfg);
    
    RomDriver_SFlash_SetSPIMode(SF_CTRL_SPI_MODE);
    
    if((pFlashCfg->ioMode&0x0f)==SF_CTRL_QO_MODE||(pFlashCfg->ioMode&0x0f)==SF_CTRL_QIO_MODE){
        RomDriver_SFlash_Qspi_Enable(pFlashCfg);
    }
    
    if(((pFlashCfg->ioMode>>4)&0x01)==1){
        RomDriver_L1C_Set_Wrap(DISABLE);
    }else{
        RomDriver_L1C_Set_Wrap(ENABLE);
        RomDriver_SFlash_Write_Enable(pFlashCfg);
        if((pFlashCfg->ioMode&0x0f)==SF_CTRL_QO_MODE||(pFlashCfg->ioMode&0x0f)==SF_CTRL_QIO_MODE){
            RomDriver_SFlash_SetBurstWrap(pFlashCfg);
        }
    }
    
    if(flashContRead){
        RomDriver_SF_Ctrl_Set_Owner(SF_CTRL_OWNER_SAHB);
        RomDriver_SFlash_Read(pFlashCfg, pFlashCfg->ioMode&0xf, 1, 0x00000000, (uint8_t *)tmp, sizeof(tmp));
    }
    
    RomDriver_SF_Ctrl_Set_Flash_Image_Offset(flashImageOffset);
    RomDriver_SFlash_Cache_Read_Enable(pFlashCfg, pFlashCfg->ioMode&0xf, flashContRead, cacheWayDisable);
}

ATTR_HBN_CODE_SECTION
void bl_hbn_restore(void)
{
#if 0
    volatile int debug = 0;
    while(!debug);
#else
    volatile uint32_t *p = (volatile uint32_t *)0x40000100;
    volatile uint32_t *q = (volatile uint32_t *)0x40000148;
    while(p <= q){
        *p++ = 0x0B000B00;
    }
#endif
    
#if 0
    GLB_GPIO_Cfg_Type gpioCfg;
    GLB_GPIO_Type gpioPin = 22;
    uint32_t *pOut = (uint32_t *)(GLB_BASE + GLB_GPIO_OUTPUT_OFFSET);
    uint32_t pos = gpioPin % 32;
    
    gpioCfg.gpioPin = gpioPin;
    gpioCfg.gpioFun = 11;
    gpioCfg.gpioMode = GPIO_MODE_OUTPUT;
    gpioCfg.pullType = GPIO_PULL_NONE;
    gpioCfg.drive = 0;
    gpioCfg.smtCtrl = 1;
    RomDriver_GLB_GPIO_Init(&gpioCfg);
    
    *pOut |= (1<<pos);
    RomDriver_BL702_Delay_US(100);
    *pOut &= ~(1<<pos);
#endif
    
    // Get hbn irq status
    hbnIrqStatus = BL_RD_REG(HBN_BASE, HBN_IRQ_STAT);
    
    // Power off RF
    *(volatile uint32_t *)(AON_BASE + AON_RF_TOP_AON_OFFSET) &= ~(uint32_t)((0x1<<0)|(0x1<<1)|(0x1<<2));
    
    // Power on XTAL32M, later will check whether it is ready for use
    *(volatile uint32_t *)(AON_BASE + AON_RF_TOP_AON_OFFSET) |= (0x1<<4)|(0x1<<5);
    
    // Switch pka clock
    *(volatile uint32_t *)(GLB_BASE + GLB_SWRST_CFG2_OFFSET) |= (0x1<<24);
    
    // Reset secure engine
    *(volatile uint32_t *)(GLB_BASE + GLB_SWRST_CFG1_OFFSET) &= ~(uint32_t)(0x1<<4);
    
    // Disable peripheral clock
    *(volatile uint32_t *)(GLB_BASE + GLB_CLK_CFG1_OFFSET) &= ~(uint32_t)((0x1<<8)|(0x1<<13)|(0x1<<24)|(0x1<<25)|(0x1<<28));
    *(volatile uint32_t *)(GLB_BASE + GLB_CLK_CFG2_OFFSET) &= ~(uint32_t)((0x1<<4)|(0x1<<23)|(0xFF<<24));
    *(volatile uint32_t *)(GLB_BASE + GLB_CLK_CFG3_OFFSET) &= ~(uint32_t)((0x1<<8)|(0x1<<24));
    
    // Gate peripheral clock
    BL_WR_REG(GLB_BASE, GLB_CGEN_CFG0, CGEN_CFG0);
    BL_WR_REG(GLB_BASE, GLB_CGEN_CFG1, CGEN_CFG1);
    
    // Restore EM select
    BL_WR_REG(GLB_BASE, GLB_SEAM_MISC, emSel);
    
#if 0
    if(hbnIrqStatus & 0x01){
        uint32_t tmp = *(uint32_t *)0x40000110;
        *(uint32_t *)0x40000110 = 0x0B020B00;
        *(uint32_t *)0x40000188 = 0x00000200;
        *(uint32_t *)0x40000190 = 0x00000200;
        RomDriver_BL702_Delay_US(500);
        *(uint32_t *)0x40000190 = 0x00000000;
        *(uint32_t *)0x40000188 = 0x00000000;
        *(uint32_t *)0x40000110 = 0x0B030B00;
        RomDriver_BL702_Delay_US(500);
        hbnIrqStatus &= ~((*(uint32_t *)0x40000180 >> 9) & 0x01);
        *(uint32_t *)0x40000110 = tmp;
    }
#endif
    
    // Configure flash (must use rom driver, since tcm code is lost and flash is power down)
    RomDriver_GLB_Set_SF_CLK(1, 1, 0);
    bl_hbn_restore_sf();
    bl_hbn_restore_flash(&sfCtrlCfg, &flashCfg, flashImageOffset, flashContRead, cacheWayDisable);
    
    // Get wakeup time in rtc cycles after flash restore
    hbnWakeupTime = bl_rtc_get_counter();
    
    // Wait until XTAL32M is ready for use
    while(!BL_IS_REG_BIT_SET(BL_RD_REG(AON_BASE, AON_TSEN), AON_XTAL_RDY));
    RomDriver_BL702_Delay_MS(1);
    
    // Select XTAL32M as root clock
    RomDriver_HBN_Set_ROOT_CLK_Sel(HBN_ROOT_CLK_XTAL);
    
    // Power on RF
    *(volatile uint32_t *)(AON_BASE + AON_RF_TOP_AON_OFFSET) |= (0x1<<0)|(0x1<<1)|(0x1<<2);
    
    // Disable global interrupt
    __disable_irq();
    
    // Call user callback
    bl_hbn_fastboot_done_callback();
    while(1);
}

ATTR_HBN_CODE_SECTION
void bl_hbn_fastboot_entry(void)
{
    __asm__ __volatile__(
            ".option push\n\t"
            ".option norelax\n\t"
            "la gp, __global_pointer$\n\t"
            ".option pop\n\t"
            "li a0, 0x40000000\n\t"
            "sw x0, 0x7C(a0)\n\t"
            "la sp, _sp_main\n\t"
            "call bl_hbn_restore\n\t"
    );
}


/* Public Functions */
void bl_hbn_fastboot_init(void)
{
    // Get device information from efuse
    EF_Ctrl_Read_Device_Info(&devInfo);
    devInfo.flash_cfg &= 0x03;
    
    // Get psram io configuration
    psramIoCfg = ((devInfo.flash_cfg == 1 || devInfo.flash_cfg == 2) && devInfo.psram_cfg != 1) ? 0x3F : 0x00;
    
    // Get cache way disable setting
    cacheWayDisable = BL_GET_REG_BITS_VAL(BL_RD_REG(L1C_BASE, L1C_CONFIG), L1C_WAY_DIS);
    
    // Get EM select
    emSel = BL_RD_REG(GLB_BASE, GLB_SEAM_MISC);
    
    // Get flash configuration
    memcpy(&flashCfg, bl_flash_get_flashCfg(), sizeof(SPI_Flash_Cfg_Type));
    
    // Get flash continuous read setting
    flashContRead = flashCfg.cReadSupport & 0x01;
    
    // Get flash image offset
    flashImageOffset = SF_Ctrl_Get_Flash_Image_Offset();
    
    // Get SF control configuration
    bl_hbn_get_sf_ctrl_cfg(&flashCfg, &sfCtrlCfg);
    
    // Overwrite default soft start delay (default 0, which may cause wakeup failure)
    AON_Set_LDO11_SOC_Sstart_Delay(2);
    
    // Select 32K (RC32K and XTAL32K are both default on)
#ifdef CFG_USE_XTAL32K
    HBN_32K_Sel(HBN_32K_XTAL);
    //HBN_Power_Off_RC32K();
#else
    HBN_32K_Sel(HBN_32K_RC);
    //HBN_Power_Off_Xtal_32K();
#endif
    
    // Disable HBN pin pull up/down to reduce PDS/HBN current
    HBN_Hw_Pu_Pd_Cfg(DISABLE);
    
    // Disable HBN pin IE/SMT
    HBN_Aon_Pad_IeSmt_Cfg(0);
    
    // Disable HBN pin wakeup
    HBN_Pin_WakeUp_Mask(0x1F);
}

void bl_hbn_gpio_wakeup_cfg(uint8_t pin_list[], uint8_t pin_num)
{
    uint8_t hbnWakeupPin = 0;
    int pin;
    int i;
    
    for(i = 0; i < pin_num; i++){
        pin = pin_list[i];
        
        if(pin >= 9 && pin <= 12){
            hbnWakeupPin |= HBN_WAKEUP_GPIO_9 << (pin - 9);
        }
    }
    
    if(hbnWakeupPin == 0){
        HBN_Aon_Pad_IeSmt_Cfg(0);
        HBN_Pin_WakeUp_Mask(0x1F);
    }else{
        HBN_Aon_Pad_IeSmt_Cfg(hbnWakeupPin);
        HBN_Pin_WakeUp_Mask(~hbnWakeupPin & 0x1F);
        HBN_GPIO_INT_Enable(HBN_GPIO_INT_TRIGGER_ASYNC_FALLING_EDGE);
    }
}

void bl_hbn_acomp_wakeup_cfg(uint8_t acomp_id, uint8_t ch_sel, uint8_t edge_sel)
{
    AON_ACOMP_CFG_Type cfg = {
        .muxEn = ENABLE,                                          /*!< ACOMP mux enable */
        .posChanSel = ch_sel,                                     /*!< ACOMP positive channel select */
        .negChanSel = AON_ACOMP_CHAN_0P3125VBAT,                  /*!< ACOMP negtive channel select */
        .levelFactor = AON_ACOMP_LEVEL_FACTOR_1,                  /*!< ACOMP level select factor */
        .biasProg = AON_ACOMP_BIAS_POWER_MODE1,                   /*!< ACOMP bias current control */
        .hysteresisPosVolt = AON_ACOMP_HYSTERESIS_VOLT_50MV,      /*!< ACOMP hysteresis voltage for positive */
        .hysteresisNegVolt = AON_ACOMP_HYSTERESIS_VOLT_50MV,      /*!< ACOMP hysteresis voltage for negtive */
    };
    
    GLB_GPIO_Type acompPinList[8] = {8, 15, 17, 11, 12, 14, 7, 9};
    GLB_GPIO_Func_Init(GPIO_FUN_ANALOG, &acompPinList[ch_sel], 1);
    
    AON_ACOMP_Init((AON_ACOMP_ID_Type)acomp_id, &cfg);
    AON_ACOMP_Enable((AON_ACOMP_ID_Type)acomp_id);
    
    if(edge_sel == HBN_ACOMP_EDGE_RISING){
        if(acomp_id == 0){
            HBN_Enable_AComp0_IRQ(HBN_ACOMP_INT_EDGE_POSEDGE);
            HBN_Disable_AComp0_IRQ(HBN_ACOMP_INT_EDGE_NEGEDGE);
        }else{
            HBN_Enable_AComp1_IRQ(HBN_ACOMP_INT_EDGE_POSEDGE);
            HBN_Disable_AComp1_IRQ(HBN_ACOMP_INT_EDGE_NEGEDGE);
        }
    }else if(edge_sel == HBN_ACOMP_EDGE_FALLING){
        if(acomp_id == 0){
            HBN_Disable_AComp0_IRQ(HBN_ACOMP_INT_EDGE_POSEDGE);
            HBN_Enable_AComp0_IRQ(HBN_ACOMP_INT_EDGE_NEGEDGE);
        }else{
            HBN_Disable_AComp1_IRQ(HBN_ACOMP_INT_EDGE_POSEDGE);
            HBN_Enable_AComp1_IRQ(HBN_ACOMP_INT_EDGE_NEGEDGE);
        }
    }else{
        if(acomp_id == 0){
            HBN_Enable_AComp0_IRQ(HBN_ACOMP_INT_EDGE_POSEDGE);
            HBN_Enable_AComp0_IRQ(HBN_ACOMP_INT_EDGE_NEGEDGE);
        }else{
            HBN_Enable_AComp1_IRQ(HBN_ACOMP_INT_EDGE_POSEDGE);
            HBN_Enable_AComp1_IRQ(HBN_ACOMP_INT_EDGE_NEGEDGE);
        }
    }
}

ATTR_HBN_CODE_SECTION
void bl_hbn_mode_enter(void)
{
    uint32_t tmpVal;
    
#if 0
    // Power down flash
    RomDriver_SF_Ctrl_Set_Owner(SF_CTRL_OWNER_SAHB);
    RomDriver_SFlash_Reset_Continue_Read(&flashCfg);
    RomDriver_SFlash_Powerdown();
#endif
    
    // Pull up flash pads
    if(devInfo.flash_cfg == 0){
        // External SF2 (GPIO23 - GPIO28)
        *(volatile uint32_t *)0x4000F02C |= (0x1F << 16);  // As boot pin, GPIO28 should not be pulled up
    }else if(devInfo.flash_cfg == 3){
        // External SF1 (GPIO17 - GPIO22)
        // Do nothing
    }else{
        // Internal SF2 (GPIO23 - GPIO28)
        // Do nothing
    }
    
    // Select RC32M
    RomDriver_GLB_Set_System_CLK(GLB_DLL_XTAL_NONE, GLB_SYS_CLK_RC32M);
    RomDriver_GLB_Power_Off_DLL();
    RomDriver_AON_Power_Off_XTAL();
    
    // Clear HBN_IRQ status
    BL_WR_REG(HBN_BASE, HBN_IRQ_CLR, 0xFFFFFFFF);
    
    // Enter HBN0
    tmpVal = BL_RD_REG(HBN_BASE, HBN_CTL);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, HBN_LDO11_AON_VOUT_SEL, HBN_LDO_LEVEL_1P00V);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, HBN_LDO11_RT_VOUT_SEL, HBN_LDO_LEVEL_1P00V);
    tmpVal = BL_CLR_REG_BIT(tmpVal, HBN_PWRDN_HBN_CORE);
    tmpVal = BL_CLR_REG_BIT(tmpVal, HBN_PWRDN_HBN_RTC);
    tmpVal = BL_CLR_REG_BIT(tmpVal, HBN_PWR_ON_OPTION);
    tmpVal = BL_SET_REG_BIT(tmpVal, HBN_MODE);
    BL_WR_REG(HBN_BASE, HBN_CTL, tmpVal);
    while(1);
}

void bl_hbn_enter_with_fastboot(uint32_t hbnSleepCycles)
{
    uint32_t valLow, valHigh;
    
    __disable_irq();
    
    if(hbnSleepCycles != 0){
        HBN_Get_RTC_Timer_Val(&valLow, &valHigh);
        
        valLow += hbnSleepCycles;
        if(valLow < hbnSleepCycles){
            valHigh++;
        }
        
        HBN_Set_RTC_Timer(HBN_RTC_INT_DELAY_0T, valLow, valHigh, HBN_RTC_COMP_BIT0_39);
    }
    
    HBN_Set_Wakeup_Addr((uint32_t)bl_hbn_fastboot_entry);
    HBN_Set_Status_Flag(HBN_STATUS_ENTER_FLAG);
    
    bl_hbn_mode_enter();
}

int bl_hbn_get_wakeup_source(void)
{
    // irq_rtc is cleared in bootrom, so we assume wakeup by RTC if not wakeup by GPIO and ACOMP
    if(hbnIrqStatus & 0x1F){
        return HBN_WAKEUP_BY_GPIO;
    }else if(hbnIrqStatus & (1U << 20)){
        return HBN_WAKEUP_BY_ACOMP0;
    }else if(hbnIrqStatus & (1U << 22)){
        return HBN_WAKEUP_BY_ACOMP1;
    }else{
        return HBN_WAKEUP_BY_RTC;
    }
}

uint32_t bl_hbn_get_wakeup_gpio(void)
{
    return (hbnIrqStatus & 0x1F) << 9;
}

uint64_t bl_hbn_get_wakeup_time(void)
{
    return hbnWakeupTime;
}

#if defined(HBN_TEST)
ATTR_HBN_NOINIT_SECTION static uint64_t ref_cnt;
void bl_hbn_test(int need_init)
{
    uint32_t xExpectedSleepTime;
    uint32_t sleepCycles;
    
    if(need_init){
        bl_hbn_fastboot_init();
#if defined(GPIO_WAKEUP_TEST)
        bl_hbn_gpio_wakeup_cfg(test_pin_list, sizeof(test_pin_list));
#endif
#if defined(ACOMP0_WAKEUP_TEST)
        bl_hbn_acomp_wakeup_cfg(0, test_acomp0_ch, HBN_ACOMP_EDGE_BOTH);
#endif
#if defined(ACOMP1_WAKEUP_TEST)
        bl_hbn_acomp_wakeup_cfg(1, test_acomp1_ch, HBN_ACOMP_EDGE_BOTH);
#endif
    }
    
    xExpectedSleepTime = 5000;
    
    printf("[%lu] will sleep: %lu ms\r\n", (uint32_t)bl_rtc_get_timestamp_ms(), xExpectedSleepTime);
    arch_delay_us(100);
    
    ref_cnt = bl_rtc_get_counter();
    
    sleepCycles = xExpectedSleepTime * 32768 / 1000;
    bl_hbn_enter_with_fastboot(sleepCycles);
}
#endif

__attribute__((weak)) void bl_hbn_fastboot_done_callback(void)
{
#if defined(HBN_TEST)
    HOSAL_UART_DEV_DECL(uart_stdio, 0, 14, 15, 2000000);
    hosal_uart_init(&uart_stdio);
    
    printf("HBN_IRQ_STAT: 0x%08lX\r\n", hbnIrqStatus);
    
    int source = bl_hbn_get_wakeup_source();
    uint32_t gpio = bl_hbn_get_wakeup_gpio();
    
    if(source == HBN_WAKEUP_BY_RTC){
        printf("wakeup source: rtc\r\n");
    }else if(source == HBN_WAKEUP_BY_GPIO){
        printf("wakeup source: gpio -> 0x%08lX\r\n", gpio);
    }else if(source == HBN_WAKEUP_BY_ACOMP0){
        printf("wakeup source: acomp0\r\n");
    }else if(source == HBN_WAKEUP_BY_ACOMP1){
        printf("wakeup source: acomp1\r\n");
    }else{
        printf("wakeup source: unknown\r\n");
    }
    
    uint32_t sleepTime = (uint32_t)bl_rtc_get_delta_time_ms(ref_cnt);
    printf("[%lu] actually sleep: %lu ms\r\n", (uint32_t)bl_rtc_get_timestamp_ms(), sleepTime);
    
    bl_hbn_test(0);
#endif
}
