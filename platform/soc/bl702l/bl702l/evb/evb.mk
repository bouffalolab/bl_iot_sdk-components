toolchains := gcc

# Component Makefile
#
## These include paths would be exported to project level
COMPONENT_ADD_INCLUDEDIRS += 
							 
## not be exported to project level
COMPONENT_PRIV_INCLUDEDIRS :=							 

## This component's src 
COMPONENT_SRCS1 := evb/src/boot/$(toolchains)/entry.S \
				  evb/src/boot/$(toolchains)/start.S \
				  evb/src/debug.c \
				  evb/src/sscanf.c \
				  evb/src/vsscanf.c \
				  evb/src/strntoumax.c

COMPONENT_SRCS1 += evb/src/vprint.c

COMPONENT_OBJS := $(patsubst %.c,%.o, $(COMPONENT_SRCS1))
COMPONENT_OBJS := $(patsubst %.S,%.o, $(COMPONENT_OBJS))
COMPONENT_SRCS := $(COMPONENT_SRCS1)
COMPONENT_SRCDIRS := evb/src/boot/$(toolchains) evb/src

CONFIG_CHIP_REVISION ?= A0

ifneq ($(CONFIG_LINK_CUSTOMER),1)
ifeq ($(CONFIG_BUILD_ROM_CODE),1)
    ifeq ($(CONFIG_LINK_RAM),1)
        LINKER_SCRIPTS := ram.ld
    else ifeq ($(CONFIG_USE_PSRAM),1)
        LINKER_SCRIPTS := psram_flash.ld
    else ifeq ($(CONFIG_PDS_ENABLE),1)
        LINKER_SCRIPTS := flash_lp.ld
    else
        LINKER_SCRIPTS := flash.ld
    endif

    $(info use $(LINKER_SCRIPTS))
    COMPONENT_ADD_LDFLAGS += -L $(COMPONENT_PATH)/evb/ld -T $(LINKER_SCRIPTS)
    COMPONENT_ADD_LINKER_DEPS := evb/ld/$(LINKER_SCRIPTS)
else
    ifeq ($(CONFIG_LINK_RAM),1)
        LINKER_SCRIPTS := ram_rom.ld
    else ifeq ($(CONFIG_USE_PSRAM),1)
        LINKER_SCRIPTS := psram_flash_rom.ld
    else ifeq ($(CONFIG_PDS_ENABLE),1)
        ifeq ($(CONFIG_BUILD_FREERTOS),1)
            LINKER_SCRIPTS := flash_rom_lp_freertos.ld
        else
            LINKER_SCRIPTS := flash_rom_lp.ld
        endif
    else
        ifeq ($(CONFIG_BUILD_FREERTOS),1)
            LINKER_SCRIPTS := flash_rom_freertos.ld
        else
            LINKER_SCRIPTS := flash_rom.ld
        endif
    endif

    $(info use $(CONFIG_CHIP_REVISION)/$(LINKER_SCRIPTS))
    COMPONENT_ADD_LDFLAGS += -L $(COMPONENT_PATH)/evb/ld/$(CONFIG_CHIP_REVISION) -T $(LINKER_SCRIPTS)
    COMPONENT_ADD_LINKER_DEPS := evb/ld/$(CONFIG_CHIP_REVISION)/$(LINKER_SCRIPTS)
endif
endif

ifeq ($(CONFIG_DISABLE_PRINT),1)
CPPFLAGS += -DDISABLE_PRINT
endif

ifeq ($(CONFIG_CPP_ENABLE), 1)
CFLAGS   += -DCFG_CPP_ENABLE=1
CPPFLAGS += -DCFG_CPP_ENABLE=1
endif

ifeq ($(CONFIG_LINK_RAM),1)
CPPFLAGS += -DRUN_IN_RAM
endif

##
#CPPFLAGS += -D
