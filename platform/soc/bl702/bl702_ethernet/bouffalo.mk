
# Component Makefile
## These include paths would be exported to project level
COMPONENT_ADD_INCLUDEDIRS += Inc

## not be exported to project level
COMPONENT_PRIV_INCLUDEDIRS := 

## This component's src 
COMPONENT_SRCS := Src/eth_bd.c \
                  Src/eth_phy.c \
                  Src/eth_phy_port.c \
                  Src/ephy_general.c \
                  Src/ephy_lan8720.c \

COMPONENT_OBJS := $(patsubst %.c,%.o, $(COMPONENT_SRCS))
COMPONENT_SRCDIRS := Src
