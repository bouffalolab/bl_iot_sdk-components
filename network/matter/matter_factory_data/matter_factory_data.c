
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#ifdef BL616
#include <bl616_romdriver_e907.h>
#define MFD_XIP_BASE BL616_FLASH_XIP_BASE
#define MFD_XIP_END BL616_FLASH_XIP_END
#elif defined BL602 || defined BL702 || defined BL702L
#ifdef BL602
#include <bl602.h>
#include <bl602_romdriver.h>
#define MFD_XIP_BASE BL602_FLASH_XIP_BASE
#define MFD_XIP_END BL602_FLASH_XIP_END
#define MFD_WRAM_START BL602_WRAM_BASE
#define MFD_WRAM_END BL602_WRAM_END
#elif defined BL702
#include <bl702.h>
#include <bl702_romdriver.h>
#define MFD_XIP_BASE BL702_FLASH_XIP_BASE
#define MFD_XIP_END BL702_FLASH_XIP_END
#define MFD_WRAM_START BL702_WRAM_BASE
#define MFD_WRAM_END BL702_WRAM_END
#else
#include <bl702l.h>
#include <bl702l_romdriver.h>
#define MFD_XIP_BASE BL702L_FLASH_XIP_BASE
#define MFD_XIP_END BL702L_FLASH_XIP_END
#endif
#include <softcrc.h>
#include <hal_boot2.h>
#else
#error "No chipname specified."
#endif

#include "matter_factory_data.h"

#define MATTER_FACTORY_DATA_NAME                    "MFD"

#define MFD_EFUSE_AES_IV_LEN                        16
#define MFD_SPAKE_2P_IT(x)                          ( ((x) % 280) + 1000 ) /** 1000 ~ 1280 if not configured by manufactory */
#define MFD_SPAKE_2P_SLAT_LEN                       16
#define MFD_SPAKE_2P_VERIFIER_LEN                   128

typedef enum {
    ELEMENT_TYPE_SKIP = 0,
    ELEMENT_TYPE_EFUSE_AES_IV = 1,                  /** not Matter factory data, which is for credential data decryption. 
                                                        It must be placed in plaintext area with type id 0x8001 */
    ELEMENT_TYPE_ELEMENT_START = 2,
    ELEMENT_TYPE_DAC_CERT = ELEMENT_TYPE_ELEMENT_START,
    ELEMENT_TYPE_DAC_PRIVATE_KEY,
    ELEMENT_TYPE_PASSCODE,
    ELEMENT_TYPE_PAI_CERT, 
    ELEMENT_TYPE_CD,
    ELEMENT_TYPE_SERIAL_NUMBER,
    ELEMENT_TYPE_DISCRIMINATOR_ID,
    ELEMEMT_TYPE_UNIQUE_ID,
    ELEMENT_TYPE_SPAKE2P_IT,
    ELEMENT_TYPE_SPAKE2P_SALT,
    ELEMENT_TYPE_SPAKE2P_VERIFIER,

    ELEMENT_TYPE_VENDOR_NAME,            
    ELEMENT_TYPE_VENDOR_ID,            
    ELEMENT_TYPE_PRODUCT_NAME,            
    ELEMENT_TYPE_PRODUCT_ID,              
    ELEMENT_TYPE_PRODUCT_PART_NUMBER,     
    ELEMENT_TYPE_PRODUCT_URL,           
    ELEMENT_TYPE_PRODUCT_LABEL,          
    ELEMENT_TYPE_MANUFACTORING_DATE,      
    ELEMENT_TYPE_HARDWARE_VERSION,        
    ELEMENT_TYPE_HARDWARE_VERSION_STRING,

    ELEMENT_TYPE_ELEMENT_END,
    
    ELEMENT_TYPE_NUM = ELEMENT_TYPE_ELEMENT_END - ELEMENT_TYPE_ELEMENT_START,
    ELEMENT_TYPE_PLAIN_FLAG = 0x8000,
    ELEMENT_TYPE_ID_MASK = 0x7fff,
} FactoryCredentialElementTypeId_t;

#define TO_LITTLE_ENDION_UINT32(x) ((x[0] << 24) | (x[1] << 16) | (x[2] << 8) | (x[3]))

typedef struct {

    uint16_t type_id;
    uint16_t value_len;
    uint8_t  value[0];

} ElementTlv_t;

typedef struct {
    uint8_t *mValuePtr;
    uint32_t mLength;
} FactoryDataElement_t;

typedef struct FactoryData
{
    uint32_t                    ptOffset;
    uint32_t                    ptSize;
    FactoryDataElement_t        item[ELEMENT_TYPE_NUM];
} FactoryData_t;

