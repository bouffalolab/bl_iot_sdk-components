#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <bl702.h>
#include <bl702_dma.h>
#include <bl702_rf_public.h>

#include "bl_efuse.h"
#include "bl_wireless.h"
#include "bl_irq.h"

typedef struct _bl_wireless_env {
    uint8_t mac_addr[8];
    int8_t power_offset_zigbee[16];
    int8_t power_offset_ble[40];
    uint8_t power_tcal_en;
} bl_wireless_env_t;

bl_wireless_env_t wireless_env;


int bl_wireless_mac_addr_set(uint8_t mac[8])
{
    memcpy(wireless_env.mac_addr, mac, 8);
    return 0;
}

int bl_wireless_mac_addr_get(uint8_t mac[8])
{
    memcpy(mac, wireless_env.mac_addr, 8);
    return 0;
}

int bl_wireless_power_offset_set(int8_t poweroffset_zigbee[16], int8_t poweroffset_ble[40])
{
    memcpy(wireless_env.power_offset_zigbee, poweroffset_zigbee, 16);
    memcpy(wireless_env.power_offset_ble, poweroffset_ble, 40);
    return 0;
}

int bl_wireless_power_offset_get(int8_t poweroffset_zigbee[16], int8_t poweroffset_ble[40])
{
    memcpy(poweroffset_zigbee, wireless_env.power_offset_zigbee, 16);
    memcpy(poweroffset_ble, wireless_env.power_offset_ble, 40);
    return 0;
}

int8_t bl_wireless_power_offset_zigbee_get(uint8_t ch)
{
    if(ch < 11){
        return wireless_env.power_offset_zigbee[0];
    }else if(ch > 26){
        return wireless_env.power_offset_zigbee[15];
    }else{
        return wireless_env.power_offset_zigbee[ch - 11];
    }
}

int8_t bl_wireless_power_offset_ble_get(uint8_t ch)
{
    if(ch > 39){
        return wireless_env.power_offset_ble[39];
    }else{
        return wireless_env.power_offset_ble[ch];
    }
}

void bl_wireless_power_tcal_en_set(uint8_t en)
{
    wireless_env.power_tcal_en = en;
}

uint8_t bl_wireless_power_tcal_en_get(void)
{
    return wireless_env.power_tcal_en;
}


void rf_full_cal_start_callback(uint32_t addr, uint32_t size)
{
    uint8_t ch;
    uint32_t DMAx;
    uint32_t tmpVal;
    uint32_t src_addr, dst_addr;
    uint8_t src_width, dst_width;
    uint8_t src_inc, dst_inc;
    uint16_t transfer_size;
    
    if(!BL_IS_REG_BIT_SET(BL_RD_REG(DMA_BASE, DMA_TOP_CONFIG), DMA_E)){
        //printf("DMA disabled\r\n");
        return;
    }
    
    for(ch = 0; ch < DMA_CH_MAX; ch++){
        DMAx = DMA_BASE + DMA_CHANNEL_OFFSET + ch * 0x100;
        if(!BL_IS_REG_BIT_SET(BL_RD_REG(DMAx, DMA_CONFIG), DMA_E)){
            //printf("DMA ch%d disabled\r\n", ch);
            continue;
        }
        
        src_addr = BL_RD_REG(DMAx, DMA_SRCADDR);
        dst_addr = BL_RD_REG(DMAx, DMA_DSTADDR);
        
        tmpVal = BL_RD_REG(DMAx, DMA_CONTROL);
        src_width = 1 << BL_GET_REG_BITS_VAL(tmpVal, DMA_SWIDTH);
        dst_width = 1 << BL_GET_REG_BITS_VAL(tmpVal, DMA_DWIDTH);
        src_inc = BL_GET_REG_BITS_VAL(tmpVal, DMA_SI);
        dst_inc = BL_GET_REG_BITS_VAL(tmpVal, DMA_DI);
        transfer_size = BL_GET_REG_BITS_VAL(tmpVal, DMA_TRANSFERSIZE);
        
        if(src_addr >= 0x42010000 && src_addr < 0x42030000){
            if(src_addr >= addr && src_addr < addr + size){
                printf("DMA ch%d src addr(0x%08lX) conflicts with rf full calibration area(0x%08lX, 0x%08lX)\r\n", 
                        ch, src_addr, addr, addr + size);
                while(1);
            }else{
                if(src_inc && src_addr < addr && src_addr + src_width * transfer_size > addr){
                    printf("DMA ch%d src area(0x%08lX, 0x%08lX) conflicts with rf full calibration area(0x%08lX, 0x%08lX)\r\n", 
                            ch, src_addr, src_addr + src_width * transfer_size, addr, addr + size);
                    while(1);
                }
            }
        }
        
        if(dst_addr >= 0x42010000 && dst_addr < 0x42030000){
            if(dst_addr >= addr && dst_addr < addr + size){
                printf("DMA ch%d dst addr(0x%08lX) conflicts with rf full calibration area(0x%08lX, 0x%08lX)\r\n", 
                        ch, dst_addr, addr, addr + size);
                while(1);
            }else{
                if(dst_inc && dst_addr < addr && dst_addr + dst_width * transfer_size > addr){
                    printf("DMA ch%d dst area(0x%08lX, 0x%08lX) conflicts with rf full calibration area(0x%08lX, 0x%08lX)\r\n", 
                            ch, dst_addr, dst_addr + dst_width * transfer_size, addr, addr + size);
                    while(1);
                }
            }
        }
        
        //printf("DMA ch%d ok\r\n");
        continue;
    }
}

void rf_reset_done_callback(void)
{
#if defined(CFG_TCAL_ENABLE)
    extern int hal_tcal_restart(void);
    hal_tcal_restart();
#endif
}
