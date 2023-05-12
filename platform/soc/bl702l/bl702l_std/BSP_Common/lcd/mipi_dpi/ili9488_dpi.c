/**
 * @file ili9488_dpi.c
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

#if defined(LCD_DPI_ILI9488) && (defined(BL808) || defined(BL616))

#include "bflb_platform.h"

#include "bl808_dvp_tsrc.h"
#include "bl808_isp_misc.h"
#include "bl808_dbi.h"
#include "bl808_glb.h"

#include "ili9488_dpi.h"

static volatile uint8_t dpi_swap_flag = 0;

/* dbi config */
static DBI_CFG_Type dbiCfg = {
    .mode = DBI_TYPE_C_4_WIRE,                          /* DBI type B or C select */
    .pixelFormat = DBI_PIXEL_RGB888,                    /* DBI pixel format */
    .fifoFormat = DBI_FIFO_888_NBGR,                    /* DBI fifo format */
    .continueEn = ENABLE,                               /* Enable:CS will stay asserted between each consecutive pixel, disable:CS will de-assert between each pixel */
    .dummyEn = DISABLE,                                 /* Enable:dummy cycle will be inserted between command phase adn data phase, disable:no dummy cycle */
    .dummyCnt = 0,                                      /* Dummy cycle count,effective only in type C(fixed to 1 in type B) */
    .clkPhase = DBI_SCL_CLK_PHASE_0,                    /* DBI clock phase */
    .clkPolarity = DBI_SCL_CLK_POLARITY_LOW,            /* DBI clock polarity */
    .period.startLen = 9,
    .period.dataPhase0Len = 9,
    .period.dataPhase1Len = 9,
    .period.intervalLen = 9
};

static DVP_TSRC_Cfg_Type dvpTsrcCfg = {
    .dataFromSensor = DISABLE,                          /* Enable: pixel data is from sensor, disable: pixel data is from AXI */
    .sensorHsyncInverse = DISABLE,                      /* Enable or disable inverse signal of sensor hsync */
    .sensorVsyncInverse = DISABLE,                      /* Enable or disable inverse signal of sensor vsync */
    .yuv420Enable = DISABLE,                            /* Enable or disable YUV420 mode, YUV420 data is from 2 different planar buffers when enable */
    .lineType = DVP_TSRC_YUV420_LINE_EVEN,              /* Select UV send in Y even lines or odd lines */
    .swapMode = ENABLE,                                 /* Enable or disable swap mode */
    .swapControl = DVP_TSRC_SWAP_SOFTWARE,              /* Set swap index controlled by hardware or software */
    .dvp2axi = DVP_TSRC_DVP2AXI_0,                      /* Choose dvp2axi used */
#if (ILI9488_DPI_PIXEL_FORMAT == ILI9488_DPI_PIXEL_FORMAT_RGB565)
    .format = DVP_TSRC_PIXEL_RGB565_16TO24BIT,          /* Set pixel data format */
    .byte0 = 0,                                         /* Byte 0 selection */
    .byte1 = 2,                                         /* Byte 1 Selection */
    .byte2 = 1,                                         /* Byte 2 Selection */
#elif (ILI9488_DPI_PIXEL_FORMAT == ILI9488_DPI_PIXEL_FORMAT_RGB888)
    .format = DVP_TSRC_PIXEL_RGBA8888_32TO24BIT,        /* Set pixel data format */
    .byte0 = 0,                                         /* Byte 0 selection */
    .byte1 = 1,                                         /* Byte 1 Selection */
    .byte2 = 2,                                         /* Byte 2 Selection */
#endif
    .burst = DVP_TSRC_BURST_TYPE_INCR16,                /* AXI burst length */
    .hTotalCnt = 349,                                   /* Horizontal total pixel count */
    .hBlankCnt = 29,                                    /* Horizontal blank stage pixel count */
    .vTotalCnt = 499,                                   /* Vertical total pixel count */
    .vBlankCnt = 19,                                    /* Vertical blank stage pixel count */

    .prefetch = 0,                                      /* Vertical prefetch start position, relativeto blank start position */
    .fifoThreshold = ILI9488_DPI_W,                   /* FIFO threshold for each DVP line to start to output */
    .memStartY0 = 0,                                    /* AXI2DVP start address, Y-planar in YUV420 mode, frame 0 in swap mode */
    .memSizeY0 = sizeof(ili9488_dpi_color_t) * ILI9488_DPI_W * ILI9488_DPI_H, /* AXI2DVP memory size of memStartY0 */
    .memStartY1 = 0,                                 /* AXI2DVP start address, Y-planar in YUV420 mode,  frame 1 in swap mode, don't care if not swap mode */
    .memStartUV0 = 0,                                   /* AXI2DVP start address, UV-planar in YUV420 mode, frame 0 in swap mode, don't care if not YUV420 mode */
    .memStartUV1 = 0,                                   /* AXI2DVP start address, UV-planar in YUV420 mode, frame 1 in swap mode, don't care if not YUV420 swap mode */
};

