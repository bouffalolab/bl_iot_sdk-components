# change log on Thread module

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
