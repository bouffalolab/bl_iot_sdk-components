COMPONENT_LIB_ONLY := 1
COMPONENT_ADD_INCLUDEDIRS += ../openthread/include ../openthread/src/core ../openthread/src ../openthread/examples/platforms ../openthread_port/include ../openthread_br/include ../openthread_br
LIBS ?= openthread_br
COMPONENT_ADD_LDFLAGS += -L$(COMPONENT_PATH)/lib $(addprefix -l,$(LIBS))
ALL_LIB_FILES := $(patsubst %,$(COMPONENT_PATH)/lib/lib%.a,$(LIBS))
COMPONENT_ADD_LINKER_DEPS := $(ALL_LIB_FILES)
