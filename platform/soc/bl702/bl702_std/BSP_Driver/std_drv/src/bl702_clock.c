/**
  ******************************************************************************
  * @file    bl702_clock.c
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

#include "bl702_clock.h"
#include "bl702_pwm.h"
#include "bl702_timer.h"

/** @addtogroup  BL702_Peripheral_Driver
 *  @{
 */

/** @addtogroup  CLOCK
 *  @{
 */

/** @defgroup  CLOCK_Private_Macros
 *  @{
 */

/*@} end of group CLOCK_Private_Macros */

/** @defgroup  CLOCK_Private_Types
 *  @{
 */

/*@} end of group CLOCK_Private_Types */

/** @defgroup  CLOCK_Private_Variables
 *  @{
 */

/*@} end of group CLOCK_Private_Variables */

/** @defgroup  CLOCK_Global_Variables
 *  @{
 */

/*@} end of group CLOCK_Global_Variables */

/** @defgroup  CLOCK_Private_Fun_Declaration
 *  @{
 */

/*@} end of group CLOCK_Private_Fun_Declaration */

/** @defgroup  CLOCK_Private_Functions
 *  @{
 */

/*@} end of group CLOCK_Private_Functions */

/** @defgroup  CLOCK_Public_Functions
 *  @{
 */

__UNUSED__ static uint32_t ATTR_CLOCK_SECTION Clock_Xtal_Output(void)
{
    /* 32m */
    return (32 * 1000 * 1000);
}

__UNUSED__ static uint32_t ATTR_CLOCK_SECTION Clock_XClk_Mux_Output(uint8_t sel)
{
    if (sel == 0) {
        /* rc32m */
        return (32 * 1000 * 1000);
    } else {
        /* xtal */
        return Clock_Xtal_Output();
    }
}

__UNUSED__ static uint8_t ATTR_CLOCK_SECTION Clock_Get_MCU_XClk_Sel_Val(void)
{
    uint32_t tmpVal = 0;

    tmpVal = BL_RD_REG(GLB_BASE, GLB_CLK_CFG0);
    tmpVal = BL_GET_REG_BITS_VAL(tmpVal, GLB_HBN_ROOT_CLK_SEL);

    return (tmpVal & 0x1);
}

__UNUSED__ static uint8_t ATTR_CLOCK_SECTION Clock_Get_MCU_Root_Clk_Sel_Val(void)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(GLB_BASE, GLB_CLK_CFG0);
    tmpVal = BL_GET_REG_BITS_VAL(tmpVal, GLB_HBN_ROOT_CLK_SEL);

    return ((tmpVal >> 1) & 0x1);
}

__UNUSED__ static uint32_t ATTR_CLOCK_SECTION Clock_MCU_Clk_Mux_Output(uint8_t sel)
{
    if (sel == 0) {
        /* pll 57.6m */
        return (576 * 100 * 1000);
    } else if (sel == 1) {
        /* pll 96m */
        return (96 * 1000 * 1000);
    } else if (sel == 2) {
        /* pll 144m */
        return (144 * 1000 * 1000);
    } else if (sel == 3) {
        /* pll 120m */
        return (120 * 1000 * 1000);
    } else {
        return 0;
    }
}

__UNUSED__ static uint32_t ATTR_CLOCK_SECTION Clock_MCU_Root_Clk_Mux_Output(uint8_t sel)
{
    uint32_t tmpVal;

    if (sel == 0) {
        /* xclk */
        return Clock_XClk_Mux_Output(Clock_Get_MCU_XClk_Sel_Val());
    } else if (sel == 1) {
        /* pll */
        tmpVal = BL_RD_REG(GLB_BASE, GLB_CLK_CFG0);
        tmpVal = BL_GET_REG_BITS_VAL(tmpVal, GLB_REG_PLL_SEL);
        return Clock_MCU_Clk_Mux_Output(tmpVal);
    } else {
        return 0;
    }
}

__UNUSED__ static uint8_t ATTR_CLOCK_SECTION Clock_Get_MCU_HClk_Div_Val(void)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(GLB_BASE, GLB_CLK_CFG0);

    return BL_GET_REG_BITS_VAL(tmpVal, GLB_REG_HCLK_DIV);
}

