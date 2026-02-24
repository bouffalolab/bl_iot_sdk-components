/**
 * @file peripheral_config.h
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

#ifndef _PERIPHERAL_CONFIG_H_
#define _PERIPHERAL_CONFIG_H_

/* PERIPHERAL USING LIST */
#define BSP_USING_ADC0
#define BSP_USING_UART0
#define BSP_USING_SPI0
#define BSP_USING_I2C0
#define BSP_USING_TIMER0
#define BSP_USING_TIMER1
#define BSP_USING_WDT
/* ----------------------*/

/* PERIPHERAL With DMA LIST */
#define BSP_USING_DMA
#define BSP_USING_DMA0_CH0
#define BSP_USING_DMA0_CH1
#define BSP_USING_DMA0_CH2
#define BSP_USING_DMA0_CH3

/* PERIPHERAL CONFIG */
#if defined(BSP_USING_ADC0)
#ifndef ADC0_CONFIG
#define ADC0_CONFIG                                        \
    {                                                      \
        .clk_div = ADC_CLOCK_DIV_32,                       \
        .vref = ADC_VREF_3V2,                              \
        .continuous_conv_mode = DISABLE,                   \
        .differential_mode = DISABLE,                      \
        .data_width = ADC_DATA_WIDTH_16B_WITH_256_AVERAGE, \
        .fifo_threshold = ADC_FIFO_THRESHOLD_1BYTE,        \
        .gain = ADC_GAIN_1                                 \
    }
#endif
#endif

#if defined(BSP_USING_UART0)
#ifndef UART0_CONFIG
#define UART0_CONFIG                 \
    {                                \
        .id = 0,                     \
        .baudrate = 2000000,         \
        .databits = UART_DATA_LEN_8, \
        .stopbits = UART_STOP_ONE,   \
        .parity = UART_PAR_NONE,     \
        .fifo_threshold = 0,         \
    }
#endif
#endif

#if defined(BSP_USING_SPI0)
#ifndef SPI0_CONFIG
#define SPI0_CONFIG                                 \
    {                                               \
        .id = 0,                                    \
        .clk = 18000000,                            \
        .mode = SPI_MASTER_MODE,                    \
        .direction = SPI_MSB_BYTE0_DIRECTION_FIRST, \
        .clk_polaraity = SPI_POLARITY_LOW,          \
        .clk_phase = SPI_PHASE_1EDGE,               \
        .datasize = SPI_DATASIZE_8BIT,              \
        .fifo_threshold = 0,                        \
        .pin_swap_enable = 1,                       \
        .delitch_cnt = 0,                           \
    }
#endif
#endif

#if defined(BSP_USING_DMA0_CH0)
#ifndef DMA0_CH0_CONFIG
#define DMA0_CH0_CONFIG                            \
    {                                              \
        .id = 0,                                   \
        .ch = 0,                                   \
        .direction = DMA_MEMORY_TO_MEMORY,         \
        .transfer_mode = DMA_LLI_ONCE_MODE,        \
        .src_req = DMA_REQUEST_NONE,               \
        .dst_req = DMA_REQUEST_NONE,               \
        .src_addr_inc = DMA_ADDR_INCREMENT_ENABLE, \
        .dst_addr_inc = DMA_ADDR_INCREMENT_ENABLE, \
        .src_burst_size = DMA_BURST_SIZE_4,        \
        .dst_burst_size = DMA_BURST_SIZE_4,        \
        .src_width = DMA_TRANSFER_WIDTH_32BIT,     \
        .dst_width = DMA_TRANSFER_WIDTH_32BIT,     \
    }
#endif
#endif

