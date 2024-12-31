#include "bl_pds.h"
#include "bl_flash.h"


#define CGEN_CFG0                  (~(uint8_t)((1<<1)|(1<<2)|(1<<4)))  // do not gate DMA[3], which affects M154_AES
#define CGEN_CFG1                  (~(uint32_t)((1<<2)|(1<<3)|(1<<4)|(1<<5)|(1<<7)|(1<<12)|(1<<13)|(0x7FDE<<16)))  // do not gate sf_ctrl[11], uart0[16], timer[21]


#if !defined(CFG_PDS_OPTIMIZE)
/* PDS0 Configuration */
static PDS_DEFAULT_LV_CFG_Type pdsCfgLevel0 = {
    .pdsCtl = {
        .pdsStart                = 1,
        .sleepForever            = 0,
        .xtalForceOff            = 0,
        .saveWifiState           = 0,
        .dcdc18Off               = 1,
        .bgSysOff                = 1,
        .gpioIePuPd              = 1,
        .puFlash                 = 0,
        .clkOff                  = 1,
        .memStby                 = 1,
        .swPuFlash               = 1,
        .isolation               = 1,
        .waitXtalRdy             = 0,
        .pdsPwrOff               = 1,
        .xtalOff                 = 1,
        .socEnbForceOn           = 1,
        .pdsRstSocEn             = 0,
        .pdsRC32mOn              = 0,
        .pdsLdoVselEn            = 0,
        .pdsRamLowPowerWithClkEn = 0,
        .cpu0WfiMask             = 0,
        .ldo11Off                = 1,
        .pdsForceRamClkEn        = 0,
        .pdsLdoVol               = 0xA,
        .pdsCtlRfSel             = 3,
        .pdsCtlPllSel            = 0,
    },
    .pdsCtl2 = {
        .forceCpuPwrOff          = 0,
        .forceBzPwrOff           = 0,
        .forceUsbPwrOff          = 0,
        .forceCpuIsoEn           = 0,
        .forceBzIsoEn            = 0,
        .forceUsbIsoEn           = 0,
        .forceCpuPdsRst          = 0,
        .forceBzPdsRst           = 0,
        .forceUsbPdsRst          = 0,
        .forceCpuMemStby         = 0,
        .forceBzMemStby          = 0,
        .forceUsbMemStby         = 0,
        .forceCpuGateClk         = 0,
        .forceBzGateClk          = 0,
        .forceUsbGateClk         = 0,
    },
    .pdsCtl3 = {
        .forceMiscPwrOff         = 0,
        .forceBlePwrOff          = 0,
        .forceBleIsoEn           = 0,
        .forceMiscPdsRst         = 0,
        .forceBlePdsRst          = 0,
        .forceMiscMemStby        = 0,
        .forceBleMemStby         = 0,
        .forceMiscGateClk        = 0,
        .forceBleGateClk         = 0,
        .CpuIsoEn                = 0,
        .BzIsoEn                 = 0,
        .BleIsoEn                = 1,
        .UsbIsoEn                = 0,
        .MiscIsoEn               = 0,
    },
    .pdsCtl4 = {
        .cpuPwrOff               = 0,
        .cpuRst                  = 0,
        .cpuMemStby              = 1,
        .cpuGateClk              = 1,
        .BzPwrOff                = 0,
        .BzRst                   = 0,
        .BzMemStby               = 1,
        .BzGateClk               = 1,
        .BlePwrOff               = 1,
        .BleRst                  = 1,
        .BleMemStby              = 1,
        .BleGateClk              = 1,
        .UsbPwrOff               = 0,
        .UsbRst                  = 0,
        .UsbMemStby              = 1,
        .UsbGateClk              = 1,
        .MiscPwrOff              = 0,
        .MiscRst                 = 0,
        .MiscMemStby             = 1,
        .MiscGateClk             = 1,
        .MiscAnaPwrOff           = 1,
        .MiscDigPwrOff           = 1,
    }
};

/* PDS1 Configuration */
static PDS_DEFAULT_LV_CFG_Type pdsCfgLevel1 = {
    .pdsCtl = {
        .pdsStart                = 1,
        .sleepForever            = 0,
        .xtalForceOff            = 0,
        .saveWifiState           = 0,
        .dcdc18Off               = 1,
        .bgSysOff                = 1,
        .gpioIePuPd              = 1,
        .puFlash                 = 0,
        .clkOff                  = 1,
        .memStby                 = 1,
        .swPuFlash               = 1,
        .isolation               = 1,
        .waitXtalRdy             = 0,
        .pdsPwrOff               = 1,
        .xtalOff                 = 1,
        .socEnbForceOn           = 1,
        .pdsRstSocEn             = 0,
        .pdsRC32mOn              = 0,
        .pdsLdoVselEn            = 0,
        .pdsRamLowPowerWithClkEn = 0,
        .cpu0WfiMask             = 0,
        .ldo11Off                = 1,
        .pdsForceRamClkEn        = 0,
        .pdsLdoVol               = 0xA,
        .pdsCtlRfSel             = 3,
        .pdsCtlPllSel            = 0,
    },
    .pdsCtl2 = {
        .forceCpuPwrOff          = 0,
        .forceBzPwrOff           = 0,
        .forceUsbPwrOff          = 0,
        .forceCpuIsoEn           = 0,
        .forceBzIsoEn            = 0,
        .forceUsbIsoEn           = 0,
        .forceCpuPdsRst          = 0,
        .forceBzPdsRst           = 0,
        .forceUsbPdsRst          = 0,
        .forceCpuMemStby         = 0,
        .forceBzMemStby          = 0,
        .forceUsbMemStby         = 0,
        .forceCpuGateClk         = 0,
        .forceBzGateClk          = 0,
        .forceUsbGateClk         = 0,
    },
    .pdsCtl3 = {
        .forceMiscPwrOff         = 0,
        .forceBlePwrOff          = 0,
        .forceBleIsoEn           = 0,
        .forceMiscPdsRst         = 0,
        .forceBlePdsRst          = 0,
        .forceMiscMemStby        = 0,
        .forceBleMemStby         = 0,
        .forceMiscGateClk        = 0,
        .forceBleGateClk         = 0,
        .CpuIsoEn                = 0,
        .BzIsoEn                 = 0,
        .BleIsoEn                = 1,
        .UsbIsoEn                = 1,
        .MiscIsoEn               = 0,
    },
    .pdsCtl4 = {
        .cpuPwrOff               = 0,
        .cpuRst                  = 0,
        .cpuMemStby              = 1,
        .cpuGateClk              = 1,
        .BzPwrOff                = 0,
        .BzRst                   = 0,
        .BzMemStby               = 1,
        .BzGateClk               = 1,
        .BlePwrOff               = 1,
        .BleRst                  = 1,
        .BleMemStby              = 1,
        .BleGateClk              = 1,
        .UsbPwrOff               = 1,
        .UsbRst                  = 1,
        .UsbMemStby              = 1,
        .UsbGateClk              = 1,
        .MiscPwrOff              = 0,
        .MiscRst                 = 0,
        .MiscMemStby             = 1,
        .MiscGateClk             = 1,
        .MiscAnaPwrOff           = 1,
        .MiscDigPwrOff           = 1,
    }
};

