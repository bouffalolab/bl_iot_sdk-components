
include $(COMPONENT_PATH)/openthread_common.mk

COMPONENT_ADD_INCLUDEDIRS := $(COMPONENT_ADD_INCLUDEDIRS) ./openthread/include ./openthread/src/include ./openthread/src
COMPONENT_PRIV_INCLUDEDIRS := ${COMPONENT_PRIV_INCLUDEDIRS}

# openthread/src/core
otlib := openthread/src/core
otlib_module_srcdir := $(otlib)

ifeq ($(CONFIG_OT_RCP),1)
otlib_module_src := \
    api/diags_api.cpp  \
    api/error_api.cpp  \
    api/instance_api.cpp  \
    api/link_raw_api.cpp  \
    api/logging_api.cpp  \
    api/random_noncrypto_api.cpp  \
    api/tasklet_api.cpp  \
    common/binary_search.cpp  \
    common/binary_search.hpp  \
    common/error.hpp  \
    common/frame_builder.cpp  \
    common/frame_builder.hpp  \
    common/log.cpp  \
    common/random.cpp  \
    common/string.cpp  \
    common/tasklet.cpp  \
    common/timer.cpp  \
    common/uptime.cpp  \
    crypto/aes_ccm.cpp  \
    crypto/aes_ecb.cpp  \
    crypto/crypto_platform.cpp  \
    crypto/storage.cpp  \
    diags/factory_diags.cpp  \
    instance/instance.cpp  \
    mac/link_raw.cpp  \
    mac/mac_frame.cpp  \
    mac/mac_header_ie.cpp  \
    mac/mac_types.cpp  \
    mac/sub_mac.cpp  \
    mac/sub_mac_callbacks.cpp  \
    mac/sub_mac_csl_receiver.cpp  \
    mac/sub_mac_wed.cpp  \
    radio/radio.cpp  \
    radio/radio_callbacks.cpp  \
    radio/radio_platform.cpp  \
    thread/link_quality.cpp  \
    utils/parse_cmdline.cpp  \
    utils/power_calibration.cpp

otlib_module_src := $(addprefix $(otlib_module_srcdir)/,${otlib_module_src}) 
otlib_module_inc :=      \
    api                  \
    common               \
    config               \
    crypto               \
    diags                \
    instance             \
    mac                  \
    meshcop              \
    radio                \
    thread               \
    utils

