#include <bl702l_romdriver.h>
#include <bl702l_sf_cfg.h>
#include <bl702l_xip_sflash.h>
#include <bl702l_l1c.h>

#include "bl_flash.h"
#include "bl_irq.h"
#include <blog.h>
#define USER_UNUSED(a) ((void)(a))


#ifdef CFG_USE_PSRAM

#define IS_PSARAM(addr) ((addr&0xFF000000) == 0x26000000 || \
                        (addr&0xFF000000) == 0x24000000 )

extern void update_sp(void);
extern void resotre_sp(void);

/** pvPortMalloc/vPortFree is dedicated for SRAM heap access */
extern void *pvPortMalloc( size_t xSize );
extern void vPortFree( void *pv );

#ifndef PSRAM_TEMP_STACK_SIZE
#define PSRAM_TEMP_STACK_SIZE 128
#endif
static __attribute__ ((aligned(16))) StackType_t __spn[PSRAM_TEMP_STACK_SIZE] = {0};
StackType_t* const __psram_taskstack_spn_top = (StackType_t*)(__spn + PSRAM_TEMP_STACK_SIZE) - 1;
uint32_t psram_taskstack_spo = 0;
#endif

static uint8_t fw_protect_en = 0;
static uint32_t fw_start_addr = 0;
static uint32_t fw_end_addr = 0;

static uint32_t flash_size = 0;

static struct {
    uint32_t magic;
    SPI_Flash_Cfg_Type flashCfg;
} boot2_flashCfg; //XXX Dont change the name of varaible, since we refer this boot2_partition_table in linker script


__attribute__((always_inline)) __STATIC_INLINE uint32_t __get_SP(void) 
{ 
  register uint32_t result; 

  __ASM volatile ("mv %0, sp\n" : "=r" (result) ); 
  return(result); 
}

int bl_flash_erase(uint32_t addr, int len)
{
    if(fw_protect_en){
        uint32_t start_addr = addr & 0xFFFFF000;
        uint32_t end_addr = (addr + len + 0xFFF) & 0xFFFFF000;
        if(!(start_addr >= fw_end_addr || end_addr <= fw_start_addr)){
            blog_error("[FLASH] Attempt to erase fw region, start_addr 0x%08lx, end_addr 0x%08lx\r\n", start_addr, end_addr);
            return -1;
        }
        if(end_addr > flash_size){
            blog_error("[FLASH] Erase overflow, start_addr 0x%08lx, end_addr 0x%08lx, flash_size 0x%08lx\r\n", start_addr, end_addr, flash_size);
            return -1;
        }
    }

    unsigned long mstatus_tmp;
    mstatus_tmp = read_csr(mstatus);
    clear_csr(mstatus, MSTATUS_MIE);
    bl_flash_erase_need_lock(addr, len);
    write_csr(mstatus, mstatus_tmp);

    return 0;
}

int bl_flash_write(uint32_t addr, uint8_t *src, int len)
{
    uint8_t* _data = src;

    if(fw_protect_en){
        uint32_t start_addr = addr;
        uint32_t end_addr = addr + len;
        if(!(start_addr >= fw_end_addr || end_addr <= fw_start_addr)){
            blog_error("[FLASH] Attempt to write fw region, start_addr 0x%08lx, end_addr 0x%08lx\r\n", start_addr, end_addr);
            return -1;
        }
        if(end_addr > flash_size){
            blog_error("[FLASH] Write overflow, start_addr 0x%08lx, end_addr 0x%08lx, flash_size 0x%08lx\r\n", start_addr, end_addr, flash_size);
            return -1;
        }
    }

#ifdef CFG_USE_PSRAM
    if(IS_PSARAM((uint32_t)src)){
        if (0 == len) {
            return 0;
        }
        _data = (uint8_t*)pvPortMalloc(len);
        if (NULL == _data) {
            return -1;
        }
        memcpy(_data, src, len);
    }
#endif

    unsigned long mstatus_tmp;
    mstatus_tmp = read_csr(mstatus);
    clear_csr(mstatus, MSTATUS_MIE);
    bl_flash_write_need_lock(addr, _data, len);
    write_csr(mstatus, mstatus_tmp);

#ifdef CFG_USE_PSRAM
    if(IS_PSARAM((uint32_t)src)){
        vPortFree(_data);
    }
#endif

    return 0;
}

