/**
 * @file bl702L_sec_dbg.h
 * @brief
 *
 * Copyright (c) 2021 Bouffalolab team
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 */
#ifndef __BL702L_SEC_DBG_H__
#define __BL702L_SEC_DBG_H__

#include "sec_dbg_reg.h"
#include "bl702l_common.h"

/** @addtogroup  BL702L_Peripheral_Driver
 *  @{
 */

/** @addtogroup  SEC_DBG
 *  @{
 */

/** @defgroup  SEC_DBG_Public_Types
 *  @{
 */

/*@} end of group SEC_DBG_Public_Types */

/** @defgroup  SEC_DBG_Public_Constants
 *  @{
 */

/*@} end of group SEC_DBG_Public_Constants */

/** @defgroup  SEC_DBG_Public_Macros
 *  @{
 */

/*@} end of group SEC_DBG_Public_Macros */

/** @defgroup  SEC_DBG_Public_Functions
 *  @{
 */
void Sec_Dbg_Read_Chip_ID(uint8_t id[8]);
void Sec_Dbg_Set_Dbg_Pwd(const uint32_t pwd[4]);
uint32_t Sec_Dbg_Read_Dbg_Mode(void);
uint32_t Sec_Dbg_Read_Dbg_Enable(void);
void Sec_Dbg_Set_Dbg_Trigger(void);

/*@} end of group SEC_DBG_Public_Functions */

/*@} end of group SEC_DBG */

/*@} end of group BL616_Peripheral_Driver */

#endif /* __BL702L_SEC_DBG_H__ */