FactoryData_t g_mfd_var;

extern bool efuse_slot_decrypt(uint8_t *p, uint32_t len, uint8_t *pIv);

#define IS_FLASH_ADDR(x) ((uint32_t)MFD_XIP_BASE <= (uint32_t)x && (uint32_t)x < (uint32_t)MFD_XIP_END )

static bool mfd_parsePartitionData(uint32_t size, uint8_t *pData, uint8_t *pIv) 
{
    ElementTlv_t tlv;
    uint32_t offset = 0;
    FactoryDataElement_t *p;
    uint32_t id;

    while (offset + offsetof(ElementTlv_t, value) < size) {

        memcpy(&tlv, pData + offset, offsetof(ElementTlv_t, value));
        p = NULL;

        id = ELEMENT_TYPE_ID_MASK & tlv.type_id;
        if (ELEMENT_TYPE_EFUSE_AES_IV == id) {
            if ((tlv.type_id & ELEMENT_TYPE_PLAIN_FLAG) && MFD_EFUSE_AES_IV_LEN == tlv.value_len) {
                /** efuse aes iv must be placed in plaintext data area with a fixed length MFD_EFUSE_AES_IV_LEN */
                memcpy(pIv, pData + offset + offsetof(ElementTlv_t, value), tlv.value_len);
            }

            offset += (offsetof(ElementTlv_t, value) + tlv.value_len);
            continue;
        }
        else if (ELEMENT_TYPE_ELEMENT_START <= id && id <= ELEMENT_TYPE_ELEMENT_END && tlv.value_len) {

            p = &(g_mfd_var.item[id - ELEMENT_TYPE_ELEMENT_START]);
            if (IS_FLASH_ADDR(pData + offset + offsetof(ElementTlv_t, value))) {
                /** for plaintext data, put mValuePtr it in flash data */
                p->mValuePtr = pData + offset + offsetof(ElementTlv_t, value);
            }
            else {
                /** for cipher data, copy it to heap memory from decrpytion osram memory */
                p->mValuePtr = (uint8_t *)malloc(tlv.value_len);
                if (NULL == p->mValuePtr) {
                    return false;
                }
                memcpy(p->mValuePtr, pData + offset + offsetof(ElementTlv_t, value), tlv.value_len);
            }
            p->mLength = tlv.value_len;
        }

        offset += (offsetof(ElementTlv_t, value) + tlv.value_len);
    }

    return true;
}

static bool mfd_parseData(void) 
{
    uint32_t mfd_decrypt_buf[2048 / sizeof(uint32_t)];

#ifdef MFD_WRAM_START
    /** mfd_decrypt_buf requires OCRAM memory for bl602 and bl702 */
    configASSERT(MFD_WRAM_START <= (uint32_t) mfd_decrypt_buf && (uint32_t) mfd_decrypt_buf < MFD_WRAM_END);
#endif

/** mfd cipher data and plaintext data structure as following:
 * 0xaa 0xaa 0xaa 0xaa  : the length of cipher data
 * ..........           : cipher data tlv
 * 0xbb 0xbb 0xbb 0xbb  : crc for cipher data
 * 0xcc 0xcc 0xcc 0xcc  : the length of plaintext data
 * ..........           : plaintext data tlv
 * 0xdd 0xdd 0xdd 0xdd  : crc for plaintext data
 *  */

    uint32_t cipher_size = 0, plaintext_size = 0;
    uint32_t crc_value;
#ifdef BL616
    uint32_t xipaddr_base = bflb_sf_ctrl_get_flash_image_offset(0, SF_CTRL_FLASH_BANK0);
#else
    uint32_t xipaddr_base = RomDriver_SF_Ctrl_Get_Flash_Image_Offset();
#endif
    uint32_t iv[(MFD_EFUSE_AES_IV_LEN + 3) / 4];

    if (g_mfd_var.ptOffset >= xipaddr_base) {
        xipaddr_base = MFD_XIP_BASE + (g_mfd_var.ptOffset - xipaddr_base);
    } else {
        return false;
    }

    /** readout size for cipher data and plaintext data  */
    memcpy(&cipher_size, (void *)xipaddr_base, sizeof(cipher_size));
    memcpy(&plaintext_size, (void *)(xipaddr_base + 4 + cipher_size + 4), sizeof(cipher_size));
    if (0 == cipher_size || 0 == plaintext_size || (cipher_size + plaintext_size) > 4096) {
        return false;;
    }

    /** verifications crc values on cipher data and plaintext data */
    crc_value = BFLB_Soft_CRC32((uint8_t *)(xipaddr_base + 4), cipher_size);
    if (crc_value != *(uint32_t *)(xipaddr_base + 4 + cipher_size)) {
        return false;;
    }
    crc_value = BFLB_Soft_CRC32((uint8_t *)((xipaddr_base + 4 + cipher_size + 4) + 4), plaintext_size);
    if (crc_value != *(uint32_t *)((xipaddr_base + 4 + cipher_size + 4) + 4 + plaintext_size)) {
        return false;;
    }

    /** parse plaintext data first and get efuse aes IV */
    if (false == mfd_parsePartitionData(plaintext_size, (uint8_t *)((xipaddr_base + 4 + cipher_size + 4) + 4), (uint8_t *)iv)) {
        return false;;
    }

    memcpy(mfd_decrypt_buf, (uint8_t *)(xipaddr_base + 4), cipher_size);

    if (false == efuse_slot_decrypt((uint8_t *)mfd_decrypt_buf, cipher_size, (uint8_t *)iv)) {
        return false;;
    }

    if (false == mfd_parsePartitionData(cipher_size, (uint8_t *)mfd_decrypt_buf, (uint8_t *)iv)) {
        return false;;
    }

    return true;
}