#if defined(BSP_USING_DMA0_CH1)
#ifndef DMA0_CH1_CONFIG
#define DMA0_CH1_CONFIG                            \
    {                                              \
        .id = 0,                                   \
        .ch = 1,                                   \
        .direction = DMA_MEMORY_TO_MEMORY,         \
        .transfer_mode = DMA_LLI_ONCE_MODE,        \
        .src_req = DMA_REQUEST_NONE,               \
        .dst_req = DMA_REQUEST_NONE,               \
        .src_addr_inc = DMA_ADDR_INCREMENT_ENABLE, \
        .dst_addr_inc = DMA_ADDR_INCREMENT_ENABLE, \
        .src_burst_size = DMA_BURST_SIZE_4,        \
        .dst_burst_size = DMA_BURST_SIZE_4,        \
        .src_width = DMA_TRANSFER_WIDTH_16BIT,     \
        .dst_width = DMA_TRANSFER_WIDTH_16BIT,     \
    }
#endif
#endif

#if defined(BSP_USING_DMA0_CH2)
#ifndef DMA0_CH2_CONFIG
#define DMA0_CH2_CONFIG                             \
    {                                               \
        .id = 0,                                    \
        .ch = 2,                                    \
        .direction = DMA_MEMORY_TO_PERIPH,          \
        .transfer_mode = DMA_LLI_ONCE_MODE,         \
        .src_req = DMA_REQUEST_NONE,                \
        .dst_req = DMA_REQUEST_NONE,                \
        .src_addr_inc = DMA_ADDR_INCREMENT_ENABLE,  \
        .dst_addr_inc = DMA_ADDR_INCREMENT_DISABLE, \
        .src_burst_size = DMA_BURST_SIZE_1,         \
        .dst_burst_size = DMA_BURST_SIZE_1,         \
        .src_width = DMA_TRANSFER_WIDTH_8BIT,       \
        .dst_width = DMA_TRANSFER_WIDTH_8BIT,       \
    }
#endif
#endif

#if defined(BSP_USING_DMA0_CH3)
#ifndef DMA0_CH3_CONFIG
#define DMA0_CH3_CONFIG                             \
    {                                               \
        .id = 0,                                    \
        .ch = 3,                                    \
        .direction = DMA_MEMORY_TO_PERIPH,          \
        .transfer_mode = DMA_LLI_ONCE_MODE,         \
        .src_req = DMA_REQUEST_NONE,                \
        .dst_req = DMA_REQUEST_NONE,                \
        .src_addr_inc = DMA_ADDR_INCREMENT_ENABLE,  \
        .dst_addr_inc = DMA_ADDR_INCREMENT_DISABLE, \
        .src_burst_size = DMA_BURST_SIZE_1,         \
        .dst_burst_size = DMA_BURST_SIZE_1,         \
        .src_width = DMA_TRANSFER_WIDTH_8BIT,       \
        .dst_width = DMA_TRANSFER_WIDTH_8BIT,       \
    }
#endif
#endif

#if defined(BSP_USING_I2C0)
#ifndef I2C0_CONFIG
#define I2C0_CONFIG          \
    {                        \
        .id = 0,             \
        .mode = I2C_HW_MODE, \
        .phase = 15,         \
    }
#endif
#endif

#if defined(BSP_USING_TIMER0)
#ifndef TIMER0_CONFIG
#define TIMER0_CONFIG                           \
    {                                           \
        .id = 0,                                \
        .cnt_mode = TIMER_CNT_PRELOAD,          \
        .trigger = TIMER_PRELOAD_TRIGGER_COMP2, \
        .reload = 0,                            \
        .timeout1 = 1000000,                    \
        .timeout2 = 2000000,                    \
        .timeout3 = 3000000,                    \
    }
#endif
#endif

#if defined(BSP_USING_TIMER1)
#ifndef TIMER1_CONFIG
#define TIMER1_CONFIG                           \
    {                                           \
        .id = 1,                                \
        .cnt_mode = TIMER_CNT_PRELOAD,          \
        .trigger = TIMER_PRELOAD_TRIGGER_COMP0, \
        .reload = 0,                            \
        .timeout1 = 1000000,                    \
        .timeout2 = 2000000,                    \
        .timeout3 = 3000000,                    \
    }
#endif
#endif

#if defined(BSP_USING_WDT)
#ifndef WDT_CONFIG
#define WDT_CONFIG           \
    {                        \
        .id = 0,             \
        .wdt_timeout = 6000, \
    }
#endif
#endif

#endif