/* PDS2 Configuration */
static PDS_DEFAULT_LV_CFG_Type pdsCfgLevel2 = {
    .pdsCtl = {
        .pdsStart                = 1,
        .sleepForever            = 0,
        .xtalForceOff            = 0,
        .saveWifiState           = 0,
        .dcdc18Off               = 1,
        .bgSysOff                = 1,
        .gpioIePuPd              = 1,
        .puFlash                 = 0,
        .clkOff                  = 1,
        .memStby                 = 1,
        .swPuFlash               = 1,
        .isolation               = 1,
        .waitXtalRdy             = 0,
        .pdsPwrOff               = 1,
        .xtalOff                 = 1,
        .socEnbForceOn           = 1,
        .pdsRstSocEn             = 0,
        .pdsRC32mOn              = 0,
        .pdsLdoVselEn            = 0,
        .pdsRamLowPowerWithClkEn = 0,
        .cpu0WfiMask             = 0,
        .ldo11Off                = 1,
        .pdsForceRamClkEn        = 0,
        .pdsLdoVol               = 0xA,
        .pdsCtlRfSel             = 2,
        .pdsCtlPllSel            = 0,
    },
    .pdsCtl2 = {
        .forceCpuPwrOff          = 0,
        .forceBzPwrOff           = 0,
        .forceUsbPwrOff          = 0,
        .forceCpuIsoEn           = 0,
        .forceBzIsoEn            = 0,
        .forceUsbIsoEn           = 0,
        .forceCpuPdsRst          = 0,
        .forceBzPdsRst           = 0,
        .forceUsbPdsRst          = 0,
        .forceCpuMemStby         = 0,
        .forceBzMemStby          = 0,
        .forceUsbMemStby         = 0,
        .forceCpuGateClk         = 0,
        .forceBzGateClk          = 0,
        .forceUsbGateClk         = 0,
    },
    .pdsCtl3 = {
        .forceMiscPwrOff         = 0,
        .forceBlePwrOff          = 0,
        .forceBleIsoEn           = 0,
        .forceMiscPdsRst         = 0,
        .forceBlePdsRst          = 0,
        .forceMiscMemStby        = 0,
        .forceBleMemStby         = 0,
        .forceMiscGateClk        = 0,
        .forceBleGateClk         = 0,
        .CpuIsoEn                = 0,
        .BzIsoEn                 = 1,
        .BleIsoEn                = 1,
        .UsbIsoEn                = 0,
        .MiscIsoEn               = 0,
    },
    .pdsCtl4 = {
        .cpuPwrOff               = 0,
        .cpuRst                  = 0,
        .cpuMemStby              = 1,
        .cpuGateClk              = 1,
        .BzPwrOff                = 1,
        .BzRst                   = 1,
        .BzMemStby               = 1,
        .BzGateClk               = 1,
        .BlePwrOff               = 1,
        .BleRst                  = 1,
        .BleMemStby              = 1,
        .BleGateClk              = 1,
        .UsbPwrOff               = 0,
        .UsbRst                  = 0,
        .UsbMemStby              = 1,
        .UsbGateClk              = 1,
        .MiscPwrOff              = 0,
        .MiscRst                 = 0,
        .MiscMemStby             = 1,
        .MiscGateClk             = 1,
        .MiscAnaPwrOff           = 1,
        .MiscDigPwrOff           = 1,
    }
};

/* PDS3 Configuration */
static PDS_DEFAULT_LV_CFG_Type pdsCfgLevel3 = {
    .pdsCtl = {
        .pdsStart                = 1,
        .sleepForever            = 0,
        .xtalForceOff            = 0,
        .saveWifiState           = 0,
        .dcdc18Off               = 1,
        .bgSysOff                = 1,
        .gpioIePuPd              = 1,
        .puFlash                 = 0,
        .clkOff                  = 1,
        .memStby                 = 1,
        .swPuFlash               = 1,
        .isolation               = 1,
        .waitXtalRdy             = 0,
        .pdsPwrOff               = 1,
        .xtalOff                 = 1,
        .socEnbForceOn           = 1,
        .pdsRstSocEn             = 0,
        .pdsRC32mOn              = 0,
        .pdsLdoVselEn            = 0,
        .pdsRamLowPowerWithClkEn = 0,
        .cpu0WfiMask             = 0,
        .ldo11Off                = 1,
        .pdsForceRamClkEn        = 0,
        .pdsLdoVol               = 0xA,
        .pdsCtlRfSel             = 2,
        .pdsCtlPllSel            = 0,
    },
    .pdsCtl2 = {
        .forceCpuPwrOff          = 0,
        .forceBzPwrOff           = 0,
        .forceUsbPwrOff          = 0,
        .forceCpuIsoEn           = 0,
        .forceBzIsoEn            = 0,
        .forceUsbIsoEn           = 0,
        .forceCpuPdsRst          = 0,
        .forceBzPdsRst           = 0,
        .forceUsbPdsRst          = 0,
        .forceCpuMemStby         = 0,
        .forceBzMemStby          = 0,
        .forceUsbMemStby         = 0,
        .forceCpuGateClk         = 0,
        .forceBzGateClk          = 0,
        .forceUsbGateClk         = 0,
    },
    .pdsCtl3 = {
        .forceMiscPwrOff         = 0,
        .forceBlePwrOff          = 0,
        .forceBleIsoEn           = 0,
        .forceMiscPdsRst         = 0,
        .forceBlePdsRst          = 0,
        .forceMiscMemStby        = 0,
        .forceBleMemStby         = 0,
        .forceMiscGateClk        = 0,
        .forceBleGateClk         = 0,
        .CpuIsoEn                = 0,
        .BzIsoEn                 = 1,
        .BleIsoEn                = 1,
        .UsbIsoEn                = 1,
        .MiscIsoEn               = 0,
    },
    .pdsCtl4 = {
        .cpuPwrOff               = 0,
        .cpuRst                  = 0,
        .cpuMemStby              = 1,
        .cpuGateClk              = 1,
        .BzPwrOff                = 1,
        .BzRst                   = 1,
        .BzMemStby               = 1,
        .BzGateClk               = 1,
        .BlePwrOff               = 1,
        .BleRst                  = 1,
        .BleMemStby              = 1,
        .BleGateClk              = 1,
        .UsbPwrOff               = 1,
        .UsbRst                  = 1,
        .UsbMemStby              = 1,
        .UsbGateClk              = 1,
        .MiscPwrOff              = 0,
        .MiscRst                 = 0,
        .MiscMemStby             = 1,
        .MiscGateClk             = 1,
        .MiscAnaPwrOff           = 1,
        .MiscDigPwrOff           = 1,
    }
};