int bl_flash_read(uint32_t addr, uint8_t *dst, int len)
{
    uint8_t* _data = dst;

#ifdef CFG_USE_PSRAM
    if(IS_PSARAM((uint32_t)dst)){
        if (0 == len) {
            return 0;
        }
        _data = (uint8_t*)pvPortMalloc(len);
        if (NULL == _data) {
            return -1;
        }
    }
#endif

    unsigned long mstatus_tmp;
    mstatus_tmp = read_csr(mstatus);
    clear_csr(mstatus, MSTATUS_MIE);
    bl_flash_read_need_lock(addr, _data, len);
    write_csr(mstatus, mstatus_tmp);

#ifdef CFG_USE_PSRAM
    if(IS_PSARAM((uint32_t)dst)){
        memcpy(dst, _data, len);
        vPortFree(_data);
    }
#endif

    return 0;
}


static inline int bl_flash_erase_need_lock_internal(uint32_t addr, int len)
{
    RomDriver_XIP_SFlash_Erase_Need_Lock(
            &boot2_flashCfg.flashCfg,
            addr,
            addr + len - 1
    );

    return 0;
}

static inline int bl_flash_write_need_lock_internal(uint32_t addr, uint8_t *src, int len)
{
    RomDriver_XIP_SFlash_Write_Need_Lock(
            &boot2_flashCfg.flashCfg,
            addr,
            src,
            len
    );

    return 0;
}

static inline int bl_flash_read_need_lock_internal(uint32_t addr, uint8_t *dst, int len)
{
    RomDriver_XIP_SFlash_Read_Need_Lock(
            &boot2_flashCfg.flashCfg,
            addr,
            dst,
            len
    );

    return 0;
}

#ifdef CFG_USE_PSRAM
static inline int bl_flash_erase_need_lock_psram(uint32_t addr, int len)
{
    /* Add code in this function need to be careful, 
     * because the stack in this function is updated 
     */
    update_sp();

    int iret = bl_flash_erase_need_lock_internal(addr, len);

    resotre_sp();

    return iret;
}

static inline int bl_flash_write_need_lock_psram(uint32_t addr, uint8_t *src, int len)
{
    /* Add code in this function need to be careful, 
     * because the stack in this function is updated 
     */
    update_sp();

    int iret = bl_flash_write_need_lock_internal(addr, src, len);

    resotre_sp();

    return iret;
}

static inline int bl_flash_read_need_lock_psram(uint32_t addr, uint8_t *dst, int len)
{
    /* Add code in this function need to be careful, 
     * because the stack in this function is updated 
     */
    update_sp();

    int iret = bl_flash_read_need_lock_internal(addr, dst, len);

    resotre_sp();

    return iret;
}
#endif

int bl_flash_erase_need_lock(uint32_t addr, int len)
{
#ifdef CFG_USE_PSRAM
    if (IS_PSARAM(__get_SP())) {
        return bl_flash_erase_need_lock_psram(addr, len);
    }
#endif

    return bl_flash_erase_need_lock_internal(addr, len);
}

int bl_flash_write_need_lock(uint32_t addr, uint8_t *src, int len)
{
#ifdef CFG_USE_PSRAM
    if (IS_PSARAM(__get_SP())) {
        return bl_flash_write_need_lock_psram(addr, src, len);
    }
#endif

    return bl_flash_write_need_lock_internal(addr, src, len);
}

int bl_flash_read_need_lock(uint32_t addr, uint8_t *dst, int len)
{
#ifdef CFG_USE_PSRAM
    if (IS_PSARAM(__get_SP())) {
        return bl_flash_read_need_lock_psram(addr, dst, len);
    }
#endif

    return bl_flash_read_need_lock_internal(addr, dst, len);
}


static uint32_t _get_flash_size()
{
    SPI_Flash_Cfg_Type *pFlashCfg = &boot2_flashCfg.flashCfg;
    uint32_t jid = 0;
    uint32_t level;
    uint32_t size;

    XIP_SFlash_GetJedecId_Need_Lock(pFlashCfg, (uint8_t *)&jid);

    level = (jid >> 16) & 0x1f;
    level -= 0x13;
    size = (1 << level) * 512 * 1024;

    return size;
}

