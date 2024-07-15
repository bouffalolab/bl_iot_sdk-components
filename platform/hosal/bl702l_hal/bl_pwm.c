#include "bl702l_glb.h"
#include "bl702l_glb_gpio.h"
#include "bl702l_pwm.h"
#include "bl702l_pwm_sc.h"
#include "bl702l_clock.h"
#include "bl_pwm.h"


#define BL_PWM_CLK            (SystemCoreClockGet()/(GLB_Get_BCLK_Div()+1))


static void gpio_init(uint8_t pin)
{
    GLB_GPIO_Cfg_Type cfg;

    cfg.drive = 0;
    cfg.smtCtrl = 1;
    cfg.gpioMode = GPIO_MODE_OUTPUT;
    cfg.pullType = GPIO_PULL_NONE;
    cfg.gpioPin = pin;
    cfg.gpioFun = 8;

    GLB_GPIO_Init(&cfg);
}

static void pwm_sc_init(uint16_t div, uint16_t period)
{
    PWM_SC_CFG_Type pwmCfg = {
        .ch = PWM_SC0,
        .clk = PWM_SC_CLK_BCLK,
        .stopMode = PWM_SC_STOP_ABRUPT,
        .pol = PWM_SC_POL_NORMAL,
        .clkDiv = div,
        .period = period,
        .threshold1 = 0,
        .threshold2 = 0,
        .intPulseCnt = 0,
        .stpInt = DISABLE,
    };

    PWM_SC_Channel_Disable(PWM_SC0);
    PWM_SC_Channel_Init(&pwmCfg);
}

static void pwm_sc_start(void)
{
    PWM_SC_Channel_Enable(PWM_SC0);
}

static void pwm_sc_stop(void)
{
    PWM_SC_Channel_Disable(PWM_SC0);
}

static void pwm_sc_set_duty(float duty, uint16_t *threshold1, uint16_t *threshold2)
{
    uint16_t period;
    uint16_t tmp1;
    uint16_t tmp2;

    PWM_SC_Channel_Get(PWM_SC0, &period, &tmp1, &tmp2);

    *threshold2 = (uint16_t)(period * duty / 100) + *threshold1;

    if(*threshold2 > period || *threshold2 < *threshold1){
        *threshold2 -= *threshold1;
        *threshold1 = 0;
    }

    PWM_SC_Channel_Set_Threshold1(PWM_SC0, *threshold1);
    PWM_SC_Channel_Set_Threshold2(PWM_SC0, *threshold2);
}

static void pwm_sc_get_duty(float *p_duty)
{
    uint16_t period;
    uint16_t threshold1;
    uint16_t threshold2;

    PWM_SC_Channel_Get(PWM_SC0, &period, &threshold1, &threshold2);

    *p_duty = (float)(threshold2 - threshold1) * 100 / period;
}

static void pwm_mc_init(uint16_t div, uint16_t period)
{
    PWMx_CFG_Type pwmxCfg = {
        .clk = PWM_CLK_BCLK,
        .stopMode = PWM_STOP_GRACEFUL,
        .clkDiv = div,
        .period = period,
        .intPulseCnt = 0,
        .extPol = PWM_BREAK_Polarity_LOW,
        .stpRept = DISABLE,
        .adcSrc = PWM_TRIGADC_SOURCE_NONE,
    };

    PWM_CHx_CFG_Type chxCfg = {
        .modP = PWM_MODE_DISABLE,
        .modN = PWM_MODE_DISABLE,
        .polP = PWM_POL_ACTIVE_HIGH,
        .polN = PWM_POL_ACTIVE_HIGH,
        .idlP = PWM_IDLE_STATE_INACTIVE,
        .idlN = PWM_IDLE_STATE_INACTIVE,
        .brkP = PWM_BREAK_STATE_INACTIVE,
        .brkN = PWM_BREAK_STATE_INACTIVE,
        .thresholdL = 0,
        .thresholdH = 0,
        .dtg = 0,
    };

    PWMx_Disable(PWM0_ID);
    PWMx_Init(PWM0_ID, &pwmxCfg);
    for(PWM_CHx_Type ch = PWM_CH0; ch < PWM_CHx_MAX; ch++){
        PWM_Channelx_Init(PWM0_ID, ch, &chxCfg);
    }
    PWMx_Enable(PWM0_ID);
}