/* PDS4 Configuration */
static PDS_DEFAULT_LV_CFG_Type pdsCfgLevel4 = {
    .pdsCtl = {
        .pdsStart                = 1,
        .sleepForever            = 0,
        .xtalForceOff            = 0,
        .saveWifiState           = 0,
        .dcdc18Off               = 1,
        .bgSysOff                = 1,
        .gpioIePuPd              = 1,
        .puFlash                 = 0,
        .clkOff                  = 1,
        .memStby                 = 1,
        .swPuFlash               = 1,
        .isolation               = 1,
        .waitXtalRdy             = 0,
        .pdsPwrOff               = 1,
        .xtalOff                 = 1,
        .socEnbForceOn           = 1,
        .pdsRstSocEn             = 0,
        .pdsRC32mOn              = 0,
        .pdsLdoVselEn            = 0,
        .pdsRamLowPowerWithClkEn = 0,
        .cpu0WfiMask             = 0,
        .ldo11Off                = 1,
        .pdsForceRamClkEn        = 0,
        .pdsLdoVol               = 0xA,
        .pdsCtlRfSel             = 3,
        .pdsCtlPllSel            = 0,
    },
    .pdsCtl2 = {
        .forceCpuPwrOff          = 0,
        .forceBzPwrOff           = 0,
        .forceUsbPwrOff          = 0,
        .forceCpuIsoEn           = 0,
        .forceBzIsoEn            = 0,
        .forceUsbIsoEn           = 0,
        .forceCpuPdsRst          = 0,
        .forceBzPdsRst           = 0,
        .forceUsbPdsRst          = 0,
        .forceCpuMemStby         = 0,
        .forceBzMemStby          = 0,
        .forceUsbMemStby         = 0,
        .forceCpuGateClk         = 0,
        .forceBzGateClk          = 0,
        .forceUsbGateClk         = 0,
    },
    .pdsCtl3 = {
        .forceMiscPwrOff         = 0,
        .forceBlePwrOff          = 0,
        .forceBleIsoEn           = 0,
        .forceMiscPdsRst         = 0,
        .forceBlePdsRst          = 0,
        .forceMiscMemStby        = 0,
        .forceBleMemStby         = 0,
        .forceMiscGateClk        = 0,
        .forceBleGateClk         = 0,
        .CpuIsoEn                = 1,
        .BzIsoEn                 = 0,
        .BleIsoEn                = 1,
        .UsbIsoEn                = 0,
        .MiscIsoEn               = 0,
    },
    .pdsCtl4 = {
        .cpuPwrOff               = 1,
        .cpuRst                  = 1,
        .cpuMemStby              = 1,
        .cpuGateClk              = 1,
        .BzPwrOff                = 0,
        .BzRst                   = 0,
        .BzMemStby               = 1,
        .BzGateClk               = 1,
        .BlePwrOff               = 1,
        .BleRst                  = 1,
        .BleMemStby              = 1,
        .BleGateClk              = 1,
        .UsbPwrOff               = 0,
        .UsbRst                  = 0,
        .UsbMemStby              = 1,
        .UsbGateClk              = 1,
        .MiscPwrOff              = 0,
        .MiscRst                 = 1,  // reset misc if bootrom_protect = 1
        .MiscMemStby             = 1,
        .MiscGateClk             = 1,
        .MiscAnaPwrOff           = 1,
        .MiscDigPwrOff           = 1,
    }
};

/* PDS5 Configuration */
static PDS_DEFAULT_LV_CFG_Type pdsCfgLevel5 = {
    .pdsCtl = {
        .pdsStart                = 1,
        .sleepForever            = 0,
        .xtalForceOff            = 0,
        .saveWifiState           = 0,
        .dcdc18Off               = 1,
        .bgSysOff                = 1,
        .gpioIePuPd              = 1,
        .puFlash                 = 0,
        .clkOff                  = 1,
        .memStby                 = 1,
        .swPuFlash               = 1,
        .isolation               = 1,
        .waitXtalRdy             = 0,
        .pdsPwrOff               = 1,
        .xtalOff                 = 1,
        .socEnbForceOn           = 1,
        .pdsRstSocEn             = 0,
        .pdsRC32mOn              = 0,
        .pdsLdoVselEn            = 0,
        .pdsRamLowPowerWithClkEn = 0,
        .cpu0WfiMask             = 0,
        .ldo11Off                = 1,
        .pdsForceRamClkEn        = 0,
        .pdsLdoVol               = 0xA,
        .pdsCtlRfSel             = 3,
        .pdsCtlPllSel            = 0,
    },
    .pdsCtl2 = {
        .forceCpuPwrOff          = 0,
        .forceBzPwrOff           = 0,
        .forceUsbPwrOff          = 0,
        .forceCpuIsoEn           = 0,
        .forceBzIsoEn            = 0,
        .forceUsbIsoEn           = 0,
        .forceCpuPdsRst          = 0,
        .forceBzPdsRst           = 0,
        .forceUsbPdsRst          = 0,
        .forceCpuMemStby         = 0,
        .forceBzMemStby          = 0,
        .forceUsbMemStby         = 0,
        .forceCpuGateClk         = 0,
        .forceBzGateClk          = 0,
        .forceUsbGateClk         = 0,
    },
    .pdsCtl3 = {
        .forceMiscPwrOff         = 0,
        .forceBlePwrOff          = 0,
        .forceBleIsoEn           = 0,
        .forceMiscPdsRst         = 0,
        .forceBlePdsRst          = 0,
        .forceMiscMemStby        = 0,
        .forceBleMemStby         = 0,
        .forceMiscGateClk        = 0,
        .forceBleGateClk         = 0,
        .CpuIsoEn                = 1,
        .BzIsoEn                 = 0,
        .BleIsoEn                = 1,
        .UsbIsoEn                = 1,
        .MiscIsoEn               = 0,
    },
    .pdsCtl4 = {
        .cpuPwrOff               = 1,
        .cpuRst                  = 1,
        .cpuMemStby              = 1,
        .cpuGateClk              = 1,
        .BzPwrOff                = 0,
        .BzRst                   = 0,
        .BzMemStby               = 1,
        .BzGateClk               = 1,
        .BlePwrOff               = 1,
        .BleRst                  = 1,
        .BleMemStby              = 1,
        .BleGateClk              = 1,
        .UsbPwrOff               = 1,
        .UsbRst                  = 1,
        .UsbMemStby              = 1,
        .UsbGateClk              = 1,
        .MiscPwrOff              = 0,
        .MiscRst                 = 1,  // reset misc if bootrom_protect = 1
        .MiscMemStby             = 1,
        .MiscGateClk             = 1,
        .MiscAnaPwrOff           = 1,
        .MiscDigPwrOff           = 1,
    }
};

/* PDS6 Configuration */
static PDS_DEFAULT_LV_CFG_Type pdsCfgLevel6 = {
    .pdsCtl = {
        .pdsStart                = 1,
        .sleepForever            = 0,
        .xtalForceOff            = 0,
        .saveWifiState           = 0,
        .dcdc18Off               = 1,
        .bgSysOff                = 1,
        .gpioIePuPd              = 1,
        .puFlash                 = 0,
        .clkOff                  = 1,
        .memStby                 = 1,
        .swPuFlash               = 1,
        .isolation               = 1,
        .waitXtalRdy             = 0,
        .pdsPwrOff               = 1,
        .xtalOff                 = 1,
        .socEnbForceOn           = 1,
        .pdsRstSocEn             = 0,
        .pdsRC32mOn              = 0,
        .pdsLdoVselEn            = 0,
        .pdsRamLowPowerWithClkEn = 0,
        .cpu0WfiMask             = 0,
        .ldo11Off                = 1,
        .pdsForceRamClkEn        = 0,
        .pdsLdoVol               = 0xA,
        .pdsCtlRfSel             = 2,
        .pdsCtlPllSel            = 0,
    },
    .pdsCtl2 = {
        .forceCpuPwrOff          = 0,
        .forceBzPwrOff           = 0,
        .forceUsbPwrOff          = 0,
        .forceCpuIsoEn           = 0,
        .forceBzIsoEn            = 0,
        .forceUsbIsoEn           = 0,
        .forceCpuPdsRst          = 0,
        .forceBzPdsRst           = 0,
        .forceUsbPdsRst          = 0,
        .forceCpuMemStby         = 0,
        .forceBzMemStby          = 0,
        .forceUsbMemStby         = 0,
        .forceCpuGateClk         = 0,
        .forceBzGateClk          = 0,
        .forceUsbGateClk         = 0,
    },
    .pdsCtl3 = {
        .forceMiscPwrOff         = 0,
        .forceBlePwrOff          = 0,
        .forceBleIsoEn           = 0,
        .forceMiscPdsRst         = 0,
        .forceBlePdsRst          = 0,
        .forceMiscMemStby        = 0,
        .forceBleMemStby         = 0,
        .forceMiscGateClk        = 0,
        .forceBleGateClk         = 0,
        .CpuIsoEn                = 1,
        .BzIsoEn                 = 1,
        .BleIsoEn                = 1,
        .UsbIsoEn                = 0,
        .MiscIsoEn               = 0,
    },
    .pdsCtl4 = {
        .cpuPwrOff               = 1,
        .cpuRst                  = 1,
        .cpuMemStby              = 1,
        .cpuGateClk              = 1,
        .BzPwrOff                = 1,
        .BzRst                   = 1,
        .BzMemStby               = 1,
        .BzGateClk               = 1,
        .BlePwrOff               = 1,
        .BleRst                  = 1,
        .BleMemStby              = 1,
        .BleGateClk              = 1,
        .UsbPwrOff               = 0,
        .UsbRst                  = 0,
        .UsbMemStby              = 1,
        .UsbGateClk              = 1,
        .MiscPwrOff              = 0,
        .MiscRst                 = 1,  // reset misc if bootrom_protect = 1
        .MiscMemStby             = 1,
        .MiscGateClk             = 1,
        .MiscAnaPwrOff           = 1,
        .MiscDigPwrOff           = 1,
    }
};

