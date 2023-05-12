/**
  ******************************************************************************
  * @file    gc0328.c
  * @version V1.0
  * @date
  * @brief   This file is the standard driver c file
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

#include "gc0328.h"
#include "bflb_platform.h"
#include "bl808_glb.h"
#include "bl808_i2c.h"

/** @addtogroup  GC0328_Driver
 *  @{
 */

/** @addtogroup  GC0328
 *  @{
 */

/** @defgroup  GC0328_Private_Macros
 *  @{
 */

#define I2C_CAMERA_ADDR 0x21
#define SENSOR_ID_REG   0xF0
#define SENSOR_ID_VAL   0x9D

/*@} end of group GC0328_Private_Macros */

/** @defgroup  GC0328_Private_Types
 *  @{
 */

/*@} end of group GC0328_Private_Types */

/** @defgroup  GC0328_Private_Variables
 *  @{
 */
static const uint8_t sensorRegList[][2] = {
    {0xfe, 0x80},
    {0xfe, 0x80},
    {0xfc, 0x16},
    {0xfc, 0x16},
    {0xfc, 0x16},
    {0xfc, 0x16},

    {0xfe, 0x00},
    {0x4f, 0x00},
    {0x42, 0x00},
    {0x03, 0x00},
    {0x04, 0xc0},
    {0x77, 0x62},
    {0x78, 0x40},
    {0x79, 0x4d},

    {0xfe, 0x01},
    {0x4f, 0x00},
    {0x4c, 0x01},
    {0xfe, 0x00},
    //////////////////////////////
    ///////////AWB///////////
    ////////////////////////////////
    {0xfe, 0x01},
    {0x51, 0x80},
    {0x52, 0x12},
    {0x53, 0x80},
    {0x54, 0x60},
    {0x55, 0x01},
    {0x56, 0x06},
    {0x5b, 0x02},
    {0xb1, 0xdc},
    {0xb2, 0xdc},
    {0x7c, 0x71},
    {0x7d, 0x00},
    {0x76, 0x00},
    {0x79, 0x20},
    {0x7b, 0x00},
    {0x70, 0xFF},
    {0x71, 0x00},
    {0x72, 0x10},
    {0x73, 0x40},
    {0x74, 0x40},
    ////AWB//
    {0x50, 0x00},
    {0xfe, 0x01},
    {0x4f, 0x00},
    {0x4c, 0x01},
    {0x4f, 0x00},
    {0x4f, 0x00},
    {0x4f, 0x00},
    {0x4d, 0x36},
    {0x4e, 0x02},
    {0x4d, 0x46},
    {0x4e, 0x02},
    {0x4e, 0x02},
    {0x4d, 0x53},
    {0x4e, 0x08},
    {0x4e, 0x04},
    {0x4e, 0x04},
    {0x4d, 0x63},
    {0x4e, 0x08},
    {0x4e, 0x08},
    {0x4d, 0x82},
    {0x4e, 0x20},
    {0x4e, 0x20},
    {0x4d, 0x92},
    {0x4e, 0x40},
    {0x4d, 0xa2},
    {0x4e, 0x40},
    {0x4f, 0x01},

    {0x50, 0x88},
    {0xfe, 0x00},
    ////////////////////////////////////////////////
    //////////// BLK //////////////////////
    ////////////////////////////////////////////////
    {0x27, 0x00},
    {0x2a, 0x40},
    {0x2b, 0x40},
    {0x2c, 0x40},
    {0x2d, 0x40},
    //////////////////////////////////////////////
    ////////// page 0 ////////////////////////
    //////////////////////////////////////////////
    {0xfe, 0x00},
    {0x05, 0x00},
    {0x06, 0xde},
    {0x07, 0x00},
    {0x08, 0xa7},

    {0x0d, 0x01},
    {0x0e, 0xe8},
    {0x0f, 0x02},
    {0x10, 0x88},
    {0x09, 0x00},
    {0x0a, 0x00},
    {0x0b, 0x00},
    {0x0c, 0x00},
    {0x16, 0x00},
    {0x17, 0x14 | 0x01},    // dir = 0x01
    {0x18, 0x0e},
    {0x19, 0x06},

    {0x1b, 0x48},
    {0x1f, 0xC8},
    {0x20, 0x01},
    {0x21, 0x78},
    {0x22, 0xb0},
    {0x23, 0x06},
    {0x24, 0x11},
    {0x26, 0x00},

    {0x50, 0x01}, // crop mode
    // global gain for range
    {0x70, 0x85},
    ////////////////////////////////////////////////
    //////////// block enable /////////////
    ////////////////////////////////////////////////
    {0x40, 0x7f},
    {0x41, 0x24},
    {0x42, 0xff},
    {0x45, 0x00},
    {0x44, 0x02}, //YUYV
    {0x46, 0x03}, //[0] VSYNC polarity 0x02

    {0x4b, 0x01},
    {0x50, 0x01},
    // DN & EEINTP
    {0x7e, 0x0a},
    {0x7f, 0x03},
    {0x81, 0x15},
    {0x82, 0x85},
    {0x83, 0x02},
    {0x84, 0xe5},
    {0x90, 0xac},
    {0x92, 0x02},
    {0x94, 0x02},
    {0x95, 0x54},
    ///////YCP
    {0xd1, 0x32},
    {0xd2, 0x32},
    {0xdd, 0x58},
    {0xde, 0x36},
    {0xe4, 0x88},
    {0xe5, 0x40},
    {0xd7, 0x0e},
    /////////////////////////////
    //////////////// GAMMA //////
    /////////////////////////////
    // rgb gamma
#if 0   /* RGBGAMMA_test */
    {0xfe, 0x00},
    {0xbf, 0x08},
    {0xc0, 0x10},
    {0xc1, 0x22},
    {0xc2, 0x32},
    {0xc3, 0x43},
    {0xc4, 0x50},
    {0xc5, 0x5e},
    {0xc6, 0x78},
    {0xc7, 0x90},
    {0xc8, 0xa6},
    {0xc9, 0xb9},
    {0xca, 0xc9},
    {0xcb, 0xd6},
    {0xcc, 0xe0},
    {0xcd, 0xee},
    {0xce, 0xf8},
    {0xcf, 0xff},
#elif 1 /* RGBGAMMA_m6 */
    {0xBF, 0x14},
    {0xc0, 0x28},
    {0xc1, 0x44},
    {0xc2, 0x5D},
    {0xc3, 0x72},
    {0xc4, 0x86},
    {0xc5, 0x95},
    {0xc6, 0xB1},
    {0xc7, 0xC6},
    {0xc8, 0xD5},
    {0xc9, 0xE1},
    {0xcA, 0xEA},
    {0xcB, 0xF1},
    {0xcC, 0xF5},
    {0xcD, 0xFB},
    {0xcE, 0xFE},
    {0xcF, 0xFF},
#endif

    /// Y gamma
    {0xfe, 0x00},
    {0x63, 0x00},
    {0x64, 0x05},
    {0x65, 0x0b},
    {0x66, 0x19},
    {0x67, 0x2e},
    {0x68, 0x40},
    {0x69, 0x54},
    {0x6a, 0x66},
    {0x6b, 0x86},
    {0x6c, 0xa7},
    {0x6d, 0xc6},
    {0x6e, 0xe4},
    {0x6f, 0xFF},
    //////ASDE
    {0xfe, 0x01},
    {0x18, 0x02},
    {0xfe, 0x00},
    {0x98, 0x00},
    {0x9b, 0x20},
    {0x9c, 0x80},
    {0xa4, 0x10},
    {0xa8, 0xB0},
    {0xaa, 0x40},
    {0xa2, 0x23},
    {0xad, 0x01},
    //////////////////////////////////////////////
    ////////// AEC ////////////////////////
    //////////////////////////////////////////////
    {0xfe, 0x01},
    {0x9c, 0x02},
    {0x08, 0xa0},
    {0x09, 0xe8},

    {0x10, 0x00},
    {0x11, 0x11},
    {0x12, 0x10},
    // {0x13, 0x80},
    {0x13, 0xB0}, //提高显示的亮度.
    {0x15, 0xfc},
    {0x18, 0x03},
    {0x21, 0xc0},
    {0x22, 0x60},
    {0x23, 0x30},
    {0x25, 0x00},
    {0x24, 0x14},
    //////////////////////////////////////
    ////////////LSC//////////////////////
    //////////////////////////////////////
    // gc0328 Alight lsc reg setting list
    ////Record date: 2013-04-01 15:59:05
    {0xfe, 0x01},
    {0xc0, 0x0d},
    {0xc1, 0x05},
    {0xc2, 0x00},
    {0xc6, 0x07},
    {0xc7, 0x03},
    {0xc8, 0x01},
    {0xba, 0x19},
    {0xbb, 0x10},
    {0xbc, 0x0a},
    {0xb4, 0x19},
    {0xb5, 0x0d},
    {0xb6, 0x09},
    {0xc3, 0x00},
    {0xc4, 0x00},
    {0xc5, 0x0e},
    {0xc9, 0x00},
    {0xca, 0x00},
    {0xcb, 0x00},
    {0xbd, 0x07},
    {0xbe, 0x00},
    {0xbf, 0x0e},
    {0xb7, 0x09},
    {0xb8, 0x00},
    {0xb9, 0x0d},
    {0xa8, 0x01},
    {0xa9, 0x00},
    {0xaa, 0x03},
    {0xab, 0x02},
    {0xac, 0x05},
    {0xad, 0x0c},
    {0xae, 0x03},
    {0xaf, 0x00},
    {0xb0, 0x04},
    {0xb1, 0x04},
    {0xb2, 0x03},
    {0xb3, 0x08},
    {0xa4, 0x00},
    {0xa5, 0x00},
    {0xa6, 0x00},
    {0xa7, 0x00},
    {0xa1, 0x3c},
    {0xa2, 0x50},
    {0xfe, 0x00},
    /// cct
    {0xB1, 0x02},
    {0xB2, 0x02},
    {0xB3, 0x07},
    {0xB4, 0xf0},
    {0xB5, 0x05},
    {0xB6, 0xf0},

    {0xfe, 0x00},
    {0x27, 0xf7},
    {0x28, 0x7F},
    {0x29, 0x20},
    {0x33, 0x20},
    {0x34, 0x20},
    {0x35, 0x20},
    {0x36, 0x20},
    {0x32, 0x08},

    {0x47, 0x00},
    {0x48, 0x00},

    {0xfe, 0x01},
    {0x79, 0x00},
    {0x7d, 0x00},
    {0x50, 0x88},
    {0x5b, 0x04},
    {0x76, 0x8f},
    {0x80, 0x70},
    {0x81, 0x70},
    {0x82, 0xb0},
    {0x70, 0xff},
    {0x71, 0x00},
    {0x72, 0x10},
    {0x73, 0x40},
    {0x74, 0x40},

    {0xfe, 0x00},
    {0x70, 0x45},
    {0x4f, 0x01},
    {0xf1, 0x07},

    {0xf2, 0x01},
    //////////// Set Frame Rate /////////////
    {0xfe, 0x00},
    {0x05, 0x02},
    {0x06, 0x2c}, // HB
    {0x07, 0x00},
    {0x08, 0x88}, // VB
    {0xfe, 0x01},
    {0x29, 0x00},
    {0x2a, 0x4e}, // step
    {0x2b, 0x02}, //high 4bit
    {0x2c, 0x70}, //low 8bit
    {0x2d, 0x03},
    {0x2e, 0x0c},
    {0x2f, 0x05},
    {0x30, 0x00},
    {0x31, 0x0f},    //夜间主循环7帧 //高位是4bit，即最大设为0x0f
    {0x32, 0x00},   //low 8bit
    {0x33, 0x30},    //4档曝光

    //////////// Set Window /////////////
    //{0xfe, 0x00},
    //{0x59, 0x22}, // subsampleratio=2
    //{0x5a, 0x03},
    //{0x5b, 0x00},
    //{0x5c, 0x00},
    //{0x5d, 0x00},
    //{0x5e, 0x00},
    //{0x5f, 0x00},
    //{0x60, 0x00},
    //{0x61, 0x00},
    //{0x62, 0x00},
    //{0x50, 0x01}, // crop 320x240 //
    //{0x51, 0x00},
    //{0x52, 0x00},
    //{0x53, 0x00},
    //{0x54, 0x00},
    //{0x55, 0x00},
    //{0x56, 0xf0}, // 240
    //{0x57, 0x01},
    //{0x58, 0x40}, // 320

    /* 640*480 */
    {0xfe, 0x00},
    {0x4b, 0x8b},
    {0x50, 0x01},
    {0x51, 0x00},
    {0x52, 0x00},
    {0x53, 0x00},
    {0x54, 0x00},
    {0x55, 0x01},
    {0x56, 0xe0},
    {0x57, 0x02},
    {0x58, 0x80},
    {0x59, 0x11},
    {0x5a, 0x02},
    {0x5b, 0x00},
    {0x5c, 0x00},
    {0x5d, 0x00},
    {0x5e, 0x00},
    {0x5f, 0x00},
    {0x60, 0x00},
    {0x61, 0x00},
    {0x62, 0x00},

    {0xFE, 0x00},
    {0xF1, 0x00},
    {0xF2, 0x00},

    {0x00, 0x00},
    
    {0xFE, 0x00},
    {0xF1, 0x07},
    {0xF2, 0x01},
};


