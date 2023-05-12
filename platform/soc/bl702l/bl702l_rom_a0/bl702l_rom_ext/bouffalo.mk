COMPONENT_LIB_ONLY := 1
COMPONENT_ADD_INCLUDEDIRS += include/rom_ext include/rom_freertos include/rom_freertos/config include/rom_freertos/panic include/rom_freertos/portable/GCC/RISC-V include/rom_lmac154/include include/rom_thread_port include/rom_zb_simple
LIBS ?= bl702l_rom_ext
COMPONENT_ADD_LDFLAGS += -L$(COMPONENT_PATH)/lib $(addprefix -l,$(LIBS))
ALL_LIB_FILES := $(patsubst %,$(COMPONENT_PATH)/lib/lib%.a,$(LIBS))
COMPONENT_ADD_LINKER_DEPS := $(ALL_LIB_FILES)
