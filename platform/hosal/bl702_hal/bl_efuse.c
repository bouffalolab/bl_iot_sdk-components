#include <bl702_ef_ctrl.h>
#include <blog.h>
#include "bl_efuse.h"

int bl_efuse_reload(void)
{
    EF_Ctrl_Load_Efuse_R0();

    return 0;
}

int bl_efuse_read(uint32_t offset, uint32_t *data, uint32_t len)
{
    if(offset % 4 != 0){
        return -1;
    }

    if(!data || !len){
        return -2;
    }

    if(offset + len * 4 > 128){
        return -3;
    }

    ARCH_MemCpy4(data, (uint32_t *)(EF_DATA_BASE + offset), len);

    return 0;
}

int bl_efuse_write(uint32_t offset, uint32_t *data, uint32_t len)
{
    if(offset % 4 != 0){
        return -1;
    }

    if(!data || !len){
        return -2;
    }

    if(offset + len * 4 > 128){
        return -3;
    }

    ARCH_MemCpy4((uint32_t *)(EF_DATA_BASE + offset), data, len);

    return 0;
}

int bl_efuse_program(void)
{
    EF_Ctrl_Program_Direct_R0(0, NULL, 0);
    while(SET == EF_Ctrl_Busy());
    EF_Ctrl_Load_Efuse_R0();

    return 0;
}

int bl_efuse_read_mac(uint8_t mac[8])
{
    uint8_t empty;

    empty = EF_Ctrl_Is_MAC_Address_Slot_Empty(2, 0);
    if(!empty){
        if(EF_Ctrl_Read_MAC_Address_Opt(2, mac, 0) == 0){
            blog_info("Read mac from slot 2\r\n");
            return 0;
        }
    }

    empty = EF_Ctrl_Is_MAC_Address_Slot_Empty(1, 0);
    if(!empty){
        if(EF_Ctrl_Read_MAC_Address_Opt(1, mac, 0) == 0){
            blog_info("Read mac from slot 1\r\n");
            return 0;
        }
    }

    empty = EF_Ctrl_Is_MAC_Address_Slot_Empty(0, 0);
    if(!empty){
        if(EF_Ctrl_Read_MAC_Address_Opt(0, mac, 0) == 0){
            blog_info("Read mac from slot 0\r\n");
            return 0;
        }
    }

    blog_info("Read mac from slot N/A\r\n");
    return -1;
}

int bl_efuse_read_mac_factory(uint8_t mac[8])
{
    return -1;
}

int bl_efuse_read_capcode(uint8_t *capcode)
{
    uint8_t empty;

    empty = EF_Ctrl_Is_CapCode_Empty(2, 0);
    if(!empty){
        if(EF_Ctrl_Read_CapCode_Opt(2, capcode, 0) == 0){
            //blog_info("Read cap code from slot 2\r\n");
            return 0;
        }
    }

    empty = EF_Ctrl_Is_CapCode_Empty(1, 0);
    if(!empty){
        if(EF_Ctrl_Read_CapCode_Opt(1, capcode, 0) == 0){
            //blog_info("Read cap code from slot 1\r\n");
            return 0;
        }
    }

    empty = EF_Ctrl_Is_CapCode_Empty(0, 0);
    if(!empty){
        if(EF_Ctrl_Read_CapCode_Opt(0, capcode, 0) == 0){
            //blog_info("Read cap code from slot 0\r\n");
            return 0;
        }
    }

    //blog_info("Read cap code from slot N/A\r\n");
    return -1;
}

int bl_efuse_read_pwroft(int8_t poweroffset[2])
{
    uint8_t empty;

    empty = EF_Ctrl_Is_PowerOffset_Slot_Empty(2, 0);
    if(!empty){
        if(EF_Ctrl_Read_PowerOffset_Opt(2, poweroffset, 0) == 0){
            //blog_info("Read power offset from slot 2\r\n");
            return 0;
        }
    }

    empty = EF_Ctrl_Is_PowerOffset_Slot_Empty(1, 0);
    if(!empty){
        if(EF_Ctrl_Read_PowerOffset_Opt(1, poweroffset, 0) == 0){
            //blog_info("Read power offset from slot 1\r\n");
            return 0;
        }
    }

    empty = EF_Ctrl_Is_PowerOffset_Slot_Empty(0, 0);
    if(!empty){
        if(EF_Ctrl_Read_PowerOffset_Opt(0, poweroffset, 0) == 0){
            //blog_info("Read power offset from slot 0\r\n");
            return 0;
        }
    }

    //blog_info("Read power offset from slot N/A\r\n");
    return -1;
}

static int8_t bl_efuse_power_offset_interpolation(int8_t poweroffset[2], uint8_t x0, uint8_t x1, uint8_t x)
{
    int8_t y0, y1;

    y0 = poweroffset[0];
    y1 = poweroffset[1];

    if(y0 == y1){
        return y0;
    }else if(x <= x0){
        return y0;
    }else if(x >= x1){
        return y1;
    }else{
        return (int8_t)round((double)(y1 - y0)*(x - x0)/(x1 - x0)) + y0;
    }
}

int bl_efuse_read_pwroft_ex(int8_t poweroffset_zigbee[16], int8_t poweroffset_ble[40])
{
    int8_t poweroffset[2];
    int i;

    if(bl_efuse_read_pwroft(poweroffset) == 0){
        for(i=0; i<16; i++){
            poweroffset_zigbee[i] = bl_efuse_power_offset_interpolation(poweroffset, 0, 15, i);
        }

        for(i=0; i<40; i++){
            poweroffset_ble[i] = bl_efuse_power_offset_interpolation(poweroffset, 0, 39, i);
        }

        return 0;
    }else{
        return -1;
    }
}

int bl_efuse_read_tsen_refcode(int16_t *refcode)
{
    Efuse_TSEN_Refcode_Corner_Type trimTsen;

    EF_Ctrl_Read_TSEN_Trim(&trimTsen);
    *refcode = (uint16_t)trimTsen.tsenRefcodeCorner;  // 12-bit unsigned value

    if(trimTsen.tsenRefcodeCornerEn == 1 && trimTsen.tsenRefcodeCornerParity == EF_Ctrl_Get_Trim_Parity(trimTsen.tsenRefcodeCorner, 12)){
        return 0;
    }else{
        return -1;
    }
}

int bl_efuse_read_adc_gain_coe(float *coe)
{
    Efuse_ADC_Gain_Coeff_Type trimAdc;
    int16_t coeff;

    EF_Ctrl_Read_ADC_Gain_Trim(&trimAdc);
    coeff = (trimAdc.adcGainCoeff < 2048) ? trimAdc.adcGainCoeff : (trimAdc.adcGainCoeff - 4096);  // 12-bit signed value

    if(trimAdc.adcGainCoeffEn == 1 && trimAdc.adcGainCoeffParity == EF_Ctrl_Get_Trim_Parity(trimAdc.adcGainCoeff, 12)){
        *coe = 1.0 - (float)coeff / 2048;
        return 0;
    }else{
        *coe = 1.0;
        return -1;
    }
}