/*@} end of group GC0328_Private_Variables */

/** @defgroup  GC0328_Global_Variables
 *  @{
 */

/*@} end of group GC0328_Global_Variables */

/** @defgroup  GC0328_Private_Fun_Declaration
 *  @{
 */
static void CAM_GPIO_Init(void);
static void I2C_GPIO_Init(void);
static BL_Err_Type CAM_Write_Byte8(uint8_t data, uint8_t cmd);
static uint8_t CAM_Read_Byte8(uint8_t cmd);
static BL_Err_Type CAM_Read_ID(void);
static BL_Err_Type CAM_Reg_Config(void);

/*@} end of group GC0328_Private_Fun_Declaration */

/** @defgroup  GC0328_Private_Functions
 *  @{
 */

/****************************************************************************/ /**
 * @brief  CAMERA GPIO Initialization
 *
 * @param  None
 *
 * @return None
 *
*******************************************************************************/
static void CAM_GPIO_Init(void)
{
    GLB_GPIO_Cfg_Type cfg;
    GLB_GPIO_Type gpioPins[] = {GLB_GPIO_PIN_16,GLB_GPIO_PIN_17,GLB_GPIO_PIN_18,GLB_GPIO_PIN_19,GLB_GPIO_PIN_24,GLB_GPIO_PIN_25,
                                GLB_GPIO_PIN_26,GLB_GPIO_PIN_27,GLB_GPIO_PIN_28,GLB_GPIO_PIN_29,GLB_GPIO_PIN_30,GLB_GPIO_PIN_31,
                                GLB_GPIO_PIN_32};

    GLB_GPIO_Func_Init(GPIO_FUN_CAM,gpioPins,sizeof(gpioPins)/sizeof(gpioPins[0]));

    // REFCLK
    cfg.gpioPin=GLB_GPIO_PIN_33;
    cfg.gpioFun=GPIO_FUN_CLOCK_OUT;
    GLB_GPIO_Init(&cfg);

    cfg.gpioMode=GPIO_MODE_OUTPUT;
    cfg.pullType=GPIO_PULL_UP;
    // PWDN
    cfg.gpioPin=GLB_GPIO_PIN_8;
    cfg.gpioFun=GPIO_FUN_GPIO;
    GLB_GPIO_Init(&cfg);
    GLB_GPIO_Write(GLB_GPIO_PIN_8, 0);
    GLB_GPIO_Output_Enable(GLB_GPIO_PIN_8);

    // RESETB
    cfg.gpioPin=GLB_GPIO_PIN_20;
    cfg.gpioFun=GPIO_FUN_GPIO;
    GLB_GPIO_Init(&cfg);
    GLB_GPIO_Write(GLB_GPIO_PIN_20, 1);
    GLB_GPIO_Output_Enable(GLB_GPIO_PIN_20);

    //GLB_Set_ClkOutSel(1, 0); // select dvp_ref_clk pin as cam_ref_clk
    GLB_Set_CAM_CLK(1, 1, 3); // select cam_ref_clk as 96M / 4 = 24M
}