/* PDS7 Configuration */
static PDS_DEFAULT_LV_CFG_Type pdsCfgLevel7 = {
    .pdsCtl = {
        .pdsStart                = 1,
        .sleepForever            = 0,
        .xtalForceOff            = 0,
        .saveWifiState           = 0,
        .dcdc18Off               = 1,
        .bgSysOff                = 1,
        .gpioIePuPd              = 1,
        .puFlash                 = 0,
        .clkOff                  = 1,
        .memStby                 = 1,
        .swPuFlash               = 1,
        .isolation               = 1,
        .waitXtalRdy             = 0,
        .pdsPwrOff               = 1,
        .xtalOff                 = 1,
        .socEnbForceOn           = 1,
        .pdsRstSocEn             = 0,
        .pdsRC32mOn              = 0,
        .pdsLdoVselEn            = 0,
        .pdsRamLowPowerWithClkEn = 0,
        .cpu0WfiMask             = 0,
        .ldo11Off                = 1,
        .pdsForceRamClkEn        = 0,
        .pdsLdoVol               = 0xA,
        .pdsCtlRfSel             = 2,
        .pdsCtlPllSel            = 0,
    },
    .pdsCtl2 = {
        .forceCpuPwrOff          = 0,
        .forceBzPwrOff           = 0,
        .forceUsbPwrOff          = 0,
        .forceCpuIsoEn           = 0,
        .forceBzIsoEn            = 0,
        .forceUsbIsoEn           = 0,
        .forceCpuPdsRst          = 0,
        .forceBzPdsRst           = 0,
        .forceUsbPdsRst          = 0,
        .forceCpuMemStby         = 0,
        .forceBzMemStby          = 0,
        .forceUsbMemStby         = 0,
        .forceCpuGateClk         = 0,
        .forceBzGateClk          = 0,
        .forceUsbGateClk         = 0,
    },
    .pdsCtl3 = {
        .forceMiscPwrOff         = 0,
        .forceBlePwrOff          = 0,
        .forceBleIsoEn           = 0,
        .forceMiscPdsRst         = 0,
        .forceBlePdsRst          = 0,
        .forceMiscMemStby        = 0,
        .forceBleMemStby         = 0,
        .forceMiscGateClk        = 0,
        .forceBleGateClk         = 0,
        .CpuIsoEn                = 1,
        .BzIsoEn                 = 1,
        .BleIsoEn                = 1,
        .UsbIsoEn                = 1,
        .MiscIsoEn               = 0,
    },
    .pdsCtl4 = {
        .cpuPwrOff               = 1,
        .cpuRst                  = 1,
        .cpuMemStby              = 1,
        .cpuGateClk              = 1,
        .BzPwrOff                = 1,
        .BzRst                   = 1,
        .BzMemStby               = 1,
        .BzGateClk               = 1,
        .BlePwrOff               = 1,
        .BleRst                  = 1,
        .BleMemStby              = 1,
        .BleGateClk              = 1,
        .UsbPwrOff               = 1,
        .UsbRst                  = 1,
        .UsbMemStby              = 1,
        .UsbGateClk              = 1,
        .MiscPwrOff              = 0,
        .MiscRst                 = 1,  // reset misc if bootrom_protect = 1
        .MiscMemStby             = 1,
        .MiscGateClk             = 1,
        .MiscAnaPwrOff           = 1,
        .MiscDigPwrOff           = 1,
    }
};
#endif

/* PDS31 Configuration */
static PDS_DEFAULT_LV_CFG_Type pdsCfgLevel31 = {
    .pdsCtl = {
        .pdsStart                = 1,
        .sleepForever            = 0,
        .xtalForceOff            = 0,
        .saveWifiState           = 0,
        .dcdc18Off               = 1,
        .bgSysOff                = 1,
        .gpioIePuPd              = 1,
        .puFlash                 = 0,
        .clkOff                  = 1,
        .memStby                 = 1,
        .swPuFlash               = 1,
        .isolation               = 1,
        .waitXtalRdy             = 0,
        .pdsPwrOff               = 1,
        .xtalOff                 = 1,
        .socEnbForceOn           = 0,
        .pdsRstSocEn             = 0,
        .pdsRC32mOn              = 0,
        .pdsLdoVselEn            = 0,
        .pdsRamLowPowerWithClkEn = 1,
        .cpu0WfiMask             = 0,
        .ldo11Off                = 1,
        .pdsForceRamClkEn        = 0,
        .pdsLdoVol               = 0xA,
        .pdsCtlRfSel             = 2,
        .pdsCtlPllSel            = 0,
    },
    .pdsCtl2 = {
        .forceCpuPwrOff          = 0,
        .forceBzPwrOff           = 0,
        .forceUsbPwrOff          = 0,
        .forceCpuIsoEn           = 0,
        .forceBzIsoEn            = 0,
        .forceUsbIsoEn           = 0,
        .forceCpuPdsRst          = 0,
        .forceBzPdsRst           = 0,
        .forceUsbPdsRst          = 0,
        .forceCpuMemStby         = 0,
        .forceBzMemStby          = 0,
        .forceUsbMemStby         = 0,
        .forceCpuGateClk         = 0,
        .forceBzGateClk          = 0,
        .forceUsbGateClk         = 0,
    },
    .pdsCtl3 = {
        .forceMiscPwrOff         = 0,
        .forceBlePwrOff          = 0,
        .forceBleIsoEn           = 0,
        .forceMiscPdsRst         = 0,
        .forceBlePdsRst          = 0,
        .forceMiscMemStby        = 0,
        .forceBleMemStby         = 0,
        .forceMiscGateClk        = 0,
        .forceBleGateClk         = 0,
        .CpuIsoEn                = 1,
        .BzIsoEn                 = 1,
        .BleIsoEn                = 1,
        .UsbIsoEn                = 1,
        .MiscIsoEn               = 1,
    },
    .pdsCtl4 = {
        .cpuPwrOff               = 1,
        .cpuRst                  = 1,
        .cpuMemStby              = 1,
        .cpuGateClk              = 1,
        .BzPwrOff                = 1,
        .BzRst                   = 1,
        .BzMemStby               = 1,
        .BzGateClk               = 1,
        .BlePwrOff               = 1,
        .BleRst                  = 1,
        .BleMemStby              = 1,
        .BleGateClk              = 1,
        .UsbPwrOff               = 1,
        .UsbRst                  = 1,
        .UsbMemStby              = 1,
        .UsbGateClk              = 1,
        .MiscPwrOff              = 1,
        .MiscRst                 = 1,
        .MiscMemStby             = 1,
        .MiscGateClk             = 1,
        .MiscAnaPwrOff           = 1,
        .MiscDigPwrOff           = 1,
    }
};

