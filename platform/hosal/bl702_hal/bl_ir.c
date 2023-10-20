#include "bl_ir.h"
#include "bl_irq.h"


static void bl_ir_irq(void)
{
    IR_Disable(IR_TX);
    IR_IntMask(IR_INT_TX, MASK);
    IR_ClrIntStatus(IR_INT_TX);

    bl_ir_swm_tx_done_callback();
}

static void bl_ir_tx_interrupt_cfg(void)
{
    bl_irq_register(IRTX_IRQn, bl_ir_irq);
    bl_irq_enable(IRTX_IRQn);
}


void bl_ir_led_drv_cfg(uint8_t led0_en, uint8_t led1_en)
{
    GLB_GPIO_Type pin;

    if(led0_en){
        pin = 22;
        GLB_GPIO_Func_Init(GPIO_FUN_ANALOG, &pin, 1);
        GLB_IR_LED_Driver_Output_Enable(pin);
    }else{
        pin = 22;
        GLB_IR_LED_Driver_Output_Disable(pin);
    }

    if(led1_en){
        pin = 23;
        GLB_GPIO_Func_Init(GPIO_FUN_ANALOG, &pin, 1);
        GLB_IR_LED_Driver_Output_Enable(pin);
    }else{
        pin = 23;
        GLB_IR_LED_Driver_Output_Disable(pin);
    }

    if(led0_en || led1_en){
        GLB_IR_LED_Driver_Enable();
    }else{
        GLB_IR_LED_Driver_Disable();
    }

    GLB_IR_LED_Driver_Ibias(15);
}

void bl_ir_custom_tx_cfg(IR_TxCfg_Type *txCfg, IR_TxPulseWidthCfg_Type *txPWCfg)
{
    /* Run IR at 2M */
    GLB_PER_Clock_UnGate(GLB_AHB_CLOCK_IR);
    GLB_Set_IR_CLK(ENABLE, GLB_IR_CLK_SRC_XCLK, 15);

    IR_Disable(IR_TX);
    IR_TxSWM(DISABLE);
    IR_TxInit(txCfg);
    IR_TxPulseWidthConfig(txPWCfg);
}

void bl_ir_nec_tx_cfg(void)
{
    IR_TxCfg_Type txCfg = {
        32,              /* 32-bit data */
        DISABLE,         /* Disable signal of tail pulse inverse */
        ENABLE,          /* Enable signal of tail pulse */
        DISABLE,         /* Disable signal of head pulse inverse */
        ENABLE,          /* Enable signal of head pulse */
        DISABLE,         /* Disable signal of logic 1 pulse inverse */
        DISABLE,         /* Disable signal of logic 0 pulse inverse */
        ENABLE,          /* Enable signal of data pulse */
        ENABLE,          /* Enable signal of output modulation */
        DISABLE,         /* Disable signal of output inverse */
    };

    IR_TxPulseWidthCfg_Type txPWCfg = {
        1,   /* Pulse width of logic 0 pulse phase 1, 562.5us @2MHz source clock*/
        1,   /* Pulse width of logic 0 pulse phase 0 */
        3,   /* Pulse width of logic 1 pulse phase 1, 1687.5us */
        1,   /* Pulse width of logic 1 pulse phase 0 */
        8,   /* Pulse width of head pulse phase 1, 4.5ms */
        16,  /* Pulse width of head pulse phase 0, 9ms */
        1,   /* Pulse width of tail pulse phase 1 */
        1,   /* Pulse width of tail pulse phase 0 */
        35,  /* Modulation phase 1 width, 37.7kHz, duty=1/3 */
        18,  /* Modulation phase 0 width, 37.7kHz, duty=1/3 */
        1125 /* Pulse width unit */
    };

    bl_ir_custom_tx_cfg(&txCfg, &txPWCfg);
}

