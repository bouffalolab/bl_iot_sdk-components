
CPPFLAGS += -DOPENTHREAD_PROJECT_CORE_CONFIG_FILE=\"${CONFIG_OPENTHREAD_FILE}\"
CPPFLAGS += -DOPENTHREAD_CORE_CONFIG_PLATFORM_CHECK_FILE=\"${CONFIG_OPENTHREAD_CHECK_FILE}\"

# Default undefined values to 0
CONFIG_OT_NCP ?= 0
CONFIG_OT_RCP ?= 0
CONFIG_OT_FTD  ?= 0
CONFIG_OT_MTD  ?= 0
CONFIG_OT_CLI  ?=

# Validation: CONFIG_OT_CLI must be empty, "shell", or "uart"
_ot_cli_val_err := $(shell echo $$([ -z "$(CONFIG_OT_CLI)" ] || [ "$(CONFIG_OT_CLI)" = "shell" ] || [ "$(CONFIG_OT_CLI)" = "uart" ] && echo 1 || echo 0))
ifeq (0,$(_ot_cli_val_err))
    $(error CONFIG_OT_CLI must be empty, "shell", or "uart")
endif

# Validation: values must be 0 or 1
_ot_val_err := $(shell echo $$(($(CONFIG_OT_NCP) != 0 && $(CONFIG_OT_NCP) != 1 ? 1 : $(CONFIG_OT_RCP) != 0 && $(CONFIG_OT_RCP) != 1 ? 1 : $(CONFIG_OT_FTD) != 0 && $(CONFIG_OT_FTD) != 1 ? 1 : $(CONFIG_OT_MTD) != 0 && $(CONFIG_OT_MTD) != 1 ? 1 : 0)))
ifeq (1,$(_ot_val_err))
    $(error CONFIG_OT_NCP/CONFIG_OT_RCP/CONFIG_OT_FTD/CONFIG_OT_MTD must be 0 or 1)
endif

# Validation: CONFIG_OT_NCP and CONFIG_OT_RCP cannot both be enabled
ifeq (2,$(shell echo $$(($(CONFIG_OT_NCP) + $(CONFIG_OT_RCP)))))
    $(error CONFIG_OT_NCP and CONFIG_OT_RCP cannot both be enabled)
endif

# Validation: CONFIG_OT_FTD and CONFIG_OT_MTD cannot both be enabled
ifeq (2,$(shell echo $$(($(CONFIG_OT_FTD) + $(CONFIG_OT_MTD)))))
    $(error CONFIG_OT_FTD and CONFIG_OT_MTD cannot both be enabled)
endif

# Validation: CONFIG_OT_RCP and CONFIG_OT_FTD cannot both be enabled
ifeq (2,$(shell echo $$(($(CONFIG_OT_RCP) + $(CONFIG_OT_FTD)))))
    $(error CONFIG_OT_RCP and CONFIG_OT_FTD cannot both be enabled)
endif

# Validation: CONFIG_OT_RCP and CONFIG_OT_MTD cannot both be enabled
ifeq (2,$(shell echo $$(($(CONFIG_OT_RCP) + $(CONFIG_OT_MTD)))))
    $(error CONFIG_OT_RCP and CONFIG_OT_MTD cannot both be enabled)
endif

# Validation: CONFIG_OT_NCP and CONFIG_OT_CLI cannot both be enabled
ifeq (1,$(CONFIG_OT_NCP))
ifneq ($(CONFIG_OT_CLI),)
    $(error CONFIG_OT_NCP and CONFIG_OT_CLI cannot both be enabled)
endif
endif

# Validation: CONFIG_OT_RCP and CONFIG_OT_CLI cannot both be enabled
ifeq (1,$(CONFIG_OT_RCP))
ifneq ($(CONFIG_OT_CLI),)
    $(error CONFIG_OT_RCP and CONFIG_OT_CLI cannot both be enabled)
endif
endif

# Direct pass-through of config values
CPPFLAGS += -DOPENTHREAD_RADIO=$(CONFIG_OT_RCP)
CPPFLAGS += -DCONFIG_OT_NCP=$(CONFIG_OT_NCP)
CPPFLAGS += -DCONFIG_OT_RCP=$(CONFIG_OT_RCP)
CPPFLAGS += -DCONFIG_OT_FTD=$(CONFIG_OT_FTD)
CPPFLAGS += -DCONFIG_OT_MTD=$(CONFIG_OT_MTD)
CPPFLAGS += -DOPENTHREAD_FTD=$(CONFIG_OT_FTD)
CPPFLAGS += -DOPENTHREAD_MTD=$(CONFIG_OT_MTD)
CPPFLAGS += -DCONFIG_OT_CLI=$(CONFIG_OT_CLI)
CPPFLAGS += -DOPENTHREAD_PLATFORM_NEXUS=0
CPPFLAGS += -DCONFIG_NXSPI_OPENTHREAD_RADIO=0