/* Backup Memory */
static uint32_t bl_pds_bak[29];
static uint32_t bl_pds_bak_addr = (uint32_t)bl_pds_bak;

/* Device Information, will get from efuse */
static Efuse_Device_Info_Type devInfo = {};

/* PSRAM IO Configuration, will get according to device information */
static uint8_t psramIoCfg = 0;

/* Cache Way Disable, will get from l1c register */
static uint8_t cacheWayDisable = 0;

/* EM Select, will get from glb register */
static uint8_t emSel = 0;

/* Flash Configuration Pointer, will get from bl_flash_get_flashCfg() */
static SPI_Flash_Cfg_Type *flashCfgPtr = NULL;

/* Flash Image Offset, will get from SF_Ctrl_Get_Flash_Image_Offset() */
static uint32_t flashImageOffset = 0;

/* Flash Continuous Read, will get based on flash configuration */
static uint8_t flashContRead = 0;

/* SF Control Configuration, will get based on flash configuration */
static SF_Ctrl_Cfg_Type sfCtrlCfg = {};

/* PDS Configuration Pointer */
static PDS_DEFAULT_LV_CFG_Type *pdsCfgPtr = NULL;

/* Backup 0x4202DFF4, which will be overwritten by bootrom */
static uint32_t patchBootrom = 0;

/* Flag whether cpu registers are stored or not */
static uint8_t cpuRegStored = 0;

/* Flag whether flash is power down */
static uint8_t flashPowerdown = 0;

/* HBN IRQ Status, will get from hbn register after wakeup */
static uint32_t hbnIrqStatus = 0;


/* Private Functions */
void bl_pds_get_sf_ctrl_cfg(SPI_Flash_Cfg_Type *pFlashCfg, SF_Ctrl_Cfg_Type *pSfCtrlCfg)
{
    const uint8_t delay[8] = {0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe};
    uint8_t index;
    
    pSfCtrlCfg->owner = SF_CTRL_OWNER_SAHB;
    
    /* bit0-3 for clk delay */
    pSfCtrlCfg->clkDelay = pFlashCfg->clkDelay & 0x0f;
    
    /* bit4-6 for do delay */
    index = (pFlashCfg->clkDelay >> 4) & 0x07;
    pSfCtrlCfg->doDelay = delay[index];
    
    /* bit0 for clk invert */
    pSfCtrlCfg->clkInvert = pFlashCfg->clkInvert & 0x01;
    
    /* bit1 for rx clk invert */
    pSfCtrlCfg->rxClkInvert=(pFlashCfg->clkInvert >> 1) & 0x01;
    
    /* bit2-4 for di delay */
    index = (pFlashCfg->clkInvert >> 2) & 0x07;
    pSfCtrlCfg->diDelay = delay[index];
    
    /* bit5-7 for oe delay */
    index = (pFlashCfg->clkInvert >> 5) & 0x07;
    pSfCtrlCfg->oeDelay = delay[index];
}

ATTR_PDS_SECTION
void bl_pds_gpio_clear_int_status(void)
{
    uint32_t tmpVal;
    
    tmpVal = BL_RD_REG(PDS_BASE, PDS_GPIO_INT);
    tmpVal = BL_SET_REG_BIT(tmpVal, PDS_GPIO_INT_CLR);
    BL_WR_REG(PDS_BASE, PDS_GPIO_INT, tmpVal);
    
    tmpVal = BL_RD_REG(PDS_BASE, PDS_GPIO_INT);
    tmpVal = BL_CLR_REG_BIT(tmpVal, PDS_GPIO_INT_CLR);
    BL_WR_REG(PDS_BASE, PDS_GPIO_INT, tmpVal);
}

ATTR_PDS_SECTION
void bl_pds_restore_sf(void)
{
    // Clear flash pads pull configuration
    *(volatile uint32_t *)0x4000E030 = 0;
    
    // Initialize flash gpio
    RomDriver_SF_Cfg_Init_Flash_Gpio((devInfo.flash_cfg<<2)|devInfo.sf_swap_cfg, 0);
    
    // Patch: BL706C-22
    *(volatile uint32_t *)0x40000130 |= (1U << 16);  // enable GPIO25 input
    *(volatile uint32_t *)0x40000134 |= (1U << 16);  // enable GPIO27 input
    
    // Patch: restore psram io configuration
    BL_WR_REG(GLB_BASE, GLB_GPIO_USE_PSRAM__IO, psramIoCfg);
}

ATTR_PDS_SECTION
void bl_pds_restore_flash(SF_Ctrl_Cfg_Type *pSfCtrlCfg, SPI_Flash_Cfg_Type *pFlashCfg, uint32_t flashImageOffset, uint8_t flashContRead, uint8_t cacheWayDisable)
{
    uint32_t tmp[1];
    
    RomDriver_SFlash_Init(pSfCtrlCfg);
    
    RomDriver_SFlash_Releae_Powerdown(pFlashCfg);
    RomDriver_BL702_Delay_US(pFlashCfg->pdDelay);
    
    RomDriver_SFlash_Reset_Continue_Read(pFlashCfg);
    
    RomDriver_SFlash_Software_Reset(pFlashCfg);
    
    RomDriver_SFlash_Write_Enable(pFlashCfg);
    
    RomDriver_SFlash_DisableBurstWrap(pFlashCfg);
    
    RomDriver_SFlash_SetSPIMode(SF_CTRL_SPI_MODE);
    
    if((pFlashCfg->ioMode&0x0f)==SF_CTRL_QO_MODE||(pFlashCfg->ioMode&0x0f)==SF_CTRL_QIO_MODE){
        RomDriver_SFlash_Qspi_Enable(pFlashCfg);
    }
    
    if(((pFlashCfg->ioMode>>4)&0x01)==1){
        RomDriver_L1C_Set_Wrap(DISABLE);
    }else{
        RomDriver_L1C_Set_Wrap(ENABLE);
        RomDriver_SFlash_Write_Enable(pFlashCfg);
        if((pFlashCfg->ioMode&0x0f)==SF_CTRL_QO_MODE||(pFlashCfg->ioMode&0x0f)==SF_CTRL_QIO_MODE){
            RomDriver_SFlash_SetBurstWrap(pFlashCfg);
        }
    }
    
    if(flashContRead){
        RomDriver_SF_Ctrl_Set_Owner(SF_CTRL_OWNER_SAHB);
        RomDriver_SFlash_Read(pFlashCfg, pFlashCfg->ioMode&0xf, 1, 0x00000000, (uint8_t *)tmp, sizeof(tmp));
    }
    
    RomDriver_SF_Ctrl_Set_Flash_Image_Offset(flashImageOffset);
    RomDriver_SFlash_Cache_Read_Enable(pFlashCfg, pFlashCfg->ioMode&0xf, flashContRead, cacheWayDisable);
}