__UNUSED__ static uint8_t ATTR_CLOCK_SECTION Clock_Get_Peri_BClk_Div_Val(void)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(GLB_BASE, GLB_CLK_CFG0);

    return BL_GET_REG_BITS_VAL(tmpVal, GLB_REG_BCLK_DIV);
}

__UNUSED__ static uint32_t ATTR_CLOCK_SECTION Clock_F32k_Mux_Output(uint8_t sel)
{
    uint32_t tmpVal;
    uint32_t div = 0;

    tmpVal = BL_RD_REG(GLB_BASE, GLB_DIG32K_WAKEUP_CTRL);
    div = BL_GET_REG_BITS_VAL(tmpVal, GLB_DIG_32K_DIV);

    if (sel == 0) {
        /* src32K */
        return (32 * 1000);
    } else if (sel == 1) {
        /* xtal 32K */
        return (32 * 1000);
    } else {
        return Clock_Xtal_Output() / (div + 1);
    }
}

__UNUSED__ static uint8_t ATTR_CLOCK_SECTION Clock_Get_F32k_Sel_Val(void)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(HBN_BASE, HBN_GLB);

    return BL_GET_REG_BITS_VAL(tmpVal, HBN_F32K_SEL);
}

__UNUSED__ static uint32_t ATTR_CLOCK_SECTION Clock_Get_AUPLL_Output()
{
    uint32_t tmpVal = 0;

    tmpVal = BL_GET_REG_BITS_VAL(BL_RD_REG(PDS_BASE, PDS_CLKPLL_SDM), PDS_CLKPLL_SDMIN);

    if (tmpVal == 0x374BC6) {
        /* aupll 12288000 */
        return (12288000);
    } else if (tmpVal == 0x32CCED) {
        if (BL_GET_REG_BITS_VAL(BL_RD_REG(PDS_BASE, PDS_CLKPLL_TOP_CTRL), PDS_CLKPLL_POSTDIV) == 36) {
            /* aupll 11289600 */
            return (11289600);
        } else {
            /* aupll 5644800 */
            return (5644800);
        }
    } else if (tmpVal == 0x6E978D) {
        /* aupll 24576000 */
        return (24576000);
    } else if (tmpVal == 0x6C0000) {
        /* aupll 24000000 */
        return (24000000);
    } else {
        return 0;
    }
}

/****************************************************************************/ /**
 * @brief  Get System Clock
 *
 * @param  type: System clock type
 *
 * @return System clock value
 *
*******************************************************************************/
uint32_t Clock_System_Clock_Get(BL_System_Clock_Type type)
{
    uint32_t clock = 0;
    uint32_t div = 0;

    switch (type) {
        case BL_SYSTEM_CLOCK_XCLK:
            /*!< mcu xclk clock */
            return Clock_XClk_Mux_Output(Clock_Get_MCU_XClk_Sel_Val());

        case BL_SYSTEM_CLOCK_ROOT:
            /*!< mcu root clock */
            return Clock_MCU_Root_Clk_Mux_Output(Clock_Get_MCU_Root_Clk_Sel_Val());

        case BL_SYSTEM_CLOCK_FCLK:
            /*!< mcu fast clock/cpu clock */
            clock = Clock_MCU_Root_Clk_Mux_Output(Clock_Get_MCU_Root_Clk_Sel_Val());
            div = Clock_Get_MCU_HClk_Div_Val();
            return clock / (div + 1);

        case BL_SYSTEM_CLOCK_BCLK:
            /*!< mcu peri bus clock */
            clock = Clock_System_Clock_Get(BL_SYSTEM_CLOCK_FCLK);
            div = Clock_Get_Peri_BClk_Div_Val();
            return clock / (div + 1);

        case BL_SYSTEM_CLOCK_XTAL:
            /*!< xtal clock */
            return Clock_Xtal_Output();

        case BL_SYSTEM_CLOCK_F32K:
            /*!< f32k clock */
            return Clock_F32k_Mux_Output(Clock_Get_F32k_Sel_Val());

        default:
            return 0;
    }
}

