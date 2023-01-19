# Component Makefile
#
## These include paths would be exported to project level
COMPONENT_ADD_INCLUDEDIRS +=

## This component's src
COMPONENT_SRCS :=   ./ramsync_low/slave/ramsync_low.c \
                    ./ramsync_low/slave/lramsync_low_test.c \

COMPONENT_SRCDIRS := ./ramsync_low/slave 

##
#CPPFLAGS +=