static bool mfd_checkMinimalSet(void) 
{
    /** the following factory data is minimal set */
    static const int minimalDataSet[] = {
        ELEMENT_TYPE_DAC_PRIVATE_KEY,
        ELEMENT_TYPE_PASSCODE,
        ELEMENT_TYPE_PAI_CERT, 
        ELEMENT_TYPE_CD,
        ELEMENT_TYPE_SERIAL_NUMBER,
        ELEMENT_TYPE_DISCRIMINATOR_ID,
        ELEMENT_TYPE_SPAKE2P_IT,
        ELEMENT_TYPE_SPAKE2P_SALT,
        ELEMENT_TYPE_SPAKE2P_VERIFIER,
    };

    for (uint32_t i = 0; i < sizeof(minimalDataSet)/sizeof(minimalDataSet[0]); i ++) {
        if (NULL == g_mfd_var.item[minimalDataSet[i] - ELEMENT_TYPE_ELEMENT_START].mValuePtr) {
            return false;
        }
    }

    return true;
}

static void mfd_free (void) 
{
    FactoryDataElement_t *p;

    for (uint32_t i = 0; i < sizeof(g_mfd_var.item) / sizeof(g_mfd_var.item[0]); i++) {
        p = &(g_mfd_var.item[i]);
        if (p->mValuePtr && !IS_FLASH_ADDR(p->mValuePtr)) {
            free(p->mValuePtr);
        }
    }

    memset(g_mfd_var.item, 0, sizeof(g_mfd_var.item));
}

bool mfd_init(void)
{
#ifdef BL616
    if (bflb_boot2_partition_addr_active(MATTER_FACTORY_DATA_NAME, &g_mfd_var.ptOffset, &g_mfd_var.ptSize)) {
        return false;
    }
#else
    if (hal_boot2_partition_addr_active(MATTER_FACTORY_DATA_NAME, &g_mfd_var.ptOffset, &g_mfd_var.ptSize)) {
        return false;
    }
#endif

    do {
        if (false == mfd_parseData()) {
            break;
        }

        if (false == mfd_checkMinimalSet()) {
            break;
        }

        return true;
    } while (0);

    mfd_free();

    return false;
}

static inline int mfd_copyDataItem(uint32_t id, uint8_t *pBuf, uint32_t size) 
{
    FactoryDataElement_t *p = &(g_mfd_var.item[id - ELEMENT_TYPE_ELEMENT_START]);

    if (0 == p->mLength || NULL == p->mValuePtr) {
        return 0;
    }

    if (size >= p->mLength) {
        memcpy(pBuf, p->mValuePtr, p->mLength);
        return p->mLength;
    }

    return -1;
}

int mfd_getDacCert(uint8_t *p, uint32_t size) 
{
    return mfd_copyDataItem(ELEMENT_TYPE_DAC_CERT, p, size);
}

uint8_t *mfd_getDacCertPtr(uint32_t *psize)
{
    FactoryDataElement_t *p = &(g_mfd_var.item[ELEMENT_TYPE_DAC_CERT - ELEMENT_TYPE_ELEMENT_START]);

    if (psize) {
        *psize = p->mLength;
        return p->mValuePtr;
    }

    return NULL;
}

