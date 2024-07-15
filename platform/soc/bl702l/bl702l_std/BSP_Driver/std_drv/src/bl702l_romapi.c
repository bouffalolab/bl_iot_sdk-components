#include "bl702l_romdriver.h"


__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type AON_Power_On_MBG(void){
    return RomDriver_AON_Power_On_MBG();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type AON_Power_Off_MBG(void){
    return RomDriver_AON_Power_Off_MBG();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type AON_Power_On_XTAL(void){
    return RomDriver_AON_Power_On_XTAL();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type AON_Set_Xtal_CapCode(uint8_t capIn, uint8_t capOut){
    return RomDriver_AON_Set_Xtal_CapCode(capIn,capOut);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
uint8_t AON_Get_Xtal_CapCode(void){
    return RomDriver_AON_Get_Xtal_CapCode();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type AON_Set_Xtal_CapCode_Extra(uint8_t extra){
    return RomDriver_AON_Set_Xtal_CapCode_Extra(extra);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type AON_Power_Off_XTAL(void){
    return RomDriver_AON_Power_Off_XTAL();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type AON_Power_On_BG(void){
    return RomDriver_AON_Power_On_BG();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type AON_Power_Off_BG(void){
    return RomDriver_AON_Power_Off_BG();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type AON_Power_On_LDO11_SOC(void){
    return RomDriver_AON_Power_On_LDO11_SOC();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type AON_Power_Off_LDO11_SOC(void){
    return RomDriver_AON_Power_Off_LDO11_SOC();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type AON_Power_On_SFReg(void){
    return RomDriver_AON_Power_On_SFReg();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type AON_Power_Off_SFReg(void){
    return RomDriver_AON_Power_Off_SFReg();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type AON_Set_LDO11_SOC_Sstart_Delay(uint8_t delay){
    return RomDriver_AON_Set_LDO11_SOC_Sstart_Delay(delay);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type AON_Set_DCDC14_Top_0(uint8_t voutSel, uint8_t vpfm){
    return RomDriver_AON_Set_DCDC14_Top_0(voutSel,vpfm);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type AON_Trim_Ldo11socVoutTrim(void){
    return RomDriver_AON_Trim_Ldo11socVoutTrim();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type AON_Trim_Ldo14VoutTrim(void){
    return RomDriver_AON_Trim_Ldo14VoutTrim();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type AON_Trim_Dcdc14VoutTrim(void){
    return RomDriver_AON_Trim_Dcdc14VoutTrim();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
uint32_t Clock_System_Clock_Get(BL_System_Clock_Type type){
    return RomDriver_Clock_System_Clock_Get(type);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
uint32_t Clock_Peripheral_Clock_Get(BL_Peripheral_Type type){
    return RomDriver_Clock_Peripheral_Clock_Get(type);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
uint32_t SystemCoreClockGet(void){
    return RomDriver_SystemCoreClockGet();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
uint32_t CPU_Get_MTimer_Clock(void){
    return RomDriver_CPU_Get_MTimer_Clock();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
uint64_t CPU_Get_MTimer_Counter(void){
    return RomDriver_CPU_Get_MTimer_Counter();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
uint64_t CPU_Get_CPU_Cycle(void){
    return RomDriver_CPU_Get_CPU_Cycle();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
uint64_t CPU_Get_MTimer_US(void){
    return RomDriver_CPU_Get_MTimer_US();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
uint64_t CPU_Get_MTimer_MS(void){
    return RomDriver_CPU_Get_MTimer_MS();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type CPU_MTimer_Delay_US(uint32_t cnt){
    return RomDriver_CPU_MTimer_Delay_US(cnt);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type CPU_MTimer_Delay_MS(uint32_t cnt){
    return RomDriver_CPU_MTimer_Delay_MS(cnt);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void BL702L_Delay_US(uint32_t cnt){
    return RomDriver_BL702L_Delay_US(cnt);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void BL702L_Delay_MS(uint32_t cnt){
    return RomDriver_BL702L_Delay_MS(cnt);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
uint8_t EF_Ctrl_Get_Trim_Parity(uint32_t val, uint8_t len){
    return RomDriver_EF_Ctrl_Get_Trim_Parity(val,len);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void EF_Ctrl_Read_Common_Trim(char *name, Efuse_Common_Trim_Type *trim){
    return RomDriver_EF_Ctrl_Read_Common_Trim(name,trim);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void EF_Ctrl_Write_Common_Trim(char *name, uint8_t trim_en, uint32_t trim_value){
    return RomDriver_EF_Ctrl_Write_Common_Trim(name,trim_en,trim_value);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
uint8_t EF_Ctrl_Is_MAC_Address_Slot_Empty(uint8_t slot, uint8_t reload){
    return RomDriver_EF_Ctrl_Is_MAC_Address_Slot_Empty(slot,reload);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type EF_Ctrl_Write_MAC_Address_Opt(uint8_t slot, uint8_t mac[8], uint8_t program){
    return RomDriver_EF_Ctrl_Write_MAC_Address_Opt(slot,mac,program);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type EF_Ctrl_Read_MAC_Address_Opt(uint8_t slot, uint8_t mac[8], uint8_t reload){
    return RomDriver_EF_Ctrl_Read_MAC_Address_Opt(slot,mac,reload);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void EF_Ctrl_Sw_AHB_Clk_0(void){
    return RomDriver_EF_Ctrl_Sw_AHB_Clk_0();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void EF_Ctrl_Program_Efuse_0(void){
    return RomDriver_EF_Ctrl_Program_Efuse_0();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void EF_Ctrl_Load_Efuse_R0(void){
    return RomDriver_EF_Ctrl_Load_Efuse_R0();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Sts_Type EF_Ctrl_Busy(void){
    return RomDriver_EF_Ctrl_Busy();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Sts_Type EF_Ctrl_AutoLoad_Done(void){
    return RomDriver_EF_Ctrl_AutoLoad_Done();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void EF_Ctrl_Write_Dbg_Pwd(uint32_t passWdLow, uint32_t passWdHigh, uint8_t program){
    return RomDriver_EF_Ctrl_Write_Dbg_Pwd(passWdLow,passWdHigh,program);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void EF_Ctrl_Read_Dbg_Pwd(uint32_t *passWdLow, uint32_t *passWdHigh){
    return RomDriver_EF_Ctrl_Read_Dbg_Pwd(passWdLow,passWdHigh);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void EF_Ctrl_Readlock_Dbg_Pwd(uint8_t program){
    return RomDriver_EF_Ctrl_Readlock_Dbg_Pwd(program);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void EF_Ctrl_Writelock_Dbg_Pwd(uint8_t program){
    return RomDriver_EF_Ctrl_Writelock_Dbg_Pwd(program);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void EF_Ctrl_Write_Secure_Cfg(EF_Ctrl_Sec_Param_Type *cfg, uint8_t program){
    return RomDriver_EF_Ctrl_Write_Secure_Cfg(cfg,program);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void EF_Ctrl_Read_Secure_Cfg(EF_Ctrl_Sec_Param_Type *cfg){
    return RomDriver_EF_Ctrl_Read_Secure_Cfg(cfg);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void EF_Ctrl_Write_Secure_Boot(uint8_t sign[1], uint8_t aes[1], uint8_t program){
    return RomDriver_EF_Ctrl_Write_Secure_Boot(sign,aes,program);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void EF_Ctrl_Read_Secure_Boot(uint8_t sign[1], uint8_t aes[1]){
    return RomDriver_EF_Ctrl_Read_Secure_Boot(sign,aes);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void EF_Ctrl_Write_Sw_Usage(uint32_t index, uint32_t usage, uint8_t program){
    return RomDriver_EF_Ctrl_Write_Sw_Usage(index,usage,program);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void EF_Ctrl_Read_Sw_Usage(uint32_t index, uint32_t *usage){
    return RomDriver_EF_Ctrl_Read_Sw_Usage(index,usage);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void EF_Ctrl_Writelock_Sw_Usage(uint32_t index, uint8_t program){
    return RomDriver_EF_Ctrl_Writelock_Sw_Usage(index,program);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void EF_Ctrl_Write_MAC_Address(uint8_t mac[8], uint8_t program){
    return RomDriver_EF_Ctrl_Write_MAC_Address(mac,program);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
uint32_t EF_Ctrl_Get_Byte_Zero_Cnt(uint8_t val){
    return RomDriver_EF_Ctrl_Get_Byte_Zero_Cnt(val);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
uint8_t EF_Ctrl_Is_All_Bits_Zero(uint32_t val, uint8_t start, uint8_t len){
    return RomDriver_EF_Ctrl_Is_All_Bits_Zero(val,start,len);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type EF_Ctrl_Read_MAC_Address(uint8_t mac[8]){
    return RomDriver_EF_Ctrl_Read_MAC_Address(mac);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void EF_Ctrl_Writelock_MAC_Address(uint8_t program){
    return RomDriver_EF_Ctrl_Writelock_MAC_Address(program);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type EF_Ctrl_Read_Chip_ID(uint8_t chipID[8]){
    return RomDriver_EF_Ctrl_Read_Chip_ID(chipID);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void EF_Ctrl_Read_Device_Info(Efuse_Device_Info_Type *deviceInfo){
    return RomDriver_EF_Ctrl_Read_Device_Info(deviceInfo);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
uint8_t EF_Ctrl_Is_CapCode_Empty(uint8_t slot, uint8_t reload){
    return RomDriver_EF_Ctrl_Is_CapCode_Empty(slot,reload);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type EF_Ctrl_Write_CapCode_Opt(uint8_t slot, uint8_t code, uint8_t program){
    return RomDriver_EF_Ctrl_Write_CapCode_Opt(slot,code,program);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type EF_Ctrl_Read_CapCode_Opt(uint8_t slot, uint8_t *code, uint8_t reload){
    return RomDriver_EF_Ctrl_Read_CapCode_Opt(slot,code,reload);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
uint8_t EF_Ctrl_Is_PowerOffset_Slot_Empty(uint8_t slot, uint8_t reload){
    return RomDriver_EF_Ctrl_Is_PowerOffset_Slot_Empty(slot,reload);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type EF_Ctrl_Write_PowerOffset_Opt(uint8_t slot, int8_t pwrOffset[2], uint8_t program){
    return RomDriver_EF_Ctrl_Write_PowerOffset_Opt(slot,pwrOffset,program);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type EF_Ctrl_Read_PowerOffset_Opt(uint8_t slot, int8_t pwrOffset[2], uint8_t reload){
    return RomDriver_EF_Ctrl_Read_PowerOffset_Opt(slot,pwrOffset,reload);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void EF_Ctrl_Write_AES_Key(uint8_t index, uint32_t *keyData, uint32_t len, uint8_t program){
    return RomDriver_EF_Ctrl_Write_AES_Key(index,keyData,len,program);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void EF_Ctrl_Read_AES_Key(uint8_t index, uint32_t *keyData, uint32_t len){
    return RomDriver_EF_Ctrl_Read_AES_Key(index,keyData,len);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void EF_Ctrl_Writelock_AES_Key(uint8_t index, uint8_t program){
    return RomDriver_EF_Ctrl_Writelock_AES_Key(index,program);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void EF_Ctrl_Readlock_AES_Key(uint8_t index, uint8_t program){
    return RomDriver_EF_Ctrl_Readlock_AES_Key(index,program);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void EF_Ctrl_Program_Direct_R0(uint32_t index, uint32_t *data, uint32_t len){
    return RomDriver_EF_Ctrl_Program_Direct_R0(index,data,len);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void EF_Ctrl_Read_Direct_R0(uint32_t index, uint32_t *data, uint32_t len){
    return RomDriver_EF_Ctrl_Read_Direct_R0(index,data,len);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void EF_Ctrl_Clear(uint32_t index, uint32_t len){
    return RomDriver_EF_Ctrl_Clear(index,len);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void EF_Ctrl_Crc_Enable(void){
    return RomDriver_EF_Ctrl_Crc_Enable();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Sts_Type EF_Ctrl_Crc_Is_Busy(void){
    return RomDriver_EF_Ctrl_Crc_Is_Busy();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void EF_Ctrl_Crc_Set_Golden(uint32_t goldenValue){
    return RomDriver_EF_Ctrl_Crc_Set_Golden(goldenValue);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type EF_Ctrl_Crc_Result(void){
    return RomDriver_EF_Ctrl_Crc_Result();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
GLB_ROOT_CLK_Type GLB_Get_Root_CLK_Sel(void){
    return RomDriver_GLB_Get_Root_CLK_Sel();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_Set_System_CLK_Div(uint8_t hclkDiv, uint8_t bclkDiv){
    return RomDriver_GLB_Set_System_CLK_Div(hclkDiv,bclkDiv);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
uint8_t GLB_Get_BCLK_Div(void){
    return RomDriver_GLB_Get_BCLK_Div();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
uint8_t GLB_Get_HCLK_Div(void){
    return RomDriver_GLB_Get_HCLK_Div();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_Set_System_CLK(GLB_DLL_XTAL_Type xtalType, GLB_SYS_CLK_Type clkFreq){
    return RomDriver_GLB_Set_System_CLK(xtalType,clkFreq);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type System_Core_Clock_Update_From_RC32M(void){
    return RomDriver_System_Core_Clock_Update_From_RC32M();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_Set_MAC154_ZIGBEE_CLK(uint8_t enable){
    return RomDriver_GLB_Set_MAC154_ZIGBEE_CLK(enable);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_Set_BLE_CLK(uint8_t enable){
    return RomDriver_GLB_Set_BLE_CLK(enable);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_Set_AUDIO_CLK(uint8_t clkDivEn, uint8_t autoDivEn, GLB_AUDIO_CLK_SRC_Type clkSel, uint8_t div){
    return RomDriver_GLB_Set_AUDIO_CLK(clkDivEn,autoDivEn,clkSel,div);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_Set_KYS_CLK(GLB_KYS_CLK_SRC_Type clkSel, uint8_t div){
    return RomDriver_GLB_Set_KYS_CLK(clkSel,div);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_Set_DMA_CLK(uint8_t enable, GLB_DMA_CLK_ID_Type clk){
    return RomDriver_GLB_Set_DMA_CLK(enable,clk);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_Set_IR_CLK(uint8_t enable, GLB_IR_CLK_SRC_Type clkSel, uint8_t div){
    return RomDriver_GLB_Set_IR_CLK(enable,clkSel,div);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_Set_SF_CLK(uint8_t enable, GLB_SFLASH_CLK_Type clkSel, uint8_t div){
    return RomDriver_GLB_Set_SF_CLK(enable,clkSel,div);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_Set_UART_CLK(uint8_t enable, HBN_UART_CLK_Type clkSel, uint8_t div){
    return RomDriver_GLB_Set_UART_CLK(enable,clkSel,div);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_Sel_TMR_GPIO_Clock(GLB_GPIO_Type gpioPin){
    return RomDriver_GLB_Sel_TMR_GPIO_Clock(gpioPin);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_Set_Chip_Out_0_CLK_Sel(GLB_CHIP_CLK_OUT_0_Type clkSel){
    return RomDriver_GLB_Set_Chip_Out_0_CLK_Sel(clkSel);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_Set_Chip_Out_1_CLK_Sel(GLB_CHIP_CLK_OUT_1_Type clkSel){
    return RomDriver_GLB_Set_Chip_Out_1_CLK_Sel(clkSel);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_Set_Chip_Out_0_CLK_Enable(uint8_t enable){
    return RomDriver_GLB_Set_Chip_Out_0_CLK_Enable(enable);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_Set_Chip_Out_1_CLK_Enable(uint8_t enable){
    return RomDriver_GLB_Set_Chip_Out_1_CLK_Enable(enable);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_Set_I2C_CLK(uint8_t enable, GLB_I2C_CLK_SRC_Type clkSel, uint8_t div){
    return RomDriver_GLB_Set_I2C_CLK(enable,clkSel,div);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_Set_SPI_CLK(uint8_t enable, GLB_SPI_CLK_SRC_Type clkSel, uint8_t div){
    return RomDriver_GLB_Set_SPI_CLK(enable,clkSel,div);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_Set_PKA_CLK_Sel(GLB_PKA_CLK_SRC_Type clkSel){
    return RomDriver_GLB_Set_PKA_CLK_Sel(clkSel);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_SW_System_Reset(void){
    return RomDriver_GLB_SW_System_Reset();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_SW_CPU_Reset(void){
    return RomDriver_GLB_SW_CPU_Reset();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_SW_POR_Reset(void){
    return RomDriver_GLB_SW_POR_Reset();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_AHB_MCU_Software_Reset(GLB_AHB_MCU_SW_Type swrst){
    return RomDriver_GLB_AHB_MCU_Software_Reset(swrst);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_Disrst_Set(uint8_t enable, GLB_DISRST_Type disrst){
    return RomDriver_GLB_Disrst_Set(enable,disrst);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_MAC154_ZIGBEE_Reset(void){
    return RomDriver_GLB_MAC154_ZIGBEE_Reset();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_BLE_Reset(void){
    return RomDriver_GLB_BLE_Reset();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_PER_Clock_Gate(uint64_t ips){
    return RomDriver_GLB_PER_Clock_Gate(ips);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_PER_Clock_UnGate(uint64_t ips){
    return RomDriver_GLB_PER_Clock_UnGate(ips);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_Set_RTC_Mode(RTC_MODE_Type rtcMode){
    return RomDriver_GLB_Set_RTC_Mode(rtcMode);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_BMX_Init(BMX_Cfg_Type *BmxCfg){
    return RomDriver_GLB_BMX_Init(BmxCfg);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_BMX_Addr_Monitor_Enable(void){
    return RomDriver_GLB_BMX_Addr_Monitor_Enable();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_BMX_Addr_Monitor_Disable(void){
    return RomDriver_GLB_BMX_Addr_Monitor_Disable();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_BMX_BusErrResponse_Enable(void){
    return RomDriver_GLB_BMX_BusErrResponse_Enable();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_BMX_BusErrResponse_Disable(void){
    return RomDriver_GLB_BMX_BusErrResponse_Disable();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Sts_Type GLB_BMX_Get_Status(BMX_BUS_ERR_Type errType){
    return RomDriver_GLB_BMX_Get_Status(errType);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
uint32_t GLB_BMX_Get_Err_Addr(void){
    return RomDriver_GLB_BMX_Get_Err_Addr();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_BMX_BusErrClr_Set(uint8_t enable){
    return RomDriver_GLB_BMX_BusErrClr_Set(enable);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_Set_SRAM_PARM(uint32_t value){
    return RomDriver_GLB_Set_SRAM_PARM(value);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
uint32_t GLB_Get_SRAM_PARM(void){
    return RomDriver_GLB_Get_SRAM_PARM();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_Set_OCRAM_PARM(uint32_t value){
    return RomDriver_GLB_Set_OCRAM_PARM(value);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
uint32_t GLB_Get_OCRAM_PARM(void){
    return RomDriver_GLB_Get_OCRAM_PARM();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_Set_EM_Sel(GLB_EM_Type emType){
    return RomDriver_GLB_Set_EM_Sel(emType);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_Set_Kys_Drv_Col(uint8_t enable){
    return RomDriver_GLB_Set_Kys_Drv_Col(enable);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_GPIO_O_Latch_Mode_Set(uint8_t enable){
    return RomDriver_GLB_GPIO_O_Latch_Mode_Set(enable);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_JTAG_Sig_Swap_Set(uint8_t swapSel){
    return RomDriver_GLB_JTAG_Sig_Swap_Set(swapSel);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_CCI_Use_IO_0_1_2_7(uint8_t enable){
    return RomDriver_GLB_CCI_Use_IO_0_1_2_7(enable);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_CCI_Use_Jtag_Pin(uint8_t enable){
    return RomDriver_GLB_CCI_Use_Jtag_Pin(enable);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_Swap_SPI_0_MOSI_With_MISO(BL_Fun_Type newState){
    return RomDriver_GLB_Swap_SPI_0_MOSI_With_MISO(newState);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_Set_SPI_0_ACT_MOD_Sel(GLB_SPI_PAD_ACT_AS_Type mod){
    return RomDriver_GLB_Set_SPI_0_ACT_MOD_Sel(mod);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_Set_Flash_Scenario(uint8_t enable){
    return RomDriver_GLB_Set_Flash_Scenario(enable);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_Set_Embedded_FLash_IO_PARM(uint8_t reverse, uint8_t swapIo3Io0, uint8_t swapIo2Cs){
    return RomDriver_GLB_Set_Embedded_FLash_IO_PARM(reverse,swapIo3Io0,swapIo2Cs);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_Set_MTimer_CLK(uint8_t enable, GLB_MTIMER_CLK_Type clkSel, uint8_t div){
    return RomDriver_GLB_Set_MTimer_CLK(enable,clkSel,div);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_Set_ADC_CLK(uint8_t enable, GLB_ADC_CLK_SRC_Type clkSel, uint8_t div){
    return RomDriver_GLB_Set_ADC_CLK(enable,clkSel,div);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_Set_DIG_32K_CLK(uint8_t enable, uint8_t compensation, GLB_DIG_CLK_SRC_Type clkSel, uint16_t div){
    return RomDriver_GLB_Set_DIG_32K_CLK(enable,compensation,clkSel,div);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_SW_BLE_WAKEUP_REQ_Set(uint8_t enable){
    return RomDriver_GLB_SW_BLE_WAKEUP_REQ_Set(enable);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_UART_Fun_Sel(GLB_UART_SIG_Type sig, GLB_UART_SIG_FUN_Type fun){
    return RomDriver_GLB_UART_Fun_Sel(sig,fun);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_Power_Off_DLL(void){
    return RomDriver_GLB_Power_Off_DLL();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_Power_On_DLL(GLB_DLL_XTAL_Type xtalType){
    return RomDriver_GLB_Power_On_DLL(xtalType);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_Enable_DLL_All_Clks(void){
    return RomDriver_GLB_Enable_DLL_All_Clks();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_Enable_DLL_Clk(GLB_DLL_CLK_Type dllClk){
    return RomDriver_GLB_Enable_DLL_Clk(dllClk);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_Disable_DLL_All_Clks(void){
    return RomDriver_GLB_Disable_DLL_All_Clks();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_Disable_DLL_Clk(GLB_DLL_CLK_Type dllClk){
    return RomDriver_GLB_Disable_DLL_Clk(dllClk);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_Set_Flash_Id_Value(uint32_t idValue){
    return RomDriver_GLB_Set_Flash_Id_Value(idValue);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
uint32_t GLB_Get_Flash_Id_Value(void){
    return RomDriver_GLB_Get_Flash_Id_Value();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_Trim_RC32M(void){
    return RomDriver_GLB_Trim_RC32M();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_Set_Xtal_Cnt32k_Process(void){
    return RomDriver_GLB_Set_Xtal_Cnt32k_Process();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_Clear_Xtal_Cnt32k_Done(void){
    return RomDriver_GLB_Clear_Xtal_Cnt32k_Done();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_RC32K_Deg_Start(void){
    return RomDriver_GLB_RC32K_Deg_Start();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_RC32K_Deg_End(void){
    return RomDriver_GLB_RC32K_Deg_End();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_RC32K_Deg_Enable(uint8_t enable){
    return RomDriver_GLB_RC32K_Deg_Enable(enable);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_Xtal_Deg_Cnt_Limit_Set(uint8_t cnt){
    return RomDriver_GLB_Xtal_Deg_Cnt_Limit_Set(cnt);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_IR_LED_Driver_Enable(void){
    return RomDriver_GLB_IR_LED_Driver_Enable();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_IR_LED_Driver_Disable(void){
    return RomDriver_GLB_IR_LED_Driver_Disable();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_IR_LED_Driver_Output_Enable(GLB_IR_LED_Type led){
    return RomDriver_GLB_IR_LED_Driver_Output_Enable(led);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_IR_LED_Driver_Output_Disable(GLB_IR_LED_Type led){
    return RomDriver_GLB_IR_LED_Driver_Output_Disable(led);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_IR_LED_Driver_Ibias(uint8_t ibias){
    return RomDriver_GLB_IR_LED_Driver_Ibias(ibias);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_GPIO_Init(GLB_GPIO_Cfg_Type *cfg){
    return RomDriver_GLB_GPIO_Init(cfg);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_GPIO_Func_Init(GLB_GPIO_FUNC_Type gpioFun, GLB_GPIO_Type *pinList, uint8_t cnt){
    return RomDriver_GLB_GPIO_Func_Init(gpioFun,pinList,cnt);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_GPIO_OUTPUT_Mode_Set(GLB_GPIO_Type gpioPin, GLB_GPIO_OUTPUT_MODE_Type mode){
    return RomDriver_GLB_GPIO_OUTPUT_Mode_Set(gpioPin,mode);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_GPIO_INPUT_Enable(GLB_GPIO_Type gpioPin){
    return RomDriver_GLB_GPIO_INPUT_Enable(gpioPin);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_GPIO_INPUT_Disable(GLB_GPIO_Type gpioPin){
    return RomDriver_GLB_GPIO_INPUT_Disable(gpioPin);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_GPIO_OUTPUT_Enable(GLB_GPIO_Type gpioPin){
    return RomDriver_GLB_GPIO_OUTPUT_Enable(gpioPin);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_GPIO_OUTPUT_Disable(GLB_GPIO_Type gpioPin){
    return RomDriver_GLB_GPIO_OUTPUT_Disable(gpioPin);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_GPIO_Set_HZ(GLB_GPIO_Type gpioPin){
    return RomDriver_GLB_GPIO_Set_HZ(gpioPin);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
uint8_t GLB_GPIO_Get_Fun(GLB_GPIO_Type gpioPin){
    return RomDriver_GLB_GPIO_Get_Fun(gpioPin);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_GPIO_Write(GLB_GPIO_Type gpioPin, uint32_t val){
    return RomDriver_GLB_GPIO_Write(gpioPin,val);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
uint32_t GLB_GPIO_Read(GLB_GPIO_Type gpioPin){
    return RomDriver_GLB_GPIO_Read(gpioPin);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_GPIO_Set(GLB_GPIO_Type gpioPin){
    return RomDriver_GLB_GPIO_Set(gpioPin);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_GPIO_Clr(GLB_GPIO_Type gpioPin){
    return RomDriver_GLB_GPIO_Clr(gpioPin);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_GPIO_IntMask(GLB_GPIO_Type gpioPin, BL_Mask_Type intMask){
    return RomDriver_GLB_GPIO_IntMask(gpioPin,intMask);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_Clr_GPIO_IntStatus(GLB_GPIO_Type gpioPin){
    return RomDriver_GLB_Clr_GPIO_IntStatus(gpioPin);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Sts_Type GLB_Get_GPIO_IntStatus(GLB_GPIO_Type gpioPin){
    return RomDriver_GLB_Get_GPIO_IntStatus(gpioPin);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type GLB_GPIO_Int_Init(GLB_GPIO_INT_Cfg_Type *intCfg){
    return RomDriver_GLB_GPIO_Int_Init(intCfg);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void HBN_Mode_Enter(HBN_APP_CFG_Type *cfg){
    return RomDriver_HBN_Mode_Enter(cfg);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void HBN_GPIO_Wakeup_Set(uint16_t gpio_wakeup_src, HBN_GPIO_INT_Trigger_Type gpio_trig_type){
    return RomDriver_HBN_GPIO_Wakeup_Set(gpio_wakeup_src,gpio_trig_type);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void HBN_Power_Down_Flash(SPI_Flash_Cfg_Type *flashCfg){
    return RomDriver_HBN_Power_Down_Flash(flashCfg);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void HBN_Enable(HBN_LDO_LEVEL_Type ldoLevel, HBN_LEVEL_Type hbnLevel){
    return RomDriver_HBN_Enable(ldoLevel,hbnLevel);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type HBN_Reset(void){
    return RomDriver_HBN_Reset();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type HBN_App_Reset(uint8_t npXtalType, uint8_t bclkDiv, uint8_t apXtalType, uint8_t fclkDiv){
    return RomDriver_HBN_App_Reset(npXtalType,bclkDiv,apXtalType,fclkDiv);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type HBN_Disable(void){
    return RomDriver_HBN_Disable();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Sts_Type HBN_Get_BOR_OUT_State(void){
    return RomDriver_HBN_Get_BOR_OUT_State();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type HBN_Set_BOR_Config(uint8_t enable, HBN_BOR_THRES_Type threshold, HBN_BOR_MODE_Type mode){
    return RomDriver_HBN_Set_BOR_Config(enable,threshold,mode);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type HBN_Set_Ldo11_Aon_Vout(HBN_LDO_LEVEL_Type ldoLevel){
    return RomDriver_HBN_Set_Ldo11_Aon_Vout(ldoLevel);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type HBN_Set_Ldo11_Soc_Vout(HBN_LDO_LEVEL_Type ldoLevel){
    return RomDriver_HBN_Set_Ldo11_Soc_Vout(ldoLevel);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type HBN_Set_Ldo11_All_Vout(HBN_LDO_LEVEL_Type ldoLevel){
    return RomDriver_HBN_Set_Ldo11_All_Vout(ldoLevel);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type HBN_32K_Sel(HBN_32K_CLK_Type clkType){
    return RomDriver_HBN_32K_Sel(clkType);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type HBN_Set_UART_CLK_Sel(HBN_UART_CLK_Type clkSel){
    return RomDriver_HBN_Set_UART_CLK_Sel(clkSel);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type HBN_Set_XCLK_CLK_Sel(HBN_XCLK_CLK_Type xClk){
    return RomDriver_HBN_Set_XCLK_CLK_Sel(xClk);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type HBN_Set_ROOT_CLK_Sel(HBN_ROOT_CLK_Type rootClk){
    return RomDriver_HBN_Set_ROOT_CLK_Sel(rootClk);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type HBN_Set_HRAM_slp(void){
    return RomDriver_HBN_Set_HRAM_slp();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type HBN_Set_HRAM_Ret(void){
    return RomDriver_HBN_Set_HRAM_Ret();
}

#if 0
__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type HBN_Power_On_Xtal_32K(void){
    return RomDriver_HBN_Power_On_Xtal_32K();
}
#endif

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type HBN_Power_Off_Xtal_32K(void){
    return RomDriver_HBN_Power_Off_Xtal_32K();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type HBN_Power_On_RC32K(void){
    return RomDriver_HBN_Power_On_RC32K();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type HBN_Power_Off_RC32K(void){
    return RomDriver_HBN_Power_Off_RC32K();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type HBN_Trim_RC32K(void){
    return RomDriver_HBN_Trim_RC32K();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
uint32_t HBN_Get_Status_Flag(void){
    return RomDriver_HBN_Get_Status_Flag();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type HBN_Set_Status_Flag(uint32_t flag){
    return RomDriver_HBN_Set_Status_Flag(flag);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
uint32_t HBN_Get_Wakeup_Addr(void){
    return RomDriver_HBN_Get_Wakeup_Addr();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type HBN_Set_Wakeup_Addr(uint32_t addr){
    return RomDriver_HBN_Set_Wakeup_Addr(addr);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
uint8_t HBN_Get_User_Boot_Config(void){
    return RomDriver_HBN_Get_User_Boot_Config();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type HBN_Set_User_Boot_Config(uint8_t ubCfg){
    return RomDriver_HBN_Set_User_Boot_Config(ubCfg);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type HBN_Clear_RTC_Counter(void){
    return RomDriver_HBN_Clear_RTC_Counter();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type HBN_Enable_RTC_Counter(void){
    return RomDriver_HBN_Enable_RTC_Counter();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type HBN_Set_RTC_Timer(HBN_RTC_INT_Delay_Type delay, uint32_t compValLow, uint32_t compValHigh, uint8_t compMode){
    return RomDriver_HBN_Set_RTC_Timer(delay,compValLow,compValHigh,compMode);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type HBN_Get_RTC_Timer_Val(uint32_t *valLow, uint32_t *valHigh){
    return RomDriver_HBN_Get_RTC_Timer_Val(valLow,valHigh);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type HBN_Clear_RTC_INT(void){
    return RomDriver_HBN_Clear_RTC_INT();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type HBN_GPIO_INT_Enable(HBN_GPIO_INT_Trigger_Type gpioIntTrigType){
    return RomDriver_HBN_GPIO_INT_Enable(gpioIntTrigType);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type HBN_GPIO_INT_Disable(void){
    return RomDriver_HBN_GPIO_INT_Disable();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Sts_Type HBN_Get_INT_State(HBN_INT_Type irqType){
    return RomDriver_HBN_Get_INT_State(irqType);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
uint8_t HBN_Get_Pin_Wakeup_Mode(void){
    return RomDriver_HBN_Get_Pin_Wakeup_Mode();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type HBN_Clear_IRQ(HBN_INT_Type irqType){
    return RomDriver_HBN_Clear_IRQ(irqType);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type HBN_Hw_Pu_Pd_Cfg(uint8_t enable){
    return RomDriver_HBN_Hw_Pu_Pd_Cfg(enable);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type HBN_Comm_Pad_Wakeup_En_Cfg(uint8_t padCfg){
    return RomDriver_HBN_Comm_Pad_Wakeup_En_Cfg(padCfg);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type HBN_Aon_Pad_IeSmt_Cfg(uint8_t padCfg){
    return RomDriver_HBN_Aon_Pad_IeSmt_Cfg(padCfg);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type HBN_Pin_WakeUp_Mask(uint16_t maskVal){
    return RomDriver_HBN_Pin_WakeUp_Mask(maskVal);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type HBN_Enable_AComp0_IRQ(HBN_ACOMP_INT_EDGE_Type edge){
    return RomDriver_HBN_Enable_AComp0_IRQ(edge);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type HBN_Disable_AComp0_IRQ(HBN_ACOMP_INT_EDGE_Type edge){
    return RomDriver_HBN_Disable_AComp0_IRQ(edge);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type HBN_Enable_AComp1_IRQ(HBN_ACOMP_INT_EDGE_Type edge){
    return RomDriver_HBN_Enable_AComp1_IRQ(edge);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type HBN_Disable_AComp1_IRQ(HBN_ACOMP_INT_EDGE_Type edge){
    return RomDriver_HBN_Disable_AComp1_IRQ(edge);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type HBN_Enable_BOR_IRQ(void){
    return RomDriver_HBN_Enable_BOR_IRQ();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type HBN_Disable_BOR_IRQ(void){
    return RomDriver_HBN_Disable_BOR_IRQ();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Sts_Type HBN_Get_Reset_Event(HBN_RST_EVENT_Type event){
    return RomDriver_HBN_Get_Reset_Event(event);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type HBN_Clear_Reset_Event(void){
    return RomDriver_HBN_Clear_Reset_Event();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type HBN_GPIO_Dbg_Pull_Cfg(BL_Fun_Type pupdEn, BL_Fun_Type dlyEn, uint8_t dlySec, HBN_INT_Type gpioIrq, BL_Mask_Type gpioMask){
    return RomDriver_HBN_GPIO_Dbg_Pull_Cfg(pupdEn,dlyEn,dlySec,gpioIrq,gpioMask);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type HBN_Set_BOR_Cfg(HBN_BOR_CFG_Type *cfg){
    return RomDriver_HBN_Set_BOR_Cfg(cfg);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void L1C_Cache_Write_Set(BL_Fun_Type wtEn, BL_Fun_Type wbEn, BL_Fun_Type waEn){
    return RomDriver_L1C_Cache_Write_Set(wtEn,wbEn,waEn);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type L1C_Cache_Enable_Set(uint8_t wayDisable){
    return RomDriver_L1C_Cache_Enable_Set(wayDisable);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type L1C_Cache_Flush(void){
    return RomDriver_L1C_Cache_Flush();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void L1C_Cache_Hit_Count_Get(uint32_t *hitCountLow, uint32_t *hitCountHigh){
    return RomDriver_L1C_Cache_Hit_Count_Get(hitCountLow,hitCountHigh);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
uint32_t L1C_Cache_Miss_Count_Get(void){
    return RomDriver_L1C_Cache_Miss_Count_Get();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void L1C_Cache_Read_Disable(void){
    return RomDriver_L1C_Cache_Read_Disable();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type L1C_Set_Wrap(BL_Fun_Type wrap){
    return RomDriver_L1C_Set_Wrap(wrap);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type L1C_Set_Way_Disable(uint8_t disableVal){
    return RomDriver_L1C_Set_Way_Disable(disableVal);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type L1C_IROM_2T_Access_Set(uint8_t enable){
    return RomDriver_L1C_IROM_2T_Access_Set(enable);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type L1C_BMX_Init(L1C_BMX_Cfg_Type *l1cBmxCfg){
    return RomDriver_L1C_BMX_Init(l1cBmxCfg);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type L1C_BMX_Addr_Monitor_Enable(void){
    return RomDriver_L1C_BMX_Addr_Monitor_Enable();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type L1C_BMX_Addr_Monitor_Disable(void){
    return RomDriver_L1C_BMX_Addr_Monitor_Disable();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type L1C_BMX_BusErrResponse_Enable(void){
    return RomDriver_L1C_BMX_BusErrResponse_Enable();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type L1C_BMX_BusErrResponse_Disable(void){
    return RomDriver_L1C_BMX_BusErrResponse_Disable();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Sts_Type L1C_BMX_Get_Status(L1C_BMX_BUS_ERR_Type errType){
    return RomDriver_L1C_BMX_Get_Status(errType);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
uint32_t L1C_BMX_Get_Err_Addr(void){
    return RomDriver_L1C_BMX_Get_Err_Addr();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type PDS_Set_GPIO_Pad_Cfg(PDS_GPIO_Type pin, PDS_GPIO_Cfg_Type *cfg){
    return RomDriver_PDS_Set_GPIO_Pad_Cfg(pin,cfg);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type PDS_GPIO_Write(PDS_GPIO_GROUP_SET_Type grp, uint32_t val){
    return RomDriver_PDS_GPIO_Write(grp,val);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type PDS_Set_GPIO_Pad_IntMask(PDS_GPIO_GROUP_SET_Type grp, BL_Mask_Type intMask){
    return RomDriver_PDS_Set_GPIO_Pad_IntMask(grp,intMask);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type PDS_Set_GPIO_Pad_IntMode(PDS_GPIO_GROUP_SET_Type grp, PDS_GPIO_INT_TRIG_Type trig){
    return RomDriver_PDS_Set_GPIO_Pad_IntMode(grp,trig);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type PDS_Set_GPIO_Pad_IntClr(PDS_GPIO_GROUP_SET_Type grp){
    return RomDriver_PDS_Set_GPIO_Pad_IntClr(grp);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type PDS_Set_All_GPIO_IntClear(void){
    return RomDriver_PDS_Set_All_GPIO_IntClear();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Sts_Type PDS_Get_GPIO_Pad_IntStatus(PDS_GPIO_Type pin){
    return RomDriver_PDS_Get_GPIO_Pad_IntStatus(pin);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type PDS_Disable_GPIO_Keep(void){
    return RomDriver_PDS_Disable_GPIO_Keep();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type PDS_Enable(PDS_CTL_Type *cfg, PDS_CTL4_Type *cfg4, uint32_t pdsSleepCnt){
    return RomDriver_PDS_Enable(cfg,cfg4,pdsSleepCnt);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type PDS_Force_Config(PDS_CTL2_Type *cfg2, PDS_CTL3_Type *cfg3){
    return RomDriver_PDS_Force_Config(cfg2,cfg3);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type PDS_RAM_Config(PDS_RAM_CFG_Type *ramCfg){
    return RomDriver_PDS_RAM_Config(ramCfg);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type PDS_Default_Level_Config(PDS_DEFAULT_LV_CFG_Type *defaultLvCfg, uint32_t pdsSleepCnt){
    return RomDriver_PDS_Default_Level_Config(defaultLvCfg,pdsSleepCnt);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type PDS_Wakeup_Src_En(PDS_WAKEUP_SRC_Type intType, BL_Fun_Type enable){
    return RomDriver_PDS_Wakeup_Src_En(intType,enable);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Sts_Type PDS_Get_Wakeup_Src(PDS_WAKEUP_SRC_Type intType){
    return RomDriver_PDS_Get_Wakeup_Src(intType);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type PDS_IntMask(PDS_INT_Type intType, BL_Mask_Type intMask){
    return RomDriver_PDS_IntMask(intType,intMask);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Sts_Type PDS_Get_IntStatus(PDS_INT_Type intType){
    return RomDriver_PDS_Get_IntStatus(intType);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type PDS_IntClear(void){
    return RomDriver_PDS_IntClear();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
PDS_RF_STS_Type PDS_Get_PdsRfStstus(void){
    return RomDriver_PDS_Get_PdsRfStstus();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
PDS_STS_Type PDS_Get_PdsStstus(void){
    return RomDriver_PDS_Get_PdsStstus();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type PDS_Clear_Reset_Event(void){
    return RomDriver_PDS_Clear_Reset_Event();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Sts_Type PDS_Get_Reset_Event(PDS_RST_EVENT_Type event){
    return RomDriver_PDS_Get_Reset_Event(event);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void PDS_Auto_Enable(PDS_AUTO_POWER_DOWN_CFG_Type *powerCfg, PDS_AUTO_NORMAL_CFG_Type *normalCfg, BL_Fun_Type enable){
    return RomDriver_PDS_Auto_Enable(powerCfg,normalCfg,enable);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void PDS_Manual_Force_Turn_Off(PDS_FORCE_Type domain){
    return RomDriver_PDS_Manual_Force_Turn_Off(domain);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void PDS_Manual_Force_Turn_On(PDS_FORCE_Type domain){
    return RomDriver_PDS_Manual_Force_Turn_On(domain);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type PDS_Set_KYD_Matrix_Size(uint8_t col_size, uint8_t row_size){
    return RomDriver_PDS_Set_KYD_Matrix_Size(col_size,row_size);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type PDS_Set_KYD_Col_Value(uint8_t val){
    return RomDriver_PDS_Set_KYD_Col_Value(val);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type PDS_Set_KYD_Row_Pull(uint8_t en){
    return RomDriver_PDS_Set_KYD_Row_Pull(en);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type PDS_Set_KYD_Wakeup_En(uint8_t en){
    return RomDriver_PDS_Set_KYD_Wakeup_En(en);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void SF_Cfg_Init_Internal_Flash_Gpio(void){
    return RomDriver_SF_Cfg_Init_Internal_Flash_Gpio();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void SF_Cfg_Init_Ext_Flash_Gpio(uint8_t extFlashPin){
    return RomDriver_SF_Cfg_Init_Ext_Flash_Gpio(extFlashPin);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void SF_Cfg_Deinit_Ext_Flash_Gpio(uint8_t extFlashPin){
    return RomDriver_SF_Cfg_Deinit_Ext_Flash_Gpio(extFlashPin);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type SF_Cfg_Get_Flash_Cfg_Need_Lock(uint32_t flashID, SPI_Flash_Cfg_Type *pFlashCfg){
    return RomDriver_SF_Cfg_Get_Flash_Cfg_Need_Lock(flashID,pFlashCfg);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void SF_Cfg_Init_Flash_Gpio(uint8_t flashPinCfg, uint8_t restoreDefault){
    return RomDriver_SF_Cfg_Init_Flash_Gpio(flashPinCfg,restoreDefault);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
uint32_t SF_Cfg_Flash_Identify(uint8_t callFromFlash, uint32_t autoScan, uint32_t flashPinCfg, uint8_t restoreDefault, SPI_Flash_Cfg_Type *pFlashCfg){
    return RomDriver_SF_Cfg_Flash_Identify(callFromFlash,autoScan,flashPinCfg,restoreDefault,pFlashCfg);
}

#if 0
__ALWAYS_INLINE ATTR_TCM_SECTION
uint32_t SF_Cfg_Flash_Identify_Ext(uint8_t callFromFlash, uint32_t autoScan, uint32_t flashPinCfg, uint8_t restoreDefault, SPI_Flash_Cfg_Type *pFlashCfg){
    return SF_Cfg_Flash_Identify(callFromFlash,autoScan,flashPinCfg,restoreDefault,pFlashCfg);
}
#endif

__ALWAYS_INLINE ATTR_TCM_SECTION
void SF_Ctrl_Enable(const SF_Ctrl_Cfg_Type *cfg){
    return RomDriver_SF_Ctrl_Enable(cfg);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void SF_Ctrl_Psram_Init(SF_Ctrl_Psram_Cfg *sfCtrlPsramCfg){
    return RomDriver_SF_Ctrl_Psram_Init(sfCtrlPsramCfg);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
uint8_t SF_Ctrl_Get_Clock_Delay(void){
    return RomDriver_SF_Ctrl_Get_Clock_Delay();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void SF_Ctrl_Set_Clock_Delay(uint8_t delay){
    return RomDriver_SF_Ctrl_Set_Clock_Delay(delay);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void SF_Ctrl_Cmds_Set(SF_Ctrl_Cmds_Cfg *cmdsCfg, SF_Ctrl_Select sel){
    return RomDriver_SF_Ctrl_Cmds_Set(cmdsCfg,sel);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void SF_Ctrl_Burst_Toggle_Set(uint8_t burstToggleEn, uint8_t mode){
    return RomDriver_SF_Ctrl_Burst_Toggle_Set(burstToggleEn,mode);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void SF_Ctrl_Select_Pad(SF_Ctrl_Pad_Select sel){
    return RomDriver_SF_Ctrl_Select_Pad(sel);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void SF_Ctrl_Select_Bank(SF_Ctrl_Select sel){
    return RomDriver_SF_Ctrl_Select_Bank(sel);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void SF_Ctrl_Set_Owner(SF_Ctrl_Owner_Type owner){
    return RomDriver_SF_Ctrl_Set_Owner(owner);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void SF_Ctrl_Disable(void){
    return RomDriver_SF_Ctrl_Disable();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void SF_Ctrl_AES_Enable_BE(void){
    return RomDriver_SF_Ctrl_AES_Enable_BE();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void SF_Ctrl_AES_Enable_LE(void){
    return RomDriver_SF_Ctrl_AES_Enable_LE();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void SF_Ctrl_AES_Set_Region(uint8_t region, uint8_t enable, uint8_t hwKey, uint32_t startAddr, uint32_t endAddr, uint8_t locked){
    return RomDriver_SF_Ctrl_AES_Set_Region(region,enable,hwKey,startAddr,endAddr,locked);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void SF_Ctrl_AES_Set_Key(uint8_t region, uint8_t *key, SF_Ctrl_AES_Key_Type keyType){
    return RomDriver_SF_Ctrl_AES_Set_Key(region,key,keyType);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void SF_Ctrl_AES_Set_Key_BE(uint8_t region, uint8_t *key, SF_Ctrl_AES_Key_Type keyType){
    return RomDriver_SF_Ctrl_AES_Set_Key_BE(region,key,keyType);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void SF_Ctrl_AES_Set_IV(uint8_t region, uint8_t *iv, uint32_t addrOffset){
    return RomDriver_SF_Ctrl_AES_Set_IV(region,iv,addrOffset);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void SF_Ctrl_AES_Set_IV_BE(uint8_t region, uint8_t *iv, uint32_t addrOffset){
    return RomDriver_SF_Ctrl_AES_Set_IV_BE(region,iv,addrOffset);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void SF_Ctrl_AES_Enable(void){
    return RomDriver_SF_Ctrl_AES_Enable();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void SF_Ctrl_AES_Disable(void){
    return RomDriver_SF_Ctrl_AES_Disable();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
uint8_t SF_Ctrl_Is_AES_Enable(void){
    return RomDriver_SF_Ctrl_Is_AES_Enable();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void SF_Ctrl_Set_Flash_Image_Offset(uint32_t addrOffset){
    return RomDriver_SF_Ctrl_Set_Flash_Image_Offset(addrOffset);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
uint32_t SF_Ctrl_Get_Flash_Image_Offset(void){
    return RomDriver_SF_Ctrl_Get_Flash_Image_Offset();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void SF_Ctrl_SendCmd(SF_Ctrl_Cmd_Cfg_Type *cfg){
    return RomDriver_SF_Ctrl_SendCmd(cfg);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void SF_Ctrl_Flash_Read_Icache_Set(SF_Ctrl_Cmd_Cfg_Type *cfg, uint8_t cmdValid){
    return RomDriver_SF_Ctrl_Flash_Read_Icache_Set(cfg,cmdValid);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void SF_Ctrl_Psram_Write_Icache_Set(SF_Ctrl_Cmd_Cfg_Type *cfg, uint8_t cmdValid){
    return RomDriver_SF_Ctrl_Psram_Write_Icache_Set(cfg,cmdValid);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void SF_Ctrl_Psram_Read_Icache_Set(SF_Ctrl_Cmd_Cfg_Type *cfg, uint8_t cmdValid){
    return RomDriver_SF_Ctrl_Psram_Read_Icache_Set(cfg,cmdValid);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Sts_Type SF_Ctrl_GetBusyState(void){
    return RomDriver_SF_Ctrl_GetBusyState();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void SFlash_Init(const SF_Ctrl_Cfg_Type *pSfCtrlCfg){
    return RomDriver_SFlash_Init(pSfCtrlCfg);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type SFlash_SetSPIMode(SF_Ctrl_Mode_Type mode){
    return RomDriver_SFlash_SetSPIMode(mode);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type SFlash_Read_Reg(SPI_Flash_Cfg_Type *flashCfg, uint8_t regIndex, uint8_t *regValue, uint8_t regLen){
    return RomDriver_SFlash_Read_Reg(flashCfg,regIndex,regValue,regLen);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type SFlash_Write_Reg(SPI_Flash_Cfg_Type *flashCfg, uint8_t regIndex, uint8_t *regValue, uint8_t regLen){
    return RomDriver_SFlash_Write_Reg(flashCfg,regIndex,regValue,regLen);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type SFlash_Read_Reg_With_Cmd(SPI_Flash_Cfg_Type *flashCfg, uint8_t readRegCmd, uint8_t *regValue, uint8_t regLen){
    return RomDriver_SFlash_Read_Reg_With_Cmd(flashCfg,readRegCmd,regValue,regLen);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type SFlash_Write_Reg_With_Cmd(SPI_Flash_Cfg_Type *flashCfg, uint8_t writeRegCmd, uint8_t *regValue, uint8_t regLen){
    return RomDriver_SFlash_Write_Reg_With_Cmd(flashCfg,writeRegCmd,regValue,regLen);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type SFlash_Clear_Status_Register(SPI_Flash_Cfg_Type *pFlashCfg){
    return RomDriver_SFlash_Clear_Status_Register(pFlashCfg);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Sts_Type SFlash_Busy(SPI_Flash_Cfg_Type *flashCfg){
    return RomDriver_SFlash_Busy(flashCfg);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type SFlash_Write_Enable(SPI_Flash_Cfg_Type *flashCfg){
    return RomDriver_SFlash_Write_Enable(flashCfg);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type SFlash_Qspi_Enable(SPI_Flash_Cfg_Type *flashCfg){
    return RomDriver_SFlash_Qspi_Enable(flashCfg);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void SFlash_Volatile_Reg_Write_Enable(SPI_Flash_Cfg_Type *flashCfg){
    return RomDriver_SFlash_Volatile_Reg_Write_Enable(flashCfg);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type SFlash_Chip_Erase(SPI_Flash_Cfg_Type *flashCfg){
    return RomDriver_SFlash_Chip_Erase(flashCfg);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type SFlash_Sector_Erase(SPI_Flash_Cfg_Type *flashCfg, uint32_t secNum){
    return RomDriver_SFlash_Sector_Erase(flashCfg,secNum);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type SFlash_Blk32_Erase(SPI_Flash_Cfg_Type *flashCfg, uint32_t blkNum){
    return RomDriver_SFlash_Blk32_Erase(flashCfg,blkNum);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type SFlash_Blk64_Erase(SPI_Flash_Cfg_Type *flashCfg, uint32_t blkNum){
    return RomDriver_SFlash_Blk64_Erase(flashCfg,blkNum);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type SFlash_Erase(SPI_Flash_Cfg_Type *flashCfg, uint32_t startaddr, uint32_t endaddr){
    return RomDriver_SFlash_Erase(flashCfg,startaddr,endaddr);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type SFlash_Program(SPI_Flash_Cfg_Type *flashCfg, SF_Ctrl_IO_Type ioMode, uint32_t addr, uint8_t *data, uint32_t len){
    return RomDriver_SFlash_Program(flashCfg,ioMode,addr,data,len);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void SFlash_GetUniqueId(uint8_t *data, uint8_t idLen){
    return RomDriver_SFlash_GetUniqueId(data,idLen);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void SFlash_GetJedecId(SPI_Flash_Cfg_Type *flashCfg, uint8_t *data){
    return RomDriver_SFlash_GetJedecId(flashCfg,data);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void SFlash_GetDeviceId(uint8_t *data){
    return RomDriver_SFlash_GetDeviceId(data);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void SFlash_Powerdown(void){
    return RomDriver_SFlash_Powerdown();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void SFlash_Releae_Powerdown(SPI_Flash_Cfg_Type *flashCfg){
    return RomDriver_SFlash_Releae_Powerdown(flashCfg);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type SFlash_Restore_From_Powerdown(SPI_Flash_Cfg_Type *pFlashCfg, uint8_t flashContRead){
    return RomDriver_SFlash_Restore_From_Powerdown(pFlashCfg,flashContRead);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void SFlash_SetBurstWrap(SPI_Flash_Cfg_Type *flashCfg){
    return RomDriver_SFlash_SetBurstWrap(flashCfg);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void SFlash_DisableBurstWrap(SPI_Flash_Cfg_Type *flashCfg){
    return RomDriver_SFlash_DisableBurstWrap(flashCfg);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type SFlash_Software_Reset(SPI_Flash_Cfg_Type *flashCfg){
    return RomDriver_SFlash_Software_Reset(flashCfg);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void SFlash_Reset_Continue_Read(SPI_Flash_Cfg_Type *flashCfg){
    return RomDriver_SFlash_Reset_Continue_Read(flashCfg);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type SFlash_Set_IDbus_Cfg(SPI_Flash_Cfg_Type *flashCfg, SF_Ctrl_IO_Type ioMode, uint8_t contRead, uint32_t addr, uint32_t len){
    return RomDriver_SFlash_Set_IDbus_Cfg(flashCfg,ioMode,contRead,addr,len);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type SFlash_IDbus_Read_Enable(SPI_Flash_Cfg_Type *flashCfg, SF_Ctrl_IO_Type ioMode, uint8_t contRead){
    return RomDriver_SFlash_IDbus_Read_Enable(flashCfg,ioMode,contRead);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type SFlash_Cache_Read_Enable(SPI_Flash_Cfg_Type *flashCfg, SF_Ctrl_IO_Type ioMode, uint8_t contRead, uint8_t wayDisable){
    return RomDriver_SFlash_Cache_Read_Enable(flashCfg,ioMode,contRead,wayDisable);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void SFlash_Cache_Read_Disable(void){
    return RomDriver_SFlash_Cache_Read_Disable();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type SFlash_Read(SPI_Flash_Cfg_Type *flashCfg, SF_Ctrl_IO_Type ioMode, uint8_t contRead, uint32_t addr, uint8_t *data, uint32_t len){
    return RomDriver_SFlash_Read(flashCfg,ioMode,contRead,addr,data,len);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type UART_SetBaudRate(UART_ID_Type uartId, uint32_t baudRate){
    return RomDriver_UART_SetBaudRate(uartId,baudRate);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type UART_Init(UART_ID_Type uartId, UART_CFG_Type *uartCfg){
    return RomDriver_UART_Init(uartId,uartCfg);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type UART_DeInit(UART_ID_Type uartId){
    return RomDriver_UART_DeInit(uartId);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type UART_FifoConfig(UART_ID_Type uartId, UART_FifoCfg_Type *fifoCfg){
    return RomDriver_UART_FifoConfig(uartId,fifoCfg);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type UART_Enable(UART_ID_Type uartId, UART_Direction_Type direct){
    return RomDriver_UART_Enable(uartId,direct);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type UART_Disable(UART_ID_Type uartId, UART_Direction_Type direct){
    return RomDriver_UART_Disable(uartId,direct);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type UART_SetTxDataLength(UART_ID_Type uartId, uint16_t length){
    return RomDriver_UART_SetTxDataLength(uartId,length);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type UART_SetRxDataLength(UART_ID_Type uartId, uint16_t length){
    return RomDriver_UART_SetRxDataLength(uartId,length);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type UART_SetRxTimeoutValue(UART_ID_Type uartId, uint8_t time){
    return RomDriver_UART_SetRxTimeoutValue(uartId,time);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type UART_SetRxByteCount(UART_ID_Type uartId, uint16_t count){
    return RomDriver_UART_SetRxByteCount(uartId,count);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type UART_SetDeglitchCount(UART_ID_Type uartId, uint8_t deglitchCnt){
    return RomDriver_UART_SetDeglitchCount(uartId,deglitchCnt);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type UART_ApplyAbrResult(UART_ID_Type uartId, UART_AutoBaudDetection_Type autoBaudDet){
    return RomDriver_UART_ApplyAbrResult(uartId,autoBaudDet);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type UART_SetRtsValue(UART_ID_Type uartId){
    return RomDriver_UART_SetRtsValue(uartId);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type UART_ClrRtsValue(UART_ID_Type uartId){
    return RomDriver_UART_ClrRtsValue(uartId);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type UART_SetTxValue(UART_ID_Type uartId){
    return RomDriver_UART_SetTxValue(uartId);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type UART_ClrTxValue(UART_ID_Type uartId){
    return RomDriver_UART_ClrTxValue(uartId);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type UART_TxFreeRun(UART_ID_Type uartId, BL_Fun_Type txFreeRun){
    return RomDriver_UART_TxFreeRun(uartId,txFreeRun);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type UART_AutoBaudDetection(UART_ID_Type uartId, BL_Fun_Type autoBaud){
    return RomDriver_UART_AutoBaudDetection(uartId,autoBaud);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type UART_SetAllowableError0X55(UART_ID_Type uartId, uint8_t allowableError){
    return RomDriver_UART_SetAllowableError0X55(uartId,allowableError);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type UART_GetBitWidth0X55(UART_ID_Type uartId, uint16_t *width){
    return RomDriver_UART_GetBitWidth0X55(uartId,width);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type UART_SetRS485(UART_ID_Type uartId, BL_Fun_Type enable, UART_RS485Polarity_Type polarity){
    return RomDriver_UART_SetRS485(uartId,enable,polarity);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type UART_TxFifoClear(UART_ID_Type uartId){
    return RomDriver_UART_TxFifoClear(uartId);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type UART_RxFifoClear(UART_ID_Type uartId){
    return RomDriver_UART_RxFifoClear(uartId);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type UART_IntMask(UART_ID_Type uartId, UART_INT_Type intType, BL_Mask_Type intMask){
    return RomDriver_UART_IntMask(uartId,intType,intMask);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type UART_IntClear(UART_ID_Type uartId, UART_INT_Type intType){
    return RomDriver_UART_IntClear(uartId,intType);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type UART_SendData(UART_ID_Type uartId, uint8_t *data, uint32_t len){
    return RomDriver_UART_SendData(uartId,data,len);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type UART_SendDataBlock(UART_ID_Type uartId, uint8_t *data, uint32_t len){
    return RomDriver_UART_SendDataBlock(uartId,data,len);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
uint32_t UART_ReceiveData(UART_ID_Type uartId, uint8_t *data, uint32_t maxLen){
    return RomDriver_UART_ReceiveData(uartId,data,maxLen);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
uint16_t UART_GetAutoBaudCount(UART_ID_Type uartId, UART_AutoBaudDetection_Type autoBaudDet){
    return RomDriver_UART_GetAutoBaudCount(uartId,autoBaudDet);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
uint16_t UART_GetRxByteCount(UART_ID_Type uartId){
    return RomDriver_UART_GetRxByteCount(uartId);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
uint8_t UART_GetTxFifoCount(UART_ID_Type uartId){
    return RomDriver_UART_GetTxFifoCount(uartId);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
uint8_t UART_GetRxFifoCount(UART_ID_Type uartId){
    return RomDriver_UART_GetRxFifoCount(uartId);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Sts_Type UART_GetIntStatus(UART_ID_Type uartId, UART_INT_Type intType){
    return RomDriver_UART_GetIntStatus(uartId,intType);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Sts_Type UART_GetTxBusBusyStatus(UART_ID_Type uartId){
    return RomDriver_UART_GetTxBusBusyStatus(uartId);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Sts_Type UART_GetRxBusBusyStatus(UART_ID_Type uartId){
    return RomDriver_UART_GetRxBusBusyStatus(uartId);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Sts_Type UART_GetOverflowStatus(UART_ID_Type uartId, UART_Overflow_Type overflow){
    return RomDriver_UART_GetOverflowStatus(uartId,overflow);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void XIP_SFlash_Opt_Enter(uint8_t *aesEnable){
    return RomDriver_XIP_SFlash_Opt_Enter(aesEnable);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void XIP_SFlash_Opt_Exit(uint8_t aesEnable){
    return RomDriver_XIP_SFlash_Opt_Exit(aesEnable);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type XIP_SFlash_State_Save(SPI_Flash_Cfg_Type *pFlashCfg, uint32_t *offset){
    return RomDriver_XIP_SFlash_State_Save(pFlashCfg,offset);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type XIP_SFlash_State_Restore(SPI_Flash_Cfg_Type *pFlashCfg, uint32_t offset){
    return RomDriver_XIP_SFlash_State_Restore(pFlashCfg,offset);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type XIP_SFlash_Erase_Need_Lock(SPI_Flash_Cfg_Type *pFlashCfg, uint32_t startaddr, uint32_t endaddr){
    return RomDriver_XIP_SFlash_Erase_Need_Lock(pFlashCfg,startaddr,endaddr);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type XIP_SFlash_Write_Need_Lock(SPI_Flash_Cfg_Type *pFlashCfg, uint32_t addr, uint8_t *data, uint32_t len){
    return RomDriver_XIP_SFlash_Write_Need_Lock(pFlashCfg,addr,data,len);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type XIP_SFlash_Read_Need_Lock(SPI_Flash_Cfg_Type *pFlashCfg, uint32_t addr, uint8_t *data, uint32_t len){
    return RomDriver_XIP_SFlash_Read_Need_Lock(pFlashCfg,addr,data,len);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type XIP_SFlash_GetJedecId_Need_Lock(SPI_Flash_Cfg_Type *pFlashCfg, uint8_t *data){
    return RomDriver_XIP_SFlash_GetJedecId_Need_Lock(pFlashCfg,data);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type XIP_SFlash_GetDeviceId_Need_Lock(SPI_Flash_Cfg_Type *pFlashCfg, uint8_t *data){
    return RomDriver_XIP_SFlash_GetDeviceId_Need_Lock(pFlashCfg,data);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type XIP_SFlash_GetUniqueId_Need_Lock(SPI_Flash_Cfg_Type *pFlashCfg, uint8_t *data, uint8_t idLen){
    return RomDriver_XIP_SFlash_GetUniqueId_Need_Lock(pFlashCfg,data,idLen);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type XIP_SFlash_Read_Via_Cache_Need_Lock(uint32_t addr, uint8_t *data, uint32_t len){
    return RomDriver_XIP_SFlash_Read_Via_Cache_Need_Lock(addr,data,len);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type XIP_SFlash_Clear_Status_Register_Need_Lock(SPI_Flash_Cfg_Type *pFlashCfg){
    return RomDriver_XIP_SFlash_Clear_Status_Register_Need_Lock(pFlashCfg);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type IR_TxInit(IR_TxCfg_Type *irTxCfg){
    return RomDriver_IR_TxInit(irTxCfg);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type IR_TxPulseWidthConfig(IR_TxPulseWidthCfg_Type *irTxPulseWidthCfg){
    return RomDriver_IR_TxPulseWidthConfig(irTxPulseWidthCfg);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type IR_FifoConfig(IR_FifoCfg_Type *fifoCfg){
    return RomDriver_IR_FifoConfig(fifoCfg);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type IR_DeInit(void){
    return RomDriver_IR_DeInit();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type IR_Enable(IR_Direction_Type direct){
    return RomDriver_IR_Enable(direct);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type IR_Disable(IR_Direction_Type direct){
    return RomDriver_IR_Disable(direct);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type IR_TxSWM(BL_Fun_Type txSWM){
    return RomDriver_IR_TxSWM(txSWM);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type IR_TxFifoClear(void){
    return RomDriver_IR_TxFifoClear();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type IR_SendData(uint32_t *data, uint8_t length){
    return RomDriver_IR_SendData(data,length);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type IR_SWMSendData(uint16_t *data, uint8_t length){
    return RomDriver_IR_SWMSendData(data,length);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type IR_SendCommand(uint32_t *data, uint8_t length){
    return RomDriver_IR_SendCommand(data,length);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type IR_SWMSendCommand(uint16_t *data, uint8_t length){
    return RomDriver_IR_SWMSendCommand(data,length);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type IR_SendNEC(uint8_t address, uint8_t command){
    return RomDriver_IR_SendNEC(address,command);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type IR_IntMask(IR_INT_Type intType, BL_Mask_Type intMask){
    return RomDriver_IR_IntMask(intType,intMask);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type IR_ClrIntStatus(IR_INT_Type intType){
    return RomDriver_IR_ClrIntStatus(intType);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Sts_Type IR_GetIntStatus(IR_INT_Type intType){
    return RomDriver_IR_GetIntStatus(intType);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Sts_Type IR_GetFifoStatus(IR_FifoStatus_Type fifoSts){
    return RomDriver_IR_GetFifoStatus(fifoSts);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
uint8_t IR_GetTxFifoCount(void){
    return RomDriver_IR_GetTxFifoCount();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type IR_LEDInit(uint8_t clk, uint8_t div, uint8_t unit, uint8_t code0H, uint8_t code0L, uint8_t code1H, uint8_t code1L){
    return RomDriver_IR_LEDInit(clk,div,unit,code0H,code0L,code1H,code1L);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type IR_LEDSend(uint32_t data){
    return RomDriver_IR_LEDSend(data);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type KYS_Init(KYS_CFG_Type *kysCfg){
    return RomDriver_KYS_Init(kysCfg);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type KYS_Enable(void){
    return RomDriver_KYS_Enable();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type KYS_Disable(void){
    return RomDriver_KYS_Disable();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type KYS_IntMask(KYS_INT_Type intType, BL_Mask_Type intMask){
    return RomDriver_KYS_IntMask(intType,intMask);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type KYS_IntClear(KYS_INT_CLR_Type intType){
    return RomDriver_KYS_IntClear(intType);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Sts_Type KYS_GetIntStatus(KYS_INT_Type intType){
    return RomDriver_KYS_GetIntStatus(intType);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void KYS_Get_FIFO_Idx(uint8_t *fifo_head, uint8_t *fifo_tail){
    return RomDriver_KYS_Get_FIFO_Idx(fifo_head,fifo_tail);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
uint8_t KYS_ReadKeyfifo(void){
    return RomDriver_KYS_ReadKeyfifo();
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void Psram_Init(SPI_Psram_Cfg_Type *psramCfg, SF_Ctrl_Cmds_Cfg *cmdsCfg, SF_Ctrl_Psram_Cfg *sfCtrlPsramCfg){
    return RomDriver_Psram_Init(psramCfg,cmdsCfg,sfCtrlPsramCfg);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void Psram_ReadReg(SPI_Psram_Cfg_Type *psramCfg, uint8_t *regValue){
    return RomDriver_Psram_ReadReg(psramCfg,regValue);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void Psram_WriteReg(SPI_Psram_Cfg_Type *psramCfg, uint8_t *regValue){
    return RomDriver_Psram_WriteReg(psramCfg,regValue);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type Psram_SetDriveStrength(SPI_Psram_Cfg_Type *psramCfg){
    return RomDriver_Psram_SetDriveStrength(psramCfg);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type Psram_SetBurstWrap(SPI_Psram_Cfg_Type *psramCfg){
    return RomDriver_Psram_SetBurstWrap(psramCfg);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void Psram_ReadId(SPI_Psram_Cfg_Type *psramCfg, uint8_t *data){
    return RomDriver_Psram_ReadId(psramCfg,data);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type Psram_EnterQuadMode(SPI_Psram_Cfg_Type *psramCfg){
    return RomDriver_Psram_EnterQuadMode(psramCfg);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type Psram_ExitQuadMode(SPI_Psram_Cfg_Type *psramCfg){
    return RomDriver_Psram_ExitQuadMode(psramCfg);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type Psram_ToggleBurstLength(SPI_Psram_Cfg_Type *psramCfg, PSRAM_Ctrl_Mode ctrlMode){
    return RomDriver_Psram_ToggleBurstLength(psramCfg,ctrlMode);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type Psram_SoftwareReset(SPI_Psram_Cfg_Type *psramCfg, PSRAM_Ctrl_Mode ctrlMode){
    return RomDriver_Psram_SoftwareReset(psramCfg,ctrlMode);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type Psram_Set_IDbus_Cfg(SPI_Psram_Cfg_Type *psramCfg, SF_Ctrl_IO_Type ioMode, uint32_t addr, uint32_t len){
    return RomDriver_Psram_Set_IDbus_Cfg(psramCfg,ioMode,addr,len);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type Psram_Cache_Write_Set(SPI_Psram_Cfg_Type *psramCfg, SF_Ctrl_IO_Type ioMode, BL_Fun_Type wtEn, BL_Fun_Type wbEn, BL_Fun_Type waEn){
    return RomDriver_Psram_Cache_Write_Set(psramCfg,ioMode,wtEn,wbEn,waEn);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type Psram_Write(SPI_Psram_Cfg_Type *psramCfg, SF_Ctrl_IO_Type ioMode, uint32_t addr, uint8_t *data, uint32_t len){
    return RomDriver_Psram_Write(psramCfg,ioMode,addr,data,len);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type Psram_Read(SPI_Psram_Cfg_Type *psramCfg, SF_Ctrl_IO_Type ioMode, uint32_t addr, uint8_t *data, uint32_t len){
    return RomDriver_Psram_Read(psramCfg,ioMode,addr,data,len);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
uint32_t TIMER_GetCompValue(TIMER_ID_Type timerId, TIMER_Chan_Type timerCh, TIMER_Comp_ID_Type cmpNo){
    return RomDriver_TIMER_GetCompValue(timerId,timerCh,cmpNo);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void TIMER_SetCompValue(TIMER_ID_Type timerId, TIMER_Chan_Type timerCh, TIMER_Comp_ID_Type cmpNo, uint32_t val){
    return RomDriver_TIMER_SetCompValue(timerId,timerCh,cmpNo,val);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void TIMER_CompValueEffectImmediately(TIMER_ID_Type timerId, TIMER_Chan_Type timerCh, BL_Fun_Type enable){
    return RomDriver_TIMER_CompValueEffectImmediately(timerId,timerCh,enable);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
uint32_t TIMER_GetCounterValue(TIMER_ID_Type timerId, TIMER_Chan_Type timerCh){
    return RomDriver_TIMER_GetCounterValue(timerId,timerCh);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void TIMER_ResetCounterValue(TIMER_ID_Type timerId, TIMER_Chan_Type timerCh){
    return RomDriver_TIMER_ResetCounterValue(timerId,timerCh);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Sts_Type TIMER_GetMatchStatus(TIMER_ID_Type timerId, TIMER_Chan_Type timerCh, TIMER_Comp_ID_Type cmpNo){
    return RomDriver_TIMER_GetMatchStatus(timerId,timerCh,cmpNo);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
uint32_t TIMER_GetPreloadValue(TIMER_ID_Type timerId, TIMER_Chan_Type timerCh){
    return RomDriver_TIMER_GetPreloadValue(timerId,timerCh);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void TIMER_SetPreloadValue(TIMER_ID_Type timerId, TIMER_Chan_Type timerCh, uint32_t val){
    return RomDriver_TIMER_SetPreloadValue(timerId,timerCh,val);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void TIMER_SetPreloadTrigSrc(TIMER_ID_Type timerId, TIMER_Chan_Type timerCh, TIMER_PreLoad_Trig_Type plSrc){
    return RomDriver_TIMER_SetPreloadTrigSrc(timerId,timerCh,plSrc);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void TIMER_SetCountMode(TIMER_ID_Type timerId, TIMER_Chan_Type timerCh, TIMER_CountMode_Type countMode){
    return RomDriver_TIMER_SetCountMode(timerId,timerCh,countMode);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void TIMER_ClearIntStatus(TIMER_ID_Type timerId, TIMER_Chan_Type timerCh, TIMER_Comp_ID_Type cmpNo){
    return RomDriver_TIMER_ClearIntStatus(timerId,timerCh,cmpNo);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type TIMER_Init(TIMER_ID_Type timerId, TIMER_CFG_Type *timerCfg){
    return RomDriver_TIMER_Init(timerId,timerCfg);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Err_Type TIMER_DeInit(TIMER_ID_Type timerId){
    return RomDriver_TIMER_DeInit(timerId);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void TIMER_Enable(TIMER_ID_Type timerId, TIMER_Chan_Type timerCh){
    return RomDriver_TIMER_Enable(timerId,timerCh);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void TIMER_Disable(TIMER_ID_Type timerId, TIMER_Chan_Type timerCh){
    return RomDriver_TIMER_Disable(timerId,timerCh);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void TIMER_IntMask(TIMER_ID_Type timerId, TIMER_Chan_Type timerCh, TIMER_INT_Type intType, BL_Mask_Type intMask){
    return RomDriver_TIMER_IntMask(timerId,timerCh,intType,intMask);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void TIMER_GPIOSetPolarity(TIMER_ID_Type timerId, TIMER_Chan_Type timerCh, TIMER_GPIO_Polarity_Type polarity){
    return RomDriver_TIMER_GPIOSetPolarity(timerId,timerCh,polarity);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void TIMER_CH0_SetMeasurePulseWidth(TIMER_ID_Type timerId, BL_Fun_Type enable){
    return RomDriver_TIMER_CH0_SetMeasurePulseWidth(timerId,enable);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
uint32_t TIMER_CH0_GetMeasurePulseWidth(TIMER_ID_Type timerId){
    return RomDriver_TIMER_CH0_GetMeasurePulseWidth(timerId);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void TIMER_ForceClockDivision(TIMER_ID_Type timerId, TIMER_Chan_Type timerCh){
    return RomDriver_TIMER_ForceClockDivision(timerId,timerCh);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void WDT_Set_Clock(WDT_ID_Type wdtId, TIMER_ClkSrc_Type clkSrc, uint8_t div){
    return RomDriver_WDT_Set_Clock(wdtId,clkSrc,div);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
uint16_t WDT_GetMatchValue(WDT_ID_Type wdtId){
    return RomDriver_WDT_GetMatchValue(wdtId);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void WDT_SetCompValue(WDT_ID_Type wdtId, uint16_t val){
    return RomDriver_WDT_SetCompValue(wdtId,val);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void WDT_CompValueEffectImmediately(WDT_ID_Type wdtId, BL_Fun_Type enable){
    return RomDriver_WDT_CompValueEffectImmediately(wdtId,enable);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
uint16_t WDT_GetCounterValue(WDT_ID_Type wdtId){
    return RomDriver_WDT_GetCounterValue(wdtId);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void WDT_ResetCounterValue(WDT_ID_Type wdtId){
    return RomDriver_WDT_ResetCounterValue(wdtId);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
BL_Sts_Type WDT_GetResetStatus(WDT_ID_Type wdtId){
    return RomDriver_WDT_GetResetStatus(wdtId);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void WDT_ClearResetStatus(WDT_ID_Type wdtId){
    return RomDriver_WDT_ClearResetStatus(wdtId);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void WDT_Enable(WDT_ID_Type wdtId){
    return RomDriver_WDT_Enable(wdtId);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void WDT_Disable(WDT_ID_Type wdtId){
    return RomDriver_WDT_Disable(wdtId);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void WDT_ForceClockDivision(WDT_ID_Type wdtId){
    return RomDriver_WDT_ForceClockDivision(wdtId);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void WDT_IntMask(WDT_ID_Type wdtId, WDT_INT_Type intType, BL_Mask_Type intMask){
    return RomDriver_WDT_IntMask(wdtId,intType,intMask);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void WDT_GPIOSetPolarity(WDT_ID_Type wdtId, TIMER_GPIO_Polarity_Type polarity){
    return RomDriver_WDT_GPIOSetPolarity(wdtId,polarity);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void * arch_memcpy(void *dst, const void *src, uint32_t n){
    return RomDriver_arch_memcpy(dst,src,n);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
uint32_t * arch_memcpy4(uint32_t *dst, const uint32_t *src, uint32_t n){
    return RomDriver_arch_memcpy4(dst,src,n);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void * arch_memcpy_fast(void *pdst, const void *psrc, uint32_t n){
    return RomDriver_arch_memcpy_fast(pdst,psrc,n);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void * arch_memset(void *s, uint8_t c, uint32_t n){
    return RomDriver_arch_memset(s,c,n);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
uint32_t * arch_memset4(uint32_t *dst, const uint32_t val, uint32_t n){
    return RomDriver_arch_memset4(dst,val,n);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
int arch_memcmp(const void *s1, const void *s2, uint32_t n){
    return RomDriver_arch_memcmp(s1,s2,n);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void memcopy_to_fifo(void *fifo_addr, uint8_t *data, uint32_t length){
    return RomDriver_memcopy_to_fifo(fifo_addr,data,length);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
void fifocopy_to_mem(void *fifo_addr, uint8_t *data, uint32_t length){
    return RomDriver_fifocopy_to_mem(fifo_addr,data,length);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
uint32_t BFLB_Soft_CRC32_Ex(uint32_t initial, void *dataIn, uint32_t len){
    return RomDriver_BFLB_Soft_CRC32_Ex(initial,dataIn,len);
}

__ALWAYS_INLINE ATTR_TCM_SECTION
uint32_t BFLB_Soft_CRC32(void *dataIn, uint32_t len){
    return RomDriver_BFLB_Soft_CRC32(dataIn,len);
}