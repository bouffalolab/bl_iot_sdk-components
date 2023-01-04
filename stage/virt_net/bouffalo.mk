# Component Makefile
#
## These include paths would be exported to project level
COMPONENT_ADD_INCLUDEDIRS += include

## not be exported to project level
COMPONENT_PRIV_INCLUDEDIRS := include

## This component's src
COMPONENT_SRCS := src/virt_net.c \
				  src/virt_net_ramsync.c \
				  test/virt_net_test.c \

COMPONENT_OBJS := $(patsubst %.c,%.o, $(COMPONENT_SRCS))

COMPONENT_SRCDIRS := src test

ifeq ($(CONFIG_USE_PSRAM), 1)
CPPFLAGS += -DCFG_USE_PSRAM
endif

