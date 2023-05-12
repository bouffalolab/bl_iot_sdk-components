/**
 * @file ili9341_spi.c
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

#include "../lcd.h"

#if defined(LCD_SPI_ILI9341)

#include "ili9341_spi.h"
#include "hal_spi.h"
#include "hal_dma.h"
#include "hal_gpio.h"

#if (defined(BL808) || defined(BL606P)) && defined(CPU_D0)
  #define  ILI9341_SPI_ID    1
  #define  ILI9341_SPI_INDEX  SPI1_INDEX
  #define  ILI9341_SPI_DMA_INDEX  DMA2_CH0_INDEX
#else
  #define  ILI9341_SPI_ID    0
  #define  ILI9341_SPI_INDEX  SPI0_INDEX
  #define  ILI9341_SPI_DMA_INDEX  DMA0_CH3_INDEX
#endif

static struct device *ili9341_spi;
static struct device *dma_spi_tx;

const ili9341_spi_init_cmd_t ili9341_spi_init_cmds[] = {
    { 0x01, NULL, 0 },
    { 0xFF, NULL, 20 },
    { 0x11, NULL, 0 }, /* Exit sleep */
    { 0xFF, NULL, 120 },

    { 0xCF, "\x00\xd9\x30", 3 },
    { 0xED, "\x64\x03\x12\x81", 4 },
    { 0xE8, "\x85\x01\x78", 3 },
    { 0xCB, "\x39\x2C\x00\x34\x02", 5 },
    { 0xF7, "\x20", 1 },
    { 0xEA, "\x00\x00", 2 },

    { 0xC0, "\x23", 1 },    /*Power control*/
    { 0xC1, "\x12", 1 },    /*Power control */
    { 0xC2, "\x11", 1 },
    { 0xC5, "\x40\x30", 2 }, /*VCOM control 1*/
    { 0xC7, "\xa9", 1 },     /*VCOM control 2*/
    { 0x36, "\x08", 1 },     /*Memory Access Control*/
    { 0x3A, "\x55", 1 },     /*Pixel Format Set*/
    { 0xB1, "\x00\x18", 2 }, /* Frame Rate Control */
    { 0xB6, "\x0a\xa2", 2 }, /* Display Function Control */
    { 0x0C, "\xd5", 1 },     /* display pixel format,RGB 16bits,MCU 16bits  */
    { 0xF2, "\x00", 1 },     /* 3Gamma Function Disable */
    { 0xF7, "\x20", 1 },

    { 0x26, "\x01", 1 },                                                          /* Gamma curve selected */
    { 0xE0, "\x1F\x1A\x18\x0A\x0F\x06\x45\x87\x32\x0A\x07\x02\x07\x05\x00", 15 }, /* Set Gamma */
    { 0XE1, "\x00\x25\x27\x05\x10\x09\x3A\x78\x4D\x05\x18\x0D\x38\x3A\x1F", 15 }, /* Set Gamma */
    { 0xB7, "\x07", 1 },

    { 0x29, NULL, 0 }, /* Display on */
    { 0xFF, NULL, 20 },
};

/**
 * @brief ili9341_spi_init
 *
 * @return int  0:succes  1:error
 */
