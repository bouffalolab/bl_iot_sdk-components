# change log on lmac154 module

## 2026/02/10
- Version: 1.6.18
  - BL702/BL702L: Improve to get frame pending result

## 2025/12/06
- Version: 1.6.17
  - Add API to set/get coex priority

## 2025/11/25
- Version: 1.6.16
- Changes
  - BL702: Add a protection to send a frame.

## 2025/9/17
- Version: 1.6.15
  - BL616: Remove unspported API

## 2025/9/11
- Version: 1.6.14
- Changes
  - BL702/BL702L: Record timestamp of rx done isr for monitor handling.

## 2025/4/25
- Version: 1.6.13
- Changes
  - BL702: Parse recieved packets and drop packets with wrong destination for standard 2015 enabled

## 2025/3/12
- Version: 1.6.12
- Changes
  - BL616: Apply TX power limit with channel and country code if phyrf lib supported

## 2025/2/8
- Version: 1.6.11
- Changes
  - Put nbr & fp table search function in  RAM

## 2024/12/13
- Version: 1.6.10
- Changes
  - BL616: Improve large network stability
  - BL702L: Improve large network stability and APIs to backup registers for sleep device

## 2024/11/18
- Version: 1.6.9
- Changes
  - BL616: Improve tx abort for ack transmition
  - BL702L: Improve tx abort for ack transmition

## 2024/10/25
- Version: 1.6.8
- Changes
  - BL616: Parse recieved packets and drop packets with wrong destination for standard 2015 enabled
  - BL702L: Parse recieved packets and drop packets with wrong destination for standard 2015 enabled

## 2024/09/05
- Version: 1.6.7
- Changes
  - BL616: Add more interfaces and log module

## 2024/7/18
- Version: 1.6.6
- Changes
  - BL616: Improve tx/rx stablity 

## 2024/6/3
- Version: 1.6.5
- Changes
  - BL702: Add register header files

## 2024/5/27
- Version: 1.6.4
- Changes
  - fix register setting for rx on when idle
  - BL702L: correct TX PPDU register to get PPDU length

## 2024/5/23
- Version: 1.6.3
- Changes
  - fix to recieve enh-ack
  - fix to send the wrong data at 125 byte

## 2024/4/25
- Version: 1.6.2
- Changes
  - Correct rx onoff state when idle for BL616 platform

## 2024/4/25
- Version: 1.6.1
- Changes
  - Adjust structure to seperate hardware platform
  - Add BL702L platform support
  - Add BLE-M154 co-exist support on BL616 platform
  - Add TX power set/get on BL616 platform


