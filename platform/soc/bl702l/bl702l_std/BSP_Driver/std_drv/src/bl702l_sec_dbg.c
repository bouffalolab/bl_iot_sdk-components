/**
 * @file bl702l_sec_dbg.c
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


#include "string.h"
#include "bl702l_sec_dbg.h"

/** @addtogroup  BL702L_Peripheral_Driver
 *  @{
 */

/** @addtogroup  SEC_DBG
 *  @{
 */

/** @defgroup  SEC_DBG_Private_Macros
 *  @{
 */

/*@} end of group SEC_DBG_Private_Macros */

/** @defgroup  SEC_DBG_Private_Types
 *  @{
 */

/*@} end of group SEC_DBG_Private_Types */

/** @defgroup  SEC_DBG_Private_Variables
 *  @{
 */

/*@} end of group SEC_DBG_Private_Variables */

/** @defgroup  SEC_DBG_Global_Variables
 *  @{
 */

/*@} end of group SEC_DBG_Global_Variables */

/** @defgroup  SEC_DBG_Private_Fun_Declaration
 *  @{
 */

/*@} end of group SEC_DBG_Private_Fun_Declaration */

/** @defgroup  SEC_DBG_Public_Functions
 *  @{
 */

/****************************************************************************/ /**
 * @brief  Sec Dbg read chip ID
 *
 * @param  id[8]: chip ID buffer
 *
 * @return None
 *
*******************************************************************************/
void Sec_Dbg_Read_Chip_ID(uint8_t id[8])
{
    uint32_t idLow, idHigh;

    idLow = BL_RD_REG(SEC_DBG_BASE, SEC_DBG_SD_CHIP_ID_LOW);
    BL_WRWD_TO_BYTEP(id, idLow);

    idHigh = BL_RD_REG(SEC_DBG_BASE, SEC_DBG_SD_CHIP_ID_HIGH);
    BL_WRWD_TO_BYTEP((id + 4), idHigh);
}

/***************************************************************************** /
 * @brief  Sec Dbg set PWD
 *
 * @param  pwd: PWD buffer
 *
 * @return None
 *
*******************************************************************************/
void Sec_Dbg_Set_Dbg_Pwd(const uint32_t pwd[4])
{
    BL_WR_REG(SEC_DBG_BASE, SEC_DBG_SD_DBG_PWD_LOW, pwd[0]);
    BL_WR_REG(SEC_DBG_BASE, SEC_DBG_SD_DBG_PWD_HIGH, pwd[1]);
    BL_WR_REG(SEC_DBG_BASE, SEC_DBG_SD_DBG_PWD2_LOW, pwd[2]);
    BL_WR_REG(SEC_DBG_BASE, SEC_DBG_SD_DBG_PWD2_HIGH, pwd[3]);
}

/****************************************************************************/ /**
 * @brief  Sec Dbg read debug mode
 *
 * @param  None
 *
 * @return debug mode status
 *
*******************************************************************************/
uint32_t Sec_Dbg_Read_Dbg_Mode(void)
{
    return BL_GET_REG_BITS_VAL(BL_RD_REG(SEC_DBG_BASE, SEC_DBG_SD_STATUS), SEC_DBG_SD_DBG_MODE);
}

/****************************************************************************/ /**
 * @brief  Sec Dbg read debug enable status
 *
 * @param  None
 *
 * @return enable status
 *
*******************************************************************************/
uint32_t Sec_Dbg_Read_Dbg_Enable(void)
{
    return BL_GET_REG_BITS_VAL(BL_RD_REG(SEC_DBG_BASE, SEC_DBG_SD_STATUS), SEC_DBG_SD_DBG_ENA);
}

/****************************************************************************/ /**
 * @brief  Sec Dbg Set debug trigger status
 *
 * @param  None
 *
 * @return None
 *
*******************************************************************************/
void Sec_Dbg_Set_Dbg_Trigger(void)
{
    uint32_t tmpVal=0;
    tmpVal=BL_RD_REG(SEC_DBG_BASE, SEC_DBG_SD_STATUS);
    tmpVal=BL_SET_REG_BIT(tmpVal,SEC_DBG_SD_DBG_PWD_TRIG);
    BL_WR_REG(SEC_DBG_BASE, SEC_DBG_SD_STATUS,tmpVal);
}

/*@} end of group SEC_DBG_Public_Functions */

/*@} end of group SEC_DBG */

/*@} end of group BL616_Peripheral_Driver */
