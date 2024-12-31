#include "bl_pds.h"
#include "bl_flash.h"
#include "bl_irq.h"


uint16_t bl_rtc_frequency = 32768;


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
        .pdsGpioKeep = 4,
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

/* Backup Memory */
static uint32_t bl_pds_bak[29];
static uint32_t bl_pds_bak_addr = (uint32_t)bl_pds_bak;

/* Flash Pin Configuration, will get from efuse */
static uint8_t flashPinCfg = 0;

/* SF Control Configuration, will get based on flash configuration */
static SF_Ctrl_Cfg_Type sfCtrlCfg = {};


static struct {
    /* PDS Configuration Pointer */
    PDS_DEFAULT_LV_CFG_Type *pdsCfgPtr;
    
    /* Flash Configuration Pointer */
    SPI_Flash_Cfg_Type *flashCfgPtr;
    
    /* Cache Way Disable */
    uint8_t cacheWayDisable;
    
    /* EM Select */
    uint8_t emSel;
    
    /* Encrypt Type */
    uint8_t encryptType;
    
    /* AES Region Lock */
    uint8_t aesRegionLock;
    
    /* AES IV */
    uint8_t aesIv[16];
    
    /* Flash Image Offset */
    uint32_t flashImageOffset;
    
    /* Flash Image Length */
    uint32_t flashImageLength;
    
    /* Flash Continuous Read */
    uint8_t flashContRead;
    
    /* Flag whether flash is power down */
    uint8_t flashPowerdown;
    
    /* Flag whether cpu registers are stored or not */
    uint8_t cpuRegStored;
    
    /* Flag whether psram needs retention */
    uint8_t psramRetention;
    
    /* PDS GPIO pull-up set */
    uint32_t pdsGpioPU;
    
    /* PDS GPIO pull-down set */
    uint32_t pdsGpioPD;
    
    /* HBN GPIO pull-up set */
    uint8_t hbnGpioPU;
    
    /* HBN GPIO pull-down set */
    uint8_t hbnGpioPD;
    
    /* Padding */
    uint8_t rsvd[2];
    
    /* HBN IRQ Status */
    uint32_t hbnIrqStatus;
    
    /* Fast Boot Done Callback */
    void (*fastbootDoneCallback)(void);
}bl_pds_misc;


ATTR_PDS_SECTION
int bl_pds_pre_process(uint32_t pdsLevel, uint32_t pdsSleepCycles, uint32_t *store, uint32_t arg[])
{
    __disable_irq();
    
    *store = !bl_pds_misc.cpuRegStored;
    
    return 0;
}

ATTR_PDS_SECTION
int bl_pds_start(uint32_t pdsLevel, uint32_t pdsSleepCycles)
{
    // Power down flash
    flash_powerdown();
    
    // Select RC32M
    RomDriver_GLB_Set_System_CLK(GLB_DLL_XTAL_NONE, GLB_SYS_CLK_RC32M);
    RomDriver_GLB_Power_Off_DLL();
    RomDriver_AON_Power_Off_XTAL();
    
    // Enable GPIO PU/PD
    void bl_pds_gpio_pull_enable(void);
    bl_pds_gpio_pull_enable();
    
    // Pull flash/psram io high during pds
    if(bl_pds_misc.psramRetention){
        *(volatile uint32_t *)0x40000140 = 0x00130013;
        *(volatile uint32_t *)0x40000144 = 0x00130013;
        *(volatile uint32_t *)0x40000148 = 0x00130013;
    }
    
    // Make HBNRAM retention
    BL_WR_REG(HBN_BASE, HBN_SRAM, (0x3<<0)|(0x1<<3)|(0x1<<6)|(0x0<<7));
    
    // Clear HBN_IRQ status
    BL_WR_REG(HBN_BASE, HBN_IRQ_CLR, 0xFFFFFFFF);
    
    // Clear PDS_GPIO status
    void bl_pds_gpio_clear_int_status(void);
    bl_pds_gpio_clear_int_status();
    
    // Clear PDS_KYD status
    void bl_pds_kyd_clear_wakeup(void);
    bl_pds_kyd_clear_wakeup();
    
    // Enter PDS mode
    RomDriver_HBN_Set_Status_Flag(HBN_STATUS_ENTER_FLAG);
    RomDriver_PDS_Default_Level_Config(bl_pds_misc.pdsCfgPtr, pdsSleepCycles);
    __WFI();
    
    // Fail to enter PDS mode due to interrupt pending, so disable PDS
    BL_WR_REG(PDS_BASE, PDS_CTL, (0x1<<10)|(0x1<<27));
    
    // Make HBNRAM active
    BL_WR_REG(HBN_BASE, HBN_SRAM, (0x3<<0)|(0x1<<3)|(0x0<<6)|(0x0<<7));
    
    // Restore flash
    flash_restore();
    
    // Select XTAL32M
    RomDriver_AON_Power_On_XTAL();
    RomDriver_HBN_Set_ROOT_CLK_Sel(HBN_ROOT_CLK_XTAL);
    
    // Disable GPIO PU/PD
    bl_pds_gpio_pull_disable();
    
    return 0;
}

ATTR_PDS_SECTION
int bl_pds_post_process(uint32_t pdsLevel, uint32_t pdsSleepCycles, uint32_t reset, uint32_t arg[])
{
    __enable_irq();
    
    return 0;
}


/* Private Functions */
void bl_pds_get_sf_ctrl_cfg(SPI_Flash_Cfg_Type *pFlashCfg, SF_Ctrl_Cfg_Type *pSfCtrlCfg)
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

void bl_pds_gpio_set_int_mode(uint8_t group, uint8_t mode)
{
    uint32_t tmpVal;
    
    tmpVal = BL_RD_REG(PDS_BASE, PDS_GPIO_INT_SET);
    tmpVal &= ~(0xF << (group << 2));
    tmpVal |= (mode << (group << 2));
    BL_WR_REG(PDS_BASE, PDS_GPIO_INT_SET, tmpVal);
}

