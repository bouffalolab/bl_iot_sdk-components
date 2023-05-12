/**
  ******************************************************************************
  * @file    bl702l_ir.c
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

#include "bl702l_ir.h"
#include "bl702l_glb.h"

/** @addtogroup  BL702L_Peripheral_Driver
 *  @{
 */

/** @addtogroup  IR
 *  @{
 */

/** @defgroup  IR_Private_Macros
 *  @{
 */
#define NEC_HEAD_H_MIN          17000
#define NEC_HEAD_H_MAX          19000
#define NEC_HEAD_L_MIN          8400
#define NEC_HEAD_L_MAX          9600
#define NEC_BIT0_H_MIN          525
#define NEC_BIT0_H_MAX          1725
#define RC5_ONE_PLUSE_MIN       1175
#define RC5_ONE_PLUSE_MAX       2375
#define RC5_TWO_PLUSE_MIN       2955
#define RC5_TWO_PLUSE_MAX       4155
#define IR_TX_INT_TIMEOUT_COUNT (100 * 320 * 1000)
#define DIVIDE_ROUND(a, b)      ((2 * a + b) / (2 * b))

/*@} end of group IR_Private_Macros */

/** @defgroup  IR_Private_Types
 *  @{
 */

/*@} end of group IR_Private_Types */

/** @defgroup  IR_Private_Variables
 *  @{
 */
#ifndef BFLB_USE_HAL_DRIVER
static intCallback_Type *irIntCbfArra[IR_INT_ALL] = { NULL };
#endif

/*@} end of group IR_Private_Variables */

/** @defgroup  IR_Global_Variables
 *  @{
 */

/*@} end of group IR_Global_Variables */

/** @defgroup  IR_Private_Fun_Declaration
 *  @{
 */

/*@} end of group IR_Private_Fun_Declaration */

/** @defgroup  IR_Private_Functions
 *  @{
 */

/*@} end of group IR_Private_Functions */

/** @defgroup  IR_Public_Functions
 *  @{
 */

/****************************************************************************/ /**
 * @brief  IR TX IRQ handler function
 *
 * @param  None
 *
 * @return None
 *
*******************************************************************************/
#ifndef BFLB_USE_HAL_DRIVER
void IRTX_IRQHandler(void)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(IR_BASE, IRTX_INT_STS);

    if (BL_IS_REG_BIT_SET(tmpVal, IRTX_END_INT) && !BL_IS_REG_BIT_SET(tmpVal, IR_CR_IRTX_END_MASK)) {
        BL_WR_REG(IR_BASE, IRTX_INT_STS, BL_SET_REG_BIT(tmpVal, IR_CR_IRTX_END_CLR));

        if (irIntCbfArra[IR_INT_TX_END] != NULL) {
            irIntCbfArra[IR_INT_TX_END]();
        }
    }

    if (BL_IS_REG_BIT_SET(tmpVal, IRTX_FRDY_INT) && !BL_IS_REG_BIT_SET(tmpVal, IR_CR_IRTX_FRDY_MASK)) {
        if (irIntCbfArra[IR_INT_TX_FIFO_REQ] != NULL) {
            irIntCbfArra[IR_INT_TX_FIFO_REQ]();
        }
    }

    if (BL_IS_REG_BIT_SET(tmpVal, IRTX_FER_INT) && !BL_IS_REG_BIT_SET(tmpVal, IR_CR_IRTX_FER_MASK)) {
        if (irIntCbfArra[IR_INT_TX_FIFO_ERROR] != NULL) {
            irIntCbfArra[IR_INT_TX_FIFO_ERROR]();
        }
    }
}
#endif