__UNUSED__ static uint32_t ATTR_CLOCK_SECTION Clock_UART_Clk_Mux_Output(uint8_t sel)
{
    if (sel == 0) {
        /* fclk */
        return Clock_System_Clock_Get(BL_SYSTEM_CLOCK_FCLK);
    } else {
        /* 96m */
        return (96 * 1000 * 1000);
    }
}

__UNUSED__ static uint8_t ATTR_CLOCK_SECTION Clock_Get_UART_Clk_Sel_Val(void)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(HBN_BASE, HBN_GLB);

    return BL_GET_REG_BITS_VAL(tmpVal, HBN_UART_CLK_SEL);
}

__UNUSED__ static uint8_t ATTR_CLOCK_SECTION Clock_Get_UART_Div_Val(void)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(GLB_BASE, GLB_CLK_CFG2);

    return BL_GET_REG_BITS_VAL(tmpVal, GLB_UART_CLK_DIV);
}

__UNUSED__ static uint32_t ATTR_CLOCK_SECTION Clock_SPI_Clk_Mux_Output(void)
{
    /* pbclk */
    return Clock_System_Clock_Get(BL_SYSTEM_CLOCK_BCLK);
}

__UNUSED__ static uint8_t ATTR_CLOCK_SECTION Clock_Get_SPI_Div_Val(void)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(GLB_BASE, GLB_CLK_CFG3);

    return BL_GET_REG_BITS_VAL(tmpVal, GLB_SPI_CLK_DIV);
}

__UNUSED__ static uint32_t ATTR_CLOCK_SECTION Clock_I2C_Clk_Mux_Output()
{
    /* pbclk */
    return Clock_System_Clock_Get(BL_SYSTEM_CLOCK_BCLK);
}

__UNUSED__ static uint8_t ATTR_CLOCK_SECTION Clock_Get_I2C_Div_Val(void)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(GLB_BASE, GLB_CLK_CFG3);

    return BL_GET_REG_BITS_VAL(tmpVal, GLB_I2C_CLK_DIV);
}

__UNUSED__ static uint32_t ATTR_CLOCK_SECTION Clock_I2S_Clk_Mux_Output(void)
{
    /* aupll clk */
    return Clock_Get_AUPLL_Output();
}

__UNUSED__ static uint8_t ATTR_CLOCK_SECTION Clock_Get_GPADC_Div_Val(void)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(GLB_BASE, GLB_GPADC_32M_SRC_CTRL);

    return BL_GET_REG_BITS_VAL(tmpVal, GLB_GPADC_32M_CLK_DIV);
}

__UNUSED__ static uint32_t ATTR_CLOCK_SECTION Clock_GPADC_Clk_Output(uint8_t sel)
{
    if (sel == 0) {
        /* aupll clk */
        return Clock_Get_AUPLL_Output();
    } else {
        /* xclk */
        return Clock_System_Clock_Get(BL_SYSTEM_CLOCK_XCLK);
    }
}

__UNUSED__ static uint8_t ATTR_CLOCK_SECTION Clock_Get_GPADC_32M_Clk_Sel_Val(void)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(GLB_BASE, GLB_GPADC_32M_SRC_CTRL);

    return BL_GET_REG_BITS_VAL(tmpVal, GLB_GPADC_32M_CLK_SEL);
}

__UNUSED__ static uint32_t ATTR_CLOCK_SECTION Clock_GPADC_Clk_Mux_Output(uint8_t sel)
{
    uint32_t div = 0;

    if (sel == 1) {
        /* f32k clk */
        return Clock_F32k_Mux_Output(Clock_Get_F32k_Sel_Val());
    } else {
        div = Clock_Get_GPADC_Div_Val();
        return Clock_GPADC_Clk_Output(Clock_Get_GPADC_32M_Clk_Sel_Val()) / (div + 1);
    }
}

__UNUSED__ static uint8_t ATTR_CLOCK_SECTION Clock_Get_GPADC_Clk_Sel_Val(void)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(HBN_BASE, HBN_PIR_CFG);

    return BL_GET_REG_BITS_VAL(tmpVal, HBN_PIR_EN);
}