static int ili9341_spi_peripheral_init(void)
{
    gpio_set_mode(ILI9341_SPI_CS_PIN, GPIO_OUTPUT_MODE);
    gpio_set_mode(ILI9341_SPI_DC_PIN, GPIO_OUTPUT_MODE);
    ILI9341_SPI_CS_HIGH;
    ILI9341_SPI_DC_HIGH;

    ili9341_spi = device_find("lcd_dev_ifs");
    if (ili9341_spi) {
        device_close(ili9341_spi);
    } else {
        spi_register(ILI9341_SPI_INDEX, "lcd_dev_ifs");
        ili9341_spi = device_find("lcd_dev_ifs");
    }
    if (ili9341_spi) {
        SPI_DEV(ili9341_spi)->mode = SPI_MASTER_MODE;
#if defined(BL808) || defined(BL606P) || defined(BL616)
        SPI_DEV(ili9341_spi)->clk = (40 * 1000 * 1000);
        SPI_DEV(ili9341_spi)->direction = SPI_MSB_BYTE3_DIRECTION_FIRST;
#else
        SPI_DEV(ili9341_spi)->clk = (36 * 1000 * 1000);
        SPI_DEV(ili9341_spi)->direction = SPI_MSB_BYTE0_DIRECTION_FIRST;
#endif
        SPI_DEV(ili9341_spi)->datasize = SPI_DATASIZE_8BIT;
        SPI_DEV(ili9341_spi)->clk_polaraity = SPI_POLARITY_LOW;
        SPI_DEV(ili9341_spi)->clk_phase = SPI_PHASE_1EDGE;
#if (SPI_FIFO_LEN >= 16)
        SPI_DEV(ili9341_spi)->fifo_threshold = 7;
#else
        SPI_DEV(ili9341_spi)->fifo_threshold = 0;
#endif

        device_open(ili9341_spi, DEVICE_OFLAG_STREAM_TX);
    } else {
        return 1;
    }

    dma_spi_tx = device_find("lcd_dev_ifs_dma");
    if (dma_spi_tx) {
        device_close(dma_spi_tx);
    } else {
        dma_register(ILI9341_SPI_DMA_INDEX, "lcd_dev_ifs_dma");
        dma_spi_tx = device_find("lcd_dev_ifs_dma");
    }
    if (dma_spi_tx) {
        DMA_DEV(dma_spi_tx)->direction = DMA_MEMORY_TO_PERIPH;
        DMA_DEV(dma_spi_tx)->transfer_mode = DMA_LLI_ONCE_MODE;
        DMA_DEV(dma_spi_tx)->src_req = DMA_REQUEST_NONE;
#if (ILI9341_SPI_ID == 0)
        DMA_DEV(dma_spi_tx)->dst_req = DMA_REQUEST_SPI0_TX;
#elif (ILI9341_SPI_ID == 1)
        DMA_DEV(dma_spi_tx)->dst_req = DMA_REQUEST_SPI1_TX;
#endif
        DMA_DEV(dma_spi_tx)->src_addr_inc = DMA_ADDR_INCREMENT_ENABLE;
        DMA_DEV(dma_spi_tx)->dst_addr_inc = DMA_ADDR_INCREMENT_DISABLE;
#if (SPI_FIFO_LEN >= 16)
        DMA_DEV(dma_spi_tx)->src_burst_size = DMA_BURST_INCR4;
        DMA_DEV(dma_spi_tx)->dst_burst_size = DMA_BURST_INCR4;
#else
        DMA_DEV(dma_spi_tx)->src_burst_size = DMA_BURST_INCR1;
        DMA_DEV(dma_spi_tx)->dst_burst_size = DMA_BURST_INCR1;
#endif
        DMA_DEV(dma_spi_tx)->src_width = DMA_TRANSFER_WIDTH_16BIT;
        DMA_DEV(dma_spi_tx)->dst_width = DMA_TRANSFER_WIDTH_16BIT;

        device_open(dma_spi_tx, 0);
        device_set_callback(dma_spi_tx, NULL);
        device_control(ili9341_spi, DEVICE_CTRL_ATTACH_TX_DMA, dma_spi_tx);
        device_control(dma_spi_tx, DEVICE_CTRL_CLR_INT, NULL);
    } else {
        return 1;
    }

    device_control(ili9341_spi, DEVICE_CTRL_TX_DMA_SUSPEND, NULL);
    return 0;
}

/**
 * @brief ili9341_spi_write_cmd
 *
 * @param cmd
 * @return int 0:succes  1:error
 */
static int ili9341_spi_write_cmd(uint8_t cmd)
{
    ILI9341_SPI_DC_LOW;
    ILI9341_SPI_CS_LOW;
    int res = device_write(ili9341_spi, 0, &cmd, 1);
    ILI9341_SPI_CS_HIGH;
    ILI9341_SPI_DC_HIGH;
    return res;
}

/**
 * @brief ili9341_spi_write_data_byte
 *
 * @param data
 * @return int 0:succes  1:error
 */
static int ili9341_spi_write_data_byte(uint8_t data)
{
    ILI9341_SPI_CS_LOW;
    int res = device_write(ili9341_spi, 0, &data, 1);
    ILI9341_SPI_CS_HIGH;
    return res;
}


/**
 * @brief ili9341_spi_draw_is_busy, After the call ili9341_spi_draw_picture_dma must check this,
 *         if ili9341_spi_draw_is_busy() == 1, Don't allow other draw !!
 *         can run in the DMA interrupt callback function.
 *
 * @return int 0:draw end; 1:Being draw
 */
