
#include "bl_emac.h"

#include "eth_phy_port.h"

void eth_phy_delay_ms(uint32_t ms)
{
    arch_delay_ms(ms);
}

int eth_phy_mdio_read(uint8_t phy_addr, uint8_t reg_addr, uint16_t *data)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(EMAC_BASE, EMAC_MIIADDRESS);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, EMAC_FIAD, phy_addr);
    BL_WR_REG(EMAC_BASE, EMAC_MIIADDRESS, tmpVal);

    return emac_phy_read(reg_addr, data);
}

int eth_phy_mdio_write(uint8_t phy_addr, uint8_t reg_addr, uint16_t data)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(EMAC_BASE, EMAC_MIIADDRESS);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, EMAC_FIAD, phy_addr);
    BL_WR_REG(EMAC_BASE, EMAC_MIIADDRESS, tmpVal);

    return emac_phy_write(reg_addr, data);
}
