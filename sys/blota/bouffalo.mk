# Component Makefile
#
## These include paths would be exported to project level
COMPONENT_ADD_INCLUDEDIRS += include

## not be exported to project level
COMPONENT_PRIV_INCLUDEDIRS :=

## This component's src
COMPONENT_SRCS := bl_sys_ota.c \
			      bl_sys_ota_cli.c \

COMPONENT_OBJS := $(patsubst %.c,%.o, $(COMPONENT_SRCS))

COMPONENT_SRCDIRS := .

##
#CPPFLAGS +=
ifneq ($(CONFIG_CLI_CMD_ENABLE),0)
CPPFLAGS += -DCONFIG_CLI_CMD_ENABLE
endif