ATTR_PDS_SECTION
void bl_pds_restore_tcm(void)
{
#if !defined(CFG_PDS_OPTIMIZE)
    extern uint8_t _tcm_load;
    extern uint8_t _tcm_run;
    extern uint8_t _tcm_run_end;
    uint32_t src = (uint32_t)&_tcm_load;
    uint32_t dst = (uint32_t)&_tcm_run;
    uint32_t end = (uint32_t)&_tcm_run_end;
#else
    extern uint8_t _tcm_load;
    extern uint8_t _tcm_run;
    extern uint8_t _pds_restore_tcm_run_end;
    uint32_t src = (uint32_t)&_tcm_load;
    uint32_t dst = (uint32_t)&_tcm_run;
    uint32_t end = (uint32_t)&_pds_restore_tcm_run_end;
#endif
    
    while(dst < end){
        *(uint32_t *)dst = *(uint32_t *)src;
        src += 4;
        dst += 4;
    }
}

ATTR_PDS_SECTION
void bl_pds_restore_cpu_reg(void)
{
    __asm__ __volatile__(
            "lw     a0,     bl_pds_bak_addr\n\t"
            "lw     ra,     0(a0)\n\t"
            "lw     sp,     1*4(a0)\n\t"
            "lw     tp,     2*4(a0)\n\t"
            "lw     t0,     3*4(a0)\n\t"
            "lw     t1,     4*4(a0)\n\t"
            "lw     t2,     5*4(a0)\n\t"
            "lw     fp,     6*4(a0)\n\t"
            "lw     s1,     7*4(a0)\n\t"
            "lw     a1,     8*4(a0)\n\t"
            "lw     a2,     9*4(a0)\n\t"
            "lw     a3,     10*4(a0)\n\t"
            "lw     a4,     11*4(a0)\n\t"
            "lw     a5,     12*4(a0)\n\t"
            "lw     a6,     13*4(a0)\n\t"
            "lw     a7,     14*4(a0)\n\t"
            "lw     s2,     15*4(a0)\n\t"
            "lw     s3,     16*4(a0)\n\t"
            "lw     s4,     17*4(a0)\n\t"
            "lw     s5,     18*4(a0)\n\t"
            "lw     s6,     19*4(a0)\n\t"
            "lw     s7,     20*4(a0)\n\t"
            "lw     s8,     21*4(a0)\n\t"
            "lw     s9,     22*4(a0)\n\t"
            "lw     s10,    23*4(a0)\n\t"
            "lw     s11,    24*4(a0)\n\t"
            "lw     t3,     25*4(a0)\n\t"
            "lw     t4,     26*4(a0)\n\t"
            "lw     t5,     27*4(a0)\n\t"
            "lw     t6,     28*4(a0)\n\t"
            "csrw   mtvec,  a1\n\t"
            "csrw   mstatus,a2\n\t"
            "ret\n\t"
    );
}

ATTR_PDS_SECTION
void bl_pds_restore(void)
{
#if 0
    volatile int debug = 0;
    while(!debug);
#else
    volatile uint32_t *p = (volatile uint32_t *)0x40000100;
    volatile uint32_t *q = (volatile uint32_t *)0x40000148;
    while(p <= q){
        *p++ = 0x0B000B00;
    }
#endif
    
#if 0
    GLB_GPIO_Cfg_Type gpioCfg;
    GLB_GPIO_Type gpioPin = 22;
    uint32_t *pOut = (uint32_t *)(GLB_BASE + GLB_GPIO_OUTPUT_OFFSET);
    uint32_t pos = gpioPin % 32;
    
    gpioCfg.gpioPin = gpioPin;
    gpioCfg.gpioFun = 11;
    gpioCfg.gpioMode = GPIO_MODE_OUTPUT;
    gpioCfg.pullType = GPIO_PULL_NONE;
    gpioCfg.drive = 0;
    gpioCfg.smtCtrl = 1;
    RomDriver_GLB_GPIO_Init(&gpioCfg);
    
    *pOut |= (1<<pos);
    RomDriver_BL702_Delay_US(100);
    *pOut &= ~(1<<pos);
#endif
    
    // Get hbn irq status
    hbnIrqStatus = BL_RD_REG(HBN_BASE, HBN_IRQ_STAT);
    
    // Power off RF
    *(volatile uint32_t *)(AON_BASE + AON_RF_TOP_AON_OFFSET) &= ~(uint32_t)((0x1<<0)|(0x1<<1)|(0x1<<2));
    
    // Power on XTAL32M, later will check whether it is ready for use
    *(volatile uint32_t *)(AON_BASE + AON_RF_TOP_AON_OFFSET) |= (0x1<<4)|(0x1<<5);
    
    // Switch pka clock
    *(volatile uint32_t *)(GLB_BASE + GLB_SWRST_CFG2_OFFSET) |= (0x1<<24);
    
    // Reset secure engine
    *(volatile uint32_t *)(GLB_BASE + GLB_SWRST_CFG1_OFFSET) &= ~(uint32_t)(0x1<<4);
    
    // Disable peripheral clock
    *(volatile uint32_t *)(GLB_BASE + GLB_CLK_CFG1_OFFSET) &= ~(uint32_t)((0x1<<8)|(0x1<<13)|(0x1<<24)|(0x1<<25)|(0x1<<28));
    *(volatile uint32_t *)(GLB_BASE + GLB_CLK_CFG2_OFFSET) &= ~(uint32_t)((0x1<<4)|(0x1<<23)|(0xFF<<24));
    *(volatile uint32_t *)(GLB_BASE + GLB_CLK_CFG3_OFFSET) &= ~(uint32_t)((0x1<<8)|(0x1<<24));
    
    // Gate peripheral clock
    BL_WR_REG(GLB_BASE, GLB_CGEN_CFG0, CGEN_CFG0);
    BL_WR_REG(GLB_BASE, GLB_CGEN_CFG1, CGEN_CFG1);
    
    // Restore EM select
    BL_WR_REG(GLB_BASE, GLB_SEAM_MISC, emSel);
    
    // Restore 0x4202DFF4, which will be overwritten by bootrom
    *(uint32_t *)0x4202DFF4 = patchBootrom;
    
    // Configure flash (must use rom driver, since tcm code is lost and flash is power down)
    flash_restore();
    
    // Restore tcm code
    bl_pds_restore_tcm();
    
    // Wait until XTAL32M is ready for use
    while(!BL_IS_REG_BIT_SET(BL_RD_REG(AON_BASE, AON_TSEN), AON_XTAL_RDY));
    RomDriver_BL702_Delay_MS(1);
    
    // Select XTAL32M as root clock
    RomDriver_HBN_Set_ROOT_CLK_Sel(HBN_ROOT_CLK_XTAL);
    
    // Power on RF
    *(volatile uint32_t *)(AON_BASE + AON_RF_TOP_AON_OFFSET) |= (0x1<<0)|(0x1<<1)|(0x1<<2);
    
    // Disable global interrupt
    __disable_irq();
    
    // Set cpuRegStored flag
    cpuRegStored = 1;
    
    // Call user callback
    bl_pds_fastboot_done_callback();
    
    // Clear cpuRegStored flag
    cpuRegStored = 0;
    
    // Restore cpu registers
    bl_pds_restore_cpu_reg();
}

ATTR_PDS_SECTION
void bl_pds_fastboot_entry(void)
{
    __asm__ __volatile__(
            ".option push\n\t"
            ".option norelax\n\t"
            "la gp, __global_pointer$\n\t"
            ".option pop\n\t"
            "li a0, 0x40000000\n\t"
            "sw x0, 0x7C(a0)\n\t"
            "la sp, _sp_main\n\t"
            "call bl_pds_restore\n\t"
    );
}