static void _dump_flash_config()
{
    flash_size = _get_flash_size();

    blog_info("======= FlashCfg magiccode @%p, code 0x%08lX =======\r\n",
            &boot2_flashCfg.magic,
            boot2_flashCfg.magic
    );
    blog_info("mid \t\t0x%X\r\n", boot2_flashCfg.flashCfg.mid);
    blog_info("ioMode \t0x%X\r\n", boot2_flashCfg.flashCfg.ioMode);
    blog_info("pdDelay \t0x%X\r\n", boot2_flashCfg.flashCfg.pdDelay);
    blog_info("clkDelay \t0x%X\r\n", boot2_flashCfg.flashCfg.clkDelay);
    blog_info("clkInvert \t0x%X\r\n", boot2_flashCfg.flashCfg.clkInvert);
    blog_info("sector size\t%uKBytes\r\n", boot2_flashCfg.flashCfg.sectorSize);
    blog_info("page size\t%uBytes\r\n", boot2_flashCfg.flashCfg.pageSize);
    blog_info("flash size\t%uKBytes\r\n", flash_size/1024);
    blog_info("---------------------------------------------------------------\r\n");
}

int bl_flash_config_update(void)
{
    _dump_flash_config();

    return 0;
}

int bl_flash_fw_protect_set(uint8_t en, uint32_t addr, uint32_t size)
{
    fw_protect_en = en;
    fw_start_addr = addr;
    fw_end_addr = addr + size;

    return 0;
}

uint32_t bl_flash_get_size(void)
{
    return flash_size;
}

void* bl_flash_get_flashCfg(void)
{
    return &boot2_flashCfg.flashCfg;
}

void bl_flash_get_encryptInfo(encrypt_info_t *info)
{
    uint32_t boot_cfg;
    uint32_t sig_len;
    
    info->flash_offset = SF_Ctrl_Get_Flash_Image_Offset();
    
    //printf("flash_offset: 0x%08lX\r\n", info->flash_offset);
    
    bl_flash_read(info->flash_offset - 0x1000 + 0x78, (uint8_t *)&info->img_len, 4);
    
    //printf("img_len: 0x%08lX\r\n", info->img_len);
    
    bl_flash_read(info->flash_offset - 0x1000 + 0x74, (uint8_t *)&boot_cfg, 4);
    
    //printf("boot_cfg: 0x%08lX\r\n", boot_cfg);
    
    info->encrypt_type = (boot_cfg >> 2) & 0x03;
    info->aes_region_lock = (boot_cfg >> 11) & 0x01;
    
    //printf("encrypt_type: %d, aes_region_lock: %d\r\n", info->encrypt_type, info->aes_region_lock);
    
    if(boot_cfg & 0x03){
        bl_flash_read(info->flash_offset - 0x1000 + 0xF0 + 68, (uint8_t *)&sig_len, 4);
        bl_flash_read(info->flash_offset - 0x1000 + 0xF0 + 68 + (4 + sig_len + 4), info->aes_iv, 16);
    }else{
        bl_flash_read(info->flash_offset - 0x1000 + 0xF0, info->aes_iv, 16);
    }
    
    //printf("aes_iv:");
    //for(int i=0; i<16; i++){
    //    printf(" %02X", info->aes_iv[i]);
    //}
    //printf("\r\n");
}

void bl_flash_get_bootcfgInfo(bootcfg_info_t *info)
{
    uint32_t boot_cfg;
    uint32_t flash_offset;
    
    bl_flash_read(0x74, (uint8_t *)&boot_cfg, 4);
    
    info->boot2_internal = (boot_cfg >> 19) & 0x01;
    if(info->boot2_internal){
        info->rollback_enable = (boot_cfg >> 20) & 0x01;
    }else{
        info->rollback_enable = 1;
    }
    
    flash_offset = SF_Ctrl_Get_Flash_Image_Offset();
    bl_flash_read(flash_offset - 0x1000 + 0x74, (uint8_t *)&boot_cfg, 4);
    
    info->crc_ignore = (boot_cfg >> 16) & 0x01;
    info->hash_ignore = (boot_cfg >> 17) & 0x01;
    
    //printf("crc_ignore: %d\r\n", info->crc_ignore);
    //printf("hash_ignore: %d\r\n", info->hash_ignore);
    //printf("boot2_internal: %d\r\n", info->boot2_internal);
    //printf("rollback_enable: %d\r\n", info->rollback_enable);
}

