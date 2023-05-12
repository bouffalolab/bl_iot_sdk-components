/**
 * @file bl702l_pwm_sc.h
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

#ifndef __BL702L_PWM_SC_H__
#define __BL702L_PWM_SC_H__

#include "pwm_reg.h"
#include "bl702l_common.h"

/** @addtogroup  BL702_Peripheral_Driver
 *  @{
 */

/** @addtogroup  PWM_SC
 *  @{
 */

/** @defgroup  PWM_SC_Public_Types
 *  @{
 */

/**
 *  @brief PWM_SC No. type definition
 */
typedef enum {
    PWM_SC0 = 0, /*!< PWM single Channel 0 define */
    PWM_SCx_MAX, /*!<  */
} PWM_SCx_Type;

/**
 *  @brief PWM Clock definition
 */
typedef enum {
    PWM_SC_CLK_XCLK = 0, /*!< PWM Clock source :XTAL CLK */
    PWM_SC_CLK_BCLK,     /*!< PWM Clock source :Bus CLK */
    PWM_SC_CLK_32K,      /*!< PWM Clock source :32K CLK */
} PWM_SC_Clk_Type;

/**
 *  @brief PWM Stop Mode definition
 */
typedef enum {
    PWM_SC_STOP_ABRUPT = 0, /*!< PWM stop abrupt select define */
    PWM_SC_STOP_GRACEFUL,   /*!< PWM stop graceful select define */
} PWM_SC_Stop_Mode_Type;

/**
 *  @brief PWM mode type def
 */
typedef enum {
    PWM_SC_POL_NORMAL = 0, /*!< PWM normal polarity mode define */
    PWM_SC_POL_INVERT,     /*!< PWM invert polarity mode define */
} PWM_SC_Polarity_Type;

/**
 *  @brief PWM interrupt type def
 */
typedef enum {
    PWM_INT_PULSE_CNT = 0, /*!< PWM Pulse count interrupt define */
    PWM_SC_INT_ALL,        /*!<  */
} PWM_SC_INT_Type;

/**
 *  @brief PWM configuration structure type definition
 */
typedef struct
{
    PWM_SCx_Type ch;                /*!< PWM channel */
    PWM_SC_Clk_Type clk;            /*!< PWM Clock */
    PWM_SC_Stop_Mode_Type stopMode; /*!< PWM Stop Mode */
    PWM_SC_Polarity_Type pol;       /*!< PWM mode type */
    uint16_t clkDiv;                /*!< PWM clkDiv num */
    uint16_t period;                /*!< PWM period set */
    uint16_t threshold1;            /*!< PWM threshold1 num */
    uint16_t threshold2;            /*!< PWM threshold2 num */
    uint16_t intPulseCnt;           /*!< PWM interrupt pulse count */
    BL_Fun_Type stpInt;             /*!< PWM Stop on int */
} PWM_SC_CFG_Type;

/*@} end of group PWM_Public_Types */

/** @defgroup  PWM_Public_Constants
 *  @{
 */

/** @defgroup  PWM_SCX_TYPE
 *  @{
 */
#define IS_PWM_SCX_TYPE(type) (((type) == PWM_SC0))

/** @defgroup  PWM_SC_CLK_TYPE
 *  @{
 */
#define IS_PWM_SC_CLK_TYPE(type) (((type) == PWM_SC_CLK_XCLK) || \
                                  ((type) == PWM_SC_CLK_BCLK) || \
                                  ((type) == PWM_SC_CLK_32K))

/** @defgroup  PWM_SC_STOP_MODE_TYPE
 *  @{
 */
#define IS_PWM_SC_STOP_MODE_TYPE(type) (((type) == PWM_SC_STOP_ABRUPT) || \
                                        ((type) == PWM_SC_STOP_GRACEFUL))

/** @defgroup  PWM_SC_POLARITY_TYPE
 *  @{
 */
#define IS_PWM_SC_POLARITY_TYPE(type) (((type) == PWM_SC_POL_NORMAL) || \
                                       ((type) == PWM_SC_POL_INVERT))

/** @defgroup  PWM_INT_TYPE
 *  @{
 */
#define IS_PWM_SC_INT_TYPE(type) (((type) == PWM_INT_PULSE_CNT) || \
                                  ((type) == PWM_SC_INT_ALL))

/*@} end of group PWM_Public_Constants */

/** @defgroup  PWM_Public_Macros
 *  @{
 */
#define IS_PWM_SCX(SCx) ((SCx) < PWM_SCx_MAX)

#define PWM_SCX_CHANNEL_OFFSET (0x40)

/*@} end of group PWM_Public_Macros */

/** @defgroup  PWM_Public_Functions
 *  @{
 */

/**
 *  @brief PWM Functions
 */
#ifndef BFLB_USE_HAL_DRIVER
void PWM_SC_IRQHandler(void);
#endif
BL_Err_Type PWM_SC_Channel_Init(PWM_SC_CFG_Type *chCfg);
void PWM_SC_Channel_Update(PWM_SCx_Type ch, uint16_t period, uint16_t threshold1, uint16_t threshold2);
void PWM_SC_Channel_Set_Div(PWM_SCx_Type ch, uint16_t div);
void PWM_SC_Channel_Set_Threshold1(PWM_SCx_Type ch, uint16_t threshold1);
void PWM_SC_Channel_Set_Threshold2(PWM_SCx_Type ch, uint16_t threshold2);
void PWM_SC_Channel_Set_Period(PWM_SCx_Type ch, uint16_t period);
void PWM_SC_Channel_Get(PWM_SCx_Type ch, uint16_t *period, uint16_t *threshold1, uint16_t *threshold2);
void PWM_SC_IntMask(PWM_SCx_Type ch, PWM_SC_INT_Type intType, BL_Mask_Type intMask);
void PWM_SC_Channel_Enable(PWM_SCx_Type ch);
void PWM_SC_Channel_Disable(PWM_SCx_Type ch);
void PWM_SC_SW_Mode(PWM_SCx_Type ch, BL_Fun_Type enable);
void PWM_SC_SW_Force_Value(PWM_SCx_Type ch, uint8_t value);
// void PWM_Int_Callback_Install(PWM_SCx_Type ch, uint32_t intType, intCallback_Type *cbFun);
BL_Err_Type PWM_SC_Smart_Configure(PWM_SCx_Type ch, uint32_t frequency, uint8_t dutyCycle);

/*@} end of group PWM_Public_Functions */

/*@} end of group PWM */

/*@} end of group BL702_Peripheral_Driver */

#endif /* __BL702L_PWM_SC_H__ */