/* Public Functions */
void bl_pds_init(void)
{
    // Get device information from efuse
    EF_Ctrl_Read_Device_Info(&devInfo);
    devInfo.flash_cfg &= 0x03;
    
    // Get psram io configuration
    psramIoCfg = ((devInfo.flash_cfg == 1 || devInfo.flash_cfg == 2) && devInfo.psram_cfg != 1) ? 0x3F : 0x00;
    
    // Get cache way disable setting
    cacheWayDisable = BL_GET_REG_BITS_VAL(BL_RD_REG(L1C_BASE, L1C_CONFIG), L1C_WAY_DIS);
    
    // Get EM select
    emSel = BL_RD_REG(GLB_BASE, GLB_SEAM_MISC);
    
    // Get flash configuration pointer
    flashCfgPtr = (SPI_Flash_Cfg_Type *)bl_flash_get_flashCfg();
    
    // Get flash continuous read setting
    flashContRead = flashCfgPtr->cReadSupport & 0x01;
    
    // Get flash image offset
    flashImageOffset = SF_Ctrl_Get_Flash_Image_Offset();
    
    // Get SF control configuration
    bl_pds_get_sf_ctrl_cfg(flashCfgPtr, &sfCtrlCfg);
    
    // Configure PDS interrupt
    BL_WR_REG(PDS_BASE, PDS_INT, 0x1<<16);
    
    // Overwrite default soft start delay (default 0, which may cause wakeup failure)
    AON_Set_LDO11_SOC_Sstart_Delay(2);
    
    // Select 32K (RC32K and XTAL32K are both default on)
#ifdef CFG_USE_XTAL32K
    HBN_32K_Sel(HBN_32K_XTAL);
    //HBN_Power_Off_RC32K();
#else
    HBN_32K_Sel(HBN_32K_RC);
    //HBN_Power_Off_Xtal_32K();
#endif
    
    // Disable HBN pin pull up/down to reduce PDS/HBN current
    HBN_Hw_Pu_Pd_Cfg(DISABLE);
    
    // Disable HBN pin IE/SMT
    HBN_Aon_Pad_IeSmt_Cfg(0);
    
    // Disable HBN pin wakeup
    HBN_Pin_WakeUp_Mask(0x1F);
    
    // Set fast boot entry
    HBN_Set_Wakeup_Addr((uint32_t)bl_pds_fastboot_entry);
}

void bl_pds_gpio_wakeup_cfg(uint8_t pin_list[], uint8_t pin_num)
{
    uint8_t hbnWakeupPin = 0;
    int8_t pdsWakeupPin = -1;
    int pin;
    int i;
    
    for(i = 0; i < pin_num; i++){
        pin = pin_list[i];
        
        if(pin >= 9 && pin <= 12){
            hbnWakeupPin |= HBN_WAKEUP_GPIO_9 << (pin - 9);
        }
        
        if(pin >= 0 && pin <= 7){
            pdsWakeupPin = pin;
        }
    }
    
    if(hbnWakeupPin == 0){
        HBN_Aon_Pad_IeSmt_Cfg(0);
        HBN_Pin_WakeUp_Mask(0x1F);
        PDS_IntEn(PDS_INT_HBN_IRQ_OUT0, DISABLE);
    }else{
        HBN_Aon_Pad_IeSmt_Cfg(hbnWakeupPin);
        HBN_Pin_WakeUp_Mask(~hbnWakeupPin & 0x1F);
        HBN_GPIO_INT_Enable(HBN_GPIO_INT_TRIGGER_ASYNC_FALLING_EDGE);
        PDS_IntEn(PDS_INT_HBN_IRQ_OUT0, ENABLE);
    }
    
    if(pdsWakeupPin == -1){
        PDS_Set_Vddcore_GPIO_IntMask(MASK);
        PDS_IntEn(PDS_INT_GPIO_IRQ, DISABLE);
    }else{
        PDS_Set_Vddcore_GPIO_IntCfg(pdsWakeupPin, PDS_AON_GPIO_INT_TRIGGER_SYNC_FALLING_EDGE);
        PDS_Set_Vddcore_GPIO_IntMask(UNMASK);
        PDS_IntEn(PDS_INT_GPIO_IRQ, ENABLE);
    }
}

void bl_pds_gpio_wakeup_cfg_ex(uint32_t bitmap)
{
    GLB_GPIO_Cfg_Type gpioCfg;
    int pin;
    
    for(pin = 0; pin <= 31; pin++){
        if(bitmap & (1 << pin)){
            gpioCfg.gpioPin = pin;
            gpioCfg.gpioFun = 11;
            gpioCfg.gpioMode = GPIO_MODE_INPUT;
            gpioCfg.pullType = GPIO_PULL_NONE;
            gpioCfg.drive = 0;
            gpioCfg.smtCtrl = 1;
            GLB_GPIO_Init(&gpioCfg);
            
            GLB_Set_GPIO_IntMod(pin, GLB_GPIO_INT_CONTROL_ASYNC, GLB_GPIO_INT_TRIG_NEG_PULSE);
            GLB_GPIO_IntClear(pin, SET);
            GLB_GPIO_IntMask(pin, UNMASK);
        }else{
            GLB_GPIO_IntMask(pin, MASK);
        }
    }
    
    HBN_Aon_Pad_IeSmt_Cfg((bitmap >> 9) & 0x1F);
    
    if(bitmap == 0){
        PDS_IntEn(PDS_INT_GPIO_IRQ, DISABLE);
    }else{
        PDS_IntEn(PDS_INT_GPIO_IRQ, ENABLE);
    }
}

void bl_pds_fastboot_cfg(uint32_t addr)
{
    HBN_Set_Wakeup_Addr(addr);
    HBN_Set_Status_Flag(HBN_STATUS_ENTER_FLAG);
}

ATTR_PDS_SECTION
int bl_pds_pre_process(uint32_t pdsLevel, uint32_t pdsSleepCycles, uint32_t *store, uint32_t arg[])
{
    if(pdsLevel == 31){
        pdsCfgPtr = &pdsCfgLevel31;
#if !defined(CFG_PDS_OPTIMIZE)
    }else if(pdsLevel == 0){
        pdsCfgPtr = &pdsCfgLevel0;
    }else if(pdsLevel == 1){
        pdsCfgPtr = &pdsCfgLevel1;
    }else if(pdsLevel == 2){
        pdsCfgPtr = &pdsCfgLevel2;
    }else if(pdsLevel == 3){
        pdsCfgPtr = &pdsCfgLevel3;
    }else if(pdsLevel == 4){
        pdsCfgPtr = &pdsCfgLevel4;
    }else if(pdsLevel == 5){
        pdsCfgPtr = &pdsCfgLevel5;
    }else if(pdsLevel == 6){
        pdsCfgPtr = &pdsCfgLevel6;
    }else if(pdsLevel == 7){
        pdsCfgPtr = &pdsCfgLevel7;
#endif
    }else{
        return -1;
    }
    
    if(devInfo.flash_cfg == 1 || devInfo.flash_cfg == 2){
        pdsCfgPtr->pdsCtl.puFlash = 1;    // Power down internal flash in pds31
        pdsCfgPtr->pdsCtl.swPuFlash = 1;  // Don't power down internal flash in pds0 - pds7
    }
    
    __disable_irq();
    
    if(pdsLevel >= 4){
        BL_WR_REG(HBN_BASE, HBN_RSV0, HBN_STATUS_ENTER_FLAG);
        patchBootrom = *(uint32_t *)0x4202DFF4;
        *store = !cpuRegStored;
    }else{
        *store = 0;
    }
    
    return 0;
}