/****************************************************************************/ /**
 * @brief  Dump image sensor register while is set
 *
 * @param  None
 *
 * @return None
 *
*******************************************************************************/
void GC0328_Dump_Register(void)
{
    uint32_t i;

    for (i = 0; i < sizeof(sensorRegList) / sizeof(sensorRegList[0]); i++) {
        MSG("reg[%02x]: %02x\n", sensorRegList[i][0], CAM_Read_Byte8(sensorRegList[i][0]));
    }
}

/****************************************************************************/ /**
 * @brief  I2C GPIO Initialization
 *
 * @param  None
 *
 * @return None
 *
*******************************************************************************/
static void I2C_GPIO_Init(void)
{
    GLB_GPIO_Type gpioPins[] = {GLB_GPIO_PIN_22,GLB_GPIO_PIN_23};

    GLB_GPIO_Func_Init(GPIO_FUN_I2C2,gpioPins,sizeof(gpioPins)/sizeof(gpioPins[0]));
}

/****************************************************************************/ /**
 * @brief  I2C CAMERA Write 8 bits
 *
 * @param  cmd: Reg Addr
 * @param  data: Reg Value
 *
 * @return SUCCESS or ERROR
 *
*******************************************************************************/
static BL_Err_Type CAM_Write_Byte8(uint8_t cmd, uint8_t data)
{
    uint8_t temp = data;
    I2C_Transfer_Cfg tranCfg;

    tranCfg.slaveAddr = I2C_CAMERA_ADDR;
    tranCfg.slaveAddr10Bit = DISABLE;
    tranCfg.stopEveryByte = DISABLE;
    tranCfg.subAddrSize = 1;
    tranCfg.subAddr = cmd;
    tranCfg.dataSize = 1;
    tranCfg.data = &temp;
    return I2C_MasterSendBlocking(I2C0_MM_ID, &tranCfg);
}

