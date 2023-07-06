/**
  ******************************************************************************
  * @file    bl702l_aon.h
  * @version V1.0
  * @date
  * @brief   This file is the standard driver header file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2020 Bouffalo Lab</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of Bouffalo Lab nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
#ifndef __BL702L_AON_H__
#define __BL702L_AON_H__

#include "aon_reg.h"
#include "glb_reg.h"
#include "hbn_reg.h"
#include "pds_reg.h"
#include "bl702l_ef_ctrl.h"
#include "bl702l_common.h"

/** @addtogroup  BL702L_Peripheral_Driver
 *  @{
 */

/** @addtogroup  AON
 *  @{
 */

/** @defgroup  AON_Public_Types
 *  @{
 */

/*@} end of group AON_Public_Types */

/** @defgroup  AON_Public_Constants
 *  @{
 */

/*@} end of group AON_Public_Constants */

/** @defgroup  AON_Public_Macros
 *  @{
 */

/*@} end of group AON_Public_Macros */

/** @defgroup  AON_Public_Functions
 *  @{
 */
/*----------*/
BL_Err_Type AON_Power_On_MBG(void);
BL_Err_Type AON_Power_Off_MBG(void);
/*----------*/
BL_Err_Type AON_Power_On_XTAL(void);
BL_Err_Type AON_Set_Xtal_CapCode(uint8_t capIn, uint8_t capOut);
uint8_t AON_Get_Xtal_CapCode(void);
BL_Err_Type AON_Set_Xtal_CapCode_Extra(uint8_t extra);
BL_Err_Type AON_Power_Off_XTAL(void);
/*----------*/
BL_Err_Type AON_Power_On_BG(void);
BL_Err_Type AON_Power_Off_BG(void);
/*----------*/
BL_Err_Type AON_Power_On_LDO11_SOC(void);
BL_Err_Type AON_Power_Off_LDO11_SOC(void);
/*----------*/
BL_Err_Type AON_Power_On_SFReg(void);
BL_Err_Type AON_Power_Off_SFReg(void);
/*----------*/
BL_Err_Type AON_LowPower_Enter_PDS0(void);
BL_Err_Type AON_LowPower_Exit_PDS0(void);
/*----------*/
BL_Err_Type AON_Set_LDO11_SOC_Sstart_Delay(uint8_t delay);
/*----------*/
BL_Err_Type AON_Set_DCDC14_Top_0(uint8_t voutSel, uint8_t vpfm);
/*----------*/
BL_Err_Type AON_Trim_Ldo11socVoutTrim(void);
BL_Err_Type AON_Trim_Ldo14VoutTrim(void);
BL_Err_Type AON_Trim_Dcdc14VoutTrim(void);
/*----------*/

/*@} end of group AON_Public_Functions */

/*@} end of group AON */

/*@} end of group BL702L_Peripheral_Driver */

#endif /* __BL702L_AON_H__ */