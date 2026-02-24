# Component Makefile
#
## These include paths would be exported to project level
COMPONENT_ADD_INCLUDEDIRS += bflb_adapter/include \
							 bflb_adapter/include/bflb_os_adapter \
							 bflb_adapter \
							 wifi_hosal/include \

## not be exported to project level
COMPONENT_PRIV_INCLUDEDIRS :=

## This component's src
COMPONENT_SRCS := bflb_adapter/cli_al.c bflb_adapter/plat_iot_sdk.c wifi_hosal/wifi_hosal.c

ifeq ($(CONFIG_CHIP_NAME),BL808)
COMPONENT_SRCS += wifi_hosal/port/wifi_hosal_bl808.c
endif

ifeq ($(CONFIG_CHIP_NAME),BL606P)
COMPONENT_SRCS += wifi_hosal/port/wifi_hosal_bl606p.c
endif

ifeq ($(CONFIG_CHIP_NAME),BL602)
COMPONENT_SRCS += wifi_hosal/port/wifi_hosal_bl602.c
endif

COMPONENT_OBJS := $(patsubst %.c,%.o, $(COMPONENT_SRCS))

COMPONENT_SRCDIRS := bflb_adapter wifi_hosal wifi_hosal/port

CFLAGS   += -DCFG_FREERTOS -DBL_IOT_SDK
##
#CPPFLAGS +=