int mfd_getDacPrivateKey(uint8_t *p, uint32_t size) 
{
    return mfd_copyDataItem(ELEMENT_TYPE_DAC_PRIVATE_KEY, p, size);
}

uint8_t *mfd_getDacPrivateKeyPtr(uint32_t *psize)
{
    FactoryDataElement_t *p = &(g_mfd_var.item[ELEMENT_TYPE_DAC_PRIVATE_KEY - ELEMENT_TYPE_ELEMENT_START]);

    if (psize) {
        *psize = p->mLength;
        return p->mValuePtr;
    }

    return NULL;
}

int mfd_getPasscode(uint8_t *p, uint32_t size) 
{
    return mfd_copyDataItem(ELEMENT_TYPE_PASSCODE, p, size);
}

int mfd_getPaiCert(uint8_t *p, uint32_t size) 
{
    return mfd_copyDataItem(ELEMENT_TYPE_PAI_CERT, p, size);
}

int mfd_getCd(uint8_t *p, uint32_t size) 
{
    return mfd_copyDataItem(ELEMENT_TYPE_CD, p, size);
}

int mfd_getSerialNumber(char *p, uint32_t size) 
{
    memset(p, 0, size);
    return mfd_copyDataItem(ELEMENT_TYPE_SERIAL_NUMBER, (uint8_t *)p, size);
}

int mfd_getDiscriminator(uint8_t *p, uint32_t size) 
{
    return mfd_copyDataItem(ELEMENT_TYPE_DISCRIMINATOR_ID, p, size);
}

int mfd_getRotatingDeviceIdUniqueId(uint8_t *p, uint32_t size) 
{
    return mfd_copyDataItem(ELEMEMT_TYPE_UNIQUE_ID, p, size);
}

int mfd_getSapke2It(uint8_t *p, uint32_t size) 
{
    return mfd_copyDataItem(ELEMENT_TYPE_SPAKE2P_IT, p, size);
}

int mfd_getSapke2Salt(uint8_t *p, uint32_t size) 
{
    return mfd_copyDataItem(ELEMENT_TYPE_SPAKE2P_SALT, p, size);
}

int mfd_getSapke2Verifier(uint8_t *p, uint32_t size) 
{
    return mfd_copyDataItem(ELEMENT_TYPE_SPAKE2P_VERIFIER, p, size);
}

int mfd_getVendorName (char * buf, uint32_t size) 
{   
    return mfd_copyDataItem(ELEMENT_TYPE_VENDOR_NAME,(uint8_t*)buf,size);
}
int mfd_getVendorId(uint8_t *buf, uint32_t size)
{
    return mfd_copyDataItem(ELEMENT_TYPE_VENDOR_ID,buf,size);
}
int mfd_getProductName(char * buf, uint32_t size) 
{
    return mfd_copyDataItem(ELEMENT_TYPE_PRODUCT_NAME,(uint8_t*)buf,size);
}

int mfd_getProductId(uint8_t *buf, uint32_t size)
{
    return mfd_copyDataItem(ELEMENT_TYPE_PRODUCT_ID,buf,size);
}

int mfd_getPartNumber(char *buf, uint32_t size)
{
    return mfd_copyDataItem(ELEMENT_TYPE_PRODUCT_PART_NUMBER,(uint8_t*)buf,size);
}

int mfd_getProductUrl(char * buf, uint32_t size)
{
    return mfd_copyDataItem(ELEMENT_TYPE_PRODUCT_URL,(uint8_t*)buf,size);
}

int mfd_getProductLabel(char * buf, uint32_t size) 
{
    return mfd_copyDataItem(ELEMENT_TYPE_PRODUCT_LABEL,(uint8_t*)buf,size);
}

bool mfd_getManufacturingDate(uint16_t *pYear, uint8_t *pMonth, uint8_t *pDay)
{
    uint8_t datetime[4];
    if(mfd_copyDataItem(ELEMENT_TYPE_MANUFACTORING_DATE,datetime,sizeof(datetime)))
    {
        *pYear=((datetime[0]<<8) + datetime[1]);
        *pMonth=datetime[2];
        *pDay=datetime[3];
        return true;
    };

    return false;
}

int mfd_getHardwareVersion(uint8_t * buf, uint32_t size) 
{
    return mfd_copyDataItem(ELEMENT_TYPE_HARDWARE_VERSION,buf,size);
}

int mfd_getHardwareVersionString(char * buf, uint32_t size)
{
    return mfd_copyDataItem(ELEMENT_TYPE_HARDWARE_VERSION_STRING,(uint8_t*)buf,size);
}
