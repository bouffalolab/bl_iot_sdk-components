#include <stdio.h>
#include <stdint.h>
#include "bl702l_pwm.h"
#include "bl702l_glb.h"
#include "bl702l_glb_gpio.h"
#include <hosal_pwm.h>

#include <blog.h>

#define PWM_CHANNAL_MAX     4

static void pwm_config_freq(hosal_pwm_dev_t *pwm)
{
    PWMx_CFG_Type pwmxCfg = {
        .clk = PWM_CLK_BCLK,
        .stopMode = PWM_STOP_GRACEFUL,
        .intPulseCnt = 0,
        .extPol = PWM_BREAK_Polarity_LOW,
        .stpRept = DISABLE,
        .adcSrc = PWM_TRIGADC_SOURCE_NONE,
    };

    /* XXX BCLK is 64MHz */
    if(pwm->config.freq <= 64){
        pwmxCfg.clkDiv = 1024;
        pwmxCfg.period = 62500/pwm->config.freq;
    }else if(pwm->config.freq <= 128){
        pwmxCfg.clkDiv = 16;
        pwmxCfg.period = 4000000/pwm->config.freq;
    }else if(pwm->config.freq <= 256){
        pwmxCfg.clkDiv = 8;
        pwmxCfg.period = 8000000/pwm->config.freq;
    }else if(pwm->config.freq <= 512){
        pwmxCfg.clkDiv = 4;
        pwmxCfg.period = 16000000/pwm->config.freq;
    }else if(pwm->config.freq <= 1024){
        pwmxCfg.clkDiv = 2;
        pwmxCfg.period = 32000000/pwm->config.freq;
    }else{
        pwmxCfg.clkDiv = 1;
        pwmxCfg.period = 64000000/pwm->config.freq;
    }

    PWMx_Disable(pwm->port);
    PWMx_Init(pwm->port, &pwmxCfg);
    PWMx_Enable(pwm->port);
}

/**
 * Initialises a PWM pin
 *
 *
 * @param[in]  pwm  the PWM device
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int hosal_pwm_init(hosal_pwm_dev_t *pwm)
{
    GLB_GPIO_Cfg_Type gpioCfg;

    if (NULL == pwm || pwm->port != PWM0_ID || pwm->config.pin % 5 == 0) {
        blog_error("arg error.\r\n");
        return -1;
    }

    gpioCfg.gpioPin = pwm->config.pin;
    gpioCfg.gpioFun = GPIO_FUN_PWM;
    gpioCfg.gpioMode = GPIO_MODE_AF;
    gpioCfg.pullType = GPIO_PULL_NONE;
    gpioCfg.drive = 1;
    gpioCfg.smtCtrl = 1;
    GLB_GPIO_Init(&gpioCfg);

    pwm_config_freq(pwm);

    return 0;
}

/**
 * Starts Pulse-Width Modulation signal output on a PWM pin
 *
 * @param[in]  pwm  the PWM device
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int hosal_pwm_start(hosal_pwm_dev_t *pwm)
{
    uint8_t ch;
    uint32_t duty;

    PWM_CHx_CFG_Type chxCfg = {
        .modP = PWM_MODE_ENABLE,
        .modN = PWM_MODE_DISABLE,
        .polP = PWM_POL_ACTIVE_HIGH,
        .polN = PWM_POL_ACTIVE_LOW,
        .idlP = PWM_IDLE_STATE_INACTIVE,
        .idlN = PWM_IDLE_STATE_ACTIVE,
        .brkP = PWM_BREAK_STATE_INACTIVE,
        .brkN = PWM_BREAK_STATE_ACTIVE,
        .thresholdL = 0,
        .thresholdH = 0,
        .dtg = 0,
    };

    if (NULL == pwm || pwm->port != PWM0_ID || pwm->config.pin % 5 == 0) {
        blog_error("arg error.\r\n");
        return -1;
    }

    ch = pwm->config.pin % 5 - 1;
    duty = pwm->config.duty_cycle / 100;

    /* XXX BCLK is 64MHz */
    if(pwm->config.freq <= 64){
        chxCfg.thresholdH = 625*duty/pwm->config.freq;
    }else if(pwm->config.freq <= 128){
        chxCfg.thresholdH = 40000*duty/pwm->config.freq;
    }else if(pwm->config.freq <= 256){
        chxCfg.thresholdH = 80000*duty/pwm->config.freq;
    }else if(pwm->config.freq <= 512){
        chxCfg.thresholdH = 160000*duty/pwm->config.freq;
    }else if(pwm->config.freq <= 1024){
        chxCfg.thresholdH = 320000*duty/pwm->config.freq;
    }else{
        chxCfg.thresholdH = 640000*duty/pwm->config.freq;
    }

    PWM_Channelx_Init(pwm->port, ch, &chxCfg);

    return 0;
}

