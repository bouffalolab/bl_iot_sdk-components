# change log on Thread module

## 2026/05/28
- openthread: 1.7.8
  - openthread_port: add api to set Thread instance for openthread port layer
- openthread_br: 1.7.2

## 2026/05/14
- openthread: 1.7.7
  - openthread_rest: fix the crash with bad http request
- openthread_br: 1.7.2
  - remove duplicate mDNS TXT publish

## 2026/05/12
- openthread: 1.7.6
  - Build: merge openthread_ncp into top-level CMakeLists.txt (single library)
  - Rename OT_NCP/OT_RCP/OT_CLI/OT_FTD/OT_MTD to CONFIG_OT_*
  - Add CONFIG_OT_RCP (radio-only) mode
  - openthread_port: drop CFG_PREFIX, gate CLI on SYS_AOS_CLI_ENABLE
- openthread_br: 1.7.1

## 2026/05/08
- openthread: 1.7.5
  - add CSL TX/RX support, enh-ack improvements and debug traces
- openthread_br: 1.7.1

## 2026/04/20
- openthread: 1.7.4
  - enh-ack timestamp handling and compatibility
- openthread_br: 1.7.1

## 2026/03/30
- openthread: 1.7.2
  - Refactor lmac154 init to app layer with stack enable API
  - Add stack_idx parameter for multi-stack radio interface support
- openthread_br: 1.7.1

## 2026/01/22
- openthread: 1.7.1
  - Update Openthread Stack to 1.4
  - Re-org openthread port and rest code
- openthread_br: 1.7.1
  - Re-org openthread br
  
## 2026/1/20

- openthread_utils: 1.6.17
- openthread_br: 1.6.19
  - fix to re-publish new mdns service
- openthread/openthread_port/openthread_rest: 1.6.21
  - fix to get active dataset with text type with otbr restful api

## 2026/1/15

- openthread_utils: 1.6.17
- openthread_br: 1.6.18
  - fix crash on mdns question without service or prot specified
- openthread/openthread_port/openthread_rest: 1.6.20

## 2025/6/9

- openthread_utils: 1.6.17
- openthread_br: 1.6.17
- openthread/openthread_port/openthread_rest: 1.6.20
  - ot_nxspi: try one more time to exhaust nxspi readbuf

## 2025/6/6

- openthread_utils: 1.6.17
- openthread_br: 1.6.17
- openthread/openthread_port/openthread_rest: 1.6.19
  - Add nxspi protocol serial support

## 2025/4/25

- openthread_utils: 1.6.17
  - BL702: Parse recieved packets and drop packets with wrong destination for standard 2015 enabled
- openthread_br: 1.6.17
- openthread/openthread_port/openthread_rest: 1.6.18
  - enable isFtd by default to enable frame pending search function for RCP

## 2025/3/12

- openthread_utils: 1.6.16
- openthread_br: 1.6.17
- openthread/openthread_port/openthread_rest: 1.6.17
  - BL616: update API to do system reset

## 2025/2/8

- openthread_utils: 1.6.16
  - fix frame pending bit set for ack frame preparation
- openthread_br: 1.6.17
- openthread/openthread_port/openthread_rest: 1.6.16

## 2025/1/8

- openthread_br: 1.6.17
- Changes:
  - fix to receive multicast ICMPv6 neighbor discovery packets on infrastructure network

- openthread/openthread_port/openthread_rest: 1.6.16

## 2024/12/31

- openthread_br: 1.6.16
- Changes:
  - fix a crash issue using a message pointer which message has been recalled
  - fix an interlocking issue between lwip task and thread task

- openthread/openthread_port/openthread_rest: 1.6.16
- Changes:
  - add interface to get stack idle state for sleepy device

## 2024/12/16

- openthread_utils: 1.6.15
- Changes:
  - use a specific version to release this module

- openthread_br: 1.6.15
- Changes:
  - use a specific version to release this module

- openthread/openthread_port/openthread_rest: 1.6.15
- Changes:
  - use a specific version to release these modules

## 2024/12/13

- Version: 1.6.14
- Changes:
  - BL616: Improve large network stability
  - BL702L: Improve large network stability

## 2024/11/26

- Version: 1.6.13
- Changes:
  - OTBR: fix memory leak when handle multicast ICMPv6

## 2024/11/18

- Version: 1.6.12
- Changes:
  - BL616: Support to get boot reason

## 2024/10/25

- Version: 1.6.11
- Changes:
  - BL702/BL702L: Support on Bouffalo SDK

## 2024/10/09

- Version: 1.6.10
- Changes:
  - OTBR: Protect all tcp signature code with TCP_SIGNATURE

## 2024/09/05

- Version: 1.6.9
- Changes:
  - BL616: Improve network stablity with limits on standard 2015

## 2024/8/30

- Version: 1.6.8
- Changes:
  - BL616: Correct efuse slot to get MAC address
  - support dynamic initialize multi file system

## 2024/8/2
- Version: 1.6.7
- Changes:
  - BL616: Fix eui64 address constructed with flash id
  - OTBR: Fix udp socket binding issue

## 2024/8/1
- Version: 1.6.6
- Changes:
  - BL616: Update eui64 with efuse 6 bytes mac address and 2 bytes of 0 padding
  - BL616: Update eui64 with flash ID if no mac address in efuse.

## 2024/7/18
- Version: 1.6.5
- Changes:
  - OTBR: Add RESTful APIs provided on OTBR
  - OTBR: Reduce dead lock between lwip task and thread task

## 2024/5/27
- Version: 1.6.4
- Changes:
  - Set RX on when idle by default on each platform
  - BL702L: disable/enable request enh-ack event according to rx promiscuous mode
  - BL702L flash: set more time for enh-ack preparing
  - BL702L flash: fix issue to check trigger delay tx time for enh-ack sending
  - OTBR: use lower API to get EUI address for hostname construction
  - Update release script to update MAC address prefix

## 2024/5/23
- Version: 1.6.3
- Changes:
  - BL702L flash version: Do not setup enh-ack for sniffer mode
  - Fix to allocate buffer for ACK frame receive buffer
  - Make critical section protection on aes operation
  - OTBR: Improve stability on MDNS module, thread safty and memory release
  - OTBR: Register MDNS hostname when IP address is assigned or changed
  - OTBR: Fix to remove MDNS service

## 2024/4/25
- Version: 1.6.2
- Changes:
  - Support to generate openthread_utils library for BL702
  - Support to generate openthread_utils library for BL702L with/without ROM code used
  - Fix to compile Openthread Stack for NCP
  - Adjust openthread_port/openthread_utils structure to seperate hardware platform
  - Add Openthread Border Router module to release
  - Add WiFi-Thread co-exist support on BL616 platform
  - Support and use littlefs by default

## 2024/2/19
- init change log
- Thread stack
7e32165bee473f260460510de7675d08c44bf53c
commit 7e32165bee473f260460510de7675d08c44bf53c (HEAD)
Date:   Tue Aug 15 12:12:02 2023 -0700