static ISP_MISC_Display_Cfg_Type displayCfg = {
    .dpiEnable = ENABLE,                                /* Enable or disable dpi function */
    .bt1120Enable = DISABLE,                            /* Enable or disable BT1120 function, BT1120 and BT656 should not be enabled at the same time */
    .inputType = ISP_MISC_DISPLAY_TSRC_RGB_OUTPUT,      /* Select display input */
    .osdType = ISP_MISC_DISPLAY_OSD_RGB2YUV422_OUTPUT,  /* Select display OSD input */
    .hsyncWidth = 5,                                    /* Horizontal synchronization width */
    .hfpWidth = 17,                                     /* Horizontal front porch width */
    .vsyncWidth = 3,                                    /* Vertical synchronization width */
    .vfpWidth = 7,                                      /* Vertical front porch width */
};

static const ili9488_dpi_init_cmd_t ili9488_dpi_mode_init_cmds[] = {
    { 0x01, NULL, 0 },          /* software reset */
    { 0xFF, NULL, 10 },         /* delay 10ms */

    { 0x11, NULL, 0 },          /* Sleep Out */
    { 0xFF, NULL, 120 },        /* delay 120ms */

    { 0xE0, "\x00\x07\x0F\x0D\x1B\x0A\x3C\x78\x4A\x07\x0E\x09\x1B\x1E\x0F", 15}, /* PGAMCTRL (Positive Gamma Control) */
    { 0xE1, "\x00\x22\x24\x26\x12\x07\x36\x47\x47\x06\x0A\x07\x30\x37\x0F", 15}, /* NGAMCTRL (Negative Gamma Control) */

    { 0xC0, "\x10\x10", 2},     /* Power Control 1 */
    { 0xC1, "\x41", 1},         /* Power Control 2 */

    { 0xC5, "\x00\x20\xd0", 3}, /* VCOM Control */
    { 0x36, "\x08", 1},         /* Memory Access Control */

    { 0x3A, "\x50", 1},         /* Interface Pixel 16bits/pixel*/

    { 0xB0, "\x00", 1},         /* Interface Mode Control */
    { 0xB1, "\xB0", 1},         /* Frame rate 70Hz */
    { 0xB4, "\x02", 1},         /* Display Inversion Control */
    // { 0xB5, "\x08\x08\x06\x12", 4},/* Display Inversion Control */
    { 0xB6, "\x30\x22\x3B", 3}, /* Display Function Control, DE Mode */
    { 0xBE, "\x00\x04", 1},
    { 0xE9, "\x00", 1},         /* Set Image Function */

    { 0xF7, "\xA9\x51\x2C\x82", 4}, /* Adjust Control 3 */

    { 0x29, NULL, 0 },          /* Display On */
    { 0xFF, NULL, 10 },
};

// void dpi_buff_swap_callback(void)
// {
//     dpi_swap_flag = 1;
//     ISP_MISC_Int_Clear(ISP_MISC_INT_DISPLAY);
// }

static int ili9488_dpi_peripheral_init(ili9488_dpi_color_t *screen_buffer)
{
    /* MIPI clock config */
    GLB_Config_MIPI_PLL(GLB_XTAL_40M, mipiPllCfg_1500M);
    GLB_Set_Display_CLK(ENABLE, GLB_DISP_CLK_MIPIPLL_1500M, 125);

    /* DBI config, Ili9488 needs to be initialized using the DBI(typeC) or SPI interface */
    DBI_Init(&dbiCfg);

    dvpTsrcCfg.memStartY0 = (uint32_t)(uintptr_t)screen_buffer;

    /* DPI config */
    DVP_TSRC_Set_Swap_Index(DVP_TSRC1_ID,1);
    DVP_TSRC_Init(DVP_TSRC1_ID,&dvpTsrcCfg);
    ISP_MISC_Display_Init(&displayCfg);

    /* Set display interrupt */
    ISP_MISC_Int_Mask(ISP_MISC_INT_ALL,MASK);
    ISP_MISC_SEOF_Set_Source(ISP_MISC_INT_DISPLAY,ISP_MISC_SEOF_DISPLAY_OUTPUT);
    ISP_MISC_SEOF_Set_Edge(ISP_MISC_INT_DISPLAY,ISP_MISC_SEOF_VSYNC_NEGEDGE);
    // ISP_MISC_Int_Callback_Install(ISP_MISC_INT_DISPLAY,&dpi_buff_swap_callback);
    // ISP_MISC_Int_Mask(ISP_MISC_INT_DISPLAY,UNMASK);
    CPU_Interrupt_Enable(DISPLAY_IRQn);
    System_NVIC_SetPriority(DISPLAY_IRQn,4,1);
    return 0;
}