/****************************************************************************/ /**
 * @brief  I2C CAMERA Read 8 bits
 *
 * @param  cmd: Reg Addr
 *
 * @return Reg Value
 *
*******************************************************************************/
static uint8_t CAM_Read_Byte8(uint8_t cmd)
{
    uint8_t temp = 0;
    I2C_Transfer_Cfg tranCfg;

    tranCfg.slaveAddr = I2C_CAMERA_ADDR;
    tranCfg.slaveAddr10Bit = DISABLE;
    tranCfg.stopEveryByte = DISABLE;
    tranCfg.subAddrSize = 1;
    tranCfg.subAddr = cmd;
    tranCfg.dataSize = 1;
    tranCfg.data = &temp;
    I2C_MasterReceiveBlocking(I2C0_MM_ID, &tranCfg);

    return temp;
}

/****************************************************************************/ /**
 * @brief  CAMERA Read ID
 *
 * @param  None
 *
 * @return SUCCESS or ERROR
 *
*******************************************************************************/
static BL_Err_Type CAM_Read_ID(void)
{
    uint8_t sensorId = 0;

    sensorId = CAM_Read_Byte8(SENSOR_ID_REG);

    if (sensorId == SENSOR_ID_VAL) {
        MSG("ID correct\n");
        return SUCCESS;
    } else {
        MSG("ID error: %02x\n",sensorId);
        return ERROR;
    }
}