else
otlib_module_src := \
    api/backbone_router_api.cpp  \
    api/backbone_router_ftd_api.cpp  \
    api/ble_secure_api.cpp  \
    api/border_agent_api.cpp  \
    api/border_router_api.cpp  \
    api/border_routing_api.cpp  \
    api/channel_manager_api.cpp  \
    api/channel_monitor_api.cpp  \
    api/child_supervision_api.cpp  \
    api/coap_api.cpp  \
    api/coap_secure_api.cpp  \
    api/commissioner_api.cpp  \
    api/crypto_api.cpp  \
    api/dataset_api.cpp  \
    api/dataset_ftd_api.cpp  \
    api/dataset_updater_api.cpp  \
    api/diags_api.cpp  \
    api/dns_api.cpp  \
    api/dns_server_api.cpp  \
    api/error_api.cpp  \
    api/heap_api.cpp  \
    api/history_tracker_api.cpp  \
    api/icmp6_api.cpp  \
    api/instance_api.cpp  \
    api/ip6_api.cpp  \
    api/jam_detection_api.cpp  \
    api/joiner_api.cpp  \
    api/link_api.cpp  \
    api/link_metrics_api.cpp  \
    api/link_raw_api.cpp  \
    api/logging_api.cpp  \
    api/mdns_api.cpp  \
    api/mesh_diag_api.cpp  \
    api/message_api.cpp  \
    api/multi_radio_api.cpp  \
    api/nat64_api.cpp  \
    api/netdata_api.cpp  \
    api/netdata_publisher_api.cpp  \
    api/netdiag_api.cpp  \
    api/network_time_api.cpp  \
    api/ping_sender_api.cpp  \
    api/radio_stats_api.cpp  \
    api/random_crypto_api.cpp  \
    api/random_noncrypto_api.cpp  \
    api/server_api.cpp  \
    api/sntp_api.cpp  \
    api/srp_client_api.cpp  \
    api/srp_client_buffers_api.cpp  \
    api/srp_server_api.cpp  \
    api/tasklet_api.cpp  \
    api/tcp_api.cpp  \
    api/tcp_ext_api.cpp  \
    api/thread_api.cpp  \
    api/thread_ftd_api.cpp  \
    api/trel_api.cpp  \
    api/udp_api.cpp  \
    api/verhoeff_checksum_api.cpp  \
    backbone_router/backbone_tmf.cpp  \
    backbone_router/backbone_tmf.hpp  \
    backbone_router/bbr_leader.cpp  \
    backbone_router/bbr_leader.hpp  \
    backbone_router/bbr_local.cpp  \
    backbone_router/bbr_local.hpp  \
    backbone_router/bbr_manager.cpp  \
    backbone_router/bbr_manager.hpp  \
    backbone_router/multicast_listeners_table.cpp  \
    backbone_router/multicast_listeners_table.hpp  \
    backbone_router/ndproxy_table.cpp  \
    backbone_router/ndproxy_table.hpp  \
    border_router/infra_if.cpp  \
    border_router/infra_if.hpp  \
    border_router/routing_manager.cpp  \
    border_router/routing_manager.hpp  \
    coap/coap.cpp  \
    coap/coap.hpp  \
    coap/coap_message.cpp  \
    coap/coap_message.hpp  \
    coap/coap_secure.cpp  \
    coap/coap_secure.hpp  \
    common/appender.cpp  \
    common/appender.hpp  \
    common/array.hpp  \
    common/as_core_type.hpp  \
    common/binary_search.cpp  \
    common/binary_search.hpp  \
    common/bit_set.hpp  \
    common/callback.hpp  \
    common/clearable.hpp  \
    common/const_cast.hpp  \
    common/crc.cpp  \
    common/crc.hpp  \
    common/data.cpp  \
    common/data.hpp  \
    common/debug.hpp  \
    common/encoding.hpp  \
    common/equatable.hpp  \
    common/error.cpp  \
    common/error.hpp  \
    common/frame_builder.cpp  \
    common/frame_builder.hpp  \
    common/frame_data.cpp  \
    common/frame_data.hpp  \
    common/heap.cpp  \
    common/heap.hpp  \
    common/heap_allocatable.hpp  \
    common/heap_array.hpp  \
    common/heap_data.cpp  \
    common/heap_data.hpp  \
    common/heap_string.cpp  \
    common/heap_string.hpp  \
    common/iterator_utils.hpp  \
    common/linked_list.hpp  \
    common/locator.hpp  \
    common/log.cpp  \
    common/log.hpp  \
    common/logging.hpp  \
    common/message.cpp  \
    common/message.hpp  \
    common/non_copyable.hpp  \
    common/notifier.cpp  \
    common/notifier.hpp  \
    common/num_utils.hpp  \
    common/numeric_limits.hpp  \
    common/offset_range.cpp  \
    common/offset_range.hpp  \
    common/owned_ptr.hpp  \
    common/owning_list.hpp  \
    common/pool.hpp  \
    common/preference.cpp  \
    common/preference.hpp  \
    common/ptr_wrapper.hpp  \
    common/random.cpp  \
    common/random.hpp  \
    common/retain_ptr.hpp  \
    common/serial_number.hpp  \
    common/settings.cpp  \
    common/settings.hpp  \
    common/settings_driver.hpp  \
    common/string.cpp  \
    common/string.hpp  \
    common/tasklet.cpp  \
    common/tasklet.hpp  \
    common/time.hpp  \
    common/time_ticker.cpp  \
    common/time_ticker.hpp  \
    common/timer.cpp  \
    common/timer.hpp  \
    common/tlvs.cpp  \
    common/tlvs.hpp  \
    common/trickle_timer.cpp  \
    common/trickle_timer.hpp  \
    common/type_traits.hpp  \
    common/uptime.cpp  \
    common/uptime.hpp  \
    crypto/aes_ccm.cpp  \
    crypto/aes_ccm.hpp  \
    crypto/aes_ecb.cpp  \
    crypto/aes_ecb.hpp  \
    crypto/context_size.hpp  \
    crypto/crypto_platform.cpp  \
    crypto/ecdsa.hpp  \
    crypto/hkdf_sha256.cpp  \
    crypto/hkdf_sha256.hpp  \
    crypto/hmac_sha256.cpp  \
    crypto/hmac_sha256.hpp  \
    crypto/mbedtls.cpp  \
    crypto/mbedtls.hpp  \
    crypto/sha256.cpp  \
    crypto/sha256.hpp  \
    crypto/storage.cpp  \
    crypto/storage.hpp  \
    diags/factory_diags.cpp  \
    diags/factory_diags.hpp  \
    instance/extension.hpp  \
    instance/instance.cpp  \
    instance/instance.hpp  \
    mac/channel_mask.cpp  \
    mac/channel_mask.hpp  \
    mac/data_poll_handler.cpp  \
    mac/data_poll_handler.hpp  \
    mac/data_poll_sender.cpp  \
    mac/data_poll_sender.hpp  \
    mac/link_raw.cpp  \
    mac/link_raw.hpp  \
    mac/mac.cpp  \
    mac/mac.hpp  \
    mac/mac_filter.cpp  \
    mac/mac_filter.hpp  \
    mac/mac_frame.cpp  \
    mac/mac_frame.hpp  \
    mac/mac_header_ie.cpp  \
    mac/mac_header_ie.hpp  \
    mac/mac_links.cpp  \
    mac/mac_links.hpp  \
    mac/mac_types.cpp  \
    mac/mac_types.hpp  \
    mac/sub_mac.cpp  \
    mac/sub_mac.hpp  \
    mac/sub_mac_callbacks.cpp  \
    mac/sub_mac_csl_receiver.cpp  \
    mac/sub_mac_wed.cpp  \
    mac/wakeup_tx_scheduler.cpp  \
    mac/wakeup_tx_scheduler.hpp  \
    meshcop/announce_begin_client.cpp  \
    meshcop/announce_begin_client.hpp  \
    meshcop/border_agent.cpp  \
    meshcop/border_agent.hpp  \
    meshcop/commissioner.cpp  \
    meshcop/commissioner.hpp  \
    meshcop/dataset.cpp  \
    meshcop/dataset.hpp  \
    meshcop/dataset_manager.cpp  \
    meshcop/dataset_manager.hpp  \
    meshcop/dataset_manager_ftd.cpp  \
    meshcop/dataset_updater.cpp  \
    meshcop/dataset_updater.hpp  \
    meshcop/energy_scan_client.cpp  \
    meshcop/energy_scan_client.hpp  \
    meshcop/extended_panid.cpp  \
    meshcop/extended_panid.hpp  \
    meshcop/joiner.cpp  \
    meshcop/joiner.hpp  \
    meshcop/joiner_router.cpp  \
    meshcop/joiner_router.hpp  \
    meshcop/meshcop.cpp  \
    meshcop/meshcop.hpp  \
    meshcop/meshcop_leader.cpp  \
    meshcop/meshcop_leader.hpp  \
    meshcop/meshcop_tlvs.cpp  \
    meshcop/meshcop_tlvs.hpp  \
    meshcop/network_name.cpp  \
    meshcop/network_name.hpp  \
    meshcop/panid_query_client.cpp  \
    meshcop/panid_query_client.hpp  \
    meshcop/secure_transport.cpp  \
    meshcop/secure_transport.hpp  \
    meshcop/tcat_agent.cpp  \
    meshcop/tcat_agent.hpp  \
    meshcop/timestamp.cpp  \
    meshcop/timestamp.hpp  \
    net/checksum.cpp  \
    net/checksum.hpp  \
    net/dhcp6_client.cpp  \
    net/dhcp6_client.hpp  \
    net/dhcp6_server.cpp  \
    net/dhcp6_server.hpp  \
    net/dhcp6_types.cpp  \
    net/dhcp6_types.hpp  \
    net/dns_client.cpp  \
    net/dns_client.hpp  \
    net/dns_dso.cpp  \
    net/dns_dso.hpp  \
    net/dns_platform.cpp  \
    net/dns_types.cpp  \
    net/dns_types.hpp  \
    net/dnssd.cpp  \
    net/dnssd.hpp  \
    net/dnssd_server.cpp  \
    net/dnssd_server.hpp  \
    net/icmp6.cpp  \
    net/icmp6.hpp  \
    net/ip4_types.cpp  \
    net/ip4_types.hpp  \
    net/ip6.cpp  \
    net/ip6.hpp  \
    net/ip6_address.cpp  \
    net/ip6_address.hpp  \
    net/ip6_filter.cpp  \
    net/ip6_filter.hpp  \
    net/ip6_headers.cpp  \
    net/ip6_headers.hpp  \
    net/ip6_mpl.cpp  \
    net/ip6_mpl.hpp  \
    net/ip6_types.hpp  \
    net/mdns.cpp  \
    net/mdns.hpp  \
    net/nat64_translator.cpp  \
    net/nat64_translator.hpp  \
    net/nd6.cpp  \
    net/nd6.hpp  \
    net/nd_agent.cpp  \
    net/nd_agent.hpp  \
    net/netif.cpp  \
    net/netif.hpp  \
    net/sntp_client.cpp  \
    net/sntp_client.hpp  \
    net/socket.cpp  \
    net/socket.hpp  \
    net/srp_advertising_proxy.cpp  \
    net/srp_advertising_proxy.hpp  \
    net/srp_client.cpp  \
    net/srp_client.hpp  \
    net/srp_server.cpp  \
    net/srp_server.hpp  \
    net/tcp6.cpp  \
    net/tcp6.hpp  \
    net/tcp6_ext.cpp  \
    net/tcp6_ext.hpp  \
    net/udp6.cpp  \
    net/udp6.hpp  \
    radio/ble_secure.cpp  \
    radio/ble_secure.hpp  \
    radio/max_power_table.hpp  \
    radio/radio.cpp  \
    radio/radio.hpp  \
    radio/radio_callbacks.cpp  \
    radio/radio_platform.cpp  \
    radio/trel_interface.cpp  \
    radio/trel_interface.hpp  \
    radio/trel_link.cpp  \
    radio/trel_link.hpp  \
    radio/trel_packet.cpp  \
    radio/trel_packet.hpp  \
    radio/trel_peer.cpp  \
    radio/trel_peer.hpp  \
    radio/trel_peer_discoverer.cpp  \
    radio/trel_peer_discoverer.hpp  \
    thread/address_resolver.cpp  \
    thread/address_resolver.hpp  \
    thread/announce_begin_server.cpp  \
    thread/announce_begin_server.hpp  \
    thread/announce_sender.cpp  \
    thread/announce_sender.hpp  \
    thread/anycast_locator.cpp  \
    thread/anycast_locator.hpp  \
    thread/child.cpp  \
    thread/child.hpp  \
    thread/child_mask.hpp  \
    thread/child_supervision.cpp  \
    thread/child_supervision.hpp  \
    thread/child_table.cpp  \
    thread/child_table.hpp  \
    thread/csl_tx_scheduler.cpp  \
    thread/csl_tx_scheduler.hpp  \
    thread/discover_scanner.cpp  \
    thread/discover_scanner.hpp  \
    thread/dua_manager.cpp  \
    thread/dua_manager.hpp  \
    thread/energy_scan_server.cpp  \
    thread/energy_scan_server.hpp  \
    thread/indirect_sender.cpp  \
    thread/indirect_sender.hpp  \
    thread/indirect_sender_frame_context.hpp  \
    thread/key_manager.cpp  \
    thread/key_manager.hpp  \
    thread/link_metrics.cpp  \
    thread/link_metrics.hpp  \
    thread/link_metrics_tlvs.hpp  \
    thread/link_metrics_types.cpp  \
    thread/link_metrics_types.hpp  \
    thread/link_quality.cpp  \
    thread/link_quality.hpp  \
    thread/lowpan.cpp  \
    thread/lowpan.hpp  \
    thread/mesh_forwarder.cpp  \
    thread/mesh_forwarder.hpp  \
    thread/mesh_forwarder_ftd.cpp  \
    thread/mesh_forwarder_mtd.cpp  \
    thread/mle.cpp  \
    thread/mle.hpp  \
    thread/mle_ftd.cpp  \
    thread/mle_tlvs.cpp  \
    thread/mle_tlvs.hpp  \
    thread/mle_types.cpp  \
    thread/mle_types.hpp  \
    thread/mlr_manager.cpp  \
    thread/mlr_manager.hpp  \
    thread/mlr_types.hpp  \
    thread/neighbor.cpp  \
    thread/neighbor.hpp  \
    thread/neighbor_table.cpp  \
    thread/neighbor_table.hpp  \
    thread/network_data.cpp  \
    thread/network_data.hpp  \
    thread/network_data_leader.cpp  \
    thread/network_data_leader.hpp  \
    thread/network_data_leader_ftd.cpp  \
    thread/network_data_local.cpp  \
    thread/network_data_local.hpp  \
    thread/network_data_notifier.cpp  \
    thread/network_data_notifier.hpp  \
    thread/network_data_publisher.cpp  \
    thread/network_data_publisher.hpp  \
    thread/network_data_service.cpp  \
    thread/network_data_service.hpp  \
    thread/network_data_tlvs.cpp  \
    thread/network_data_tlvs.hpp  \
    thread/network_data_types.cpp  \
    thread/network_data_types.hpp  \
    thread/network_diagnostic.cpp  \
    thread/network_diagnostic.hpp  \
    thread/network_diagnostic_tlvs.cpp  \
    thread/network_diagnostic_tlvs.hpp  \
    thread/panid_query_server.cpp  \
    thread/panid_query_server.hpp  \
    thread/radio_selector.cpp  \
    thread/radio_selector.hpp  \
    thread/router.cpp  \
    thread/router.hpp  \
    thread/router_table.cpp  \
    thread/router_table.hpp  \
    thread/src_match_controller.cpp  \
    thread/src_match_controller.hpp  \
    thread/thread_link_info.cpp  \
    thread/thread_link_info.hpp  \
    thread/thread_netif.cpp  \
    thread/thread_netif.hpp  \
    thread/thread_tlvs.hpp  \
    thread/time_sync_service.cpp  \
    thread/time_sync_service.hpp  \
    thread/tmf.cpp  \
    thread/tmf.hpp  \
    thread/uri_paths.cpp  \
    thread/uri_paths.hpp  \
    thread/version.cpp  \
    thread/version.hpp  \
    utils/channel_manager.cpp  \
    utils/channel_manager.hpp  \
    utils/channel_monitor.cpp  \
    utils/channel_monitor.hpp  \
    utils/flash.cpp  \
    utils/flash.hpp  \
    utils/heap.cpp  \
    utils/heap.hpp  \
    utils/history_tracker.cpp  \
    utils/history_tracker.hpp  \
    utils/jam_detector.cpp  \
    utils/jam_detector.hpp  \
    utils/link_metrics_manager.cpp  \
    utils/link_metrics_manager.hpp  \
    utils/mesh_diag.cpp  \
    utils/mesh_diag.hpp  \
    utils/otns.cpp  \
    utils/otns.hpp  \
    utils/parse_cmdline.cpp  \
    utils/parse_cmdline.hpp  \
    utils/ping_sender.cpp  \
    utils/ping_sender.hpp  \
    utils/power_calibration.cpp  \
    utils/power_calibration.hpp  \
    utils/slaac_address.cpp  \
    utils/slaac_address.hpp  \
    utils/srp_client_buffers.cpp  \
    utils/srp_client_buffers.hpp  \
    utils/static_counter.hpp  \
    utils/verhoeff_checksum.cpp  \
    utils/verhoeff_checksum.hpp