/**
 * Stops output on a PWM pin
 *
 * @param[in]  pwm  the PWM device, para  set duty and  freq
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int hosal_pwm_para_chg(hosal_pwm_dev_t *pwm, hosal_pwm_config_t para)
{
    if (NULL == pwm || pwm->port != PWM0_ID) {
        blog_error("arg error.\r\n");
        return -1;
    }

    pwm->config.freq = para.freq;
    pwm->config.duty_cycle = para.duty_cycle;

    pwm_config_freq(pwm);
    hosal_pwm_start(pwm);

    return 0;
}

/**
 * set pwm freq
 *
 *@param[in] id pwm channel
 *@param[in] freq pwm freq
 *
 *@return 0 : on success -1 : fail
 */
int hosal_pwm_freq_set(hosal_pwm_dev_t *pwm, uint32_t freq)
{
    if (NULL == pwm || pwm->port != PWM0_ID) {
        blog_error("arg error.\r\n");
        return -1;
    }

    pwm->config.freq = freq;

    pwm_config_freq(pwm);
    hosal_pwm_start(pwm);

    return 0;
}

int hosal_pwm_freq_get(hosal_pwm_dev_t *pwm,  uint32_t *p_freq)
{
    if (NULL == pwm || pwm->port != PWM0_ID || NULL == p_freq) {
        blog_error("arg error.\r\n");
        return -1;
    }
    *p_freq = pwm->config.freq;

    return 0;
}

int hosal_pwm_duty_set(hosal_pwm_dev_t *pwm, uint32_t duty)
{
	if (NULL == pwm || pwm->port != PWM0_ID || duty > 10000) {
        blog_error("arg error.\r\n");
        return -1;
    }
    pwm->config.duty_cycle = duty;

    hosal_pwm_start(pwm);
    return 0;
}

int hosal_pwm_duty_get(hosal_pwm_dev_t *pwm, uint32_t *p_duty)
{
	if (NULL == pwm || pwm->port >= PWM_CHANNAL_MAX || NULL == p_duty) {
        blog_error("arg error.\r\n");
        return -1;
    }
    *p_duty = pwm->config.duty_cycle;

    return 0;
}
/**
 * Stops output on a PWM pin
 *
 * @param[in]  pwm  the PWM device
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int hosal_pwm_stop(hosal_pwm_dev_t *pwm)
{
    uint8_t ch;

    PWM_CHx_CFG_Type chxCfg ={
        .modP = PWM_MODE_DISABLE,
        .modN = PWM_MODE_DISABLE,
        .polP = PWM_POL_ACTIVE_LOW,
        .polN = PWM_POL_ACTIVE_LOW,
        .idlP = PWM_IDLE_STATE_INACTIVE,
        .idlN = PWM_IDLE_STATE_ACTIVE,
        .brkP = PWM_BREAK_STATE_INACTIVE,
        .brkN = PWM_BREAK_STATE_ACTIVE,
        .thresholdL = 0,
        .thresholdH = 0,
        .dtg = 0,
    };

    if (NULL == pwm || pwm->port != PWM0_ID || pwm->config.pin % 5 == 0) {
        blog_error("arg error.\r\n");
        return -1;
    }

    ch = pwm->config.pin % 5 - 1;

    PWM_Channelx_Init(pwm->port, ch, &chxCfg);
    return 0;
}

/**
 * De-initialises an PWM interface, Turns off an PWM hardware interface
 *
 * @param[in]  pwm  the interface which should be de-initialised
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int hosal_pwm_finalize(hosal_pwm_dev_t *pwm)
{
    if (NULL == pwm || pwm->port != PWM0_ID) {
        blog_error("arg error.\r\n");
        return -1;
    }

    PWMx_Disable(pwm->port);
    return 0;
}