/****************************************************************************/ /**
 * @brief  IR tx initialization function
 *
 * @param  irTxCfg: IR tx configuration structure pointer
 *
 * @return SUCCESS
 *
*******************************************************************************/
BL_Err_Type IR_TxInit(IR_TxCfg_Type *irTxCfg)
{
    uint32_t tmpVal;

    /* Disable clock gate */
    GLB_PER_Clock_UnGate(GLB_AHB_CLOCK_IR);

    tmpVal = BL_RD_REG(IR_BASE, IRTX_CONFIG);
    /* Set data bit */
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, IR_CR_IRTX_DATA_NUM, irTxCfg->dataBits - 1);
    /* Set tail pulse */
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, IR_CR_IRTX_TAIL_HL_INV, irTxCfg->tailPulseInverse);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, IR_CR_IRTX_TAIL_EN, irTxCfg->tailPulse);
    /* Set head pulse */
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, IR_CR_IRTX_HEAD_HL_INV, irTxCfg->headPulseInverse);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, IR_CR_IRTX_HEAD_EN, irTxCfg->headPulse);
    /* Enable or disable logic 1 and 0 pulse inverse */
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, IR_CR_IRTX_LOGIC1_HL_INV, irTxCfg->logic1PulseInverse);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, IR_CR_IRTX_LOGIC0_HL_INV, irTxCfg->logic0PulseInverse);
    /* Enable or disable data pulse */
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, IR_CR_IRTX_DATA_EN, irTxCfg->dataPulse);
    /* Enable or disable output modulation */
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, IR_CR_IRTX_MOD_EN, irTxCfg->outputModulation);
    /* Enable or disable output inverse */
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, IR_CR_IRTX_OUT_INV, irTxCfg->outputInverse);

    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, IR_CR_IRTX_FRM_EN, irTxCfg->freerunEnable);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, IR_CR_IRTX_FRM_CONT_EN, irTxCfg->continueEnable);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, IR_CR_IRTX_FRM_FRAME_SIZE, irTxCfg->frameSize);

    /* Write back */
    BL_WR_REG(IR_BASE, IRTX_CONFIG, tmpVal);

#ifndef BFLB_USE_HAL_DRIVER
    Interrupt_Handler_Register(IRTX_IRQn, IRTX_IRQHandler);
#endif
    return SUCCESS;
}

/****************************************************************************/ /**
 * @brief  IR tx pulse width configure function
 *
 * @param  irTxPulseWidthCfg: IR tx pulse width configuration structure pointer
 *
 * @return SUCCESS
 *
*******************************************************************************/
BL_Err_Type IR_TxPulseWidthConfig(IR_TxPulseWidthCfg_Type *irTxPulseWidthCfg)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(IR_BASE, IRTX_PW_0);
    /* Set logic 0 pulse phase 0 width */
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, IR_CR_IRTX_LOGIC0_PH0_W, irTxPulseWidthCfg->logic0PulseWidth_0 - 1);
    /* Set logic 0 pulse phase 1 width */
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, IR_CR_IRTX_LOGIC0_PH1_W, irTxPulseWidthCfg->logic0PulseWidth_1 - 1);
    /* Set logic 1 pulse phase 0 width */
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, IR_CR_IRTX_LOGIC1_PH0_W, irTxPulseWidthCfg->logic1PulseWidth_0 - 1);
    /* Set logic 1 pulse phase 1 width */
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, IR_CR_IRTX_LOGIC1_PH1_W, irTxPulseWidthCfg->logic1PulseWidth_1 - 1);
    BL_WR_REG(IR_BASE, IRTX_PW_0, tmpVal);

    tmpVal = BL_RD_REG(IR_BASE, IRTX_PW_1);
    /* Set head pulse phase 0 width */
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, IR_CR_IRTX_HEAD_PH0_W, irTxPulseWidthCfg->headPulseWidth_0 - 1);
    /* Set head pulse phase 1 width */
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, IR_CR_IRTX_HEAD_PH1_W, irTxPulseWidthCfg->headPulseWidth_1 - 1);
    /* Set tail pulse phase 0 width */
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, IR_CR_IRTX_TAIL_PH0_W, irTxPulseWidthCfg->tailPulseWidth_0 - 1);
    /* Set tail pulse phase 1 width */
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, IR_CR_IRTX_TAIL_PH1_W, irTxPulseWidthCfg->tailPulseWidth_1 - 1);
    BL_WR_REG(IR_BASE, IRTX_PW_1, tmpVal);

    tmpVal = BL_RD_REG(IR_BASE, IRTX_PULSE_WIDTH);
    /* Set modulation phase 0 width */
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, IR_CR_IRTX_MOD_PH0_W, irTxPulseWidthCfg->moduWidth_0 - 1);
    /* Set modulation phase 1 width */
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, IR_CR_IRTX_MOD_PH1_W, irTxPulseWidthCfg->moduWidth_1 - 1);
    /* Set pulse width unit */
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, IR_CR_IRTX_PW_UNIT, irTxPulseWidthCfg->pulseWidthUnit - 1);
    BL_WR_REG(IR_BASE, IRTX_PULSE_WIDTH, tmpVal);

    return SUCCESS;
}