otlib_module_src := $(addprefix $(otlib_module_srcdir)/,${otlib_module_src}) 
otlib_module_inc :=      \
    api                  \
    backbone_router      \
    border_router        \
    coap                 \
    common               \
    config               \
    crypto               \
    diags                \
    instance             \
    mac                  \
    meshcop              \
    net                  \
    radio                \
    thread               \
    utils
endif

otlib_module_inc := $(addprefix $(otlib_module_srcdir)/,${otlib_module_inc})
#otlib_module_srcdir := $(addprefix $(otlib_module_inc)/,${otlib_module_srcdir})
otlib_module_inc := $(otlib_module_inc) $(otlib)
otlib_module_srcdir := $(otlib_module_inc)

COMPONENT_ADD_INCLUDEDIRS := $(COMPONENT_ADD_INCLUDEDIRS) $(otlib_module_inc)
COMPONENT_SRCS := $(otlib_module_src)
COMPONENT_SRCDIRS := $(otlib_module_srcdir)

ifneq ($(CONFIG_OT_CLI),)
# src/cli 
otlib := openthread/src
otlib_module_srcdir := cli
otlib_module_src := \
    cli.cpp  \
    cli_bbr.cpp  \
    cli_br.cpp  \
    cli_coap.cpp  \
    cli_coap_secure.cpp  \
    cli_commissioner.cpp  \
    cli_dataset.cpp  \
    cli_dns.cpp  \
    cli_history.cpp  \
    cli_joiner.cpp  \
    cli_link_metrics.cpp  \
    cli_mac_filter.cpp  \
    cli_mdns.cpp  \
    cli_mesh_diag.cpp  \
    cli_network_data.cpp  \
    cli_ping.cpp  \
    cli_srp_client.cpp  \
    cli_srp_server.cpp  \
    cli_tcp.cpp  \
    cli_udp.cpp  \
    cli_utils.cpp

