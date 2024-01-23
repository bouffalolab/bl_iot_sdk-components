# Component Makefile
#
## These include paths would be exported to project level
COMPONENT_ADD_INCLUDEDIRS += easyflash_port	\
														 littlefs	\
														 port

## not be exported to project level
COMPONENT_PRIV_INCLUDEDIRS :=

## This component's src
COMPONENT_SRCS := easyflash_port/lfs_easyflash.c \
									easyflash_port/easyflash_cli.c	\
									littlefs/lfs.c	\
									littlefs/lfs_util.c	\
									port/lfs_xip_flash.c

COMPONENT_OBJS := $(patsubst %.c,%.o, $(COMPONENT_SRCS))

COMPONENT_SRCDIRS := littlefs easyflash_port port
CPPFLAGS += -DLFS_THREADSAFE -DCONFIG_FREERTOS

#CPPFLAGS +=
