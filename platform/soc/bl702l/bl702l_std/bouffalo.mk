# Component Makefile
#
## These include paths would be exported to project level
COMPONENT_ADD_INCLUDEDIRS += BSP_Driver \
                             BSP_Driver/regs \
                             BSP_Driver/risc-v/Core/Include \
                             BSP_Driver/startup \
                             BSP_Driver/std_drv/inc \
                             BSP_Common/platform \
                             MCU_Common/device \
                             MCU_Common/list \
                             MCU_Common/memheap \
                             MCU_Common/misc \
                             MCU_Common/partition \
                             MCU_Common/ring_buffer \
                             MCU_Common/soft_crc \
                             cipher_suite/inc \

ifeq ($(CONFIG_BL702_USE_BSP_STARTUP),1)
COMPONENT_ADD_INCLUDEDIRS += BSP_Board
COMPONENT_ADD_INCLUDEDIRS += BSP_Driver/hal_drv/default_config
COMPONENT_ADD_INCLUDEDIRS += BSP_Driver/hal_drv/inc
endif


## not be exported to project level
COMPONENT_PRIV_INCLUDEDIRS := 


## This component's src 
COMPONENT_SRCS := BSP_Driver/std_drv/src/bl702l_acomp.c \
                  BSP_Driver/std_drv/src/bl702l_adc.c \
                  BSP_Driver/std_drv/src/bl702l_auadc.c \
                  BSP_Driver/std_drv/src/bl702l_cks.c \
                  BSP_Driver/std_drv/src/bl702l_dma.c \
                  BSP_Driver/std_drv/src/bl702l_i2c.c \
                  BSP_Driver/std_drv/src/bl702l_pwm.c \
                  BSP_Driver/std_drv/src/bl702l_pwm_sc.c \
                  BSP_Driver/std_drv/src/bl702l_sec_dbg.c \
                  BSP_Driver/std_drv/src/bl702l_sec_eng.c \
                  BSP_Driver/std_drv/src/bl702l_spi.c \
                  BSP_Driver/std_drv/src/bl702l_tzc_sec.c \
                  MCU_Common/ring_buffer/ring_buffer.c \
                  cipher_suite/src/bflb_crypt.c \
                  cipher_suite/src/bflb_dsa.c \
                  cipher_suite/src/bflb_ecdsa.c \
                  cipher_suite/src/bflb_hash.c \

ifeq ($(CONFIG_BL702_USE_ROM_DRIVER),1)
#COMPONENT_SRCS += BSP_Driver/std_drv/src/bl702l_romapi.c
COMPONENT_SRCS += bl702l_romapi_patch.c
else
COMPONENT_SRCS += BSP_Driver/std_drv/src/bl702l_aon.c
COMPONENT_SRCS += BSP_Driver/std_drv/src/bl702l_clock.c
COMPONENT_SRCS += BSP_Driver/std_drv/src/bl702l_common.c
COMPONENT_SRCS += BSP_Driver/std_drv/src/bl702l_ef_cfg.c
COMPONENT_SRCS += BSP_Driver/std_drv/src/bl702l_ef_ctrl.c
COMPONENT_SRCS += BSP_Driver/std_drv/src/bl702l_glb.c
COMPONENT_SRCS += BSP_Driver/std_drv/src/bl702l_glb_gpio.c
COMPONENT_SRCS += BSP_Driver/std_drv/src/bl702l_hbn.c
COMPONENT_SRCS += BSP_Driver/std_drv/src/bl702l_ir.c
COMPONENT_SRCS += BSP_Driver/std_drv/src/bl702l_kys.c
COMPONENT_SRCS += BSP_Driver/std_drv/src/bl702l_l1c.c
COMPONENT_SRCS += BSP_Driver/std_drv/src/bl702l_pds.c
COMPONENT_SRCS += BSP_Driver/std_drv/src/bl702l_psram.c
COMPONENT_SRCS += BSP_Driver/std_drv/src/bl702l_sf_cfg.c
COMPONENT_SRCS += BSP_Driver/std_drv/src/bl702l_sf_ctrl.c
COMPONENT_SRCS += BSP_Driver/std_drv/src/bl702l_sflash.c
COMPONENT_SRCS += BSP_Driver/std_drv/src/bl702l_timer.c
COMPONENT_SRCS += BSP_Driver/std_drv/src/bl702l_uart.c
COMPONENT_SRCS += BSP_Driver/std_drv/src/bl702l_xip_sflash.c
COMPONENT_SRCS += MCU_Common/misc/misc.c
COMPONENT_SRCS += MCU_Common/soft_crc/softcrc.c
endif

ifeq ($(CONFIG_BL702_USE_BSP_STARTUP),1)
COMPONENT_SRCS += BSP_Driver/startup/interrupt.c
COMPONENT_SRCS += BSP_Driver/startup/system_bl702l.c
COMPONENT_SRCS += BSP_Driver/startup/GCC/entry.S
COMPONENT_SRCS += BSP_Driver/startup/GCC/start_load.c
COMPONENT_SRCS += BSP_Common/platform/bflb_platform.c
COMPONENT_SRCS += BSP_Common/platform/syscalls.c
COMPONENT_SRCS += MCU_Common/device/drv_device.c
COMPONENT_SRCS += MCU_Common/memheap/drv_mmheap.c
COMPONENT_SRCS += MCU_Common/partition/partition.c
COMPONENT_SRCS += BSP_Board/board.c
HAL_DRV_SRCS := $(notdir $(wildcard $(BL60X_SDK_PATH)/components/platform/soc/bl702l/bl702l_std/BSP_Driver/hal_drv/src/*.c))
COMPONENT_SRCS += $(addprefix BSP_Driver/hal_drv/src/, $(HAL_DRV_SRCS))
CPPFLAGS += -Dbl706_iot
endif

ifeq ($(CONFIG_BL702_USE_LHAL),1)
COMPONENT_SRCS += port/bl702l_clock.c
COMPONENT_SRCS += port/bl702l_ef_cfg.c
endif


COMPONENT_OBJS := $(patsubst %.c,%.o, $(COMPONENT_SRCS))
COMPONENT_OBJS := $(patsubst %.S,%.o, $(COMPONENT_OBJS))


COMPONENT_SRCDIRS := BSP_Driver/startup \
                     BSP_Driver/startup/GCC \
                     BSP_Driver/std_drv/src \
                     BSP_Common/platform \
                     MCU_Common/device \
                     MCU_Common/list \
                     MCU_Common/memheap \
                     MCU_Common/misc \
                     MCU_Common/partition \
                     MCU_Common/ring_buffer \
                     MCU_Common/soft_crc \
                     cipher_suite/src \
                     . \

ifeq ($(CONFIG_BL702_USE_BSP_STARTUP),1)
COMPONENT_SRCDIRS += BSP_Board
COMPONENT_SRCDIRS += BSP_Driver/hal_drv/src
endif

ifeq ($(CONFIG_BL702_USE_LHAL),1)
COMPONENT_SRCDIRS += port
endif
