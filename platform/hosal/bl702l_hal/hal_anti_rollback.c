#include "hal_anti_rollback.h"
#include "bl702l_ef_ctrl.h"

int32_t hal_get_boot2_version_from_efuse(uint8_t *version)
{
    uint32_t anti_rollback_version = 0;

    if(NULL == version){
        return -1;
    }

    /* load efuse data to efuse register anyway */
    EF_Ctrl_Load_Efuse_R0();

    /* get real version from efuse */
    anti_rollback_version = *(uint32_t *)(EF_DATA_BASE + 0x4C);

    /* version_real[31:0] case */
    if(anti_rollback_version){
        *version = 32 - __builtin_clz(anti_rollback_version);
        return SUCCESS;
    }

    *version = 0;
    return SUCCESS;
}

int32_t hal_set_boot2_version_to_efuse(uint8_t version)
{
    uint8_t version_old = 0;

    if(version > 32){
        return -1;
    }

    if(hal_get_boot2_version_from_efuse(&version_old) != SUCCESS){
        return -2;
    }

    if(version_old >= version){
        return -3;
    }

    /* program anti-rollback enable bit and boot2 version */
    *(uint32_t *)(EF_DATA_BASE + 0x74) = 0x1 << 13;
    *(uint32_t *)(EF_DATA_BASE + 0x4C) = 0x1 << (version - 1);
    EF_Ctrl_Program_Direct_R0(0, NULL, 0);
    while(SET == EF_Ctrl_Busy());

    /* load efuse data to efuse register again */
    EF_Ctrl_Load_Efuse_R0();

    return SUCCESS;
}

int32_t hal_get_app_version_from_efuse(uint8_t *version)
{
    uint32_t anti_rollback_version_low = 0;
    uint32_t anti_rollback_version_high = 0;

    if(NULL == version){
        return -1;
    }

    /* load efuse data to efuse register anyway */
    EF_Ctrl_Load_Efuse_R0();

    /* get real version from efuse */
    anti_rollback_version_low = *(uint32_t *)(EF_DATA_BASE + 0x04);
    anti_rollback_version_high = *(uint32_t *)(EF_DATA_BASE + 0x08);

    /* version_real[63:32] case */
    if(anti_rollback_version_high){
        *version = 64 - __builtin_clz(anti_rollback_version_high);
        return SUCCESS;
    }

    /* version_real[31:0] case */
    if(anti_rollback_version_low){
        *version = 32 - __builtin_clz(anti_rollback_version_low);
        return SUCCESS;
    }

    *version = 0;
    return SUCCESS;
}

int32_t hal_set_app_version_to_efuse(uint8_t version)
{
    uint8_t version_old = 0;

    if(version > 64){
        return -1;
    }

    if(hal_get_app_version_from_efuse(&version_old) != SUCCESS){
        return -2;
    }

    if(version_old >= version){
        return -3;
    }

    /* program anti-rollback enable bit and application version */
    *(uint32_t *)(EF_DATA_BASE + 0x74) = 0x1 << 13;
    if(version > 32){
        *(uint32_t *)(EF_DATA_BASE + 0x08) = 0x1 << (version - 32 - 1);
    }else{
        *(uint32_t *)(EF_DATA_BASE + 0x04) = 0x1 << (version - 1);
    }
    EF_Ctrl_Program_Direct_R0(0, NULL, 0);
    while(SET == EF_Ctrl_Busy());

    /* load efuse data to efuse register again */
    EF_Ctrl_Load_Efuse_R0();

    return SUCCESS;
}
