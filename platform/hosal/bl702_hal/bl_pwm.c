#include "bl702_glb.h"
#include "bl702_pwm.h"
#include "bl702_clock.h"
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

static void pwm_init(uint8_t id, uint16_t div, uint16_t period)
{
    PWM_CH_CFG_Type pwmCfg = {
        .ch = id,
        .clk = PWM_CLK_BCLK,
        .stopMode = PWM_STOP_ABRUPT,
        .pol = PWM_POL_NORMAL,
        .clkDiv = div,
        .period = period,
        .threshold1 = 0,
        .threshold2 = 0,
        .intPulseCnt = 0,
    };

    PWM_Channel_Disable(id);
    PWM_Channel_Init(&pwmCfg);
}

static void pwm_start(uint8_t id)
{
    PWM_Channel_Enable(id);
}

static void pwm_stop(uint8_t id)
{
    PWM_Channel_Disable(id);
}

static void pwm_set_duty(uint8_t id, float duty, uint16_t *threshold1, uint16_t *threshold2)
{
    uint16_t period;
    uint16_t tmp1;
    uint16_t tmp2;

    PWM_Channel_Get(id, &period, &tmp1, &tmp2);

    *threshold2 = (uint16_t)(period * duty / 100) + *threshold1;

    if(*threshold2 > period || *threshold2 < *threshold1){
        *threshold2 -= *threshold1;
        *threshold1 = 0;
    }

    PWM_Channel_Set_Threshold1(id, *threshold1);
    PWM_Channel_Set_Threshold2(id, *threshold2);
}

static void pwm_get_duty(uint8_t id, float *p_duty)
{
    uint16_t period;
    uint16_t threshold1;
    uint16_t threshold2;

    PWM_Channel_Get(id, &period, &threshold1, &threshold2);

    *p_duty = (float)(threshold2 - threshold1) * 100 / period;
}


int32_t bl_pwm_port_init(bl_pwm_t id, uint32_t freq)
{
    if(id > 4){
        return -1;
    }

    uint32_t div = 1;
    uint32_t period = BL_PWM_CLK/freq;
    while(period >= 65536){
        div <<= 1;
        period >>= 1;
    }

    GLB_PER_Clock_UnGate(GLB_AHB_CLOCK_PWM);

    pwm_init(id, (uint16_t)div, (uint16_t)period);

    return 0;
}

int32_t bl_pwm_gpio_init(bl_pwm_t id, uint8_t pin)
{
    if(id > 4 || pin > 31 || id != pin % 5){
        return -1;
    }

    gpio_init(pin);

    return 0;
}

int32_t bl_pwm_init(bl_pwm_t id, uint8_t pin, uint32_t freq)
{
    if(id > 4 || pin > 31 || id != pin % 5){
        return -1;
    }

    bl_pwm_port_init(id, freq);
    bl_pwm_gpio_init(id, pin);

    return 0;
}

int32_t bl_pwm_start(bl_pwm_t id)
{
    if(id > 4){
        return -1;
    }

    pwm_start(id);

    return 0;
}

int32_t bl_pwm_stop(bl_pwm_t id)
{
    if(id > 4){
        return -1;
    }

    pwm_stop(id);

    return 0;
}

int32_t bl_pwm_set_duty(bl_pwm_t id, float duty)
{
    uint16_t threshold1 = 0;
    uint16_t threshold2;

    if(id > 4){
        return -1;
    }

    if(duty <= 0){
        duty = 0;
    }

    if(duty >= 100){
        duty = 100;
    }

    pwm_set_duty(id, duty, &threshold1, &threshold2);

    return 0;
}

int32_t bl_pwm_get_duty(bl_pwm_t id, float *p_duty)
{
    if(id > 4){
        return -1;
    }

    pwm_get_duty(id, p_duty);

    return 0;
}
