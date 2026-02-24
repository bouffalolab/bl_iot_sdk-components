/*
 * Copyright (c) 2016-2026 Bouffalolab.
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
#include <bl702l_ef_cfg.h>
#include <bl702l_ef_ctrl.h>
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

#if 0
    empty = EF_Ctrl_Is_MAC_Address_Slot_Empty(2, 0);
    if(!empty){
        if(EF_Ctrl_Read_MAC_Address_Opt(2, mac, 0) == 0){
            blog_info("Read mac from slot 2\r\n");
            return 0;
        }
    }
#endif

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

int bl_efuse_read_pwroft(int8_t poweroffset[4])
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

int bl_efuse_read_pwroft_ex(int8_t poweroffset_zigbee[16], int8_t poweroffset_ble[4])
{
    int8_t poweroffset[4];
    int i;

    if(bl_efuse_read_pwroft(poweroffset) == 0){
        for(i=0; i<4; i++){
            poweroffset_zigbee[4*i] = poweroffset[i];
            poweroffset_zigbee[4*i+1] = poweroffset[i];
            poweroffset_zigbee[4*i+2] = poweroffset[i];
            poweroffset_zigbee[4*i+3] = poweroffset[i];
            poweroffset_ble[i] = poweroffset[i];
        }

        return 0;
    }else{
        return -1;
    }
}

int bl_efuse_read_tsen_refcode(int16_t *refcode)
{
    Efuse_Common_Trim_Type trimTsen;

    EF_Ctrl_Read_Common_Trim("tsen", &trimTsen);
    *refcode = (uint16_t)trimTsen.value;  // 12-bit unsigned value

    if(trimTsen.en == 1 && trimTsen.parity == EF_Ctrl_Get_Trim_Parity(trimTsen.value, trimTsen.len)){
        return 0;
    }else{
        return -1;
    }
}

int bl_efuse_read_adc_gain_coe(float *coe)
{
    Efuse_Common_Trim_Type trimAdc;
    int16_t coeff;

    EF_Ctrl_Read_Common_Trim("gpadc_gain", &trimAdc);
    coeff = (trimAdc.value < 2048) ? trimAdc.value : (trimAdc.value - 4096);  // 12-bit signed value

    if(trimAdc.en == 1 && trimAdc.parity == EF_Ctrl_Get_Trim_Parity(trimAdc.value, trimAdc.len)){
        *coe = 1.0 - (float)coeff / 2048;
        return 0;
    }else{
        *coe = 1.0;
        return -1;
    }
}