void bl_pds_gpio_set_int_mask(uint8_t group, uint8_t mask)
{
    uint32_t tmpVal;
    
    tmpVal = BL_RD_REG(PDS_BASE, PDS_CFG_PDS_KEY_SCAN);
    if(mask){
        tmpVal |= (1 << (group + PDS_CR_PDS_GPIO_SET_INT_MASK_POS));
    }else{
        tmpVal &= ~(1 << (group + PDS_CR_PDS_GPIO_SET_INT_MASK_POS));
    }
    BL_WR_REG(PDS_BASE, PDS_CFG_PDS_KEY_SCAN, tmpVal);
}

ATTR_PDS_SECTION
void bl_pds_gpio_clear_int_status(void)
{
    uint32_t tmpVal;
    
    tmpVal = BL_RD_REG(PDS_BASE, PDS_CFG_PDS_KEY_SCAN);
    tmpVal |= (0xFF << PDS_CR_PDS_GPIO0_SET_INT_CLR_POS);
    BL_WR_REG(PDS_BASE, PDS_CFG_PDS_KEY_SCAN, tmpVal);
}

void bl_pds_kyd_init(uint8_t row_num, uint8_t col_num, uint8_t col_out, uint8_t row_pull)
{
    uint32_t tmpVal;
    
    tmpVal = BL_RD_REG(PDS_BASE, PDS_KYD_CTL);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, PDS_CR_PDS_KYD_ROW_MATRIX_SIZE, row_num - 1);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, PDS_CR_PDS_KYD_COL_MATRIX_SIZE, col_num - 1);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, PDS_CR_PDS_KYD_COL_O_VAL, col_out);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, PDS_CR_PDS_KYD_ROW_I_PULL_EN, row_pull);
    BL_WR_REG(PDS_BASE, PDS_KYD_CTL, tmpVal);
}

void bl_pds_kyd_matrix_init(uint8_t row_num, uint8_t col_num, uint8_t row_pins[], uint8_t col_pins[], uint8_t row_det_cnt)
{
    uint32_t row_sel[2] = {0, 0};
    uint32_t col_sel[2] = {0, 0};
    int i;
    
    for(i=0; i<row_num; i++){
        row_sel[i/4] |= (uint32_t)row_pins[i] << (8 * (i%4));
    }
    
    for(i=row_num; i<8; i++){
        row_sel[i/4] |= 4 << (8 * (i%4));  // use GPIO4 as row padding
    }
    
    for(i=0; i<col_num; i++){
        col_sel[i/4] |= (uint32_t)col_pins[i] << (8 * (i%4));
    }
    
    BL_WR_REG(PDS_BASE, PDS_KYS_ROW_I_GPIO_SEL0, row_sel[0] | (row_det_cnt << 29));
    BL_WR_REG(PDS_BASE, PDS_KYS_ROW_I_GPIO_SEL1, row_sel[1]);
    BL_WR_REG(PDS_BASE, PDS_KYS_COL_O_GPIO_SEL0, col_sel[0]);
    BL_WR_REG(PDS_BASE, PDS_KYS_COL_O_GPIO_SEL1, col_sel[1]);
}

uint8_t bl_pds_kyd_get_wakeup_key_index(uint8_t *row_index, uint8_t *col_index)
{
    uint32_t tmpVal;
    
    tmpVal = BL_RD_REG(PDS_BASE, PDS_KYD_CTL);
    
    if(BL_GET_REG_BITS_VAL(tmpVal, PDS_RO_PDS_KYD_WAKEUP)){
        *row_index = BL_GET_REG_BITS_VAL(tmpVal, PDS_RO_PDS_KYD_WAKEUP_ROW_INDEX);
        *col_index = BL_GET_REG_BITS_VAL(tmpVal, PDS_RO_PDS_KYD_WAKEUP_COL_INDEX);
        return 1;
    }else{
        return 0;
    }
}

uint8_t bl_pds_kyd_get_key_code(uint8_t row_index, uint8_t col_index)
{
    uint32_t tmpVal;
    uint8_t row_num;
    uint8_t key_code;
    
    tmpVal = BL_RD_REG(PDS_BASE, PDS_KYD_CTL);
    row_num = BL_GET_REG_BITS_VAL(tmpVal, PDS_CR_PDS_KYD_ROW_MATRIX_SIZE) + 1;
    key_code = row_index + col_index * row_num;
    
    return key_code;
}

ATTR_PDS_SECTION
void bl_pds_kyd_clear_wakeup(void)
{
    uint32_t tmpVal;
    
    tmpVal = BL_RD_REG(PDS_BASE, PDS_KYD_CTL);
    tmpVal = BL_SET_REG_BIT(tmpVal, PDS_CLR_PDS_KYD_WAKEUP);
    BL_WR_REG(PDS_BASE, PDS_KYD_CTL, tmpVal);
}

