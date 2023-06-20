# Component Makefile
#
## These include paths would be exported to project level
COMPONENT_ADD_INCLUDEDIRS +=

## This component's src
COMPONENT_SRCS :=   ./ramsync_low/master/ramsync_low.c \
                    ./ramsync_low/master/lramsync_low_test.c

COMPONENT_SRCDIRS := ./ramsync_low/master/

##
#CPPFLAGS +=