/****************************************************************************/ /**
 * @brief  CAMERA Reg Config
 *
 * @param  None
 *
 * @return SUCCESS or ERROR
 *
*******************************************************************************/
static BL_Err_Type CAM_Reg_Config(void)
{
    int i;

    for (i = 0; i < sizeof(sensorRegList) / sizeof(sensorRegList[0]); i++) {
        if (CAM_Write_Byte8(sensorRegList[i][0], sensorRegList[i][1]) != SUCCESS) {
            return ERROR;
        }

        bflb_platform_delay_ms(1);
    }

    return SUCCESS;
}

/*@} end of group GC0328_Private_Functions */

/** @defgroup  GC0328_Public_Functions
 *  @{
 */

/****************************************************************************/ /**
 * @brief  Image sensor initialization
 *
 * @param  camId: DVP2BUS ID
 * @param  mjpegEn: Enable or disable mjpeg
 * @param  camCfg: CAM configuration structure pointer
 * @param  mjpegCfg: MJPEG configuration structure pointer
 *
 * @return SUCCESS or ERROR
 *
*******************************************************************************/
BL_Err_Type GC0328_Init(CAM_ID_Type camId, BL_Fun_Type mjpegEn, CAM_CFG_Type *camCfg, MJPEG_CFG_Type *mjpegCfg)
{
    /* Gpio init */
    CAM_GPIO_Init();
    I2C_GPIO_Init();

    bflb_platform_delay_ms(1);

    if (CAM_Read_ID() != SUCCESS) {
        return ERROR;
    }

    if (CAM_Reg_Config() != SUCCESS) {
        return ERROR;
    }

    CAM_Disable(camId);
    MJPEG_Disable();

    if (mjpegEn) {
        CAM_Init(camId, camCfg);
        MJPEG_Init(mjpegCfg);
        MJPEG_Enable();
    } else {
        CAM_Init(camId, camCfg);
    }

    return SUCCESS;
}