/****************************************************************************/ /**
 * @brief  IR configure fifo function
 *
 * @param  fifoCfg: FIFO configuration structure pointer
 *
 * @return SUCCESS
 *
*******************************************************************************/
BL_Err_Type IR_FifoConfig(IR_FifoCfg_Type *fifoCfg)
{
    uint32_t tmpVal;

    /* Set fifo threshold value */
    tmpVal = BL_RD_REG(IR_BASE, IR_FIFO_CONFIG_1);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, IR_TX_FIFO_TH, fifoCfg->txFifoThreshold);
    BL_WR_REG(IR_BASE, IR_FIFO_CONFIG_1, tmpVal);

    /* Enable or disable dma function */
    tmpVal = BL_RD_REG(IR_BASE, IR_FIFO_CONFIG_0);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, IRTX_DMA_EN, fifoCfg->txFifoDmaEnable);
    BL_WR_REG(IR_BASE, IR_FIFO_CONFIG_0, tmpVal);

    return SUCCESS;
}

/****************************************************************************/ /**
 * @brief  IR set default value of all registers function
 *
 * @param  None
 *
 * @return SUCCESS
 *
*******************************************************************************/
BL_Err_Type IR_DeInit(void)
{
    GLB_AHB_MCU_Software_Reset(GLB_AHB_MCU_SW_IR_REMOTE);

    return SUCCESS;
}

/****************************************************************************/ /**
 * @brief  IR enable function
 *
 * @param  direct: IR direction type
 *
 * @return SUCCESS
 *
*******************************************************************************/
BL_Err_Type IR_Enable(IR_Direction_Type direct)
{
    uint32_t tmpVal;

    /* Check the parameters */
    CHECK_PARAM(IS_IR_DIRECTION_TYPE(direct));

    if (direct == IR_TX ) {
        /* Enable ir tx unit */
        tmpVal = BL_RD_REG(IR_BASE, IRTX_CONFIG);
        BL_WR_REG(IR_BASE, IRTX_CONFIG, BL_SET_REG_BIT(tmpVal, IR_CR_IRTX_EN));
    }

    return SUCCESS;
}

/****************************************************************************/ /**
 * @brief  IR disable function
 *
 * @param  direct: IR direction type
 *
 * @return SUCCESS
 *
*******************************************************************************/
BL_Err_Type IR_Disable(IR_Direction_Type direct)
{
    uint32_t tmpVal;

    /* Check the parameters */
    CHECK_PARAM(IS_IR_DIRECTION_TYPE(direct));

    if (direct == IR_TX ) {
        /* Disable ir tx unit */
        tmpVal = BL_RD_REG(IR_BASE, IRTX_CONFIG);
        BL_WR_REG(IR_BASE, IRTX_CONFIG, BL_CLR_REG_BIT(tmpVal, IR_CR_IRTX_EN));
    }

    return SUCCESS;
}