void bl_ir_rc5_tx_cfg(void)
{
    IR_TxCfg_Type txCfg = {
        13,              /* 13-bit data, head pulse as the first start bit */
        DISABLE,         /* Disable signal of tail pulse inverse */
        DISABLE,         /* Disable signal of tail pulse */
        ENABLE,          /* Enable signal of head pulse inverse */
        ENABLE,          /* Enable signal of head pulse */
        ENABLE,          /* Enable signal of logic 1 pulse inverse */
        DISABLE,         /* Disable signal of logic 0 pulse inverse */
        ENABLE,          /* Enable signal of data pulse */
        ENABLE,          /* Enable signal of output modulation */
        DISABLE,         /* Disable signal of output inverse */
    };

    IR_TxPulseWidthCfg_Type txPWCfg = {
        1,   /* Pulse width of logic 0 pulse phase 1, 889us @2MHz source clock*/
        1,   /* Pulse width of logic 0 pulse phase 0 */
        1,   /* Pulse width of logic 1 pulse phase 1 */
        1,   /* Pulse width of logic 1 pulse phase 0 */
        1,   /* Pulse width of head pulse phase 1 */
        1,   /* Pulse width of head pulse phase 0 */
        1,   /* Pulse width of tail pulse phase 1 */
        1,   /* Pulse width of tail pulse phase 0 */
        35,  /* Modulation phase 1 width, 37.7kHz, duty=1/3 */
        18,  /* Modulation phase 0 width, 37.7kHz, duty=1/3 */
        1778 /* Pulse width unit */
    };

    bl_ir_custom_tx_cfg(&txCfg, &txPWCfg);
}

void bl_ir_swm_tx_cfg(float freq_hz, float duty_cycle)
{
    uint16_t pw_unit = (uint16_t)(2000000 / freq_hz);
    uint8_t mod_width_0 = (uint8_t)(pw_unit * duty_cycle + 0.5);
    uint8_t mod_width_1 = pw_unit - mod_width_0;

    IR_TxCfg_Type txCfg = {
        1,                                                   /* Number of pulse */
        DISABLE,                                             /* Don't care when SWM is enabled */
        DISABLE,                                             /* Don't care when SWM is enabled */
        DISABLE,                                             /* Don't care when SWM is enabled */
        DISABLE,                                             /* Don't care when SWM is enabled */
        DISABLE,                                             /* Don't care when SWM is enabled */
        DISABLE,                                             /* Don't care when SWM is enabled */
        DISABLE,                                             /* Don't care when SWM is enabled */
        ENABLE,                                              /* Enable signal of output modulation */
        DISABLE,                                             /* Disable signal of output inverse */
    };

    IR_TxPulseWidthCfg_Type txPWCfg = {
        0,                                                   /* Don't care when SWM is enabled */
        0,                                                   /* Don't care when SWM is enabled */
        0,                                                   /* Don't care when SWM is enabled */
        0,                                                   /* Don't care when SWM is enabled */
        0,                                                   /* Don't care when SWM is enabled */
        0,                                                   /* Don't care when SWM is enabled */
        0,                                                   /* Don't care when SWM is enabled */
        0,                                                   /* Don't care when SWM is enabled */
        mod_width_1,                                         /* Modulation phase 1 width */
        mod_width_0,                                         /* Modulation phase 0 width */
        pw_unit                                              /* Pulse width unit */
    };

    bl_ir_custom_tx_cfg(&txCfg, &txPWCfg);
    bl_ir_tx_interrupt_cfg();
}

void bl_ir_nec_tx(uint32_t wdata)
{
    IR_SendCommand(0, wdata);
}

void bl_ir_rc5_tx(uint32_t wdata)
{
    IR_SendCommand(0, wdata);
}

int bl_ir_swm_tx(uint16_t data[], uint8_t len)
{
    uint32_t tmpVal;
    uint32_t pwVal;
    int i, j;

    tmpVal = BL_RD_REG(IR_BASE, IRTX_CONFIG);
    if(BL_GET_REG_BITS_VAL(tmpVal, IR_CR_IRTX_EN)){
        return -1;
    }

    tmpVal = BL_RD_REG(IR_BASE, IRTX_CONFIG);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, IR_CR_IRTX_DATA_NUM, len - 1);
    BL_WR_REG(IR_BASE, IRTX_CONFIG, tmpVal);

    pwVal = 0;
    for(i=0; i<len; i++){
        j = i % 8;
        pwVal |= ((data[i] - 1) & 0x0F) << 4 * j;
        if(i == len - 1 || j == 7){
            *(volatile uint32_t *)(IR_BASE + IRTX_SWM_PW_0_OFFSET + i / 8 * 4) = pwVal;
            pwVal = 0;
        }
    }

    IR_IntMask(IR_INT_TX, UNMASK);
    IR_TxSWM(ENABLE);
    IR_Enable(IR_TX);

    return 0;
}

int bl_ir_swm_tx_busy(void)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(IR_BASE, IRTX_CONFIG);
    return BL_GET_REG_BITS_VAL(tmpVal, IR_CR_IRTX_EN);
}

__attribute__((weak)) void bl_ir_swm_tx_done_callback(void)
{
    
}