__UNUSED__ static uint32_t ATTR_CLOCK_SECTION Clock_GPDAC_Clk_Mux_Output(uint8_t sel)
{
    if (sel == 0) {
        /* pll 32m */
        return (32 * 1000 * 1000);
    } else if (sel == 1) {
        /* xclk */
        return Clock_System_Clock_Get(BL_SYSTEM_CLOCK_XCLK);
    } else {
        /* aupll clk */
        return Clock_Get_AUPLL_Output();
    }
}

__UNUSED__ static uint8_t ATTR_CLOCK_SECTION Clock_Get_GPDAC_Div_Val(void)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(GLB_BASE, GLB_DIG32K_WAKEUP_CTRL);

    return BL_GET_REG_BITS_VAL(tmpVal, GLB_DIG_512K_DIV);
}

__UNUSED__ static uint8_t ATTR_CLOCK_SECTION Clock_Get_GPDAC_Clk_Sel_Val(void)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(GLB_BASE, GLB_DIG32K_WAKEUP_CTRL);

    return BL_GET_REG_BITS_VAL(tmpVal, GLB_DIG_CLK_SRC_SEL);
}

__UNUSED__ static uint32_t ATTR_CLOCK_SECTION Clock_PWM_Clk_Mux_Output(uint8_t sel)
{
    if (sel == 0) {
        /* xclk */
        return Clock_System_Clock_Get(BL_SYSTEM_CLOCK_XCLK);
    } else if (sel == 1) {
        /* pbclk */
        return Clock_System_Clock_Get(BL_SYSTEM_CLOCK_BCLK);
    } else {
        /* f32k clk */
        return Clock_F32k_Mux_Output(Clock_Get_F32k_Sel_Val());
    }
}

__UNUSED__ static uint8_t ATTR_CLOCK_SECTION Clock_Get_PWM_Div_Val(void)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(PWM_BASE + PWM_CHANNEL_OFFSET, PWM_CLKDIV);

    return tmpVal;
}

__UNUSED__ static uint8_t ATTR_CLOCK_SECTION Clock_Get_PWM_Clk_Sel_Val(void)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(PWM_BASE + PWM_CHANNEL_OFFSET, PWM_CONFIG);

    return BL_GET_REG_BITS_VAL(tmpVal, PWM_REG_CLK_SEL);
}

__UNUSED__ static uint32_t ATTR_CLOCK_SECTION Clock_IR_Clk_Mux_Output(void)
{
    /* xclk */
    return Clock_System_Clock_Get(BL_SYSTEM_CLOCK_XCLK);
}

__UNUSED__ static uint8_t ATTR_CLOCK_SECTION Clock_Get_IR_Div_Val(void)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(GLB_BASE, GLB_CLK_CFG2);

    return BL_GET_REG_BITS_VAL(tmpVal, GLB_IR_CLK_DIV);
}

__UNUSED__ static uint32_t ATTR_CLOCK_SECTION Clock_CAM_Clk_Mux_Output(uint8_t sel)
{
    if (sel == 0) {
        /* xclk */
        return Clock_System_Clock_Get(BL_SYSTEM_CLOCK_XCLK);
    } else {
        /* 96m */
        return (96 * 1000 * 1000);
    }
}

__UNUSED__ static uint8_t ATTR_CLOCK_SECTION Clock_Get_CAM_Clk_Sel_Val(void)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(GLB_BASE, GLB_CLK_CFG1);

    return BL_GET_REG_BITS_VAL(tmpVal, GLB_REG_CAM_REF_CLK_SRC_SEL);
}

__UNUSED__ static uint8_t ATTR_CLOCK_SECTION Clock_Get_CAM_Div_Val(void)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(GLB_BASE, GLB_CLK_CFG1);

    return BL_GET_REG_BITS_VAL(tmpVal, GLB_REG_CAM_REF_CLK_DIV);
}

__UNUSED__ static uint8_t ATTR_CLOCK_SECTION Clock_Get_SF_Clk_Sel2_Val(void)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(GLB_BASE, GLB_CLK_CFG2);

    return BL_GET_REG_BITS_VAL(tmpVal, GLB_SF_CLK_SEL2);
}

