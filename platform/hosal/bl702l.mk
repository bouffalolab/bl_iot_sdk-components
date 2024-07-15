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
                  bl702l_hal/bl_gpio_ir.c \
                  bl702l_hal/bl_gpio_uart.c \
                  bl702l_hal/bl_hbn.c \
                  bl702l_hal/bl_ir.c \
                  bl702l_hal/bl_irq.c \
                  bl702l_hal/bl_kys.c \
                  bl702l_hal/bl_os_port.c \
                  bl702l_hal/bl_pwm.c \
                  bl702l_hal/bl_pwm_ir.c \
                  bl702l_hal/bl_rtc.c \
                  bl702l_hal/bl_sec.c \
                  bl702l_hal/bl_sec_aes.c \
                  bl702l_hal/bl_sys.c \
                  bl702l_hal/bl_timer.c \
                  bl702l_hal/bl_uart.c \
                  bl702l_hal/bl_wdt.c \
                  bl702l_hal/bl_wireless.c \
                  bl702l_hal/hal_anti_rollback.c \
                  bl702l_hal/hal_board.c \
                  bl702l_hal/hal_boot2.c \
                  bl702l_hal/hal_button.c \
                  bl702l_hal/hal_gpio.c \
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