ATTR_PDS_SECTION
int bl_pds_start(uint32_t pdsLevel, uint32_t pdsSleepCycles)
{
    // Power down flash
    flash_powerdown();
    
    // Pull up flash pads
    if(devInfo.flash_cfg == 0){
        // External SF2 (GPIO23 - GPIO28)
        *(volatile uint32_t *)0x4000E030 = (0x1F << 24);  // As boot pin, GPIO28 should not be pulled up
    }else if(devInfo.flash_cfg == 3){
        // External SF1 (GPIO17 - GPIO22)
        *(volatile uint32_t *)0x4000E030 = (0x3F << 8);
    }else{
        // Internal SF2 (GPIO23 - GPIO28)
        // Do nothing
    }
    
    // Select RC32M
    RomDriver_GLB_Set_System_CLK(GLB_DLL_XTAL_NONE, GLB_SYS_CLK_RC32M);
    RomDriver_GLB_Power_Off_DLL();
    RomDriver_AON_Power_Off_XTAL();
    
    // Clear HBN_IRQ status
    BL_WR_REG(HBN_BASE, HBN_IRQ_CLR, 0xFFFFFFFF);
    
    // Clear PDS_GPIO status
    bl_pds_gpio_clear_int_status();
    
    // Enter PDS mode
    PDS_Default_Level_Config(pdsCfgPtr, pdsSleepCycles);
    __WFI();
    
    return 0;
}

ATTR_PDS_SECTION
int bl_pds_post_process(uint32_t pdsLevel, uint32_t pdsSleepCycles, uint32_t reset, uint32_t arg[])
{
    __enable_irq();
    
    return 0;
}

ATTR_PDS_SECTION
void bl_pds_enter_do(uint32_t pdsLevel, uint32_t pdsSleepCycles, uint32_t store, uint32_t *reset)
{
    if(store){
        *(uint32_t *)bl_pds_bak_addr = (uint32_t)__builtin_return_address(0);
    }
    
    bl_pds_start(pdsLevel, pdsSleepCycles);
    
    *reset = 0;
}

ATTR_PDS_SECTION
int bl_pds_enter(uint32_t pdsLevel, uint32_t pdsSleepCycles)
{
    uint32_t store = 1;  // set whether cpu registers need store
    uint32_t reset = 1;  // get whether cpu is reset
    uint32_t arg[4];
    int status;
    
    // Pre-process
    status = bl_pds_pre_process(pdsLevel, pdsSleepCycles, &store, arg);
    if(status != 0){
        return status;
    }
    
    // Store cpu registers except for ra
    if(store){
        __asm__ __volatile__(
            "csrr   a1,     mtvec\n\t"
            "csrr   a2,     mstatus\n\t"
            "lw     a0,     bl_pds_bak_addr\n\t"
            "sw     sp,     1*4(a0)\n\t"
            "sw     tp,     2*4(a0)\n\t"
            "sw     t0,     3*4(a0)\n\t"
            "sw     t1,     4*4(a0)\n\t"
            "sw     t2,     5*4(a0)\n\t"
            "sw     fp,     6*4(a0)\n\t"
            "sw     s1,     7*4(a0)\n\t"
            "sw     a1,     8*4(a0)\n\t"
            "sw     a2,     9*4(a0)\n\t"
            "sw     a3,     10*4(a0)\n\t"
            "sw     a4,     11*4(a0)\n\t"
            "sw     a5,     12*4(a0)\n\t"
            "sw     a6,     13*4(a0)\n\t"
            "sw     a7,     14*4(a0)\n\t"
            "sw     s2,     15*4(a0)\n\t"
            "sw     s3,     16*4(a0)\n\t"
            "sw     s4,     17*4(a0)\n\t"
            "sw     s5,     18*4(a0)\n\t"
            "sw     s6,     19*4(a0)\n\t"
            "sw     s7,     20*4(a0)\n\t"
            "sw     s8,     21*4(a0)\n\t"
            "sw     s9,     22*4(a0)\n\t"
            "sw     s10,    23*4(a0)\n\t"
            "sw     s11,    24*4(a0)\n\t"
            "sw     t3,     25*4(a0)\n\t"
            "sw     t4,     26*4(a0)\n\t"
            "sw     t5,     27*4(a0)\n\t"
            "sw     t6,     28*4(a0)\n\t"
        );
    }
    
    // Enter PDS
    bl_pds_enter_do(pdsLevel, pdsSleepCycles, store, &reset);
    
    // Post-process
    status = bl_pds_post_process(pdsLevel, pdsSleepCycles, reset, arg);
    if(status != 0){
        return status;
    }
    
    return 0;
}

int bl_pds_get_wakeup_source(void)
{
    uint8_t wakeupEvent = BL_GET_REG_BITS_VAL(BL_RD_REG(PDS_BASE, PDS_INT), PDS_RO_PDS_WAKEUP_EVENT);
    
    if(wakeupEvent & 0x01){
        return PDS_WAKEUP_BY_SLEEP_CNT;
    }else if(wakeupEvent & 0x0A){
        return PDS_WAKEUP_BY_GPIO;
    }else{
        return 0;
    }
}

uint32_t bl_pds_get_wakeup_gpio(void)
{
    uint8_t wakeupEvent = BL_GET_REG_BITS_VAL(BL_RD_REG(PDS_BASE, PDS_INT), PDS_RO_PDS_WAKEUP_EVENT);
    uint32_t wakeupPin = 0;
    
    if(wakeupEvent & 0x02){
        wakeupPin |= (hbnIrqStatus & 0x1F) << 9;
    }
    
    if(wakeupEvent & 0x08){
        if(BL_GET_REG_BITS_VAL(BL_RD_REG(PDS_BASE, PDS_GPIO_INT), PDS_GPIO_INT_STAT)){
            wakeupPin |= 1 << BL_GET_REG_BITS_VAL(BL_RD_REG(PDS_BASE, PDS_GPIO_INT), PDS_GPIO_INT_SELECT);
        }
    }
    
    return wakeupPin;
}

ATTR_PDS_SECTION
__attribute__((weak)) void bl_pds_fastboot_done_callback(void)
{
    
}

ATTR_PDS_SECTION
void flash_powerdown(void)
{
    if(flashPowerdown){
        return;
    }else{
        flashPowerdown = 1;
    }
    
    RomDriver_SF_Ctrl_Set_Owner(SF_CTRL_OWNER_SAHB);
    RomDriver_SFlash_Reset_Continue_Read(flashCfgPtr);
    RomDriver_SFlash_Powerdown();
    RomDriver_GLB_Set_SF_CLK(0, 1, 0);
}

ATTR_PDS_SECTION
void flash_restore(void)
{
    if(!flashPowerdown){
        return;
    }else{
        flashPowerdown = 0;
    }
    
    RomDriver_GLB_Set_SF_CLK(1, 1, 0);
    
    if(RomDriver_SF_Ctrl_Get_Flash_Image_Offset() == 0){
        bl_pds_restore_sf();
        bl_pds_restore_flash(&sfCtrlCfg, flashCfgPtr, flashImageOffset, flashContRead, cacheWayDisable);
    }else{
        RomDriver_SF_Ctrl_Set_Owner(SF_CTRL_OWNER_SAHB);
        RomDriver_SFlash_Restore_From_Powerdown(flashCfgPtr, flashContRead);
    }
}
