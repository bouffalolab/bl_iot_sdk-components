# Component Makefile
#
## These include paths would be exported to project level
COMPONENT_ADD_INCLUDEDIRS += bl702l_hal

## not be exported to project level
COMPONENT_PRIV_INCLUDEDIRS +=

## This component's src
COMPONENT_SRCS := bl702l_hal/bl_adc.c \
                  bl702l_hal/bl_audio.c \
                  bl702l_hal/bl_boot2.c \
                  bl702l_hal/bl_chip.c \
                  bl702l_hal/bl_dma.c \
                  bl702l_hal/bl_dma_uart.c \
                  bl702l_hal/bl_efuse.c \
                  bl702l_hal/bl_flash.c \
                  bl702l_hal/bl_gpio.c \
                  bl702l_hal/bl_gpio_uart.c \
                  bl702l_hal/bl_hbn.c \
                  bl702l_hal/bl_ir.c \
                  bl702l_hal/bl_irq.c \
                  bl702l_hal/bl_kys.c \
                  bl702l_hal/bl_pwm.c \
                  bl702l_hal/bl_rtc.c \
                  bl702l_hal/bl_sec.c \
                  bl702l_hal/bl_sec_aes.c \
                  bl702l_hal/bl_sys.c \
                  bl702l_hal/bl_timer.c \
                  bl702l_hal/bl_uart.c \
                  bl702l_hal/bl_wdt.c \
                  bl702l_hal/bl_wireless.c \
                  bl702l_hal/hal_board.c \
                  bl702l_hal/hal_boot2.c \
                  bl702l_hal/hal_button.c \
                  bl702l_hal/hal_gpio.c \
                  bl702l_hal/hal_hbn.c \
                  bl702l_hal/hal_hwtimer.c \
                  bl702l_hal/hal_ota.c \
                  bl702l_hal/hal_sys.c \
                  bl702l_hal/hal_tcal.c \
                  bl702l_hal/hosal_adc.c \
                  bl702l_hal/hosal_dma.c \
                  bl702l_hal/hosal_flash.c \
                  bl702l_hal/hosal_gpio.c \
                  bl702l_hal/hosal_i2c.c \
                  bl702l_hal/hosal_ota.c \
                  bl702l_hal/hosal_pwm.c \
                  bl702l_hal/hosal_rng.c \
                  bl702l_hal/hosal_rtc.c \
                  bl702l_hal/hosal_spi.c \
                  bl702l_hal/hosal_timer.c \
                  bl702l_hal/hosal_uart.c \
                  bl702l_hal/hosal_wdg.c \
                  sec_common/bl_sec_pka.c \
                  sec_common/bl_sec_aes.c \
                  sec_common/bl_sec_common.c \
                  sec_common/bl_sec_pka.c \
                  sec_common/bl_sec_sha.c \
                  sec_common/bl_sec_psk.c \

ifeq ($(CONFIG_BUILD_ROM_CODE),1)
COMPONENT_SRCS += bl702l_hal/bl_pds.c
COMPONENT_SRCS += bl702l_hal/hal_pds.c
endif

ifeq ($(CONFIG_USE_PSRAM),1)
COMPONENT_SRCS +=  bl702l_hal/bl_psram.c
COMPONENT_SRCS +=  bl702l_hal/bl_psram_sp.S
endif

COMPONENT_SRCDIRS += bl702l_hal sec_common

COMPONENT_OBJS := $(patsubst %.c,%.o, $(COMPONENT_SRCS))
COMPONENT_OBJS := $(patsubst %.S,%.o, $(COMPONENT_OBJS))

##
CPPFLAGS += -DARCH_RISCV -DBFLB_CRYPT_HARDWARE
ifndef CONFIG_USE_STD_DRIVER
CPPFLAGS += -DBFLB_USE_HAL_DRIVER
endif

ifeq ($(CONFIG_BLUETOOTH),1)
CPPFLAGS += -DCFG_BLE_ENABLE
endif

ifeq ($(CONFIG_BLE_MFG),1)
CPPFLAGS += -DCONFIG_BLE_MFG
endif

ifeq ($(CONFIG_ZIGBEE),1)
CPPFLAGS += -DCFG_ZIGBEE_ENABLE
endif

ifeq ($(CONFIG_THREAD),1)
CPPFLAGS += -DCFG_OPENTHREAD_ENABLE
endif

ifeq ($(CONFIG_TCAL),1)
CPPFLAGS += -DCFG_TCAL_ENABLE
endif

ifeq ($(CONFIG_PDS_ENABLE),1)
CPPFLAGS += -DCFG_PDS_ENABLE
CONFIG_PDS_LEVEL ?= 31
CPPFLAGS += -DCFG_PDS_LEVEL=$(CONFIG_PDS_LEVEL)
ifeq ($(CONFIG_PDS_LEVEL),31)
CPPFLAGS += -DCFG_PDS_OPTIMIZE
endif
CPPFLAGS += -DCONFIG_HW_SEC_ENG_DISABLE
endif

ifeq ($(CONFIG_HBN_ENABLE),1)
CPPFLAGS += -DCFG_HBN_ENABLE
CPPFLAGS += -DCFG_HBN_OPTIMIZE
endif

ifeq ($(CONFIG_USE_PSRAM),1)
CPPFLAGS += -DCFG_USE_PSRAM
endif

ifeq ($(CONFIG_USE_XTAL32K),1)
CPPFLAGS += -DCFG_USE_XTAL32K
endif
