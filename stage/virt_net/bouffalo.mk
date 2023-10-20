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

ifdef CONFIG_VIRT_NET_STACK_DEPTH
CPPFLAGS += -DCFG_VIRT_NET_STACK_DEPTH=$(CONFIG_VIRT_NET_STACK_DEPTH)
endif

ifeq ($(CONFIG_IPV4), 1)
CFLAGS   += -DLWIP_IPV4=1
CPPFLAGS += -DLWIP_IPV4=1
CFLAGS   += -DLWIP_DHCP=1
CPPFLAGS += -DLWIP_DHCP=1
endif

ifeq ($(CONFIG_IPV6), 1)
CFLAGS   += -DLWIP_IPV6=1
CPPFLAGS += -DLWIP_IPV6=1
CFLAGS   += -DLWIP_IPV6_DHCP6=1
CPPFLAGS += -DLWIP_IPV6_DHCP6=1
endif