otlib_module_srcdir := $(addprefix $(otlib)/,${otlib_module_srcdir})
otlib_module_src := $(addprefix $(otlib_module_srcdir)/,${otlib_module_src}) 

otlib_module_inc := $(otlib_module_srcdir)

COMPONENT_SRCS += $(otlib_module_src)
COMPONENT_SRCDIRS += $(otlib_module_srcdir)
COMPONENT_ADD_INCLUDEDIRS := $(COMPONENT_ADD_INCLUDEDIRS) $(otlib_module_inc)
endif

ifneq (00,$(CONFIG_OT_NCP)$(CONFIG_OT_RCP))
# src/ncp (NCP/RCP mode)
ifeq (1,$(CONFIG_OT_NCP))
    CPPFLAGS += -DOPENTHREAD_SPINEL_CONFIG_OPENTHREAD_MESSAGE_ENABLE=1
endif

otlib := ./openthread/src
otlib_module_srcdir := ncp
otlib_module_src := \
    changed_props_set.cpp \
    multipan_platform.cpp \
    ncp_base.cpp \
    ncp_base_dispatcher.cpp \
    ncp_base_radio.cpp \
    ncp_spi.cpp \
    ncp_hdlc.cpp \
    ncp_base_ftd.cpp \
    ncp_base_mtd.cpp \
    platform/dnssd.cpp \
    platform/infra_if.cpp