__UNUSED__ static uint32_t ATTR_CLOCK_SECTION Clock_SF_SEL2_Clk_Mux_Output(uint8_t sel)
{
    if (sel == 0) {
        /* 144m */
        return (144 * 1000 * 1000);
    } else if (sel == 1) {
        /* xclk */
        return Clock_System_Clock_Get(BL_SYSTEM_CLOCK_XCLK);
    } else {
        /* 57.6m */
        return (576 * 100 * 1000);
    }
}

__UNUSED__ static uint32_t ATTR_CLOCK_SECTION Clock_SF_SEL_Clk_Mux_Output(uint8_t sel)
{
    if (sel == 0) {
        /* sf sel2 */
        return Clock_SF_SEL2_Clk_Mux_Output(Clock_Get_SF_Clk_Sel2_Val());
    } else if (sel == 1) {
        /* 72m */
        return (72 * 1000 * 1000);
    } else if (sel == 2) {
        /* pbclk */
        return Clock_System_Clock_Get(BL_SYSTEM_CLOCK_BCLK);
    } else {
        /* 96m */
        return (96 * 1000 * 1000);
    }
}

__UNUSED__ static uint8_t ATTR_CLOCK_SECTION Clock_Get_SF_Clk_Sel_Val(void)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(GLB_BASE, GLB_CLK_CFG2);

    return BL_GET_REG_BITS_VAL(tmpVal, GLB_SF_CLK_SEL);
}

__UNUSED__ static uint8_t ATTR_CLOCK_SECTION Clock_Get_SF_Div_Val(void)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(GLB_BASE, GLB_CLK_CFG2);

    return BL_GET_REG_BITS_VAL(tmpVal, GLB_SF_CLK_DIV);
}

__UNUSED__ static uint32_t ATTR_CLOCK_SECTION Clock_QDEC_Clk_Mux_Output(uint8_t sel)
{
    if (sel == 0) {
        /* xclk */
        return Clock_System_Clock_Get(BL_SYSTEM_CLOCK_XCLK);
    } else {
        /* f32k clk */
        return Clock_F32k_Mux_Output(Clock_Get_F32k_Sel_Val());
    }
}

__UNUSED__ static uint8_t ATTR_CLOCK_SECTION Clock_Get_QDEC_Clk_Sel_Val(void)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(GLB_BASE, GLB_CLK_CFG1);

    return BL_GET_REG_BITS_VAL(tmpVal, GLB_QDEC_CLK_SEL);
}

__UNUSED__ static uint8_t ATTR_CLOCK_SECTION Clock_Get_QDEC_Div_Val(void)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(GLB_BASE, GLB_CLK_CFG1);

    return BL_GET_REG_BITS_VAL(tmpVal, GLB_QDEC_CLK_DIV);
}

__UNUSED__ static uint32_t ATTR_CLOCK_SECTION Clock_TIMER_Clk_Mux_Output(uint8_t sel)
{
    if (sel == 0) {
        /* fclk */
        return Clock_System_Clock_Get(BL_SYSTEM_CLOCK_FCLK);
    } else if (sel == 1) {
        /* f32k clk */
        return Clock_F32k_Mux_Output(Clock_Get_F32k_Sel_Val());
    } else if (sel == 2) {
        /* 1k */
        return (1000);
    } else {
        /* 32m */
        return (32 * 1000 * 1000);
    }
}

__UNUSED__ static uint8_t ATTR_CLOCK_SECTION Clock_Get_TIMER0_Clk_Sel_Val(void)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(TIMER_BASE, TIMER_TCCR);

    return BL_GET_REG_BITS_VAL(tmpVal, TIMER_CS_1);
}

__UNUSED__ static uint8_t ATTR_CLOCK_SECTION Clock_Get_TIMER1_Clk_Sel_Val(void)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(TIMER_BASE, TIMER_TCCR);

    return BL_GET_REG_BITS_VAL(tmpVal, TIMER_CS_2);
}

__UNUSED__ static uint8_t ATTR_CLOCK_SECTION Clock_Get_WDT_Clk_Sel_Val(void)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(TIMER_BASE, TIMER_TCCR);

    return BL_GET_REG_BITS_VAL(tmpVal, TIMER_CS_WDT);
}

