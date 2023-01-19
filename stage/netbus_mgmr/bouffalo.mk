# Component Makefile
#
## These include paths would be exported to project level
COMPONENT_ADD_INCLUDEDIRS += .

## not be exported to project level
COMPONENT_PRIV_INCLUDEDIRS :=

COMPONENT_ADAPTER_SRCS := adapter/netbus_transceiver_tpsync.c

## This component's src
COMPONENT_SRCS := netbus_mgmr.c \
                  netbus_utils.c \
                  netbus_wifi_mgmr_cmd_handlers.c \
                  $(COMPONENT_ADAPTER_SRCS) \
 
COMPONENT_OBJS := $(patsubst %.c,%.o, $(COMPONENT_SRCS))
COMPONENT_OBJS := $(patsubst %.cpp,%.o, $(COMPONENT_OBJS))

COMPONENT_SRCDIRS := . ./adapter

##
#CPPFLAGS +=
