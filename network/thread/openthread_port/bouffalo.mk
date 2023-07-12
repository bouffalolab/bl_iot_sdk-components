# Component Makefile
#

## These include paths would be exported to project level
COMPONENT_ADD_INCLUDEDIRS += ../openthread/include ../openthread/src/core ../openthread/src ../openthread/examples/platforms

## not be exported to project level
COMPONENT_PRIV_INCLUDEDIRS :=

ifeq ($(CONFIG_PDS_ENABLE),1)
CONFIG_PDS_LEVEL ?= 31
CPPFLAGS += -DCFG_PDS_LEVEL=$(CONFIG_PDS_LEVEL)
CPPFLAGS += -DCFG_PDS_ENABLE
CPPFLAGS += -DCFG_DATA_POLL_CSMA=${CONFIG_DATA_POLL_CSMA}
endif

## This component's src 
COMPONENT_SRCS := ot_alarm.c \
                  ot_diag.c \
                  ot_entropy.c \
                  ot_settings.c \
                  ot_logging.c \
                  ot_misc.c \
                  ot_radio.c \
                  ot_uart.c \
                  ot_freertos.c \
                  ot_linkmetric.c

ifndef CONFIG_NCP
COMPONENT_SRCS += ot_extern.cpp
endif

COMPONENT_OBJS := $(patsubst %.cpp,%.o, $(filter %.cpp,$(COMPONENT_SRCS))) $(patsubst %.c,%.o, $(filter %.c,$(COMPONENT_SRCS))) $(patsubst %.S,%.o, $(filter %.S,$(COMPONENT_SRCS)))
COMPONENT_SRCDIRS := .

ifeq ($(CONFIG_SYS_AOS_LOOP_ENABLE),1)
CPPFLAGS += -DSYS_AOS_LOOP_ENABLE
endif

ifeq ($(CONFIG_SYS_AOS_CLI_ENABLE),1)
CPPFLAGS += -DSYS_AOS_CLI_ENABLE

ifdef CONFIG_PREFIX
CPPFLAGS += -DCFG_PREFIX=\"${CONFIG_PREFIX}\"
endif
endif

ifeq ($(CONFIG_USB_CDC),1)
CPPFLAGS += -DCFG_USB_CDC_ENABLE
endif

ifeq ($(OPENTHREAD_TESTS_UNIT),1)
CPPFLAGS += -DCFG_OPENTHREAD_TESTS_UNIT
endif

ifeq ($(CONFIG_USE_PSRAM), 1)
CPPFLAGS += -DCFG_USE_PSRAM=1
endif

ifeq ($(CONFIG_OTBR),1)
CFLAGS += -DOPENTHREAD_BORDER_ROUTER
endif

CPPFLAGS += -D$(CONFIG_CHIP_NAME)