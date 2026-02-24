/*
 * Copyright (c) 2016-2026 Bouffalolab.
 *
 * This file is part of
 *     *** Bouffalolab Software Dev Kit ***
 *      (see www.bouffalolab.com).
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
 */
#include "bl_gpio_i2c.h"
#include "bl702l_glb.h"


static uint8_t i2c_scl = 0;
static uint8_t i2c_sda = 0;
static uint8_t i2c_ok = 0;


#define SET_SCL_LO()               GLB_GPIO_OUTPUT_Enable(i2c_scl)
#define SET_SCL_HI()               GLB_GPIO_OUTPUT_Disable(i2c_scl)
#define GET_SCL()                  GLB_GPIO_Read(i2c_scl)
#define SET_SDA_LO()               GLB_GPIO_OUTPUT_Enable(i2c_sda)
#define SET_SDA_HI()               GLB_GPIO_OUTPUT_Disable(i2c_sda)
#define GET_SDA()                  GLB_GPIO_Read(i2c_sda)
#define I2C_DELAY()                arch_delay_us(10)


int bl_gpio_i2c_init(uint8_t scl_pin, uint8_t sda_pin)
{
    if(scl_pin > 31 || sda_pin > 31){
        return ERR_INIT;
    }
    
    if(scl_pin == sda_pin){
        return ERR_INIT;
    }
    
    GLB_GPIO_Cfg_Type cfg = {
        .gpioPin = 0,
        .gpioFun = GPIO_FUN_GPIO,
        .gpioMode = GPIO_MODE_INPUT,  // default input, will not drive SCL/SDA
        .pullType = GPIO_PULL_NONE,
        .drive = 0,
        .smtCtrl = 1,
    };
    
    cfg.gpioPin = scl_pin;
    GLB_GPIO_Init(&cfg);
    GLB_GPIO_Write(scl_pin, 0);  // preset output value, not yet valid
    
    cfg.gpioPin = sda_pin;
    GLB_GPIO_Init(&cfg);
    GLB_GPIO_Write(sda_pin, 0);  // preset output value, not yet valid
    
    i2c_scl = scl_pin;
    i2c_sda = sda_pin;
    i2c_ok = 1;
    
    return 0;
}

int bl_gpio_i2c_start_write(uint8_t dev_addr)
{
    if(!i2c_ok){
        return ERR_INIT;
    }
    
    // Start Condition:
    // SDA goes low before SCL
    SET_SDA_LO();
    I2C_DELAY();
    SET_SCL_LO();
    
    // Set write operation
    dev_addr <<= 1;
    
    return bl_gpio_i2c_write_byte(dev_addr);
}

int bl_gpio_i2c_start_read(uint8_t dev_addr)
{
    if(!i2c_ok){
        return ERR_INIT;
    }
    
    // Start Condition:
    // SDA goes low before SCL
    SET_SDA_LO();
    I2C_DELAY();
    SET_SCL_LO();
    
    // Set read operation
    dev_addr <<= 1;
    dev_addr |= 1;
    
    return bl_gpio_i2c_write_byte(dev_addr);
}

int bl_gpio_i2c_write_byte(uint8_t data)
{
    uint8_t nack;
    
    if(!i2c_ok){
        return ERR_INIT;
    }
    
    for(uint8_t mask = 0x80; mask != 0x00; mask >>= 1){
        if(data & mask){
            SET_SDA_HI();
        }else{
            SET_SDA_LO();
        }
        
        // Set SCL high to indicate a new bit is available
        SET_SCL_HI();
        I2C_DELAY();
        SET_SCL_LO();
    }
    
    // Let the slave drive SDA
    SET_SDA_HI();
    I2C_DELAY();
    
    // Read Ack/Nak
    SET_SCL_HI();
    while(GET_SCL() == 0);  // clock stretching
    I2C_DELAY();
    nack = GET_SDA();
    
    // Prepare for next operation
    SET_SCL_LO();
    I2C_DELAY();
    
    if(nack){
        return ERR_NAK;
    }
    
    return 0;
}

int bl_gpio_i2c_read_byte(uint8_t *data)
{
    if(!i2c_ok){
        return ERR_INIT;
    }
    
    // Let the slave drive SDA
    SET_SDA_HI();
    I2C_DELAY();
    
    *data = 0;
    for(int i=0; i<8; i++){
        // Read bit
        SET_SCL_HI();
        while(GET_SCL() == 0);  // clock stretching
        I2C_DELAY();
        *data <<= 1;
        *data |= GET_SDA();
        
        // Prepare for next bit
        SET_SCL_LO();
        I2C_DELAY();
    }
    
    return 0;
}

int bl_gpio_i2c_ack(void)
{
    if(!i2c_ok){
        return ERR_INIT;
    }
    
    SET_SDA_LO();
    SET_SCL_HI();
    I2C_DELAY();
    SET_SCL_LO();
    
    return 0;
}

int bl_gpio_i2c_nak(void)
{
    if(!i2c_ok){
        return ERR_INIT;
    }
    
    SET_SDA_HI();
    SET_SCL_HI();
    I2C_DELAY();
    SET_SCL_LO();
    
    return 0;
}

int bl_gpio_i2c_repeat(void)
{
    if(!i2c_ok){
        return ERR_INIT;
    }
    
    SET_SDA_HI();
    I2C_DELAY();
    SET_SCL_HI();
    while(GET_SCL() == 0);  // clock stretching
    I2C_DELAY();  // repeated start setup time, minimum 4.7us
    
    return 0;
}

int bl_gpio_i2c_stop(void)
{
    if(!i2c_ok){
        return ERR_INIT;
    }
    
    SET_SDA_LO();
    I2C_DELAY();
    
    // Stop Condition:
    // SDA goes high after SCL
    SET_SCL_HI();
    while(GET_SCL() == 0);  // clock stretching
    I2C_DELAY();  // stop bit setup time, minimum 4us
    SET_SDA_HI();
    
    return 0;
}