int ili9341_spi_draw_is_busy(void)
{
    if (dma_channel_check_busy(SPI_DEV(ili9341_spi)->tx_dma)) {
        return 1;
    } else {
        /* Wait for tx FIFO to be empty */
        while(device_control(ili9341_spi, DEVICE_CTRL_SPI_GET_TX_FIFO, NULL) < SPI_FIFO_LEN);
        /* Wait for the SPI bus to be idle */
        while(device_control(ili9341_spi, DEVICE_CTRL_SPI_GET_BUS_BUSY_STATUS, NULL) != 0);
        /* Switch the SPI to non-DMA mode */
        device_control(ili9341_spi, DEVICE_CTRL_TX_DMA_SUSPEND, NULL);
        ILI9341_SPI_CS_HIGH;

        device_control(ili9341_spi, DEVICE_CTRL_SPI_SET_FRAME_SIZE, (void *)SPI_DATASIZE_8BIT);
        return 0;
    }
}

/**
 * @brief ili9341_spi_init
 *
 * @return int
 */
int ili9341_spi_init()
{
    int res = ili9341_spi_peripheral_init();
    if (res) {
        return res;
    }

    for (uint16_t i = 0; i < (sizeof(ili9341_spi_init_cmds) / sizeof(ili9341_spi_init_cmd_t)); i++) {
        if(ili9341_spi_init_cmds[i].cmd == 0xFF){
            bflb_platform_delay_ms(ili9341_spi_init_cmds[i].databytes);
        }else{
            /* send register address */
            res |= ili9341_spi_write_cmd(ili9341_spi_init_cmds[i].cmd);

            /* send register data */
            for (uint8_t j = 0; j < (ili9341_spi_init_cmds[i].databytes & 0x7F); j++) {
                res |= ili9341_spi_write_data_byte(ili9341_spi_init_cmds[i].data[j]);
            }

            if (res) {
                return res;
            }
        }
    }
    ili9341_spi_set_draw_window(0, 0, ILI9341_SPI_H, ILI9341_SPI_W);
    return res;
}

#if defined(BL808) || defined(BL606P) || defined(BL616)
#else

/**
 * @brief lcd_swap_color_data16
 *
 * @param dst destination
 * @param src source
 * @param color_num color num
 * @return int
 */
static int lcd_swap_color_data16(uint16_t *dst, uint16_t *src, uint32_t color_num)
{
    for (size_t i = 0; i < color_num; i++) {
        dst[i] = (src[i] << 8) | (src[i] >> 8);
    }
    return 0;
}

#endif

/**
 * @brief
 *
 * @param dir
 * @param mir_flag
 */
int ili9341_spi_set_dir(uint8_t dir, uint8_t mir_flag)
{
    ili9341_spi_write_cmd(0x36);
    switch (dir) {
        case 0:
            if (!mir_flag)
                ili9341_spi_write_data_byte(0x08);
            else
                ili9341_spi_write_data_byte(0x48);
            break;
        case 1:
            if (!mir_flag)
                ili9341_spi_write_data_byte(0xA8);
            else
                ili9341_spi_write_data_byte(0x28);
            break;
        case 2:
            if (!mir_flag)
                ili9341_spi_write_data_byte(0xC8);
            else
                ili9341_spi_write_data_byte(0x88);
            break;
        case 3:
            if (!mir_flag)
                ili9341_spi_write_data_byte(0x68);
            else
                ili9341_spi_write_data_byte(0xE8);

            break;
        default:
            return -1;
            break;
    }
    return dir;
}

/**
 * @brief ili9341_spi_set_draw_window
 *
 * @param x1
 * @param y1
 * @param x2
 * @param y2
 */
void ili9341_spi_set_draw_window(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2)
{
#if ILI9341_SPI_OFFSET_X
    x1 += ILI9341_SPI_OFFSET_X;
    x2 += ILI9341_SPI_OFFSET_X;
#endif
#if ILI9341_SPI_OFFSET_Y
    y1 += ILI9341_SPI_OFFSET_Y;
    y2 += ILI9341_SPI_OFFSET_Y;
#endif

    ili9341_spi_write_cmd(0x2a);
    ili9341_spi_write_data_byte(x1>>8);
    ili9341_spi_write_data_byte(x1);
    ili9341_spi_write_data_byte(x2>>8);
    ili9341_spi_write_data_byte(x2);

    ili9341_spi_write_cmd(0x2b);
    ili9341_spi_write_data_byte(y1>>8);
    ili9341_spi_write_data_byte(y1);
    ili9341_spi_write_data_byte(y2>>8);
    ili9341_spi_write_data_byte(y2);

    ili9341_spi_write_cmd(0x2c);
}

