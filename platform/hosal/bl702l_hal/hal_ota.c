#include "hal_ota.h"
#include "hal_boot2.h"
#include "bl_boot2.h"
#include "bl_flash.h"
#include "bl_sec_sha.h"
#include "softcrc.h"


static PtTable_Stuff_Config *ota_pt = NULL;
static uint32_t ota_pt_len = 0;
static uint8_t *ota_buffer = NULL;
static uint32_t ota_buffer_size = 0;
static uint32_t ota_write_size = 0;
static uint8_t ota_entry_index = 0;
static uint32_t ota_entry_addr = 0;
static uint32_t ota_entry_size = 0;


static uint8_t PtTable_Valid(PtTable_Stuff_Config *ptStuff)
{
    PtTable_Config *ptTable = &ptStuff->ptTable;
    PtTable_Entry_Config *ptEntries = ptStuff->ptEntries;
    uint32_t ptEntriesLen;
    uint32_t ptEntriesCrc;
    
    if(ptTable->magicCode != BFLB_PT_MAGIC_CODE){
        printf("[OTA] PT Magic Code Error\r\n");
        return 0;
    }
    
    if(ptTable->entryCnt > PT_ENTRY_MAX){
        printf("[OTA] PT Entry Count Error\r\n");
        return 0;
    }
    
    if(ptTable->crc32 != BFLB_Soft_CRC32((uint8_t *)ptTable, sizeof(PtTable_Config) - 4)){
        printf("[OTA] PT CRC Error\r\n");
        return 0;
    }
    
    ptEntriesLen = sizeof(PtTable_Entry_Config) * ptTable->entryCnt;
    ptEntriesCrc = *(uint32_t *)((uint32_t)ptEntries + ptEntriesLen);
    if(ptEntriesCrc != BFLB_Soft_CRC32((uint8_t *)ptEntries, ptEntriesLen)){
        printf("[OTA] PT Entry CRC Error\r\n");
        return 0;
    }
    
    return 1;
}

static void PtTable_Update(PtTable_Stuff_Config *ptStuff, uint32_t ptTableAge, uint8_t fwActiveIndex)
{
    PtTable_Config *ptTable = &ptStuff->ptTable;
    PtTable_Entry_Config *ptEntries = ptStuff->ptEntries;
    uint32_t ptEntriesLen;
    
    ptTable->age = ptTableAge;
    ptTable->crc32 = BFLB_Soft_CRC32((uint8_t *)ptTable, sizeof(PtTable_Config) - 4);
    
    ptEntries[0].activeIndex = fwActiveIndex;
    ptEntriesLen = sizeof(PtTable_Entry_Config) * ptTable->entryCnt;
    *(uint32_t *)((uint32_t)ptEntries + ptEntriesLen) = BFLB_Soft_CRC32((uint8_t *)ptEntries, ptEntriesLen);
}


int hal_ota_start(uint8_t *pt, uint32_t len)
{
    uint32_t addr[2] = {0, 0};
    uint32_t size[2] = {0, 0};
    int active_index = 0;
    
    if(ota_pt){
        free(ota_pt);
        ota_pt = NULL;
    }
    
    if(pt && len){
        ota_pt = malloc(len);
        memcpy(ota_pt, pt, len);
        ota_pt_len = len;
        
        if(!PtTable_Valid(ota_pt)){
            free(ota_pt);
            ota_pt = NULL;
            return -1;
        }
    }
    
    if(!ota_buffer){
        ota_buffer = malloc(4096);
    }
    
    ota_buffer_size = 0;
    ota_write_size = 0;
    
    hal_boot2_partition_addr("FW", &addr[0], &addr[1], &size[0], &size[1], &active_index);
    
    ota_entry_index = !active_index;
    
    if(!ota_pt){
        ota_entry_addr = addr[ota_entry_index];
        ota_entry_size = size[ota_entry_index];
    }else{
        ota_entry_addr = ota_pt->ptEntries[0].Address[ota_entry_index];
        ota_entry_size = ota_pt->ptEntries[0].maxLen[ota_entry_index];
    }
    
    printf("[OTA] index: %d, addr: 0x%lx, size: 0x%lx\r\n", ota_entry_index, ota_entry_addr, ota_entry_size);
    
    return 0;
}