static void pwm_mc_start(uint8_t ch)
{
    PWM_Channelx_Positive_Pwm_Mode_Set(PWM0_ID, ch, PWM_MODE_ENABLE);
}

static void pwm_mc_stop(uint8_t ch)
{
    PWM_Channelx_Positive_Pwm_Mode_Set(PWM0_ID, ch, PWM_MODE_DISABLE);
}

static void pwm_mc_set_duty(uint8_t ch, float duty, uint16_t *threshold1, uint16_t *threshold2)
{
    uint16_t period;

    PWMx_Period_Get(PWM0_ID, &period);

    *threshold2 = (uint16_t)(period * duty / 100) + *threshold1;

    if(*threshold2 > period || *threshold2 < *threshold1){
        *threshold2 -= *threshold1;
        *threshold1 = 0;
    }

    PWM_Channelx_Threshold_Set(PWM0_ID, ch, *threshold1, *threshold2);
}

static void pwm_mc_get_duty(uint8_t ch, float *p_duty)
{
    uint16_t period;
    uint16_t threshold1;
    uint16_t threshold2;

    PWMx_Period_Get(PWM0_ID, &period);
    PWM_Channelx_Threshold_Get(PWM0_ID, ch, &threshold1, &threshold2);

    *p_duty = (float)(threshold2 - threshold1) * 100 / period;
}


int32_t bl_pwm_port_init(bl_pwm_t id, uint32_t freq)
{
    if(id > 1){
        return -1;
    }

    uint32_t div = 1;
    uint32_t period = BL_PWM_CLK/freq;
    while(period >= 65536){
        div <<= 1;
        period >>= 1;
    }

    GLB_PER_Clock_UnGate(GLB_AHB_CLOCK_PWM);

    if(id == 0){
        pwm_sc_init((uint16_t)div, (uint16_t)period);
    }else{
        pwm_mc_init((uint16_t)div, (uint16_t)period);
    }

    return 0;
}

int32_t bl_pwm_gpio_init(bl_pwm_ch_t ch, uint8_t pin)
{
    if(ch > 4 || pin > 31 || ch != pin % 5){
        return -1;
    }

    gpio_init(pin);

    return 0;
}

int32_t bl_pwm_start(bl_pwm_ch_t ch)
{
    if(ch > 4){
        return -1;
    }

    if(ch == 0){
        pwm_sc_start();
    }else{
        pwm_mc_start(ch - 1);
    }

    return 0;
}

int32_t bl_pwm_stop(bl_pwm_ch_t ch)
{
    if(ch > 4){
        return -1;
    }

    if(ch == 0){
        pwm_sc_stop();
    }else{
        pwm_mc_stop(ch - 1);
    }

    return 0;
}

int32_t bl_pwm_set_duty(bl_pwm_ch_t ch, float duty)
{
    uint16_t threshold1 = 0;
    uint16_t threshold2;

    if(ch > 4){
        return -1;
    }

    if(duty <= 0){
        duty = 0;
    }

    if(duty >= 100){
        duty = 100;
    }

    if(ch == 0){
        pwm_sc_set_duty(duty, &threshold1, &threshold2);
    }else{
        pwm_mc_set_duty(ch - 1, duty, &threshold1, &threshold2);
    }

    return 0;
}

int32_t bl_pwm_set_duty_ex(bl_pwm_ch_t ch, float duty, uint16_t *threshold1, uint16_t *threshold2)
{
    if(ch > 4){
        return -1;
    }

    if(duty <= 0){
        duty = 0;
    }

    if(duty >= 100){
        duty = 100;
    }

    if(ch == 0){
        pwm_sc_set_duty(duty, threshold1, threshold2);
    }else{
        pwm_mc_set_duty(ch - 1, duty, threshold1, threshold2);
    }

    return 0;
}

int32_t bl_pwm_get_duty(bl_pwm_ch_t ch, float *p_duty)
{
    if(ch > 4){
        return -1;
    }

    if(ch == 0){
        pwm_sc_get_duty(p_duty);
    }else{
        pwm_mc_get_duty(ch - 1, p_duty);
    }

    return 0;
}