otlib_module_srcdir := $(addprefix $(otlib)/,${otlib_module_srcdir})
otlib_module_src := $(addprefix $(otlib_module_srcdir)/,${otlib_module_src})
otlib_module_srcdir := $(otlib_module_srcdir) $(otlib_module_srcdir)/platform
otlib_module_inc := $(otlib_module_srcdir)

COMPONENT_SRCS += $(otlib_module_src)
COMPONENT_SRCDIRS += $(otlib_module_srcdir)
COMPONENT_ADD_INCLUDEDIRS := $(COMPONENT_ADD_INCLUDEDIRS) $(otlib_module_inc)

# hdlc (same as NCP)
otlib_module_srcdir := lib/hdlc
otlib_module_src := hdlc.cpp
otlib_module_srcdir := $(addprefix $(otlib)/,${otlib_module_srcdir})
otlib_module_src := $(addprefix $(otlib_module_srcdir)/,${otlib_module_src})
COMPONENT_SRCS += $(otlib_module_src)
COMPONENT_SRCDIRS += $(otlib_module_srcdir)
COMPONENT_ADD_INCLUDEDIRS := $(COMPONENT_ADD_INCLUDEDIRS) $(COMPONENT_PATH)/../openthread/src/lib/hdlc

otlib_module_srcdir := lib/spinel
otlib_module_src := \
    spinel.c \
    spinel_buffer.cpp \
    spinel_decoder.cpp \
    spinel_encoder.cpp \
    spinel_helper.cpp \
    spinel_prop_codec.cpp
