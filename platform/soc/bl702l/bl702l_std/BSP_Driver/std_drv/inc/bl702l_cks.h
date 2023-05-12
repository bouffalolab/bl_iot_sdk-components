/**
 * @file bl606p_cks.h
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
#include "bl702l_common.h"
#include "cks_reg.h"

/** @addtogroup  BL606P_Peripheral_Driver
 *  @{
 */

/** @addtogroup  CKS
 *  @{
 */

/** @defgroup  CKS_Public_Types
 *  @{
 */

/*@} end of group CKS_Public_Types */

/** @defgroup  CKS_Public_Constants
 *  @{
 */

/** @defgroup Reset_CKS
 *  @{
 */
#define CKS_Reset() BL_WR_REG(CKS_BASE, CKS_CONFIG, 0x1)

/*@} end of group Reset_CKS */

/** @defgroup Set_CKS_Big_Endian
 *  @{
 */
#define CKS_Set_Big_Endian() BL_WR_REG(CKS_BASE, CKS_CONFIG, 0x2)

/*@} end of group Set_CKS_Big_Endian */

/** @defgroup Input_CKS_DATA
 *  @{
 */
#define CKS_Data_In(n) BL_WR_REG(CKS_BASE, CKS_DATA_IN, (n & 0xFF))

/*@} end of group Input_CKS_DATA */

/** @defgroup Output_CKS_Result
 *  @{
 */
#define CKS_Data_Out() BL_RD_REG16(CKS_BASE, CKS_OUT)

/*@} end of group Output_CKS_Result */

/*@} end of group CKS_Public_Constants */

/** @defgroup DRIVER_Public_FunctionDeclaration
 *  @brief DRIVER functions declaration
 *  @{
 */
uint16_t CKS_Compute(uint8_t *data, uint32_t len);
/*@} end of group DRIVER_COMMON  */
