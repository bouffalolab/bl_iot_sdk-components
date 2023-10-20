/**
 * Copyright (c) 2016-2021 Bouffalolab Co., Ltd.
 *
 * Contact information:
 * web site:    https://www.bouffalolab.com/
 */

#include <FreeRTOS.h>
#include <task.h>
#include "bl702l.h"
#include "bl702l_i2c.h"
#include "bl702l_gpio.h"
#include "bl702l_glb.h"
#include "hosal_i2c.h"
#include "blog.h"

int hosal_i2c_init(hosal_i2c_dev_t *i2c)
{
    GLB_GPIO_Type gpiopins[2];

    if (NULL == i2c || i2c->port != 0) {
        blog_error("parameter is error!\r\n");
        return -1;
    }

    GLB_PER_Clock_UnGate(GLB_AHB_CLOCK_I2C);
    GLB_Set_I2C_CLK(1, GLB_I2C_CLK_SRC_BCLK, 0);

    gpiopins[0] = i2c->config.scl;
    gpiopins[1] = i2c->config.sda;

    GLB_GPIO_Func_Init(GPIO_FUN_I2C, gpiopins, sizeof(gpiopins) / sizeof(gpiopins[0]));
    I2C_Disable(i2c->port);

    return 0;
}

int hosal_i2c_master_send(hosal_i2c_dev_t *i2c, uint16_t dev_addr, const uint8_t *data,
                            uint16_t size, uint32_t timeout)
{
    I2C_Transfer_Cfg i2c_cfg_send = {
        .slaveAddr = 0,
        .slaveAddr10Bit = DISABLE,
        .stopEveryByte = DISABLE,
        .subAddrSize = 0,
        .subAddr = 0x00,
        .dataSize = 0,
        .data = NULL,
        .clk = 0,
    };

    if (NULL == i2c || NULL == data || size < 1) {
        blog_error("parameter is error!\r\n");
        return -1;
    }

    i2c_cfg_send.clk = i2c->config.freq;
    i2c_cfg_send.slaveAddr = dev_addr;
    i2c_cfg_send.data = (uint8_t *)data;
    i2c_cfg_send.dataSize = size;

    return I2C_MasterSendBlocking(0, &i2c_cfg_send);
}

int hosal_i2c_master_recv(hosal_i2c_dev_t *i2c, uint16_t dev_addr, uint8_t *data,
                            uint16_t size, uint32_t timeout)
{
    I2C_Transfer_Cfg i2c_cfg_recv = {
        .slaveAddr = 0,
        .slaveAddr10Bit = DISABLE,
        .stopEveryByte = DISABLE,
        .subAddrSize = 0,
        .subAddr = 0x00,
        .dataSize = 0,
        .data = NULL,
        .clk = 0,
    };

    if (NULL == i2c || NULL == data || size < 1) {
        blog_error("parameter is error!\r\n");
        return -1;
    }

    i2c_cfg_recv.clk = i2c->config.freq;
    i2c_cfg_recv.slaveAddr = dev_addr;
    i2c_cfg_recv.data = (uint8_t *)data;
    i2c_cfg_recv.dataSize = size;

    return I2C_MasterReceiveBlocking(0, &i2c_cfg_recv);
}

int hosal_i2c_slave_send(hosal_i2c_dev_t *i2c, const uint8_t *data, uint16_t size, uint32_t timeout)
{
    if (NULL == i2c || NULL == data || size < 1) {
        blog_error("parameter is error!\r\n");
        return -1;
    }

    blog_error("not support now!\r\n");

    return -1;
}

int hosal_i2c_slave_recv(hosal_i2c_dev_t *i2c, uint8_t *data, uint16_t size, uint32_t timeout)
{
    if (NULL == i2c || NULL == data || size < 1) {
        blog_error("parameter is error!\r\n");
        return -1;
    }

    blog_error("not support now!\r\n");

    return -1;
}

int hosal_i2c_mem_write(hosal_i2c_dev_t *i2c, uint16_t dev_addr, uint32_t mem_addr,
                          uint16_t mem_addr_size, const uint8_t *data, uint16_t size,
                          uint32_t timeout)
{
    I2C_Transfer_Cfg i2c_cfg_send = {
        .slaveAddr = 0,
        .slaveAddr10Bit = DISABLE,
        .stopEveryByte = DISABLE,
        .subAddrSize = 0,
        .subAddr = 0x00,
        .dataSize = 0,
        .data = NULL,
        .clk = 0,
    };

    if (NULL == i2c || NULL == data || size < 1) {
        blog_error("parameter is error!\r\n");
        return -1;
    }

    i2c_cfg_send.clk = i2c->config.freq;
    i2c_cfg_send.slaveAddr = dev_addr;
    i2c_cfg_send.subAddr = mem_addr;
    i2c_cfg_send.subAddrSize = mem_addr_size;
    i2c_cfg_send.data = (uint8_t *)data;
    i2c_cfg_send.dataSize = size;

    return I2C_MasterSendBlocking(0, &i2c_cfg_send);
}

int hosal_i2c_mem_read(hosal_i2c_dev_t *i2c, uint16_t dev_addr, uint32_t mem_addr,
                         uint16_t mem_addr_size, uint8_t *data, uint16_t size,
                         uint32_t timeout)
{
    I2C_Transfer_Cfg i2c_cfg_recv = {
        .slaveAddr = 0,
        .slaveAddr10Bit = DISABLE,
        .stopEveryByte = DISABLE,
        .subAddrSize = 0,
        .subAddr = 0x00,
        .dataSize = 0,
        .data = NULL,
        .clk = 0,
    };

    if (NULL == i2c || NULL == data || size < 1) {
        blog_error("parameter is error!\r\n");
        return -1;
    }

    i2c_cfg_recv.clk = i2c->config.freq;
    i2c_cfg_recv.slaveAddr = dev_addr;
    i2c_cfg_recv.subAddr = mem_addr;
    i2c_cfg_recv.subAddrSize = mem_addr_size;
    i2c_cfg_recv.data = (uint8_t *)data;
    i2c_cfg_recv.dataSize = size;

    return I2C_MasterReceiveBlocking(0, &i2c_cfg_recv);
}

int hosal_i2c_finalize(hosal_i2c_dev_t *i2c)
{
    if (NULL == i2c || i2c->port != 0) {
        blog_error("parameter is error!\r\n");
        return -1;
    }

    I2C_Disable(i2c->port);

    return 0;
}