otlib_module_srcdir := $(addprefix $(otlib)/,${otlib_module_srcdir})
otlib_module_src := $(addprefix $(otlib_module_srcdir)/,${otlib_module_src})
COMPONENT_SRCS += $(otlib_module_src)
COMPONENT_SRCDIRS += $(otlib_module_srcdir)
COMPONENT_ADD_INCLUDEDIRS := $(COMPONENT_ADD_INCLUDEDIRS) $(COMPONENT_PATH)/../openthread/src/lib/spinel

otlib := ./openthread/examples
otlib_module_srcdir := apps/ncp
otlib_module_src := ncp.c
otlib_module_srcdir := $(otlib)/${otlib_module_srcdir}
otlib_module_src := $(addprefix $(otlib_module_srcdir)/,${otlib_module_src})
COMPONENT_SRCS += $(otlib_module_src)
COMPONENT_SRCDIRS += $(otlib_module_srcdir)
COMPONENT_ADD_INCLUDEDIRS := $(COMPONENT_ADD_INCLUDEDIRS) $(otlib_module_srcdir) $(otlib)/platforms
endif

# src/lib/platform
otlib := ./openthread/src/lib
otlib_module_srcdir := platform
otlib_module_src := exit_code.c exit_code.h
otlib_module_srcdir := $(addprefix $(otlib)/,${otlib_module_srcdir})
otlib_module_src := $(addprefix $(otlib_module_srcdir)/,${otlib_module_src})
COMPONENT_SRCS += $(otlib_module_src)
COMPONENT_SRCDIRS += $(otlib_module_srcdir)
COMPONENT_ADD_INCLUDEDIRS := $(COMPONENT_ADD_INCLUDEDIRS) $(otlib_module_srcdir)

