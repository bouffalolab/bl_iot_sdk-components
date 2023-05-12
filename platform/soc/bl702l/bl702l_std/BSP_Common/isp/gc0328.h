/**
  ******************************************************************************
  * @file    gc0328.h
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
#ifndef __GC0328_H__
#define __GC0328_H__

#include "bl808_cam.h"
#include "bl808_mjpeg.h"

/** @addtogroup  GC0328_Driver
 *  @{
 */

/** @addtogroup  GC0328
 *  @{
 */

/** @defgroup  GC0328_Public_Types
 *  @{
 */

/*@} end of group GC0328_Public_Types */

/** @defgroup  GC0328_Public_Constants
 *  @{
 */

/*@} end of group GC0328_Public_Constants */

/** @defgroup  GC0328_Public_Macros
 *  @{
 */

/*@} end of group GC0328_Public_Macros */

/** @defgroup  GC0328_Public_Functions
 *  @{
 */
BL_Err_Type GC0328_Init(CAM_ID_Type camId, BL_Fun_Type mjpegEn, CAM_CFG_Type *camCfg, MJPEG_CFG_Type *mjpegCfg);
void GC0328_Dump_Register(void);
BL_Err_Type GC0328_CAM_Get(CAM_ID_Type camId, uint8_t **pic, uint32_t *len);
uint8_t GC0328_CAM_Frame_Count(CAM_ID_Type camId);
void GC0328_CAM_Release(CAM_ID_Type camId);
void GC0328_CAM_Open(CAM_ID_Type camId);
void GC0328_CAM_Close(CAM_ID_Type camId);
void GC0328_CAM_Deinit(CAM_ID_Type camId);
BL_Err_Type GC0328_MJPEG_Get(uint8_t **pic, uint32_t *len);
uint8_t GC0328_MJPEG_Frame_Count(void);
void GC0328_MJPEG_Release(void);
void GC0328_MJPEG_Open(void);
void GC0328_MJPEG_Close(void);
void GC0328_MJPEG_Deinit(void);

/*@} end of group GC0328_Public_Functions */

/*@} end of group GC0328 */

/*@} end of group GC0328_Driver */

#endif /* __GC0328_H__ */