static inline int bl_flash_read_byxip_internal(uint32_t addr, uint8_t *dst, int len)
{
    uint32_t offset;
    uint32_t xipaddr;

    offset = SF_Ctrl_Get_Flash_Image_Offset();

    if ((addr < offset) || (addr >= 0x1000000)) {
        // not support or arg err ?
        return -1;
    }

    xipaddr =  0x23000000 - offset + addr;

    memcpy(dst, (void *)xipaddr, len);

    return 0;
}

#ifdef CFG_USE_PSRAM
static inline int bl_flash_read_byxip_psram(uint32_t addr, uint8_t *dst, int len)
{

    /* Add code in this function need to be careful, 
     * because the stack in this function is updated 
     */
    update_sp();

    bl_flash_read_byxip_internal(addr, dst, len);

    resotre_sp();

    return 0;
}
#endif

int bl_flash_read_byxip(uint32_t addr, uint8_t *dst, int len)
{
#ifdef CFG_USE_PSRAM
    uint8_t* _data = dst;
    int iret;

    if(IS_PSARAM((uint32_t)dst)){
        if (0 == len) {
            return 0;
        }
        _data = (uint8_t*)pvPortMalloc(len);
        if (NULL == _data) {
            return -1;
        }
    }

    if (IS_PSARAM(__get_SP())) {
        iret = bl_flash_read_byxip_psram(addr, _data, len);
    }
    else {
        iret = bl_flash_read_byxip_internal(addr, _data, len);
    }

    if(IS_PSARAM((uint32_t)dst)){
        memcpy(dst, _data, len);
        vPortFree(_data);
    }

    return iret;
#else

    return bl_flash_read_byxip_internal(addr, dst, len);
#endif

}

int ATTR_TCM_SECTION bl_flash_init(void)
{
    uint8_t aesEnable;
    SPI_Flash_Cfg_Type *pFlashCfg = &boot2_flashCfg.flashCfg;
    uint8_t clkDelay = 1;
    uint8_t clkInvert = 1;
    uint32_t jid = 0;
    uint32_t offset = 0;
    int ret = 0;

    unsigned long mstatus_tmp;
    mstatus_tmp = read_csr(mstatus);
    clear_csr(mstatus, MSTATUS_MIE);

    // get flash config from bootheader
    //L1C_Cache_Flush();
    //XIP_SFlash_Read_Via_Cache_Need_Lock(8 + BL702L_FLASH_XIP_BASE, (uint8_t *)&boot2_flashCfg, 4 + sizeof(SPI_Flash_Cfg_Type));
    //L1C_Cache_Flush();

    XIP_SFlash_Opt_Enter(&aesEnable);
    XIP_SFlash_State_Save(pFlashCfg, &offset);

    // store clock delay and clock invert, which have already been used for flash boot
    clkDelay = pFlashCfg->clkDelay;
    clkInvert = pFlashCfg->clkInvert;

    SFlash_GetJedecId(pFlashCfg, (uint8_t *)&jid);
    ret = SF_Cfg_Get_Flash_Cfg_Need_Lock_Ext(jid, pFlashCfg);
    if (ret == 0) {
        if ((pFlashCfg->ioMode & 0x0f) == SF_CTRL_QO_MODE || (pFlashCfg->ioMode & 0x0f) == SF_CTRL_QIO_MODE) {
            SFlash_Qspi_Enable(pFlashCfg);
        }

        if (((pFlashCfg->ioMode >> 4) & 0x01) == 1) {
            L1C_Set_Wrap(DISABLE);
        } else {
            L1C_Set_Wrap(ENABLE);
            if ((pFlashCfg->ioMode & 0x0f) == SF_CTRL_QO_MODE || (pFlashCfg->ioMode & 0x0f) == SF_CTRL_QIO_MODE) {
                SFlash_SetBurstWrap(pFlashCfg);
            }
        }
    }

    // replace clock delay and clock invert
    pFlashCfg->clkDelay = clkDelay;
    pFlashCfg->clkInvert = clkInvert;

    XIP_SFlash_State_Restore(pFlashCfg, offset);
    XIP_SFlash_Opt_Exit(aesEnable);

    L1C_Cache_Flush();

    write_csr(mstatus, mstatus_tmp);

    return ret;
}
