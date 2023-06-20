# Component Makefile
#
## These include paths would be exported to project level

SOC_DRV = $(shell echo $(CONFIG_CHIP_NAME) | tr A-Z a-z)
HAL_DRV_HAS_CONFIG := 1
include $(COMPONENT_PATH)/$(SOC_DRV).mk

COMPONENT_ADD_INCLUDEDIRS += ramsync_upper desc_buf ramsync_low blmem

## not be exported to project level
COMPONENT_PRIV_INCLUDEDIRS += dbg test 

## This component's src
COMPONENT_SRCS +=   dbg/tpdbg.c \
                    desc_buf/desc_buf.c \
                    ramsync_upper/ramsync_upper.c \
                    test/ramsync_test.c \
                    ./blmem/blmalloc.c \


COMPONENT_OBJS := $(patsubst %.c,%.o, $(COMPONENT_SRCS))

COMPONENT_SRCDIRS += dbg desc_buf ramsync_low ramsync_upper test blmem

ifeq ($(CONFIG_USE_DTS_SPI_CONFIG), 1)
CPPFLAGS += -DCFG_USE_DTS_SPI_CONFIG
endif

CPPFLAGS += -D${CONFIG_CHIP_NAME}

##
#CPPFLAGS +=