/****************************************************************************/ /**
 * @brief  Get one camera frame
 *
 * @param  camId: DVP2BUS ID
 * @param  pic: Pointer of picture start address
 * @param  len: Length of picture
 *
 * @return SUCCESS or ERROR
 *
*******************************************************************************/
BL_Err_Type GC0328_CAM_Get(CAM_ID_Type camId, uint8_t **pic, uint32_t *len)
{
    CAM_Frame_Info info;
    ARCH_MemSet(&info, 0, sizeof(info));

    CAM_Get_Frame_Info(camId, &info);

    if (info.validFrames == 0) {
        return ERROR;
    }

    *pic = (uint8_t *)(uintptr_t)(info.curFrameAddr);
    *len = info.curFrameBytes;

    return SUCCESS;
}

/****************************************************************************/ /**
 * @brief  Get available count of camera frames
 *
 * @param  camId: DVP2BUS ID
 *
 * @return Frames count
 *
*******************************************************************************/
uint8_t GC0328_CAM_Frame_Count(CAM_ID_Type camId)
{
    return CAM_Get_Frame_Count(camId);
}

/****************************************************************************/ /**
 * @brief  Pop one camera frame
 *
 * @param  camId: DVP2BUS ID
 *
 * @return None
 *
*******************************************************************************/
void GC0328_CAM_Release(CAM_ID_Type camId)
{
    CAM_Pop_Frame(camId);
}

/****************************************************************************/ /**
 * @brief  Open camera function
 *
 * @param  camId: DVP2BUS ID
 *
 * @return None
 *
*******************************************************************************/
void GC0328_CAM_Open(CAM_ID_Type camId)
{
    CAM_Enable(camId);
}

/****************************************************************************/ /**
 * @brief  Close camera function
 *
 * @param  camId: DVP2BUS ID
 *
 * @return None
 *
*******************************************************************************/
void GC0328_CAM_Close(CAM_ID_Type camId)
{
    CAM_Disable(camId);
}

/****************************************************************************/ /**
 * @brief  Deinit camera module
 *
 * @param  camId: DVP2BUS ID
 *
 * @return None
 *
*******************************************************************************/
void GC0328_CAM_Deinit(CAM_ID_Type camId)
{
    CAM_Deinit(camId);
}

/****************************************************************************/ /**
 * @brief  Get one mjpeg frame
 *
 * @param  pic: Pointer of picture start address
 * @param  len: Length of picture
 *
 * @return SUCCESS or ERROR
 *
*******************************************************************************/
BL_Err_Type GC0328_MJPEG_Get(uint8_t **pic, uint32_t *len)
{
    MJPEG_Frame_Info info;
    ARCH_MemSet(&info, 0, sizeof(info));

    MJPEG_Get_Frame_Info(&info);

    if (info.validFrames == 0) {
        return ERROR;
    }

    *pic = (uint8_t *)(uintptr_t)(info.curFrameAddr);
    *len = info.curFrameBytes;

    return SUCCESS;
}

/****************************************************************************/ /**
 * @brief  Get available count of mjpeg frames
 *
 * @param  None
 *
 * @return Frames count
 *
*******************************************************************************/
uint8_t GC0328_MJPEG_Frame_Count(void)
{
    return MJPEG_Get_Frame_Count();
}

/****************************************************************************/ /**
 * @brief  Pop one mjpeg frame
 *
 * @param  None
 *
 * @return None
 *
*******************************************************************************/
void GC0328_MJPEG_Release(void)
{
    MJPEG_Pop_Frame();
}

/****************************************************************************/ /**
 * @brief  Open mjpeg function
 *
 * @param  None
 *
 * @return None
 *
*******************************************************************************/
void GC0328_MJPEG_Open(void)
{
    MJPEG_Enable();
}

/****************************************************************************/ /**
 * @brief  Close mjpeg function
 *
 * @param  None
 *
 * @return None
 *
*******************************************************************************/
void GC0328_MJPEG_Close(void)
{
    MJPEG_Disable();
}

/****************************************************************************/ /**
 * @brief  Deinit mjpeg module
 *
 * @param  None
 *
 * @return None
 *
*******************************************************************************/
void GC0328_MJPEG_Deinit(void)
{
    MJPEG_Deinit();
}

/*@} end of group GC0328_Public_Functions */

/*@} end of group GC0328 */

/*@} end of group GC0328_Driver */