void bl_pds_kyd_set_white_key(uint8_t id, uint8_t row_index, uint8_t col_index, uint8_t mode)
{
    uint32_t tmpVal;
    
    switch(id){
        case 0:
            tmpVal = BL_RD_REG(PDS_BASE, PDS_KYD_WHITE_SET);
            tmpVal = BL_SET_REG_BITS_VAL(tmpVal, PDS_CFG_WHITE_KEY0_ROW_INDEX, row_index);
            tmpVal = BL_SET_REG_BITS_VAL(tmpVal, PDS_CFG_WHITE_KEY0_COL_INDEX, col_index);
            tmpVal = BL_SET_REG_BITS_VAL(tmpVal, PDS_CFG_WHITE_KEY0_KYD_MODE, mode);
            BL_WR_REG(PDS_BASE, PDS_KYD_WHITE_SET, tmpVal);
        break;
        
        case 1:
            tmpVal = BL_RD_REG(PDS_BASE, PDS_KYD_WHITE_SET);
            tmpVal = BL_SET_REG_BITS_VAL(tmpVal, PDS_CFG_WHITE_KEY1_ROW_INDEX, row_index);
            tmpVal = BL_SET_REG_BITS_VAL(tmpVal, PDS_CFG_WHITE_KEY1_COL_INDEX, col_index);
            tmpVal = BL_SET_REG_BITS_VAL(tmpVal, PDS_CFG_WHITE_KEY1_KYD_MODE, mode);
            BL_WR_REG(PDS_BASE, PDS_KYD_WHITE_SET, tmpVal);
        break;
        
        case 2:
            tmpVal = BL_RD_REG(PDS_BASE, PDS_KYD_WHITE_SET);
            tmpVal = BL_SET_REG_BITS_VAL(tmpVal, PDS_CFG_WHITE_KEY2_ROW_INDEX, row_index);
            tmpVal = BL_SET_REG_BITS_VAL(tmpVal, PDS_CFG_WHITE_KEY2_COL_INDEX, col_index);
            tmpVal = BL_SET_REG_BITS_VAL(tmpVal, PDS_CFG_WHITE_KEY2_KYD_MODE, mode);
            BL_WR_REG(PDS_BASE, PDS_KYD_WHITE_SET, tmpVal);
        break;
        
        case 3:
            tmpVal = BL_RD_REG(PDS_BASE, PDS_KYD_WHITE_SET);
            tmpVal = BL_SET_REG_BITS_VAL(tmpVal, PDS_CFG_WHITE_KEY3_ROW_INDEX, row_index);
            tmpVal = BL_SET_REG_BITS_VAL(tmpVal, PDS_CFG_WHITE_KEY3_COL_INDEX, col_index);
            tmpVal = BL_SET_REG_BITS_VAL(tmpVal, PDS_CFG_WHITE_KEY3_KYD_MODE, mode);
            BL_WR_REG(PDS_BASE, PDS_KYD_WHITE_SET, tmpVal);
        break;
        
        default:
        break;
    }
}

uint8_t bl_pds_kyd_is_white_key(uint8_t row_index, uint8_t col_index, uint8_t mode)
{
    uint32_t white_set;
    uint8_t pattern;
    
    white_set = BL_RD_REG(PDS_BASE, PDS_KYD_WHITE_SET);
    pattern = (row_index << 0) + (col_index << 3) + (mode << 6);
    
    while(white_set != 0){
        if((white_set & 0xFF) == pattern){
            return 1;
        }
        white_set >>= 8;
    }
    
    return 0;
}

ATTR_PDS_SECTION
void bl_pds_restore_sf(void)
{
    // Initialize flash gpio
    RomDriver_SF_Cfg_Init_Flash_Gpio(flashPinCfg, 0);
}

