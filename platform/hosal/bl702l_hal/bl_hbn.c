#include <bl_hbn.h>
#include <bl_irq.h>
#include <bl_flash.h>
#include <bl_rtc.h>
#include <hosal_uart.h>
#include <utils_log.h>


/* ACOMP Pin List */
static const GLB_GPIO_Type acompPinList[8] = {14, 15, 17, 18, 19, 20, 21, 7};


/* Flash Pin Configuration, will get from efuse */
ATTR_HBN_NOINIT_SECTION static uint8_t flashPinCfg;

/* Cache Way Disable, will get from l1c register */
ATTR_HBN_NOINIT_SECTION static uint8_t cacheWayDisable;

/* EM Select, will get from glb register */
ATTR_HBN_NOINIT_SECTION static uint8_t emSel;

/* Flash Configuration, will get from bl_flash_get_flashCfg() */
ATTR_HBN_NOINIT_SECTION static SPI_Flash_Cfg_Type flashCfg;

/* Flash Image Offset, will get from SF_Ctrl_Get_Flash_Image_Offset() */
ATTR_HBN_NOINIT_SECTION static uint32_t flashImageOffset;

/* SF Control Configuration, will set based on flash configuration */
ATTR_HBN_NOINIT_SECTION static SF_Ctrl_Cfg_Type sfCtrlCfg;

/* HBN IRQ Status, will get from hbn register after wakeup */
ATTR_HBN_NOINIT_SECTION static uint32_t hbnIrqStatus;

/* HBN Wakeup Time, will get in rtc cycles after wakeup */
ATTR_HBN_NOINIT_SECTION static uint64_t hbnWakeupTime;


static void bl_hbn_set_sf_ctrl(SPI_Flash_Cfg_Type *pFlashCfg)
{
    uint8_t index;
    uint8_t delay[8] = {0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe};
    
    sfCtrlCfg.owner = SF_CTRL_OWNER_SAHB;
    
    /* bit0-3 for clk delay */
    sfCtrlCfg.clkDelay = pFlashCfg->clkDelay&0x0f;
    
    /* bit4-6 for do delay */
    index = (pFlashCfg->clkDelay>>4)&0x07;
    sfCtrlCfg.doDelay = delay[index];
    
    /* bit0 for clk invert */
    sfCtrlCfg.clkInvert = pFlashCfg->clkInvert&0x01;
    
    /* bit1 for rx clk invert */
    sfCtrlCfg.rxClkInvert=(pFlashCfg->clkInvert>>1)&0x01;
    
    /* bit2-4 for di delay */
    index = (pFlashCfg->clkInvert>>2)&0x07;
    sfCtrlCfg.diDelay = delay[index];
    
    /* bit5-7 for oe delay */
    index = (pFlashCfg->clkInvert>>5)&0x07;
    sfCtrlCfg.oeDelay = delay[index];
}


void bl_hbn_fastboot_init(void)
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
    
    // Get cache way disable setting
    cacheWayDisable = BL_GET_REG_BITS_VAL(BL_RD_REG(L1C_BASE, L1C_CONFIG), L1C_WAY_DIS);
    
    // Get EM select
    emSel = BL_RD_REG(GLB_BASE, GLB_SEAM_MISC);
    
    // Get flash configuration
    memcpy(&flashCfg, bl_flash_get_flashCfg(), sizeof(SPI_Flash_Cfg_Type));
    
    // Get flash image offset
    flashImageOffset = SF_Ctrl_Get_Flash_Image_Offset();
    
    // Set SF control configuration
    bl_hbn_set_sf_ctrl(&flashCfg);
    
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

