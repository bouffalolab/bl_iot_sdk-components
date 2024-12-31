# change log on lmac154 module

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


