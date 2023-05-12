/**
  ******************************************************************************
  * @file    image_sensor.h
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
#ifndef __IMAGE_SENSOR_H__
#define __IMAGE_SENSOR_H__

#include "bl808_i2c.h"
#include "bl808_cam.h"
#include "bl808_mjpeg.h"

/** @addtogroup  Image_Sensor_Driver
 *  @{
 */

/** @addtogroup  IMAGE_SENSOR
 *  @{
 */

/** @defgroup  IMAGE_SENSOR_Public_Types
 *  @{
 */

/*@} end of group IMAGE_SENSOR_Public_Types */

/** @defgroup  IMAGE_SENSOR_Public_Constants
 *  @{
 */

/*@} end of group IMAGE_SENSOR_Public_Constants */

/** @defgroup  IMAGE_SENSOR_Public_Macros
 *  @{
 */
#define IMAGE_SENSOR_BF2013 0
#define IMAGE_SENSOR_GC0308 1
#define IMAGE_SENSOR_GC2053 2
#define IMAGE_SENSOR_USE    IMAGE_SENSOR_GC2053

#define SEC_TO_NS(x)                    ((x) * 1000000000)
#define NS_TO_SEC(x)                    DIV_ROUND(x, 1000000000)

#define INT_TO_EXPO_TIME(x)             (BL_EXPO_TIME)(DIV_ROUND(SEC_TO_NS(1),x))
#define EXPO_TIME_TO_INT(x)             (SEC_TO_NS(1) / (int)(x))
#define EXPO_TIME_1_SEC                 1000000000
#define E50
#ifdef E50
#define EXPO_TIME_1_25_SEC              INT_TO_EXPO_TIME(25)//40000000
#define EXPO_TIME_1_50_SEC              INT_TO_EXPO_TIME(50)
#define EXPO_TIME_1_100_SEC             INT_TO_EXPO_TIME(100)//10000000
#elif defined(E60)
#define EXPO_TIME_1_25_SEC              INT_TO_EXPO_TIME(30)//40000000
#define EXPO_TIME_1_50_SEC              INT_TO_EXPO_TIME(60)//INT_TO_EXPO_TIME(50)
#define EXPO_TIME_1_100_SEC             INT_TO_EXPO_TIME(120)//8333333//10000000
#endif
#define EXPO_TIME_1_30_SEC              INT_TO_EXPO_TIME(30)
#define EXPO_TIME_1_60_SEC              INT_TO_EXPO_TIME(60)
#define EXPO_TIME_1_120_SEC             8333333
#define EXPO_TIME_MIN                   1


#define INT_TO_GAIN_DB(x)               (BL_GAIN_DB)((x) << 8)
#define GAIN_DB_TO_INT(x)               ((int)(x) >> 8)

#define GAIN_X_TO_DB(x)                 (log2((float)(x)) * 6)
#define GAIN_DB_FLOAT_TO_INT(x)         ((int)((float)(x) * 256 + 0.5))
#define GAIN_DB_INT_TO_FLOAT(x)         ((float)(x) / 256)

#define GAIN_6_DB                       1536    /* 6 << 8 */
#define GAIN_0_DB                       0

#if (IMAGE_SENSOR_USE == IMAGE_SENSOR_GC2053)
#define SENSOR_MAX_EXPO_TIME            EXPO_TIME_1_25_SEC              /* 1/25 sec */
#define SENSOR_MIN_EXPO_TIME            143282                          /* 143282 ns, 4 line */
#define SENSOR_MAX_GAIN                 GAIN_DB_FLOAT_TO_INT(36)        /* limit max gain t0 36 dB to avoid bad noise */
#define SENSOR_MAX_AGAIN                GAIN_DB_FLOAT_TO_INT(23.625)    /* 23.625 dB or 15.5x */
#define SENSOR_MIN_GAIN_STEP            (GAIN_6_DB / 16)                /* 0.375 dB */
#endif

#define ROTATE_LEFT(value, n)  ((value >> (32 - n)) | (value << n))
#define ROTATE_RIGHT(value, n) ((value << (32 - n)) | (value >> n))
#define ROTATE16(value, n)     ((value<<(16-n))|(value>>n))

/*@} end of group IMAGE_SENSOR_Public_Macros */

/** @defgroup  IMAGE_SENSOR_Public_Functions
 *  @{
 */
void Image_Sensor_PSRAM_Init(void);
BL_Err_Type Image_Sensor_Init(CAM_ID_Type camId, BL_Fun_Type mjpegEn, CAM_CFG_Type *camCfg, MJPEG_CFG_Type *mjpegCfg);
void Image_Sensor_Dump_Register(void);
BL_Err_Type Image_Sensor_CAM_Get(CAM_ID_Type camId, uint8_t **pic, uint32_t *len);
uint8_t Image_Sensor_CAM_Frame_Count(CAM_ID_Type camId);
void Image_Sensor_CAM_Release(CAM_ID_Type camId);
void Image_Sensor_CAM_Open(CAM_ID_Type camId);
void Image_Sensor_CAM_Close(CAM_ID_Type camId);
void Image_Sensor_CAM_Deinit(CAM_ID_Type camId);
BL_Err_Type Image_Sensor_MJPEG_Get(uint8_t **pic, uint32_t *len);
uint8_t Image_Sensor_MJPEG_Frame_Count(void);
void Image_Sensor_MJPEG_Release(void);
void Image_Sensor_MJPEG_Open(void);
void Image_Sensor_MJPEG_Close(void);
void Image_Sensor_MJPEG_Deinit(void);

/*@} end of group IMAGE_SENSOR_Public_Functions */

/*@} end of group IMAGE_SENSOR */

/*@} end of group Image_Sensor_Driver */

#endif /* __IMAGE_SENSOR_H__ */