ifneq ($(CONFIG_OT_CLI),)
otlib := ./openthread/examples
otlib_module_srcdir := apps/cli
otlib_module_src := cli_uart.cpp
otlib_module_srcdir := $(otlib)/${otlib_module_srcdir}
otlib_module_src := $(addprefix $(otlib_module_srcdir)/,${otlib_module_src})
COMPONENT_SRCS += $(otlib_module_src)
COMPONENT_SRCDIRS += $(otlib_module_srcdir)
COMPONENT_ADD_INCLUDEDIRS := $(COMPONENT_ADD_INCLUDEDIRS) $(otlib_module_srcdir) $(otlib)/platforms
endif


ifneq (00,$(CONFIG_OT_FTD)$(CONFIG_OT_MTD))
otlib := ./openthread/third_party/tcplp
otlib_module_srcdir := $(otlib)
otlib_module_src := \
    bsdtcp/cc/cc_newreno.c \
    bsdtcp/tcp_fastopen.c \
    bsdtcp/tcp_input.c \
    bsdtcp/tcp_output.c \
    bsdtcp/tcp_reass.c \
    bsdtcp/tcp_sack.c \
    bsdtcp/tcp_subr.c \
    bsdtcp/tcp_timer.c \
    bsdtcp/tcp_timewait.c \
    bsdtcp/tcp_usrreq.c \
    lib/bitmap.c \
    lib/cbuf.c \
    lib/lbuf.c

otlib_module_src := $(addprefix $(otlib_module_srcdir)/,${otlib_module_src})
otlib_module_inc := \
    bdstcp/cc \
    bdstcp/sys \
    lib

otlib_module_inc := $(addprefix $(otlib_module_srcdir)/,${otlib_module_inc})
otlib_module_inc := $(otlib_module_inc) $(otlib)
otlib_module_srcdir := $(otlib_module_inc)

COMPONENT_ADD_INCLUDEDIRS := $(COMPONENT_ADD_INCLUDEDIRS) $(otlib_module_inc)
COMPONENT_SRCS += $(otlib_module_src)
COMPONENT_SRCDIRS += openthread/third_party/tcplp/bsdtcp openthread/third_party/tcplp/lib openthread/third_party/tcplp/bsdtcp/cc

CPPFLAGS += -Dtcp_input=ot_tcp_input
CPPFLAGS += -Dtcp_output=ot_tcp_output
CPPFLAGS += -Dtcp_close=ot_tcp_close
endif

CPPFLAGS += -Wimplicit-fallthrough=0
CPPFLAGS += -Wno-switch-default
CPPFLAGS += -Wno-format

## This component's src
COMPONENT_OBJS := $(patsubst %.cpp,%.o, $(filter %.cpp,$(COMPONENT_SRCS))) $(patsubst %.c,%.o, $(filter %.c,$(COMPONENT_SRCS))) $(patsubst %.S,%.o, $(filter %.S,$(COMPONENT_SRCS)))