/****************************************************************************/ /**
 * @brief  IR tx software mode enable or disable function
 *
 * @param  txSWM: Enable or disable
 *
 * @return SUCCESS
 *
*******************************************************************************/
BL_Err_Type IR_TxSWM(BL_Fun_Type txSWM)
{
    uint32_t tmpVal;

    /* Enable or disable tx swm */
    tmpVal = BL_RD_REG(IR_BASE, IRTX_CONFIG);

    if (ENABLE == txSWM) {
        BL_WR_REG(IR_BASE, IRTX_CONFIG, BL_SET_REG_BIT(tmpVal, IR_CR_IRTX_SWM_EN));
    } else {
        BL_WR_REG(IR_BASE, IRTX_CONFIG, BL_CLR_REG_BIT(tmpVal, IR_CR_IRTX_SWM_EN));
    }

    return SUCCESS;
}

/****************************************************************************/ /**
 * @brief  IR clear tx fifo function
 *
 * @param  None
 *
 * @return SUCCESS
 *
*******************************************************************************/
BL_Err_Type IR_TxFifoClear(void)
{
    uint32_t tmpVal;

    /* Clear tx fifo */
    tmpVal = BL_RD_REG(IR_BASE, IR_FIFO_CONFIG_0);
    BL_WR_REG(IR_BASE, IR_FIFO_CONFIG_0, BL_SET_REG_BIT(tmpVal, IR_TX_FIFO_CLR));

    return SUCCESS;
}

/****************************************************************************/ /**
 * @brief  IR send data function
 *
 * @param  data: data to send
 * @param  length: Length of send buffer
 *
 * @return SUCCESS
 *
*******************************************************************************/
BL_Err_Type IR_SendData(uint32_t *data, uint8_t length)
{
    uint32_t txLen = 0;
    uint32_t timeoutCnt = IR_TX_INT_TIMEOUT_COUNT;

    while (txLen < length) {
        if (IR_GetTxFifoCount() > 0) {
            BL_WR_REG(IR_BASE, IR_FIFO_WDATA, data[txLen++]);
            timeoutCnt = IR_TX_INT_TIMEOUT_COUNT;
        } else {
            timeoutCnt--;

            if (timeoutCnt == 0) {
                return TIMEOUT;
            }
        }
    }

    return SUCCESS;
}

/****************************************************************************/ /**
 * @brief  IR software mode send pulse width data function
 *
 * @param  data: data to send
 * @param  length: Length of send buffer
 *
 * @return SUCCESS
 *
*******************************************************************************/
BL_Err_Type IR_SWMSendData(uint16_t *data, uint8_t length)
{
    uint8_t i = 0, j = 0;
    uint16_t minData = data[0];
    uint32_t tmpVal;
    uint32_t count = (length + 3) / 4;
    uint32_t pwVal = 0;

    /* Search for min value */
    for (i = 1; i < length; i++) {
        if (minData > data[i] && data[i] != 0) {
            minData = data[i];
        }
    }

    /* Set min value as pulse width unit */
    tmpVal = BL_RD_REG(IR_BASE, IRTX_PULSE_WIDTH);
    BL_WR_REG(IR_BASE, IRTX_PULSE_WIDTH, BL_SET_REG_BITS_VAL(tmpVal, IR_CR_IRTX_PW_UNIT, minData));

    /* Calculate tx SWM pulse width data as multiples of pulse width unit */
    for (i = 0; i < count; i++) {
        pwVal = 0;

        if (i < count - 1) {
            /* Put every four pulse width together as a 32-bit value to tx fifo */
            for (j = 0; j < 4; j++) {
                /* Every pulse width divided by pulse width unit */
                tmpVal = (DIVIDE_ROUND(data[j + i * 4], minData) - 1) & 0xff;
                /* Tx fifo 32-bit value: pwVal[7:0]:first pulse width, pwVal[15:8]:second pulse width... */
                pwVal |= tmpVal << (8 * j);
            }
        } else {
            /* Deal with pulse width data remained which is less than 4 */
            for (j = 0; j < length % 4; j++) {
                tmpVal = (DIVIDE_ROUND(data[j + i * 4], minData) - 1) & 0xff;
                pwVal |= tmpVal << (8 * j);
            }
        }

        /* Write to tx fifo */
        IR_SendData(&pwVal, 1);
    }

    return SUCCESS;
}

