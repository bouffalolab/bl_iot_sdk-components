COMPONENT_LIB_ONLY := 1
COMPONENT_ADD_INCLUDEDIRS += ../btblecontroller/btble_inc
LIBS ?= btblecontroller_702l_m1s1p
COMPONENT_ADD_LDFLAGS += -L$(COMPONENT_PATH)/lib $(addprefix -l,$(LIBS))
ALL_LIB_FILES := $(patsubst %,$(COMPONENT_PATH)/lib/lib%.a,$(LIBS))
COMPONENT_ADD_LINKER_DEPS := $(ALL_LIB_FILES)
