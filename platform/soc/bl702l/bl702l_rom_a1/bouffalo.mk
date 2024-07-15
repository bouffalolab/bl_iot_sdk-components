COMPONENT_LIB_ONLY := 1
COMPONENT_ADD_INCLUDEDIRS += bl702l_rom_ext/include/rom_ext bl702l_rom_ext/include/rom_lmac154/include bl702l_rom_ext/include/rom_thread_port bl702l_rom_ext/include/rom_zb_simple bl702l_rom_ext/include/rom_freertos bl702l_rom_ext/include/rom_freertos/config bl702l_rom_ext/include/rom_freertos/panic bl702l_rom_ext/include/rom_freertos/portable/GCC/RISC-V
LIBS ?= bl702l_rom_a1
COMPONENT_ADD_LDFLAGS += -L$(COMPONENT_PATH)/lib $(addprefix -l,$(LIBS))
ALL_LIB_FILES := $(patsubst %,$(COMPONENT_PATH)/lib/lib%.a,$(LIBS))
COMPONENT_ADD_LINKER_DEPS := $(ALL_LIB_FILES)