/****************************************************************************/ /**
 * @brief  Get Peripheral Clock
 *
 * @param  type: Peripheral clock type
 *
 * @return Peripheral clock value
 *
*******************************************************************************/
uint32_t Clock_Peripheral_Clock_Get(BL_Peripheral_Type type)
{
    uint32_t clock = 0;
    uint32_t div = 0;

    switch (type) {
        /*!< uart0 clock */
        case BL_PERIPHERAL_CLOCK_UART0:
            ATTR_FALLTHROUGH();

        /*!< uart1 clock */
        case BL_PERIPHERAL_CLOCK_UART1:
            clock = Clock_UART_Clk_Mux_Output(Clock_Get_UART_Clk_Sel_Val());
            div = Clock_Get_UART_Div_Val();
            return clock / (div + 1);

        /*!< FLASH clock */
        case BL_PERIPHERAL_CLOCK_FLASH:
            clock = Clock_SF_SEL_Clk_Mux_Output(Clock_Get_SF_Clk_Sel_Val());
            div = Clock_Get_SF_Div_Val();
            return clock / (div + 1);

        /*!< spi0 clock */
        case BL_PERIPHERAL_CLOCK_SPI0:
            clock = Clock_SPI_Clk_Mux_Output();
            div = Clock_Get_SPI_Div_Val();
            return clock / (div + 1);

        /*!< i2c0 clock */
        case BL_PERIPHERAL_CLOCK_I2C0:
            clock = Clock_I2C_Clk_Mux_Output();
            div = Clock_Get_I2C_Div_Val();
            return clock / (div + 1);

        /*!< I2S clock */
        case BL_PERIPHERAL_CLOCK_I2S:
            clock = Clock_I2S_Clk_Mux_Output();
            return clock;

        /*!< GPADC clock */
        case BL_PERIPHERAL_CLOCK_GPADC:
            return Clock_GPADC_Clk_Mux_Output(Clock_Get_GPADC_Clk_Sel_Val());

        /*!< GPDAC clock */
        case BL_PERIPHERAL_CLOCK_GPDAC:
            clock = Clock_GPDAC_Clk_Mux_Output(Clock_Get_GPDAC_Clk_Sel_Val());
            div = Clock_Get_GPDAC_Div_Val();
            return clock / (div + 1);

        /*!< PWM clock */
        case BL_PERIPHERAL_CLOCK_PWM:
            clock = Clock_PWM_Clk_Mux_Output(Clock_Get_PWM_Clk_Sel_Val());
            div = Clock_Get_PWM_Div_Val();
            return clock / div;

        /*!< IR clock */
        case BL_PERIPHERAL_CLOCK_IR:
            clock = Clock_IR_Clk_Mux_Output();
            div = Clock_Get_IR_Div_Val();
            return clock / (div + 1);

        /*!< CAM clock */
        case BL_PERIPHERAL_CLOCK_CAM:
            clock = Clock_CAM_Clk_Mux_Output(Clock_Get_CAM_Clk_Sel_Val());
            div = Clock_Get_CAM_Div_Val();
            return clock / (div + 1);

        /*!< QDEC clock */
        case BL_PERIPHERAL_CLOCK_QDEC:
            clock = Clock_QDEC_Clk_Mux_Output(Clock_Get_QDEC_Clk_Sel_Val());
            div = Clock_Get_QDEC_Div_Val();
            return clock / (div + 1);

        /*!< TIMER0 clock */
        case BL_PERIPHERAL_CLOCK_TIMER0:
            return Clock_TIMER_Clk_Mux_Output(Clock_Get_TIMER0_Clk_Sel_Val());

        /*!< TIMER1 clock */
        case BL_PERIPHERAL_CLOCK_TIMER1:
            return Clock_TIMER_Clk_Mux_Output(Clock_Get_TIMER1_Clk_Sel_Val());

        /*!< WDT clock */
        case BL_PERIPHERAL_CLOCK_WDT:
            return Clock_TIMER_Clk_Mux_Output(Clock_Get_WDT_Clk_Sel_Val());

        default:
            return 0;
    }
}

/*@} end of group CLOCK_Public_Functions */

/*@} end of group CLOCK */

/*@} end of group BL702_Peripheral_Driver */