void bl_hbn_gpio_wakeup_cfg(uint8_t pin_list[], uint8_t pin_num, uint8_t edge_sel)
{
    uint16_t hbnWakeupPin = 0;
    int pin;
    int i;
    
    for(i = 0; i < pin_num; i++){
        pin = pin_list[i];
        
        if(pin >= 9 && pin <= 13){
            hbnWakeupPin |= HBN_WAKEUP_GPIO_9 << (pin - 9);
        }else if(pin >= 30 && pin <= 31){
            hbnWakeupPin |= HBN_WAKEUP_GPIO_30 << (pin - 30);
        }else if(pin == 8){
            hbnWakeupPin |= HBN_WAKEUP_GPIO_8;
        }else if(pin == 14){
            hbnWakeupPin |= HBN_WAKEUP_GPIO_14;
        }else if(pin == 22){
            hbnWakeupPin |= HBN_WAKEUP_GPIO_22;
        }else{
            continue;
        }
    }
    
    // Configure hbn wakeup pins
    hbnWakeupPin |= 0xFC00;  // workaround due to bug in HBN_GPIO_Wakeup_Set
    HBN_GPIO_Wakeup_Set(hbnWakeupPin, edge_sel);
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
    
    GLB_GPIO_Func_Init(GPIO_FUN_ANALOG, (GLB_GPIO_Type *)&acompPinList[ch_sel], 1);
    
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

// must be placed in hbncode section
static void ATTR_HBN_CODE_SECTION bl_hbn_restore_flash(SF_Ctrl_Cfg_Type *pSfCtrlCfg, SPI_Flash_Cfg_Type *pFlashCfg, uint32_t flashImageOffset, uint8_t cacheWayDisable)
{
    uint32_t tmp[1];
    
    RomDriver_SFlash_Init(pSfCtrlCfg);
    
    RomDriver_SFlash_Releae_Powerdown(pFlashCfg);
    
    RomDriver_SFlash_Reset_Continue_Read(pFlashCfg);
    
    RomDriver_SFlash_Software_Reset(pFlashCfg);
    
    RomDriver_SFlash_Write_Enable(pFlashCfg);
    
    RomDriver_SFlash_DisableBurstWrap(pFlashCfg);
    
    RomDriver_SFlash_SetSPIMode(SF_CTRL_SPI_MODE);
    
    RomDriver_SF_Ctrl_Set_Flash_Image_Offset(0);
    
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
    
    if(pFlashCfg->cReadSupport){
        RomDriver_SF_Ctrl_Set_Owner(SF_CTRL_OWNER_SAHB);
        RomDriver_SFlash_Read(pFlashCfg, pFlashCfg->ioMode&0xf, 1, 0x00000000, (uint8_t *)tmp, sizeof(tmp));
    }
    
    RomDriver_SF_Ctrl_Set_Flash_Image_Offset(flashImageOffset);
    RomDriver_SFlash_Cache_Read_Enable(pFlashCfg, pFlashCfg->ioMode&0xf, pFlashCfg->cReadSupport, cacheWayDisable);
}

// can be placed in flash, here placed in hbncode section to reduce fast boot time
static void ATTR_NOINLINE ATTR_HBN_CODE_SECTION bl_hbn_restore_data(uint32_t src, uint32_t dst, uint32_t end)
{
    while(dst < end){
        *(uint32_t *)dst = *(uint32_t *)src;
        src += 4;
        dst += 4;
    }
}

// can be placed in flash, here placed in hbncode section to reduce fast boot time
static void ATTR_NOINLINE ATTR_HBN_CODE_SECTION bl_hbn_restore_bss(uint32_t dst, uint32_t end)
{
    while(dst < end){
        *(uint32_t *)dst = 0;
        dst += 4;
    }
}

// must be placed in hbncode section
static void ATTR_HBN_CODE_SECTION bl_hbn_fastboot_entry(void)
{
    __asm__ __volatile__(
            ".option push\n\t"
            ".option norelax\n\t"
            "la gp, __global_pointer$\n\t"
            ".option pop\n\t"
            "la sp, _sp_main\n\t"
    );
    
#if 0
    GLB_GPIO_Type pinList[4];
    
    pinList[0] = 0;
    pinList[1] = 1;
    pinList[2] = 2;
    pinList[3] = 7;
    RomDriver_GLB_GPIO_Func_Init(GPIO_FUN_E21_JTAG, pinList, 4);
#endif
    
#if 0
    GLB_GPIO_Cfg_Type gpioCfg;
    GLB_GPIO_Type gpioPin=18;
    
    gpioCfg.gpioPin=gpioPin;
    gpioCfg.gpioFun=11;
    gpioCfg.gpioMode=GPIO_MODE_OUTPUT;
    gpioCfg.pullType=GPIO_PULL_NONE;
    gpioCfg.drive=0;
    gpioCfg.smtCtrl=1;
    RomDriver_GLB_GPIO_Init(&gpioCfg);
    
    RomDriver_GLB_GPIO_Write(gpioPin, 1);
    RomDriver_BL702L_Delay_US(100);
    RomDriver_GLB_GPIO_Write(gpioPin, 0);
    
    //volatile uint32_t debug = 0;
    //while(!debug);
#endif
    
    // FIX: disable all irq
    for(uint32_t n = 0; n < IRQn_LAST; n += 4){
        *(uint32_t *)(CLIC_HART0_ADDR + CLIC_INTIE + n) = 0;
    }
    
    // Get hbn irq status
    hbnIrqStatus = BL_RD_REG(HBN_BASE, HBN_IRQ_STAT);
    
    // Power off RF
    *(volatile uint32_t *)(AON_BASE + AON_RF_TOP_AON_OFFSET) &= ~(uint32_t)((0x1<<0)|(0x1<<1)|(0x1<<2));
    
    // Power on XTAL32M
    RomDriver_AON_Power_On_XTAL();
    
    // Select XTAL32M as root clock
    RomDriver_HBN_Set_ROOT_CLK_Sel(HBN_ROOT_CLK_XTAL);
    
    // Enable SF clock
    RomDriver_GLB_Set_SF_CLK(1, 0, 0);
    
    // Initialize flash gpio
    RomDriver_SF_Cfg_Init_Flash_Gpio(flashPinCfg, 1);
    
    // Configure flash (must use rom driver, since tcm code is lost and flash is power down)
    bl_hbn_restore_flash(&sfCtrlCfg, &flashCfg, flashImageOffset, cacheWayDisable);
    
    // Get wakeup time in rtc cycles
    hbnWakeupTime = bl_rtc_get_counter();
    
    // Restore EM select before using new stack pointer
    BL_WR_REG(GLB_BASE, GLB_SEAM_MISC, emSel);
    
    // Restore tcmcode section
    extern uint8_t _tcm_load, _tcm_run, _tcm_run_end;
    bl_hbn_restore_data((uint32_t)&_tcm_load, (uint32_t)&_tcm_run, (uint32_t)&_tcm_run_end);
    
    // Restore data section
    extern uint8_t _data_load, _data_run, _data_run_end;
    bl_hbn_restore_data((uint32_t)&_data_load, (uint32_t)&_data_run, (uint32_t)&_data_run_end);
    
    // Restore bss section
    extern uint8_t __bss_start, __bss_end;
    bl_hbn_restore_bss((uint32_t)&__bss_start, (uint32_t)&__bss_end);
    
    // Call user callback
    bl_hbn_fastboot_done_callback();
    while(1);
}

// can be placed in tcm section, here placed in hbncode section to reduce fast boot time
static void ATTR_NOINLINE ATTR_HBN_CODE_SECTION bl_hbn_mode_enter(void)
{
    uint32_t tmpVal;
    
#if 0
    // Power down flash
    RomDriver_SF_Ctrl_Set_Owner(SF_CTRL_OWNER_SAHB);
    RomDriver_SFlash_Reset_Continue_Read(&flashCfg);
    RomDriver_SFlash_Powerdown();
#endif
    
    // Select RC32M
    RomDriver_HBN_Set_ROOT_CLK_Sel(HBN_ROOT_CLK_RC32M);
    RomDriver_GLB_Power_Off_DLL();
    RomDriver_AON_Power_Off_XTAL();
    
    // Enter HBN0
    tmpVal = BL_RD_REG(HBN_BASE, HBN_CTL);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, HBN_LDO11_AON_VOUT_SEL, HBN_LDO_LEVEL_1P00V);
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
    
    BL_WR_REG(HBN_BASE, HBN_IRQ_CLR, 0xFFFFFFFF);
    
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
    if(hbnIrqStatus & 0x10000){
        return HBN_WAKEUP_BY_RTC;
    }else if(hbnIrqStatus & 0x3FF){
        return HBN_WAKEUP_BY_GPIO;
    }else if(hbnIrqStatus & (1U << 20)){
        return HBN_WAKEUP_BY_ACOMP0;
    }else if(hbnIrqStatus & (1U << 22)){
        return HBN_WAKEUP_BY_ACOMP1;
    }else{
        return 0;
    }
}

