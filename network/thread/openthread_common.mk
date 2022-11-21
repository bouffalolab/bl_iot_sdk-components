ifdef DISABLE_PRINT
CPPFLAGS += -DOPENTHREAD_CONFIG_LOG_OUTPUT=OPENTHREAD_CONFIG_LOG_OUTPUT_NONE
endif

CPPFLAGS += -D__int64_t_defined=1

CPPFLAGS += -DPACKAGE_NAME=\"OPENTHREAD\"
CPPFLAGS += -DPACKAGE_VERSION=\"2ce3d3bf0\"
CPPFLAGS += -DMBEDTLS_CONFIG_FILE=\"mbedtls-config.h\"

CPPFLAGS += -DOPENTHREAD_CONFIG_MAC_SOFTWARE_ACK_TIMEOUT_ENABLE=0
CPPFLAGS += -DOPENTHREAD_CONFIG_MAC_SOFTWARE_RETRANSMIT_ENABLE=1
CPPFLAGS += -DOPENTHREAD_CONFIG_MAC_SOFTWARE_CSMA_BACKOFF_ENABLE=0
CPPFLAGS += -DOPENTHREAD_CONFIG_MAC_SOFTWARE_TX_SECURITY_ENABLE=1
CPPFLAGS += -DOPENTHREAD_CONFIG_MAC_SOFTWARE_TX_TIMING_ENABLE=1
CPPFLAGS += -DOPENTHREAD_CONFIG_MAC_SOFTWARE_ENERGY_SCAN_ENABLE=1

CPPFLAGS += -DOT_CLI_UART_LOCK_HDR_FILE=\"openthread_port.h\"

CPPFLAGS += -DOPENTHREAD_EXAMPLES_SIMULATION=0

CPPFLAGS += -DOPENTHREAD_CONFIG_IP6_SLAAC_ENABLE=1

CPPFLAGS += -DOPENTHREAD_CONFIG_THREAD_VERSION=3

CPPFLAGS += -DOPENTHREAD_CONFIG_PLATFORM_USEC_TIMER_ENABLE=1

CPPFLAGS += -DOPENTHREAD_CONFIG_ECDSA_ENABLE=1

CPPFLAGS += -DOPENTHREAD_CONFIG_SRP_CLIENT_ENABLE=1

CPPFLAGS += -DOPENTHREAD_CONFIG_SRP_CLIENT_AUTO_START_API_ENABLE=1

CPPFLAGS += -DOT_FREERTOS_ENABLE=1

CPPFLAGS += -DOPENTHREAD_ENABLE_NCP_VENDOR_HOOK=0

CPPFLAGS += -DOPENTHREAD_CONFIG_LEGACY_ENABLE=0

CPPFLAGS += -DOPENTHREAD_PLATFORM_POSIX=0

CPPFLAGS += -DOPENTHREAD_POSIX=0

CPPFLAGS += -DOPENTHREAD_ENABLE_VENDOR_EXTENSION=0

CPPFLAGS += -Wimplicit-fallthrough=0 -Wno-switch-default