/****************************************************************************/ /**
 * @brief  IR send command function
 *
 * @param  data: data to send
 * @param  length: Length of send buffer
 *
 * @return SUCCESS
 *
*******************************************************************************/
BL_Err_Type IR_SendCommand(uint32_t *data, uint8_t length)
{
    uint32_t timeoutCnt = IR_TX_INT_TIMEOUT_COUNT;

    /* Write data */
    IR_SendData(data, length);

    /* Mask tx interrupt */
    IR_IntMask(IR_INT_TX_END, MASK);

    /* Clear tx interrupt */
    IR_ClrIntStatus(IR_INT_TX_END);

    /* Enable ir tx */
    IR_Enable(IR_TX);

    /* Wait for tx interrupt */
    while (SET != IR_GetIntStatus(IR_INT_TX_END)) {
        timeoutCnt--;

        if (timeoutCnt == 0) {
            IR_Disable(IR_TX);

            return TIMEOUT;
        }
    }

    /* Disable ir tx */
    IR_Disable(IR_TX);

    /* Clear tx interrupt */
    IR_ClrIntStatus(IR_INT_TX_END);

    return SUCCESS;
}

/****************************************************************************/ /**
 * @brief  IR send command in software mode function
 *
 * @param  data: IR fifo data to send
 * @param  length: Length of data
 *
 * @return SUCCESS
 *
*******************************************************************************/
BL_Err_Type IR_SWMSendCommand(uint16_t *data, uint8_t length)
{
    uint32_t timeoutCnt = IR_TX_INT_TIMEOUT_COUNT;

    /* Mask tx interrupt */
    IR_IntMask(IR_INT_TX_END, MASK);

    /* Clear tx interrupt */
    IR_ClrIntStatus(IR_INT_TX_END);

    /* Enable ir tx */
    IR_Enable(IR_TX);

    /* Write fifo */
    IR_SWMSendData(data, length);

    /* Wait for tx interrupt */
    while (SET != IR_GetIntStatus(IR_INT_TX_END)) {
        timeoutCnt--;

        if (timeoutCnt == 0) {
            IR_Disable(IR_TX);

            return TIMEOUT;
        }
    }

    /* Disable ir tx */
    IR_Disable(IR_TX);

    /* Clear tx interrupt */
    IR_ClrIntStatus(IR_INT_TX_END);

    return SUCCESS;
}

/****************************************************************************/ /**
 * @brief  IR send in NEC protocol
 *
 * @param  address: Address
 * @param  command: Command
 *
 * @return SUCCESS
 *
*******************************************************************************/
BL_Err_Type IR_SendNEC(uint8_t address, uint8_t command)
{
    uint32_t tmpVal = ((~command & 0xff) << 24) + (command << 16) + ((~address & 0xff) << 8) + address;

    IR_SendCommand(&tmpVal, 1);

    return SUCCESS;
}

/****************************************************************************/ /**
 * @brief  IR interrupt mask or unmask function
 *
 * @param  intType: IR interrupt type
 * @param  intMask: Mask or unmask
 *
 * @return SUCCESS
 *
*******************************************************************************/
BL_Err_Type IR_IntMask(IR_INT_Type intType, BL_Mask_Type intMask)
{
    uint32_t tmpVal;

    /* Check the parameters */
    CHECK_PARAM(IS_IR_INT_TYPE(intType));

    if (intType < 3) {
        /* Mask or unmask tx interrupt */
        tmpVal = BL_RD_REG(IR_BASE, IRTX_INT_STS);

        if (MASK == intMask) {
            tmpVal |= 1 << (intType + 8);
        } else {
            tmpVal &= ~(1 << (intType + 8));
        }

        BL_WR_REG(IR_BASE, IRTX_INT_STS, tmpVal);
    } else {
        if (MASK == intMask) {
            tmpVal = BL_RD_REG(IR_BASE, IRTX_INT_STS);
            tmpVal |= 0x700;
            tmpVal = BL_RD_REG(IR_BASE, IRTX_INT_STS);
        } else {
            tmpVal = BL_RD_REG(IR_BASE, IRTX_INT_STS);
            tmpVal &= ~0x700;
            tmpVal = BL_RD_REG(IR_BASE, IRTX_INT_STS);
        }
    }

    return SUCCESS;
}