uint32_t bl_hbn_get_wakeup_gpio(void)
{
    uint32_t wakeupPin = 0;
    
    if(hbnIrqStatus & 0x3FF){
        wakeupPin |= (hbnIrqStatus & 0x1F) << 9;
        wakeupPin |= ((hbnIrqStatus >> 5) & 0x03) << 30;
        wakeupPin |= ((hbnIrqStatus >> 7) & 0x01) << 8;
        wakeupPin |= ((hbnIrqStatus >> 8) & 0x01) << 14;
        wakeupPin |= ((hbnIrqStatus >> 9) & 0x01) << 22;
    }
    
    return wakeupPin;
}

uint64_t bl_hbn_get_wakeup_time(void)
{
    return hbnWakeupTime;
}

__attribute__((weak)) void bl_hbn_fastboot_done_callback(void)
{
    HOSAL_UART_DEV_DECL(uart_stdio, 0, 14, 15, 2000000);
    hosal_uart_init(&uart_stdio);
    
    while(1){
        printf("HBN fast boot done!\r\n");
        printf("HBN_IRQ_STAT: 0x%08lX\r\n", hbnIrqStatus);
        arch_delay_ms(1000);
    }
}


int bl_hbn_enter(hbn_type_t *hbn, uint32_t *time)
{
#if 0
    SPI_Flash_Cfg_Type Gd_Q80E_Q16E = {
            .resetCreadCmd=0xff,
            .resetCreadCmdSize=3,
            .mid=0xc8,

            .deBurstWrapCmd=0x77,
            .deBurstWrapCmdDmyClk=0x3,
            .deBurstWrapDataMode=SF_CTRL_DATA_4_LINES,
            .deBurstWrapData=0xF0,

            /*reg*/
            .writeEnableCmd=0x06,
            .wrEnableIndex=0x00,
            .wrEnableBit=0x01,
            .wrEnableReadRegLen=0x01,

            .qeIndex=1,
            .qeBit=0x01,
            .qeWriteRegLen=0x02,
            .qeReadRegLen=0x1,

            .busyIndex=0,
            .busyBit=0x00,
            .busyReadRegLen=0x1,
            .releasePowerDown=0xab,

            .readRegCmd[0]=0x05,
            .readRegCmd[1]=0x35,
            .writeRegCmd[0]=0x01,
            .writeRegCmd[1]=0x01,

            .fastReadQioCmd=0xeb,
            .frQioDmyClk=16/8,
            .cReadSupport=1,
            .cReadMode=0xA0,

            .burstWrapCmd=0x77,
            .burstWrapCmdDmyClk=0x3,
            .burstWrapDataMode=SF_CTRL_DATA_4_LINES,
            .burstWrapData=0x40,
             /*erase*/
            .chipEraseCmd=0xc7,
            .sectorEraseCmd=0x20,
            .blk32EraseCmd=0x52,
            .blk64EraseCmd=0xd8,
            /*write*/
            .pageProgramCmd=0x02,
            .qpageProgramCmd=0x32,
            .qppAddrMode=SF_CTRL_ADDR_1_LINE,

            .ioMode=SF_CTRL_QIO_MODE,
            .clkDelay=1,
            .clkInvert=0x3f,

            .resetEnCmd=0x66,
            .resetCmd=0x99,
            .cRExit=0xff,
            .wrEnableWriteRegLen=0x00,

            /*id*/
            .jedecIdCmd=0x9f,
            .jedecIdCmdDmyClk=0,
            .qpiJedecIdCmd=0x9f,
            .qpiJedecIdCmdDmyClk=0x00,
            .sectorSize=4,
            .pageSize=256,

            /*read*/
            .fastReadCmd=0x0b,
            .frDmyClk=8/8,
            .qpiFastReadCmd =0x0b,
            .qpiFrDmyClk=8/8,
            .fastReadDoCmd=0x3b,
            .frDoDmyClk=8/8,
            .fastReadDioCmd=0xbb,
            .frDioDmyClk=0,
            .fastReadQoCmd=0x6b,
            .frQoDmyClk=8/8,

            .qpiFastReadQioCmd=0xeb,
            .qpiFrQioDmyClk=16/8,
            .qpiPageProgramCmd=0x02,
            .writeVregEnableCmd=0x50,

            /* qpi mode */
            .enterQpi=0x38,
            .exitQpi=0xff,

             /*AC*/
            .timeEsector=300,
            .timeE32k=1200,
            .timeE64k=1200,
            .timePagePgm=5,
            .timeCe=20*1000,
            .pdDelay=20,
            .qeData=0,
    };
#endif
    HBN_APP_CFG_Type cfg = {
        .useXtal32k = 0,                                         /*!< Wheather use xtal 32K as 32K clock source,otherwise use rc32k */
        .sleepTime = 0,                                          /*!< HBN sleep time */
        .hw_pu_pd_en = 0,                                        /*!< Pull up or pull down enable in the hbn mode */
        .flashCfg = NULL,                                        /*!< Flash config pointer, used when power down flash */
        .hbnLevel = HBN_LEVEL_1,                                 /*!< HBN level */
        .ldoLevel = HBN_LDO_LEVEL_1P10V,                         /*!< LDO level */
    };

    uint16_t gpioWakeupSrc = 0;

    printf("hbn.buflen = %d\r\n", hbn->buflen);
    printf("hbn.active = %d\r\n", hbn->active);
    log_buf(hbn->buf, hbn->buflen);

    cfg.sleepTime = (*time << 15) / 1000;  // ms -> rtc cycles
    if (hbn->buflen > 10) {
        printf("not support arg.\r\n");
        return -1;
    }

    int i;
    for (i=0; i<hbn->buflen; i++) {
        if (hbn->buf[i] == 9) {
            gpioWakeupSrc |= HBN_WAKEUP_GPIO_9;
        } else if (hbn->buf[i] == 10) {
            gpioWakeupSrc |= HBN_WAKEUP_GPIO_10;
        } else if (hbn->buf[i] == 11) {
            gpioWakeupSrc |= HBN_WAKEUP_GPIO_11;
        } else if (hbn->buf[i] == 12) {
            gpioWakeupSrc |= HBN_WAKEUP_GPIO_12;
        } else if (hbn->buf[i] == 13) {
            gpioWakeupSrc |= HBN_WAKEUP_GPIO_13;
        } else if (hbn->buf[i] == 30) {
            gpioWakeupSrc |= HBN_WAKEUP_GPIO_30;
        } else if (hbn->buf[i] == 31) {
            gpioWakeupSrc |= HBN_WAKEUP_GPIO_31;
        } else if (hbn->buf[i] == 8) {
            gpioWakeupSrc |= HBN_WAKEUP_GPIO_8;
        } else if (hbn->buf[i] == 14) {
            gpioWakeupSrc |= HBN_WAKEUP_GPIO_14;
        } else if (hbn->buf[i] == 22) {
            gpioWakeupSrc |= HBN_WAKEUP_GPIO_22;
        } else {
            printf("invalid arg.\r\n");
            return -1;
        }
    }
    if (hbn->buflen > 0) {
        printf("hbn");
        for (i=0; i<hbn->buflen; i++) {
            printf(" gpio%d", hbn->buf[i]);
        }
        printf(".\r\n");
    }

    cfg.flashCfg = bl_flash_get_flashCfg();

    gpioWakeupSrc |= 0xFC00;  // workaround due to bug in HBN_GPIO_Wakeup_Set
    HBN_GPIO_Wakeup_Set(gpioWakeupSrc, 0x04);

    HBN_Mode_Enter(&cfg);
    return -1;
}

