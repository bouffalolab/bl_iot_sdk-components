#include <string.h>
#include <bl702l_aon.h>

#include "bl_sys.h"
#include "hal_sys.h"

void hal_reboot()
{
    bl_sys_reset_por();
}

void hal_sys_reset()
{
    bl_sys_reset_system();
}

void hal_poweroff()
{
    /*TODO empty now*/
}

void hal_sys_capcode_update(uint8_t capin, uint8_t capout)
{
    static uint8_t capin_static, capout_static;

    if (255 != capin && 255 != capout) {
        AON_Set_Xtal_CapCode(capin, capout);
        capin_static = capin;
        capout_static = capout;
    } else {
        AON_Set_Xtal_CapCode(capin_static, capout_static);
    }
}

uint8_t hal_sys_capcode_get(void)
{
    return AON_Get_Xtal_CapCode();
}