/****************************************************************************/ /**
 * @brief  Clear ir interrupt function
 *
 * @param  intType: IR interrupt type
 *
 * @return SUCCESS
 *
*******************************************************************************/
BL_Err_Type IR_ClrIntStatus(IR_INT_Type intType)
{
    uint32_t tmpVal;

    /* Check the parameters */
    CHECK_PARAM(IS_IR_INT_TYPE(intType));

    if (intType == IR_INT_TX_END || intType == IR_INT_ALL) {
        /* Clear tx interrupt */
        tmpVal = BL_RD_REG(IR_BASE, IRTX_INT_STS);
        BL_WR_REG(IR_BASE, IRTX_INT_STS, BL_SET_REG_BIT(tmpVal, IR_CR_IRTX_END_CLR));
    }

    return SUCCESS;
}

/****************************************************************************/ /**
 * @brief  IR install interrupt callback function
 *
 * @param  intType: IR interrupt type
 * @param  cbFun: Pointer to interrupt callback function. The type should be void (*fn)(void)
 *
 * @return SUCCESS
 *
*******************************************************************************/
#ifndef BFLB_USE_HAL_DRIVER
BL_Err_Type IR_Int_Callback_Install(IR_INT_Type intType, intCallback_Type *cbFun)
{
    /* Check the parameters */
    CHECK_PARAM(IS_IR_INT_TYPE(intType));

    irIntCbfArra[intType] = cbFun;

    return SUCCESS;
}
#endif

/****************************************************************************/ /**
 * @brief  IR get interrupt status function
 *
 * @param  intType: IR int type
 *
 * @return IR tx interrupt status
 *
*******************************************************************************/
BL_Sts_Type IR_GetIntStatus(IR_INT_Type intType)
{
    uint32_t tmpVal = 0;

    /* Check the parameters */
    CHECK_PARAM(IS_IR_INT_TYPE(intType));

    /* Read tx interrupt status */
    if (intType < 3) {
        tmpVal = BL_RD_REG(IR_BASE, IRTX_INT_STS);
        tmpVal &= 1 << intType;
    } else {
    }

    if (tmpVal) {
        return SET;
    } else {
        return RESET;
    }
}

/****************************************************************************/ /**
 * @brief  IR get tx fifo underflow or overflow status function
 *
 * @param  fifoSts: IR fifo status type
 *
 * @return IR fifo status
 *
*******************************************************************************/
BL_Sts_Type IR_GetFifoStatus(IR_FifoStatus_Type fifoSts)
{
    uint32_t tmpVal;

    /* Check the parameters */
    CHECK_PARAM(IS_IR_FIFOSTATUS_TYPE(fifoSts));

    /* Read tx fifo status */
    tmpVal = BL_RD_REG(IR_BASE, IR_FIFO_CONFIG_0);

    if ((tmpVal & (1U << (fifoSts + IR_TX_FIFO_OVERFLOW_POS))) != 0) {
        return SET;
    } else {
        return RESET;
    }
}

/****************************************************************************/ /**
 * @brief  IR get tx fifo count function
 *
 * @param  None
 *
 * @return IR tx fifo available count
 *
*******************************************************************************/
uint8_t IR_GetTxFifoCount(void)
{
    uint32_t tmpVal;

    /* Read tx fifo count */
    tmpVal = BL_RD_REG(IR_BASE, IR_FIFO_CONFIG_1);
    tmpVal = BL_GET_REG_BITS_VAL(tmpVal, IR_TX_FIFO_CNT);

    return tmpVal;
}