int ili9488_dpi_init(ili9488_dpi_color_t *screen_buffer)
{
    if(screen_buffer == NULL){
        return -1;
    }

    /* init DBI and DPI peripheral*/
    ili9488_dpi_peripheral_init(screen_buffer);

    /* Ili9488 needs to be initialized using the DBI(typeC) or SPI interface */
    for (uint16_t i = 0; i < (sizeof(ili9488_dpi_mode_init_cmds) / sizeof(ili9488_dpi_init_cmd_t)); i++) {
        if(ili9488_dpi_mode_init_cmds[i].cmd == 0xFF){
            bflb_platform_delay_ms(ili9488_dpi_mode_init_cmds[i].databytes);
        }else{
            DBI_SetPhaseState(ENABLE, DISABLE);
            DBI_SendCmdWithNormalData(ili9488_dpi_mode_init_cmds[i].cmd, 0, NULL);
            DBI_SetPhaseState(DISABLE, ENABLE);
            for(uint8_t j=0; j<ili9488_dpi_mode_init_cmds[i].databytes; j++){
                DBI_SendCmdWithNormalData(0xFF, 1, (uint8_t *)&ili9488_dpi_mode_init_cmds[i].data[j]);
            }
        }
    }
    DBI_SetPhaseState(DISABLE, DISABLE);

    /* enable DPI, Start output data*/
    DVP_TSRC_Enable(DVP_TSRC1_ID);
    return 0;
}

int ili9488_dpi_screen_switch(ili9488_dpi_color_t *screen_buffer)
{
    uint32_t tmpVal;

    if(screen_buffer == NULL){
        return -1;
    }

    csi_dcache_clean_range((void *)screen_buffer, (sizeof(ili9488_dpi_color_t) * ILI9488_DPI_W * ILI9488_DPI_H));

    tmpVal = BL_RD_REG(DVP_TSRC1_BASE, DTSRC_CONFIG);
    tmpVal = BL_GET_REG_BITS_VAL(tmpVal, DTSRC_CR_AXI_SWAP_IDX_SWV);
    if(tmpVal){
        BL_WR_REG(DVP_TSRC1_BASE, DTSRC_AXI2DVP_SWAP_ADDR_BY, (uint32_t)(uintptr_t)screen_buffer);
        DVP_TSRC_Set_Swap_Index(DVP_TSRC1_ID, 0);
    }else{
        BL_WR_REG(DVP_TSRC1_BASE, DTSRC_AXI2DVP_START_ADDR_BY, (uint32_t)(uintptr_t)screen_buffer);
        DVP_TSRC_Set_Swap_Index(DVP_TSRC1_ID, 1);
    }

    // dpi_swap_flag = 0;
    // while(dpi_swap_flag == 0){
    // }

    return 0;
}

ili9488_dpi_color_t *ili9488_dpi_get_screen_using(void)
{
    uint32_t tmpVal;
    tmpVal = BL_RD_REG(DVP_TSRC1_BASE, DTSRC_CONFIG);
    tmpVal = BL_GET_REG_BITS_VAL(tmpVal, DTSRC_CR_AXI_SWAP_IDX_SWV);

    if(tmpVal){
        tmpVal = BL_RD_REG(DVP_TSRC1_BASE, DTSRC_AXI2DVP_START_ADDR_BY);
    }else{
        tmpVal = BL_RD_REG(DVP_TSRC1_BASE, DTSRC_AXI2DVP_SWAP_ADDR_BY);
    }
    return (ili9488_dpi_color_t *)(uintptr_t)tmpVal;
}

#elif defined(LCD_DPI_ILI9488)
    #error "Devices that do not support DPI! Replace the driver port (lcd.h)"
#endif
