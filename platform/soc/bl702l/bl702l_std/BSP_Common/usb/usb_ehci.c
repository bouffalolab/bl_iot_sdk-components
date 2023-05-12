#if defined(BL808)
#include "bl808_usb.h"
#include "bl808_pds.h"
#elif defined(BL606P)
#include "bl606p_usb.h"
#include "bl606p_pds.h"
#elif defined(BL616)
#include "bl616_usb.h"
#include "bl616_pds.h"
#endif

extern void usb_ehci_interrupt();

void USBH_IRQHandler()
{
    if (USB_Global_IntStatus(USB_GLOBAL_INT_HC)) {
        usb_ehci_interrupt();
    }
}

void usb_hc_low_level_init(void)
{
    CPU_Interrupt_Disable(USB_IRQn);

    PDS_Turn_On_USB(1);

    uint32_t tmpVal;
    /* enable device-A for host */
    tmpVal = BL_RD_REG(PDS_BASE, PDS_USB_CTL);
    tmpVal = BL_CLR_REG_BIT(tmpVal, PDS_REG_USB_IDDIG);
    BL_WR_REG(PDS_BASE, PDS_USB_CTL, tmpVal);

    tmpVal = BL_RD_REG(USB_BASE, USB_OTG_CSR);
    tmpVal = BL_SET_REG_BIT(tmpVal, USB_A_BUS_DROP_HOV);
    tmpVal = BL_CLR_REG_BIT(tmpVal, USB_A_BUS_REQ_HOV);
    BL_WR_REG(USB_BASE, USB_OTG_CSR, tmpVal);

    arch_delay_ms(10);
    /* enable vbus and bus control */
    tmpVal = BL_RD_REG(USB_BASE, USB_OTG_CSR);
    tmpVal = BL_CLR_REG_BIT(tmpVal, USB_A_BUS_DROP_HOV);
    tmpVal = BL_SET_REG_BIT(tmpVal, USB_A_BUS_REQ_HOV);
    BL_WR_REG(USB_BASE, USB_OTG_CSR, tmpVal);

    USB_Global_IntMask(USB_GLOBAL_INT_HC, UNMASK);
    USB_Global_IntMask(USB_GLOBAL_INT_OTG, MASK);
    USB_Global_IntMask(USB_GLOBAL_INT_DEV, MASK);

    Interrupt_Handler_Register(USB_IRQn, USBH_IRQHandler);
    CPU_Interrupt_Enable(USB_IRQn);
}

uint8_t usbh_get_port_speed(const uint8_t port)
{
    uint32_t tmpVal;
    tmpVal = BL_RD_REG(USB_BASE, USB_OTG_CSR);

    if (BL_GET_REG_BITS_VAL(tmpVal, USB_SPD_TYP_HOV_POV) == 0) {
        return 2;
    } else if (BL_GET_REG_BITS_VAL(tmpVal, USB_SPD_TYP_HOV_POV) == 1) {
        return 1;
    } else if (BL_GET_REG_BITS_VAL(tmpVal, USB_SPD_TYP_HOV_POV) == 2) {
        return 3;
    }
}

void usb_ehci_dcache_clean(uintptr_t addr, uint32_t len)
{
#if (__riscv_xlen == 32)
    csi_dcache_clean_range((uint32_t *)addr, len);
#elif (__riscv_xlen == 64)
    csi_dcache_clean_range(addr, len);
#endif
}

void usb_ehci_dcache_invalidate(uintptr_t addr, uint32_t len)
{
#if (__riscv_xlen == 32)
    csi_dcache_invalid_range((uint32_t *)addr, len);
#elif (__riscv_xlen == 64)
    csi_dcache_invalid_range(addr, len);
#endif
}

void usb_ehci_dcache_clean_invalidate(uintptr_t addr, uint32_t len)
{
#if (__riscv_xlen == 32)
    csi_dcache_clean_invalid_range((uint32_t *)addr, len);
#elif (__riscv_xlen == 64)
    csi_dcache_clean_invalid_range(addr, len);
#endif
}