/****************************************************************************/ /**
 * @brief  IR init to control led function
 *
 * @param  clk: Clock source
 * @param  div: Clock division(1~64)
 * @param  unit: Pulse width unit(multiples of clock pulse width, 1~4096)
 * @param  code0H: code 0 high level time(multiples of pulse width unit, 1~16)
 * @param  code0L: code 0 low level time(multiples of pulse width unit, 1~16)
 * @param  code1H: code 1 high level time(multiples of pulse width unit, 1~16)
 * @param  code1L: code 1 low level time(multiples of pulse width unit, 1~16)
 *
 * @return SUCCESS
 *
*******************************************************************************/
BL_Err_Type IR_LEDInit(uint8_t clk, uint8_t div, uint8_t unit, uint8_t code0H, uint8_t code0L, uint8_t code1H, uint8_t code1L)
{
    IR_TxCfg_Type txCfg = {
        24,              /* 24-bit data */
        DISABLE,         /* Disable signal of tail pulse inverse */
        DISABLE,         /* Disable signal of tail pulse */
        DISABLE,         /* Disable signal of head pulse inverse */
        DISABLE,         /* Disable signal of head pulse */
        DISABLE,         /* Disable signal of logic 1 pulse inverse */
        DISABLE,         /* Disable signal of logic 0 pulse inverse */
        ENABLE,          /* Enable signal of data pulse */
        DISABLE,         /* Disable signal of output modulation */
        ENABLE,          /* Enable signal of output inverse */
        DISABLE,         /* Disable tx freerun mode */
        DISABLE,         /* Disable tx freerun continuous mode */
        IR_FRAME_SIZE_32 /* Frame size */
    };

    IR_TxPulseWidthCfg_Type txPWCfg = {
        code0L, /* Pulse width of logic 0 pulse phase 1 */
        code0H, /* Pulse width of logic 0 pulse phase 0 */
        code1L, /* Pulse width of logic 1 pulse phase 1 */
        code1H, /* Pulse width of logic 1 pulse phase 0 */
        1,      /* Pulse width of head pulse phase 1 */
        1,      /* Pulse width of head pulse phase 0 */
        1,      /* Pulse width of tail pulse phase 1 */
        1,      /* Pulse width of tail pulse phase 0 */
        1,      /* Modulation phase 1 width */
        1,      /* Modulation phase 0 width */
        unit    /* Pulse width unit */
    };

    // HBN_Set_MCU_XCLK_Sel(clk);
    GLB_Set_IR_CLK(ENABLE, GLB_IR_CLK_SRC_XCLK, div - 1);

    /* Disable ir before config */
    IR_Disable(IR_TX);

    /* IR tx init */
    IR_TxInit(&txCfg);
    IR_TxPulseWidthConfig(&txPWCfg);

    return SUCCESS;
}

/****************************************************************************/ /**
 * @brief  IR send 24-bit data to control led function
 *
 * @param  data: Data to send(24-bit)
 *
 * @return SUCCESS
 *
*******************************************************************************/
BL_Err_Type IR_LEDSend(uint32_t data)
{
    /* Change MSB_first to LSB_first */
    data = ((data >> 1) & 0x55555555) | ((data << 1) & 0xaaaaaaaa);
    data = ((data >> 2) & 0x33333333) | ((data << 2) & 0xcccccccc);
    data = ((data >> 4) & 0x0f0f0f0f) | ((data << 4) & 0xf0f0f0f0);
    data = ((data >> 16) & 0xff) | (data & 0xff00) | ((data << 16) & 0xff0000);
    IR_SendCommand(&data, 1);

    return SUCCESS;
}

/*@} end of group IR_Public_Functions */

/*@} end of group IR */

/*@} end of group BL702L_Peripheral_Driver */
