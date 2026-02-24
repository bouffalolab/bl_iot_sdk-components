
#include "eth_phy.h"
#include "ephy_general.h"
#include "ephy_lan8720.h"

#include "utils_log.h"

eth_phy_drv_t ephy_lan8720_drv = {
    .phy_id = EPHY_LAN8720_ID,
    .phy_id_mask = EPHY_LAN8720_MASK,
    .phy_name = "LAN8720",

    .phy_init = ephy_lan8720_init,
    .phy_ctrl = ephy_lan8720_ctrl,
    .dump_registers = ephy_lan8720_dump_registers,
};

int ephy_lan8720_init(eth_phy_ctrl_t *phy_ctrl, eth_phy_init_cfg_t *cfg)
{
#if 0
    uint16_t regval;

    /* close all capable */
    eth_phy_mdio_read(phy_ctrl->phy_addr, LAN8720_SPECIAL_MODE_OFFSET, &regval);
    regval &= ~LAN8720_MODE_MASK;
    eth_phy_mdio_write(phy_ctrl->phy_addr, LAN8720_SPECIAL_MODE_OFFSET, regval);
#endif

    ephy_general_init(phy_ctrl, cfg);

    return 0;
}

int ephy_lan8720_ctrl(eth_phy_ctrl_t *phy_ctrl, uint32_t cmd, uint32_t arg)
{
    int ret = 0;
    uint16_t regval;

    /* check cmd type */
    uint32_t cmd_type = (cmd & EPHY_CMD_TYPE_MASK) >> EPHY_CMD_TYPE_SHIFT;
    if (cmd_type == EPHY_CMD_TYPE_GENERAL) {
        /* general cmd */
        ret = ephy_general_ctrl(phy_ctrl, cmd, arg);
        return ret;
    } else if (cmd_type != EPHY_CMD_TYPE_LAN8720) {
        /* cmd error */
        log_error("cmd type error: 0x%08X\r\n", cmd);
        return -1;
    }

    switch (cmd) {
        case EPHY_CMD_LAN8720_SET_FAR_LOOPBACK:
            eth_phy_mdio_read(phy_ctrl->phy_addr, LAN8720_MODE_CTRL_STA_OFFSET, &regval);
            if (arg) {
                regval |= LAN8720_FARLOOPBACK_BIT;
            } else {
                regval &= ~LAN8720_FARLOOPBACK_BIT;
            }
            eth_phy_mdio_write(phy_ctrl->phy_addr, LAN8720_MODE_CTRL_STA_OFFSET, regval);
            break;

        default:
            log_error("not support cmd: 0x%08X\r\n", cmd);
            ret = -1;
    }

    return ret;
}

int ephy_lan8720_dump_registers(eth_phy_ctrl_t *phy_ctrl)
{
    extern uint16_t bl_eth_smi_read(uint32_t num);

    log_info("%s Registers:\r\n", phy_ctrl->phy_drv->phy_name);
    log_info("[00]BCR    0x%04x\r\n", bl_eth_smi_read(0));
    log_info("[01]BSR    0x%04x\r\n", bl_eth_smi_read(1));
    log_info("[02]PHY1   0x%04x\r\n", bl_eth_smi_read(2));
    log_info("[03]PHY2   0x%04x\r\n", bl_eth_smi_read(3));
    log_info("[04]ANAR   0x%04x\r\n", bl_eth_smi_read(4));
    log_info("[05]ANLPAR 0x%04x\r\n", bl_eth_smi_read(5));
    log_info("[06]ANER   0x%04x\r\n", bl_eth_smi_read(6));
    log_info("[17]MCSR   0x%04x\r\n", bl_eth_smi_read(17));
    log_info("[18]SM     0x%04x\r\n", bl_eth_smi_read(18));
    log_info("[26]SECR   0x%04x\r\n", bl_eth_smi_read(26));
    log_info("[27]CSIR   0x%04x\r\n", bl_eth_smi_read(27));
    log_info("[29]ISR    0x%04x\r\n", bl_eth_smi_read(29));
    log_info("[30]IMR    0x%04x\r\n", bl_eth_smi_read(30));
    log_info("[31]PSCSR  0x%04x\r\n", bl_eth_smi_read(31));

    return 0;
}
