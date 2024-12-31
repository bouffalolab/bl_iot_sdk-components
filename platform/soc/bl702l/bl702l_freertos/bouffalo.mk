
# Component Makefile
#
COMPONENT_ADD_INCLUDEDIRS += config portable/GCC/RISC-V panic
ifeq ($(CONFIG_USE_SW_FP),1)
COMPONENT_ADD_INCLUDEDIRS += portable/GCC/RISC-V/chip_specific_extensions/RV32I_CLINT_no_extensions
else
COMPONENT_ADD_INCLUDEDIRS += portable/GCC/RISC-V/chip_specific_extensions/RV32F_float_abi_single
endif

COMPONENT_SRCS += event_groups.c \
                  list.c \
                  queue.c \
                  stream_buffer.c \
                  tasks.c \
                  timers.c \
                  misaligned/misaligned_ldst.c \
                  misaligned/fp_asm.S \
                  panic/panic_c.c \
                  portable/GCC/RISC-V/port.c \
                  portable/GCC/RISC-V/portASM.S \

ifeq ($(CONFIG_USE_TLSF),1)
COMPONENT_SRCS += portable/MemMang/tlsf.c
COMPONENT_SRCS += portable/MemMang/heap_tlsf.c
COMPONENT_SRCS += portable/MemMang/heap_tlsf_psram.c
else
COMPONENT_SRCS += portable/MemMang/heap_5.c
COMPONENT_SRCS += portable/MemMang/heap_5_psram.c
endif

COMPONENT_OBJS := $(patsubst %.c,%.o, $(COMPONENT_SRCS))
COMPONENT_OBJS := $(patsubst %.S,%.o, $(COMPONENT_OBJS))

COMPONENT_SRCDIRS := . portable portable/GCC/RISC-V portable/MemMang misaligned panic

OPT_FLAG_G := $(findstring -Og, $(CFLAGS))
ifeq ($(strip $(OPT_FLAG_G)),-Og)
CFLAGS := $(patsubst -Og,-O2,$(CFLAGS))
endif
OPT_FLAG_S := $(findstring -Os, $(CFLAGS))
ifeq ($(strip $(OPT_FLAG_S)),-Os)
CFLAGS := $(patsubst -Os,-O2,$(CFLAGS))
endif

ASMFLAGS += -DportasmHANDLE_INTERRUPT=interrupt_entry