int hal_ota_update(uint8_t *data, uint32_t len)
{
    if(!ota_buffer){
        printf("[OTA] forget to call hal_ota_start?\r\n");
        return -1;
    }
    
    if(ota_write_size + ota_buffer_size + len > ota_entry_size){
        printf("[OTA] fw overflow\r\n");
        return -2;
    }
    
    if(ota_buffer_size != 0){
        if(len < 4096 - ota_buffer_size){
            memcpy(ota_buffer + ota_buffer_size, data, len);
            ota_buffer_size += len;
            return 0;
        }else{
            memcpy(ota_buffer + ota_buffer_size, data, 4096 - ota_buffer_size);
            data += 4096 - ota_buffer_size;
            len -= 4096 - ota_buffer_size;
            ota_buffer_size = 0;
            
            bl_flash_erase(ota_entry_addr + ota_write_size, 1);
            bl_flash_write(ota_entry_addr + ota_write_size, ota_buffer, 4096);
            printf("[OTA] flash write: 0x%04lx, %lu\r\n", ota_write_size, 4096UL);
            ota_write_size += 4096;
        }
    }
    
    while(len >= 4096){
        bl_flash_erase(ota_entry_addr + ota_write_size, 1);
        bl_flash_write(ota_entry_addr + ota_write_size, data, 4096);
        printf("[OTA] flash write: 0x%04lx, %lu\r\n", ota_write_size, 4096UL);
        ota_write_size += 4096;
        
        data += 4096;
        len -= 4096;
    }
    
    if(len != 0){
        memcpy(ota_buffer, data, len);
        ota_buffer_size = len;
    }
    
    return 0;
}

int hal_ota_finish(void)
{
    bl_sha_ctx_t ctx;
    uint8_t hash[32];
    uint32_t addr;
    uint32_t size;
    int ret = 0;
    
    if(ota_buffer_size != 0){
        bl_flash_erase(ota_entry_addr + ota_write_size, 1);
        bl_flash_write(ota_entry_addr + ota_write_size, ota_buffer, ota_buffer_size);
        printf("[OTA] flash write: 0x%04lx, %lu\r\n", ota_write_size, ota_buffer_size);
        ota_write_size += ota_buffer_size;
    }
    
    if(ota_write_size <= 32){
        printf("[OTA] fw too small %lu\r\n", ota_write_size);
        ret = -1;
        goto finish;
    }
    
    bl_sha_init(&ctx, BL_SHA256);
    
    addr = ota_entry_addr;
    size = ota_write_size - 32;
    
    while(size >= 4096){
        bl_flash_read(addr, ota_buffer, 4096);
        bl_sha_update(&ctx, ota_buffer, 4096);
        addr += 4096;
        size -= 4096;
    }
    
    if(size != 0){
        bl_flash_read(addr, ota_buffer, size);
        bl_sha_update(&ctx, ota_buffer, size);
        addr += size;
    }
    
    bl_sha_finish(&ctx, hash);
    
#if 1
    printf("[OTA] hash:");
    for(int i=0; i<32; i++){
        printf(" %02x", hash[i]);
    }
    printf("\r\n");
#endif
    
    bl_flash_read(addr, ota_buffer, 32);
    
    if(memcmp(ota_buffer, hash, 32) != 0){
        printf("[OTA] hash check error\r\n");
        ret = -2;
        goto finish;
    }
    
    if(!ota_pt){
        HALPartition_Entry_Config ptEntry;
        hal_boot2_get_active_entries_byname((uint8_t *)"FW", &ptEntry);
        hal_boot2_update_ptable(&ptEntry);
        printf("[OTA] fw update success\r\n");
    }else{
        uint8_t idx = hal_boot2_get_active_partition();
        uint32_t age = hal_boot2_get_active_partition_age();
        PtTable_Update(ota_pt, age + 1, ota_entry_index);
        bl_flash_read(164 + 4 * idx, (uint8_t *)&addr, 4);
        bl_flash_erase(addr, 1);
        bl_flash_write(addr, (uint8_t *)ota_pt, ota_pt_len);
        printf("[OTA] pt&fw update success\r\n");
    }
    
finish:
    if(ota_pt){
        free(ota_pt);
        ota_pt = NULL;
    }
    
    if(ota_buffer){
        free(ota_buffer);
        ota_buffer = NULL;
    }
    
    return ret;
}
