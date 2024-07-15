#include <stdio.h>
#include <stdint.h>
#include <bl_pwm.h>
#include <hosal_pwm.h>
#include <blog.h>


/**
 * Initialises a PWM device
 *
 * @param[in]  pwm  the PWM device
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int hosal_pwm_init(hosal_pwm_dev_t *pwm)
{
    if (pwm == NULL) {
        blog_error("arg null.\r\n");
        return -1;
    }

    if (pwm->port > 4) {
        blog_error("arg error.\r\n");
        return -1;
    }

    if (pwm->config.pin > 31) {
        blog_error("arg error.\r\n");
        return -1;
    }

    if (pwm->port != pwm->config.pin % 5) {
        blog_error("pwm port and pin do not match!\r\n");
        return -1;
    }

    bl_pwm_port_init(pwm->port, pwm->config.freq);
    bl_pwm_gpio_init(pwm->port, pwm->config.pin);
    bl_pwm_set_duty(pwm->port, pwm->config.duty_cycle / 100);

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
    if (pwm == NULL) {
        blog_error("arg null.\r\n");
        return -1;
    }

    if (pwm->port > 4) {
        blog_error("arg error.\r\n");
        return -1;
    }

    bl_pwm_start(pwm->port);

    return 0;
}

/**
 * Changes parameters of the PWM device
 *
 * @param[in]  pwm  the PWM device
 * @param[in]  para  the new parameters (frequency and duty cycle)
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int hosal_pwm_para_chg(hosal_pwm_dev_t *pwm, hosal_pwm_config_t para)
{
    if (pwm == NULL) {
        blog_error("arg null.\r\n");
        return -1;
    }

    if (pwm->port > 4) {
        blog_error("arg error.\r\n");
        return -1;
    }

    pwm->config.freq = para.freq;
    bl_pwm_port_init(pwm->port, pwm->config.freq);

    pwm->config.duty_cycle = para.duty_cycle;
    bl_pwm_set_duty(pwm->port, pwm->config.duty_cycle / 100);

    bl_pwm_start(pwm->port);

    return 0;
}

/**
 * Changes frequency of the PWM device
 *
 * @param[in]  pwm  the PWM device
 * @param[in]  freq  the new frequency
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int hosal_pwm_freq_set(hosal_pwm_dev_t *pwm, uint32_t freq)
{
    if (pwm == NULL) {
        blog_error("arg null.\r\n");
        return -1;
    }

    if (pwm->port > 4) {
        blog_error("arg error.\r\n");
        return -1;
    }

    pwm->config.freq = freq;
    bl_pwm_port_init(pwm->port, pwm->config.freq);

    bl_pwm_set_duty(pwm->port, pwm->config.duty_cycle / 100);

    bl_pwm_start(pwm->port);

    return 0;
}

int hosal_pwm_freq_get(hosal_pwm_dev_t *pwm,  uint32_t *p_freq)
{
    if (pwm == NULL) {
        blog_error("arg null.\r\n");
        return -1;
    }

    if (pwm->port > 4) {
        blog_error("arg error.\r\n");
        return -1;
    }

    *p_freq = pwm->config.freq;

    return 0;
}

/**
 * Changes duty cycle of the PWM device
 *
 * @param[in]  pwm  the PWM device
 * @param[in]  duty  the new duty cycle
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int hosal_pwm_duty_set(hosal_pwm_dev_t *pwm, uint32_t duty)
{
    if (pwm == NULL) {
        blog_error("arg null.\r\n");
        return -1;
    }

    if (pwm->port > 4) {
        blog_error("arg error.\r\n");
        return -1;
    }

    pwm->config.duty_cycle = duty;
    bl_pwm_set_duty(pwm->port, pwm->config.duty_cycle / 100);

    return 0;
}

int hosal_pwm_duty_get(hosal_pwm_dev_t *pwm, uint32_t *p_duty)
{
    float duty;

    if (pwm == NULL) {
        blog_error("arg null.\r\n");
        return -1;
    }

    if (pwm->port > 4) {
        blog_error("arg error.\r\n");
        return -1;
    }

    bl_pwm_get_duty(pwm->port, &duty);
    *p_duty = (uint32_t)(duty * 100);

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
    if (pwm == NULL) {
        blog_error("arg null.\r\n");
        return -1;
    }

    if (pwm->port > 4) {
        blog_error("arg error.\r\n");
        return -1;
    }

    bl_pwm_stop(pwm->port);

    return 0;
}

/**
 * De-initialises the PWM device
 *
 * @param[in]  pwm  the PWM device
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int hosal_pwm_finalize(hosal_pwm_dev_t *pwm)
{
    if (pwm == NULL) {
        blog_error("arg null.\r\n");
        return -1;
    }

    if (pwm->port > 4) {
        blog_error("arg error.\r\n");
        return -1;
    }

    bl_pwm_stop(pwm->port);

    return 0;
}