ATTR_PDS_SECTION
void bl_pds_restore_flash(SF_Ctrl_Cfg_Type *pSfCtrlCfg, SPI_Flash_Cfg_Type *pFlashCfg, uint32_t flashImageOffset, uint8_t flashContRead, uint8_t cacheWayDisable)
{
    uint32_t tmp[1];
    
    RomDriver_SFlash_Init(pSfCtrlCfg);
    
    RomDriver_SFlash_Releae_Powerdown(pFlashCfg);
    RomDriver_BL702L_Delay_US(pFlashCfg->pdDelay);
    
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

ATTR_PDS_SECTION
void bl_pds_restore_flash_encryption(uint8_t *aes_iv, uint32_t flash_offset, uint32_t img_len, uint8_t aes_region_lock)
{
    RomDriver_SF_Ctrl_AES_Set_Key_BE(0, NULL, SF_CTRL_AES_128BITS);
    RomDriver_SF_Ctrl_AES_Set_IV_BE(0, aes_iv, flash_offset);
    RomDriver_SF_Ctrl_AES_Set_Region(0, 1, 1, flash_offset, flash_offset + img_len - 1, aes_region_lock);
    RomDriver_SF_Ctrl_AES_Enable_BE();
    RomDriver_SF_Ctrl_AES_Enable();
}

ATTR_PDS_SECTION
void bl_pds_restore_cpu_reg(void)
{
    __asm__ __volatile__(
            "lw     a0,     bl_pds_bak_addr\n\t"
            "lw     ra,     0(a0)\n\t"
            "lw     sp,     1*4(a0)\n\t"
            "lw     tp,     2*4(a0)\n\t"
            "lw     t0,     3*4(a0)\n\t"
            "lw     t1,     4*4(a0)\n\t"
            "lw     t2,     5*4(a0)\n\t"
            "lw     fp,     6*4(a0)\n\t"
            "lw     s1,     7*4(a0)\n\t"
            "lw     a1,     8*4(a0)\n\t"
            "lw     a2,     9*4(a0)\n\t"
            "lw     a3,     10*4(a0)\n\t"
            "lw     a4,     11*4(a0)\n\t"
            "lw     a5,     12*4(a0)\n\t"
            "lw     a6,     13*4(a0)\n\t"
            "lw     a7,     14*4(a0)\n\t"
            "lw     s2,     15*4(a0)\n\t"
            "lw     s3,     16*4(a0)\n\t"
            "lw     s4,     17*4(a0)\n\t"
            "lw     s5,     18*4(a0)\n\t"
            "lw     s6,     19*4(a0)\n\t"
            "lw     s7,     20*4(a0)\n\t"
            "lw     s8,     21*4(a0)\n\t"
            "lw     s9,     22*4(a0)\n\t"
            "lw     s10,    23*4(a0)\n\t"
            "lw     s11,    24*4(a0)\n\t"
            "lw     t3,     25*4(a0)\n\t"
            "lw     t4,     26*4(a0)\n\t"
            "lw     t5,     27*4(a0)\n\t"
            "lw     t6,     28*4(a0)\n\t"
            "csrw   mtvec,  a1\n\t"
            "csrw   mstatus,a2\n\t"
            "ret\n\t"
    );
}

ATTR_PDS_SECTION
void bl_pds_restore(void)
{
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
    GLB_GPIO_Type gpioPin = 18;
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
    RomDriver_BL702L_Delay_US(100);
    *pOut &= ~(1<<pos);
#endif
    
    // Power off RF
    *(volatile uint32_t *)(AON_BASE + AON_RF_TOP_AON_OFFSET) &= ~(uint32_t)((0x1<<0)|(0x1<<1)|(0x1<<2));
    
    // Power on XTAL32M, later will check whether it is ready for use
    *(volatile uint32_t *)(AON_BASE + AON_RF_TOP_AON_OFFSET) |= (0x1<<5)|(0x1<<6);
    
    // Disable PDS GPIO ISO mode
    *(volatile uint32_t *)(PDS_BASE + PDS_CTL_OFFSET) &= ~(0x1<<31);
    
    // Get hbn irq status
    bl_pds_misc.hbnIrqStatus = BL_RD_REG(HBN_BASE, HBN_IRQ_STAT);
    
    // Make HBNRAM active
    BL_WR_REG(HBN_BASE, HBN_SRAM, (0x3<<0)|(0x1<<3)|(0x0<<6)|(0x0<<7));
    
    // Restore EM select
    BL_WR_REG(GLB_BASE, GLB_SEAM_MISC, bl_pds_misc.emSel);
    
    // Reset secure engine
    RomDriver_GLB_AHB_MCU_Software_Reset(GLB_AHB_MCU_SW_SEC_ENG);
    
    // FIX: disable all irq
    for(uint32_t n = 0; n < IRQn_LAST; n += 4){
        *(volatile uint32_t *)(CLIC_HART0_ADDR + CLIC_INTIE + n) = 0;
        *(volatile uint32_t *)(CLIC_HART0_ADDR + CLIC_INTIP + n) = 0;
    }
    
    // Configure flash (must use rom driver, since tcm code is lost and flash is power down)
    flash_restore();
    
    // Set cpuRegStored flag
    bl_pds_misc.cpuRegStored = 1;
    
    // Call user callback
    if(bl_pds_misc.fastbootDoneCallback){
        bl_pds_misc.fastbootDoneCallback();
    }
    
    // Clear cpuRegStored flag
    bl_pds_misc.cpuRegStored = 0;
    
    // Restore cpu registers
    bl_pds_restore_cpu_reg();
}

ATTR_PDS_SECTION
void bl_pds_fastboot_entry(void)
{
#ifndef __riscv_float_abi_soft
    __asm__ __volatile__(
            "li t0, 0x00006000\n\t"
            "csrs mstatus, t0\n\t"
            "fssr x0\n\t"
    );
#endif
    
    __asm__ __volatile__(
            ".option push\n\t"
            ".option norelax\n\t"
            "la gp, __global_pointer$\n\t"
            ".option pop\n\t"
            "la sp, _sp_main\n\t"
            "call bl_pds_restore\n\t"
    );
}


/* Public Functions */
void bl_pds_init(void)
{
    Efuse_Device_Info_Type devInfo;
    encrypt_info_t encryptInfo;
    
    EF_Ctrl_Read_Device_Info(&devInfo);
    bl_flash_get_encryptInfo(&encryptInfo);
    
    bl_pds_misc.pdsCfgPtr = &pdsCfgLevel31;
    bl_pds_misc.flashCfgPtr = (SPI_Flash_Cfg_Type *)bl_flash_get_flashCfg();
    bl_pds_misc.cacheWayDisable = BL_GET_REG_BITS_VAL(BL_RD_REG(L1C_BASE, L1C_CONFIG), L1C_WAY_DIS);
    bl_pds_misc.emSel = BL_RD_REG(GLB_BASE, GLB_SEAM_MISC);
    bl_pds_misc.encryptType = encryptInfo.encrypt_type;
    bl_pds_misc.aesRegionLock = encryptInfo.aes_region_lock;
    memcpy(bl_pds_misc.aesIv, encryptInfo.aes_iv, 16);
    bl_pds_misc.flashImageOffset = encryptInfo.flash_offset;
    bl_pds_misc.flashImageLength = encryptInfo.img_len;
    bl_pds_misc.flashContRead = encryptInfo.encrypt_type ? 0 : (bl_pds_misc.flashCfgPtr->cReadSupport & 0x01);
    
    // Get flash pin configuration from efuse
    if(devInfo.flash_cfg == 0){
        flashPinCfg = SF_CTRL_SEL_EXTERNAL_FLASH;
    }else{
        if(devInfo.sf_reverse == 0){
            flashPinCfg = devInfo.sf_swap_cfg + 1;
        }else{
            flashPinCfg = devInfo.sf_swap_cfg + 5;
        }
    }
    
    // Get SF control configuration
    bl_pds_get_sf_ctrl_cfg(bl_pds_misc.flashCfgPtr, &sfCtrlCfg);
    
    // Update rtc frequency
    bl_rtc_process_xtal_cnt_32k();
    
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
    
    // Set fast boot entry
    HBN_Set_Wakeup_Addr((uint32_t)bl_pds_fastboot_entry);
}

void bl_pds_fastboot_cfg(uint32_t addr)
{
    HBN_Set_Wakeup_Addr(addr);
    HBN_Set_Status_Flag(HBN_STATUS_ENTER_FLAG);
}

void bl_pds_set_psram_retention(uint8_t enable)
{
    if(enable){
        pdsCfgLevel31.pdsCtl.puFlash = 1;
        pdsCfgLevel31.pdsCtl.pdsGpioIsoMod = 1;
        
        bl_pds_misc.psramRetention = 1;
    }else{
        pdsCfgLevel31.pdsCtl.puFlash = 0;
        pdsCfgLevel31.pdsCtl.pdsGpioIsoMod = 0;
        
        bl_pds_misc.psramRetention = 0;
    }
}

uint8_t bl_pds_get_gpio_group(uint8_t pin)
{
    const uint8_t table[] = {
        [0 ... 3] = 0,
        [4 ... 6] = -1,  // N/A
        [7]       = 1,
        [8]       = 2,
        [9 ... 13] = 8,
        [14 ... 15] = 3,
        [16 ... 19] = 4,
        [20 ... 23] = 5,
        [24 ... 27] = 6,
        [28 ... 29] = 7,
        [30 ... 31] = 8,
    };
    
    return table[pin];
}

void bl_pds_gpio_group_wakeup_cfg(uint8_t group, uint8_t pin_list[], uint8_t pin_num, uint8_t edge_sel)
{
    uint16_t hbnWakeupPin = 0;
    uint32_t pdsWakeupPin = 0;
    int pin;
    int i;
    
    if(group > PDS_GPIO_GROUP_8){
        return;
    }
    
    for(i = 0; i < pin_num; i++){
        pin = pin_list[i];
        
        if(pin >= 9 && pin <= 13){
            hbnWakeupPin |= HBN_WAKEUP_GPIO_9 << (pin - 9);
        }else if(pin >= 30 && pin <= 31){
            hbnWakeupPin |= HBN_WAKEUP_GPIO_30 << (pin - 30);
        }else if(pin >= 4 && pin <= 6){
            // GPIO4/5/6 do not exist
            continue;
        }else{
            pdsWakeupPin |= 1 << pin;
        }
    }
    
    // Configure hbn wakeup pins
    if(group == PDS_GPIO_GROUP_8){
        hbnWakeupPin |= 0xFC00;  // workaround due to bug in HBN_GPIO_Wakeup_Set
        HBN_GPIO_Wakeup_Set(hbnWakeupPin, edge_sel);
        return;
    }
    
    // Configure pds wakeup pins
    uint32_t tmpVal = BL_RD_REG(PDS_BASE, PDS_GPIO_IE_SET);
    switch(group){
        case PDS_GPIO_GROUP_0:
            bl_pds_gpio_set_int_mode(0, edge_sel);
            
            if(pdsWakeupPin & (0xF << 0)){
                bl_pds_gpio_set_int_mask(0, 0);
            }else{
                bl_pds_gpio_set_int_mask(0, 1);
            }
            
            tmpVal &= ~(0xF << 0);
            tmpVal |= pdsWakeupPin & (0xF << 0);
            break;
        case PDS_GPIO_GROUP_1:
            bl_pds_gpio_set_int_mode(1, edge_sel);
            
            if(pdsWakeupPin & (0x1 << 7)){
                bl_pds_gpio_set_int_mask(1, 0);
            }else{
                bl_pds_gpio_set_int_mask(1, 1);
            }
            
            tmpVal &= ~(0x1 << 7);
            tmpVal |= pdsWakeupPin & (0x1 << 7);
            break;
        case PDS_GPIO_GROUP_2:
            bl_pds_gpio_set_int_mode(2, edge_sel);
            
            if(pdsWakeupPin & (0x1 << 8)){
                bl_pds_gpio_set_int_mask(2, 0);
            }else{
                bl_pds_gpio_set_int_mask(2, 1);
            }
            
            tmpVal &= ~(0x1 << 8);
            tmpVal |= pdsWakeupPin & (0x1 << 8);
            break;
        case PDS_GPIO_GROUP_3:
            bl_pds_gpio_set_int_mode(3, edge_sel);
            
            if(pdsWakeupPin & (0x3 << 14)){
                bl_pds_gpio_set_int_mask(3, 0);
            }else{
                bl_pds_gpio_set_int_mask(3, 1);
            }
            
            tmpVal &= ~(0x3 << 14);
            tmpVal |= pdsWakeupPin & (0x3 << 14);
            break;
        case PDS_GPIO_GROUP_4:
            bl_pds_gpio_set_int_mode(4, edge_sel);
            
            if(pdsWakeupPin & (0xF << 16)){
                bl_pds_gpio_set_int_mask(4, 0);
            }else{
                bl_pds_gpio_set_int_mask(4, 1);
            }
            
            tmpVal &= ~(0xF << 16);
            tmpVal |= pdsWakeupPin & (0xF << 16);
            break;
        case PDS_GPIO_GROUP_5:
            bl_pds_gpio_set_int_mode(5, edge_sel);
            
            if(pdsWakeupPin & (0xF << 20)){
                bl_pds_gpio_set_int_mask(5, 0);
            }else{
                bl_pds_gpio_set_int_mask(5, 1);
            }
            
            tmpVal &= ~(0xF << 20);
            tmpVal |= pdsWakeupPin & (0xF << 20);
            break;
        case PDS_GPIO_GROUP_6:
            bl_pds_gpio_set_int_mode(6, edge_sel);
            
            if(pdsWakeupPin & (0xF << 24)){
                bl_pds_gpio_set_int_mask(6, 0);
            }else{
                bl_pds_gpio_set_int_mask(6, 1);
            }
            
            tmpVal &= ~(0xF << 24);
            tmpVal |= pdsWakeupPin & (0xF << 24);
            break;
        case PDS_GPIO_GROUP_7:
            bl_pds_gpio_set_int_mode(7, edge_sel);
            
            if(pdsWakeupPin & (0x3 << 28)){
                bl_pds_gpio_set_int_mask(7, 0);
            }else{
                bl_pds_gpio_set_int_mask(7, 1);
            }
            
            tmpVal &= ~(0x3 << 28);
            tmpVal |= pdsWakeupPin & (0x3 << 28);
            break;
    }
    BL_WR_REG(PDS_BASE, PDS_GPIO_IE_SET, tmpVal);
    
    if(tmpVal == 0){
        PDS_Wakeup_Src_En(PDS_WAKEUP_SRC_PDS_IO_INT, DISABLE);
    }else{
        PDS_Wakeup_Src_En(PDS_WAKEUP_SRC_PDS_IO_INT, ENABLE);
    }
}

void bl_pds_gpio_wakeup_cfg(uint8_t pin_list[], uint8_t pin_num, uint8_t edge_sel)
{
    uint16_t hbnWakeupPin = 0;
    uint32_t pdsWakeupPin = 0;
    int pin;
    int i;
    
    for(i = 0; i < pin_num; i++){
        pin = pin_list[i];
        
        if(pin >= 9 && pin <= 13){
            hbnWakeupPin |= HBN_WAKEUP_GPIO_9 << (pin - 9);
        }else if(pin >= 30 && pin <= 31){
            hbnWakeupPin |= HBN_WAKEUP_GPIO_30 << (pin - 30);
        }else if(pin >= 4 && pin <= 6){
            // GPIO4/5/6 do not exist
            continue;
        }else{
            pdsWakeupPin |= 1 << pin;
        }
    }
    
    // Configure hbn wakeup pins
    hbnWakeupPin |= 0xFC00;  // workaround due to bug in HBN_GPIO_Wakeup_Set
    HBN_GPIO_Wakeup_Set(hbnWakeupPin, edge_sel);
    
    // Configure pds wakeup pins
    bl_pds_gpio_set_int_mode(0, edge_sel);
    bl_pds_gpio_set_int_mode(1, edge_sel);
    bl_pds_gpio_set_int_mode(2, edge_sel);
    bl_pds_gpio_set_int_mode(3, edge_sel);
    bl_pds_gpio_set_int_mode(4, edge_sel);
    bl_pds_gpio_set_int_mode(5, edge_sel);
    bl_pds_gpio_set_int_mode(6, edge_sel);
    bl_pds_gpio_set_int_mode(7, edge_sel);
    
    if(pdsWakeupPin & (0xF << 0)){
        bl_pds_gpio_set_int_mask(0, 0);
    }else{
        bl_pds_gpio_set_int_mask(0, 1);
    }
    
    if(pdsWakeupPin & (0x1 << 7)){
        bl_pds_gpio_set_int_mask(1, 0);
    }else{
        bl_pds_gpio_set_int_mask(1, 1);
    }
    
    if(pdsWakeupPin & (0x1 << 8)){
        bl_pds_gpio_set_int_mask(2, 0);
    }else{
        bl_pds_gpio_set_int_mask(2, 1);
    }
    
    if(pdsWakeupPin & (0x3 << 14)){
        bl_pds_gpio_set_int_mask(3, 0);
    }else{
        bl_pds_gpio_set_int_mask(3, 1);
    }
    
    if(pdsWakeupPin & (0xF << 16)){
        bl_pds_gpio_set_int_mask(4, 0);
    }else{
        bl_pds_gpio_set_int_mask(4, 1);
    }
    
    if(pdsWakeupPin & (0xF << 20)){
        bl_pds_gpio_set_int_mask(5, 0);
    }else{
        bl_pds_gpio_set_int_mask(5, 1);
    }
    
    if(pdsWakeupPin & (0xF << 24)){
        bl_pds_gpio_set_int_mask(6, 0);
    }else{
        bl_pds_gpio_set_int_mask(6, 1);
    }
    
    if(pdsWakeupPin & (0x3 << 28)){
        bl_pds_gpio_set_int_mask(7, 0);
    }else{
        bl_pds_gpio_set_int_mask(7, 1);
    }
    
    BL_WR_REG(PDS_BASE, PDS_GPIO_IE_SET, pdsWakeupPin);
    
    if(pdsWakeupPin == 0){
        PDS_Wakeup_Src_En(PDS_WAKEUP_SRC_PDS_IO_INT, DISABLE);
    }else{
        PDS_Wakeup_Src_En(PDS_WAKEUP_SRC_PDS_IO_INT, ENABLE);
    }
}

void bl_pds_gpio_pull_get(uint32_t *pullup_bitmap, uint32_t *pulldown_bitmap)
{
    if(pullup_bitmap){
        *pullup_bitmap = bl_pds_misc.pdsGpioPU;
    }
    
    if(pulldown_bitmap){
        *pulldown_bitmap = bl_pds_misc.pdsGpioPD;
    }
}

void bl_pds_gpio_pull_set(uint32_t pullup_bitmap, uint32_t pulldown_bitmap)
{
    bl_pds_misc.pdsGpioPU = pullup_bitmap;
    bl_pds_misc.pdsGpioPD = pulldown_bitmap;
    
    bl_pds_misc.hbnGpioPU = (pullup_bitmap >> 9) & 0x1F;
    bl_pds_misc.hbnGpioPU |= (pullup_bitmap >> 30) << 5;
    bl_pds_misc.hbnGpioPD = (pulldown_bitmap >> 9) & 0x1F;
    bl_pds_misc.hbnGpioPD |= (pulldown_bitmap >> 30) << 5;
}

ATTR_PDS_SECTION
void bl_pds_gpio_pull_enable(void)
{
    uint32_t tmpVal;
    
    tmpVal = BL_RD_REG(PDS_BASE, PDS_GPIO_PU_SET);
    tmpVal |= bl_pds_misc.pdsGpioPU;
    BL_WR_REG(PDS_BASE, PDS_GPIO_PU_SET, tmpVal);
    
    tmpVal = BL_RD_REG(PDS_BASE, PDS_GPIO_PD_SET);
    tmpVal |= bl_pds_misc.pdsGpioPD;
    BL_WR_REG(PDS_BASE, PDS_GPIO_PD_SET, tmpVal);
    
    tmpVal = BL_RD_REG(HBN_BASE, HBN_PAD_CTRL_1);
    tmpVal |= (uint32_t)bl_pds_misc.hbnGpioPU << 16;
    tmpVal |= (uint32_t)bl_pds_misc.hbnGpioPD << 8;
    BL_WR_REG(HBN_BASE, HBN_PAD_CTRL_1, tmpVal);
    
    tmpVal = BL_RD_REG(HBN_BASE, HBN_PAD_CTRL_0);
    tmpVal |= (uint32_t)(bl_pds_misc.hbnGpioPU | bl_pds_misc.hbnGpioPD) << 20;
    BL_WR_REG(HBN_BASE, HBN_PAD_CTRL_0, tmpVal);
}

void bl_pds_gpio_pull_disable(void)
{
    uint32_t tmpVal;
    
    tmpVal = BL_RD_REG(PDS_BASE, PDS_GPIO_PU_SET);
    tmpVal &= ~bl_pds_misc.pdsGpioPU;
    BL_WR_REG(PDS_BASE, PDS_GPIO_PU_SET, tmpVal);
    
    tmpVal = BL_RD_REG(PDS_BASE, PDS_GPIO_PD_SET);
    tmpVal &= ~bl_pds_misc.pdsGpioPD;
    BL_WR_REG(PDS_BASE, PDS_GPIO_PD_SET, tmpVal);
    
    tmpVal = BL_RD_REG(HBN_BASE, HBN_PAD_CTRL_1);
    tmpVal &= ~((uint32_t)bl_pds_misc.hbnGpioPU << 16);
    tmpVal &= ~((uint32_t)bl_pds_misc.hbnGpioPD << 8);
    BL_WR_REG(HBN_BASE, HBN_PAD_CTRL_1, tmpVal);
    
    tmpVal = BL_RD_REG(HBN_BASE, HBN_PAD_CTRL_0);
    tmpVal &= ~((uint32_t)(bl_pds_misc.hbnGpioPU | bl_pds_misc.hbnGpioPD) << 20);
    BL_WR_REG(HBN_BASE, HBN_PAD_CTRL_0, tmpVal);
}

void bl_pds_key_wakeup_cfg(uint8_t row_num, uint8_t col_num, uint8_t row_pins[], uint8_t col_pins[])
{
    if(row_num == 0 || col_num == 0){
        PDS_Set_KYD_Wakeup_En(0);
    }else{
        PDS_Set_KYD_Wakeup_En(1);
        
        bl_pds_kyd_init(row_num, col_num, 1, 1);
        
        bl_pds_kyd_matrix_init(row_num, col_num, row_pins, col_pins, row_num);
    }
}

void bl_pds_set_white_keys(uint8_t white_key_num, uint8_t row_idx[], uint8_t col_idx[])
{
    int i;
    
    for(i=0; i<4; i++){
        if(i < white_key_num){
            bl_pds_kyd_set_white_key(i, row_idx[i], col_idx[i], 1);
        }else{
            bl_pds_kyd_set_white_key(i, 0, 0, 0);
        }
    }
}

void bl_pds_register_fastboot_done_callback(bl_pds_cb_t cb)
{
    bl_pds_misc.fastbootDoneCallback = cb;
}

uint32_t bl_pds_get_wakeup_gpio(void)
{
    uint8_t pdsWakeupEvent = BL_GET_REG_BITS_VAL(BL_RD_REG(PDS_BASE, PDS_INT), PDS_RO_PDS_WAKEUP_EVENT);
    uint32_t wakeupPin = 0;
    
    if(pdsWakeupEvent & 0x01){
        if(bl_pds_misc.hbnIrqStatus & 0x7F){
            wakeupPin |= (bl_pds_misc.hbnIrqStatus & 0x1F) << 9;
            wakeupPin |= ((bl_pds_misc.hbnIrqStatus >> 5) & 0x03) << 30;
        }
    }else if(pdsWakeupEvent & 0x08){
        wakeupPin = BL_RD_REG(PDS_BASE, PDS_GPIO_STAT);
        wakeupPin &= BL_RD_REG(PDS_BASE, PDS_GPIO_IE_SET);
    }else{
        // empty
    }
    
    return wakeupPin;
}

int bl_pds_get_wakeup_key_index(uint8_t *row_idx, uint8_t *col_idx)
{
    if(bl_pds_kyd_get_wakeup_key_index(row_idx, col_idx) != 0){
        if(bl_pds_kyd_is_white_key(*row_idx, *col_idx, 1)){
            return PDS_KEY_EVENT_RELEASE;
        }else{
            return PDS_KEY_EVENT_PRESS;
        }
    }else{
        return 0;
    }
}

int bl_pds_get_wakeup_key_code(uint8_t *key_code)
{
    uint8_t row_idx;
    uint8_t col_idx;
    
    if(bl_pds_kyd_get_wakeup_key_index(&row_idx, &col_idx) != 0){
        *key_code = bl_pds_kyd_get_key_code(row_idx, col_idx);
        if(bl_pds_kyd_is_white_key(row_idx, col_idx, 1)){
            return PDS_KEY_EVENT_RELEASE;
        }else{
            return PDS_KEY_EVENT_PRESS;
        }
    }else{
        return 0;
    }
}

ATTR_PDS_SECTION
void bl_pds_enter_do(uint32_t pdsLevel, uint32_t pdsSleepCycles, uint32_t store, uint32_t *reset)
{
    if(store){
        *(uint32_t *)bl_pds_bak_addr = (uint32_t)__builtin_return_address(0);
    }
    
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
    
    // Store cpu registers except for ra
    if(store){
        __asm__ __volatile__(
            "csrr   a1,     mtvec\n\t"
            "csrr   a2,     mstatus\n\t"
            "lw     a0,     bl_pds_bak_addr\n\t"
            "sw     sp,     1*4(a0)\n\t"
            "sw     tp,     2*4(a0)\n\t"
            "sw     t0,     3*4(a0)\n\t"
            "sw     t1,     4*4(a0)\n\t"
            "sw     t2,     5*4(a0)\n\t"
            "sw     fp,     6*4(a0)\n\t"
            "sw     s1,     7*4(a0)\n\t"
            "sw     a1,     8*4(a0)\n\t"
            "sw     a2,     9*4(a0)\n\t"
            "sw     a3,     10*4(a0)\n\t"
            "sw     a4,     11*4(a0)\n\t"
            "sw     a5,     12*4(a0)\n\t"
            "sw     a6,     13*4(a0)\n\t"
            "sw     a7,     14*4(a0)\n\t"
            "sw     s2,     15*4(a0)\n\t"
            "sw     s3,     16*4(a0)\n\t"
            "sw     s4,     17*4(a0)\n\t"
            "sw     s5,     18*4(a0)\n\t"
            "sw     s6,     19*4(a0)\n\t"
            "sw     s7,     20*4(a0)\n\t"
            "sw     s8,     21*4(a0)\n\t"
            "sw     s9,     22*4(a0)\n\t"
            "sw     s10,    23*4(a0)\n\t"
            "sw     s11,    24*4(a0)\n\t"
            "sw     t3,     25*4(a0)\n\t"
            "sw     t4,     26*4(a0)\n\t"
            "sw     t5,     27*4(a0)\n\t"
            "sw     t6,     28*4(a0)\n\t"
        );
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

ATTR_PDS_SECTION
int bl_pds_get_wakeup_source(void)
{
    uint8_t pdsWakeupEvent = BL_GET_REG_BITS_VAL(BL_RD_REG(PDS_BASE, PDS_INT), PDS_RO_PDS_WAKEUP_EVENT);
    
    if(pdsWakeupEvent & 0x01){
        if(bl_pds_misc.hbnIrqStatus & 0x10000){
            return PDS_WAKEUP_BY_RTC;
        }else if(bl_pds_misc.hbnIrqStatus & 0x7F){
            return PDS_WAKEUP_BY_GPIO;
        }else{
            return 0;
        }
    }else if(pdsWakeupEvent & 0x08){
        return PDS_WAKEUP_BY_GPIO;
    }else if(pdsWakeupEvent & 0x02){
        return PDS_WAKEUP_BY_KEY;
    }else{
        return 0;
    }
}

ATTR_PDS_SECTION
int bl_pds_wakeup_by_rtc(void)
{
    return bl_pds_get_wakeup_source() == PDS_WAKEUP_BY_RTC;
}

ATTR_PDS_SECTION
void bl_rtc_trigger_xtal_cnt_32k(void)
{
    uint32_t tmpVal;
    
    tmpVal = BL_RD_REG(HBN_BASE, HBN_GLB);
    if(BL_GET_REG_BITS_VAL(tmpVal, HBN_F32K_SEL) == 1){
        return;
    }
    
    tmpVal = BL_RD_REG(GLB_BASE, GLB_XTAL_DEG_32K);
    tmpVal = BL_SET_REG_BIT(tmpVal, GLB_CLR_XTAL_CNT_32K_DONE);
    BL_WR_REG(GLB_BASE, GLB_XTAL_DEG_32K, tmpVal);
    
    tmpVal = BL_RD_REG(GLB_BASE, GLB_XTAL_DEG_32K);
    tmpVal = BL_SET_REG_BIT(tmpVal, GLB_XTAL_CNT_32K_SW_TRIG_PS);
    BL_WR_REG(GLB_BASE, GLB_XTAL_DEG_32K, tmpVal);
}

ATTR_PDS_SECTION
uint16_t bl_rtc_process_xtal_cnt_32k(void)
{
    uint32_t tmpVal;
    uint16_t cnt;  // 11-bits
    uint16_t res;  // 6-bits
    
    tmpVal = BL_RD_REG(PDS_BASE, PDS_XTAL_CNT_32K);
    if(BL_GET_REG_BITS_VAL(tmpVal, PDS_XTAL_CNT_32K_PROCESS) == 1){
        do {
            tmpVal = BL_RD_REG(PDS_BASE, PDS_XTAL_CNT_32K);
        } while(BL_GET_REG_BITS_VAL(tmpVal, PDS_XTAL_CNT_32K_DONE) == 0);
    }
    
    cnt = BL_GET_REG_BITS_VAL(tmpVal, PDS_RO_XTAL_CNT_32K_CNT);
    res = BL_GET_REG_BITS_VAL(tmpVal, PDS_RO_XTAL_CNT_32K_RES);
    
    bl_rtc_frequency = (32000000UL * 64) / ((uint32_t)cnt * 64 + res);
    
    return cnt;
}

ATTR_PDS_SECTION
uint32_t bl_rtc_32k_to_32m(uint32_t cycles)
{
    uint32_t tmpVal;
    uint16_t cnt;  // 11-bits
    uint16_t res;  // 6-bits
    
    tmpVal = BL_RD_REG(PDS_BASE, PDS_XTAL_CNT_32K);
    cnt = BL_GET_REG_BITS_VAL(tmpVal, PDS_RO_XTAL_CNT_32K_CNT);
    res = BL_GET_REG_BITS_VAL(tmpVal, PDS_RO_XTAL_CNT_32K_RES);
    
    return cycles * cnt + cycles * res / 64;
}

ATTR_PDS_SECTION
uint32_t bl_rtc_counter_to_ms(uint32_t cnt)
{
    uint32_t q = cnt / bl_rtc_frequency;
    uint32_t r = cnt % bl_rtc_frequency;
    
    return q * 1000 + r * 1000 / bl_rtc_frequency;
}

ATTR_PDS_SECTION
uint32_t bl_rtc_ms_to_counter(uint32_t ms)
{
    uint32_t q = ms / 1000;
    uint32_t r = ms % 1000;
    
    return q * bl_rtc_frequency + r * bl_rtc_frequency / 1000;
}

ATTR_PDS_SECTION
void flash_powerdown(void)
{
    if(bl_pds_misc.flashPowerdown){
        return;
    }else{
        bl_pds_misc.flashPowerdown = 1;
    }
    
    RomDriver_HBN_Power_Down_Flash(bl_pds_misc.flashCfgPtr);
    RomDriver_GLB_Set_SF_CLK(0, 0, 0);
}

ATTR_PDS_SECTION
void flash_restore(void)
{
    if(!bl_pds_misc.flashPowerdown){
        return;
    }else{
        bl_pds_misc.flashPowerdown = 0;
    }
    
    RomDriver_GLB_Set_SF_CLK(1, 0, 0);
    
    if(RomDriver_SF_Ctrl_Get_Flash_Image_Offset() == 0){
        bl_pds_restore_sf();
        bl_pds_restore_flash(&sfCtrlCfg, bl_pds_misc.flashCfgPtr, bl_pds_misc.flashImageOffset, bl_pds_misc.flashContRead, bl_pds_misc.cacheWayDisable);
        if(bl_pds_misc.encryptType){
            bl_pds_restore_flash_encryption(bl_pds_misc.aesIv, bl_pds_misc.flashImageOffset, bl_pds_misc.flashImageLength, bl_pds_misc.aesRegionLock);
        }
    }else{
        RomDriver_SF_Ctrl_Set_Owner(SF_CTRL_OWNER_SAHB);
        RomDriver_SFlash_Restore_From_Powerdown(bl_pds_misc.flashCfgPtr, bl_pds_misc.flashContRead);
    }
}