/**
 * @brief ili9341_spi_draw_point
 *
 * @param x
 * @param y
 * @param color
 */
void ili9341_spi_draw_point(uint16_t x, uint16_t y, ili9341_spi_color_t color)
{
#if defined(BL808) || defined(BL606P) || defined(BL616)
#else
    color = ((color >> 8) & 0xFF) | color << 8;
#endif

    ili9341_spi_set_draw_window(x, y, x, y);
    ILI9341_SPI_DC_HIGH;
    ILI9341_SPI_CS_LOW;
    device_write(ili9341_spi, 0, &color, 2);
    ILI9341_SPI_CS_HIGH;
}

/**
 * @brief ili9341_draw_area
 *
 * @param x1
 * @param y1
 * @param x2
 * @param y2
 * @param color
 */
void ili9341_spi_draw_area(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, ili9341_spi_color_t color)
{
    uint32_t pixelDataSize, PixelCount = (x2- x1 + 1) * (y2 - y1 + 1);
    uint32_t color_src;

#if defined(BL808) || defined(BL606P) || defined(BL616)
    color_src = color;
    csi_dcache_clean_range((void *)(&color_src), sizeof(color_src));
#else
    color_src = ((color >> 8) & 0xFF) | color << 8;
#endif

    ili9341_spi_set_draw_window(x1, y1, x2, y2);

    /* get pixel Data Size */
    pixelDataSize = PixelCount << 1;

    device_control(ili9341_spi, DEVICE_CTRL_SPI_SET_FRAME_SIZE, (void *)SPI_DATASIZE_16BIT);

    /* disable src_addr_inc */
    device_control(dma_spi_tx, DEVICE_CTRL_DMA_CONFIG_SI, (void*)DISABLE);

    ILI9341_SPI_CS_LOW;

    /* dma write */
    device_control(ili9341_spi, DEVICE_CTRL_TX_DMA_RESUME, NULL);
    device_write(ili9341_spi, 0, &color_src, pixelDataSize);

    /* Wait to finish, and cs high */
    while (ili9341_spi_draw_is_busy()) {
    };

    /* enable src_addr_inc */
    device_control(dma_spi_tx, DEVICE_CTRL_DMA_CONFIG_SI, (void*)ENABLE);
}

/**
 * @brief ili9341_draw_picture_dma, Non-blocking! Using DMA acceleration, Not waiting for the draw end
 *  After the call, No other operations are allowed until (ili9341_draw_is_busy()==0)
 *
 * @param x1
 * @param y1
 * @param x2
 * @param y2
 * @param picture
 */
void ili9341_spi_draw_picture_nonblocking(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, ili9341_spi_color_t *picture)
{
    uint8_t ret = 0;
    size_t picture_size = (x2 - x1 + 1) * (y2 - y1 + 1);

    ili9341_spi_set_draw_window(x1, y1, x2, y2);

#if defined(BL808) || defined(BL606P) || defined(BL616)
    csi_dcache_clean_range((void *)(picture), (picture_size<<1));
#else
    lcd_swap_color_data16(picture, picture, picture_size);
#endif

    device_control(ili9341_spi, DEVICE_CTRL_SPI_SET_FRAME_SIZE, (void *)SPI_DATASIZE_16BIT);

    ILI9341_SPI_CS_LOW;

    device_control(ili9341_spi, DEVICE_CTRL_TX_DMA_RESUME, NULL);
    ret = device_write(ili9341_spi, 0, picture, (picture_size<<1));
    if (ret != 0) {
        MSG("device write fail!\r\n");
    }
}

/**
 * @brief ili9341_draw_picture,Blocking,Using DMA acceleration,Waiting for the draw end
 *
 * @param x1
 * @param y1
 * @param x2
 * @param y2
 * @param picture
 */
void ili9341_spi_draw_picture_blocking(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, ili9341_spi_color_t *picture)
{
    ili9341_spi_draw_picture_nonblocking(x1, y1, x2, y2, picture);
    while (ili9341_spi_draw_is_busy()) {
    };
}

#endif
