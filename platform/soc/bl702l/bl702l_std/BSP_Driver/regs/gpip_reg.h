/**
  ******************************************************************************
  * @file    gpip_reg.h
  * @version V1.0
  * @date    2022-05-27
  * @brief   This file is the description of.IP register
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2020 Bouffalo Lab</center></h2>
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
  *
  ******************************************************************************
  */
#ifndef __GPIP_REG_H__
#define __GPIP_REG_H__

#include "bl702l.h"

/* 0x0 : gpadc_config */
#define GPIP_GPADC_CONFIG_OFFSET           (0x0)
#define GPIP_GPADC_DMA_EN                  GPIP_GPADC_DMA_EN
#define GPIP_GPADC_DMA_EN_POS              (0U)
#define GPIP_GPADC_DMA_EN_LEN              (1U)
#define GPIP_GPADC_DMA_EN_MSK              (((1U << GPIP_GPADC_DMA_EN_LEN) - 1) << GPIP_GPADC_DMA_EN_POS)
#define GPIP_GPADC_DMA_EN_UMSK             (~(((1U << GPIP_GPADC_DMA_EN_LEN) - 1) << GPIP_GPADC_DMA_EN_POS))
#define GPIP_GPADC_FIFO_CLR                GPIP_GPADC_FIFO_CLR
#define GPIP_GPADC_FIFO_CLR_POS            (1U)
#define GPIP_GPADC_FIFO_CLR_LEN            (1U)
#define GPIP_GPADC_FIFO_CLR_MSK            (((1U << GPIP_GPADC_FIFO_CLR_LEN) - 1) << GPIP_GPADC_FIFO_CLR_POS)
#define GPIP_GPADC_FIFO_CLR_UMSK           (~(((1U << GPIP_GPADC_FIFO_CLR_LEN) - 1) << GPIP_GPADC_FIFO_CLR_POS))
#define GPIP_GPADC_FIFO_NE                 GPIP_GPADC_FIFO_NE
#define GPIP_GPADC_FIFO_NE_POS             (2U)
#define GPIP_GPADC_FIFO_NE_LEN             (1U)
#define GPIP_GPADC_FIFO_NE_MSK             (((1U << GPIP_GPADC_FIFO_NE_LEN) - 1) << GPIP_GPADC_FIFO_NE_POS)
#define GPIP_GPADC_FIFO_NE_UMSK            (~(((1U << GPIP_GPADC_FIFO_NE_LEN) - 1) << GPIP_GPADC_FIFO_NE_POS))
#define GPIP_GPADC_FIFO_FULL               GPIP_GPADC_FIFO_FULL
#define GPIP_GPADC_FIFO_FULL_POS           (3U)
#define GPIP_GPADC_FIFO_FULL_LEN           (1U)
#define GPIP_GPADC_FIFO_FULL_MSK           (((1U << GPIP_GPADC_FIFO_FULL_LEN) - 1) << GPIP_GPADC_FIFO_FULL_POS)
#define GPIP_GPADC_FIFO_FULL_UMSK          (~(((1U << GPIP_GPADC_FIFO_FULL_LEN) - 1) << GPIP_GPADC_FIFO_FULL_POS))
#define GPIP_GPADC_RDY                     GPIP_GPADC_RDY
#define GPIP_GPADC_RDY_POS                 (4U)
#define GPIP_GPADC_RDY_LEN                 (1U)
#define GPIP_GPADC_RDY_MSK                 (((1U << GPIP_GPADC_RDY_LEN) - 1) << GPIP_GPADC_RDY_POS)
#define GPIP_GPADC_RDY_UMSK                (~(((1U << GPIP_GPADC_RDY_LEN) - 1) << GPIP_GPADC_RDY_POS))
#define GPIP_GPADC_FIFO_OVERRUN            GPIP_GPADC_FIFO_OVERRUN
#define GPIP_GPADC_FIFO_OVERRUN_POS        (5U)
#define GPIP_GPADC_FIFO_OVERRUN_LEN        (1U)
#define GPIP_GPADC_FIFO_OVERRUN_MSK        (((1U << GPIP_GPADC_FIFO_OVERRUN_LEN) - 1) << GPIP_GPADC_FIFO_OVERRUN_POS)
#define GPIP_GPADC_FIFO_OVERRUN_UMSK       (~(((1U << GPIP_GPADC_FIFO_OVERRUN_LEN) - 1) << GPIP_GPADC_FIFO_OVERRUN_POS))
#define GPIP_GPADC_FIFO_UNDERRUN           GPIP_GPADC_FIFO_UNDERRUN
#define GPIP_GPADC_FIFO_UNDERRUN_POS       (6U)
#define GPIP_GPADC_FIFO_UNDERRUN_LEN       (1U)
#define GPIP_GPADC_FIFO_UNDERRUN_MSK       (((1U << GPIP_GPADC_FIFO_UNDERRUN_LEN) - 1) << GPIP_GPADC_FIFO_UNDERRUN_POS)
#define GPIP_GPADC_FIFO_UNDERRUN_UMSK      (~(((1U << GPIP_GPADC_FIFO_UNDERRUN_LEN) - 1) << GPIP_GPADC_FIFO_UNDERRUN_POS))
#define GPIP_GPADC_FIFO_RDY                GPIP_GPADC_FIFO_RDY
#define GPIP_GPADC_FIFO_RDY_POS            (7U)
#define GPIP_GPADC_FIFO_RDY_LEN            (1U)
#define GPIP_GPADC_FIFO_RDY_MSK            (((1U << GPIP_GPADC_FIFO_RDY_LEN) - 1) << GPIP_GPADC_FIFO_RDY_POS)
#define GPIP_GPADC_FIFO_RDY_UMSK           (~(((1U << GPIP_GPADC_FIFO_RDY_LEN) - 1) << GPIP_GPADC_FIFO_RDY_POS))
#define GPIP_GPADC_RDY_CLR                 GPIP_GPADC_RDY_CLR
#define GPIP_GPADC_RDY_CLR_POS             (8U)
#define GPIP_GPADC_RDY_CLR_LEN             (1U)
#define GPIP_GPADC_RDY_CLR_MSK             (((1U << GPIP_GPADC_RDY_CLR_LEN) - 1) << GPIP_GPADC_RDY_CLR_POS)
#define GPIP_GPADC_RDY_CLR_UMSK            (~(((1U << GPIP_GPADC_RDY_CLR_LEN) - 1) << GPIP_GPADC_RDY_CLR_POS))
#define GPIP_GPADC_FIFO_OVERRUN_CLR        GPIP_GPADC_FIFO_OVERRUN_CLR
#define GPIP_GPADC_FIFO_OVERRUN_CLR_POS    (9U)
#define GPIP_GPADC_FIFO_OVERRUN_CLR_LEN    (1U)
#define GPIP_GPADC_FIFO_OVERRUN_CLR_MSK    (((1U << GPIP_GPADC_FIFO_OVERRUN_CLR_LEN) - 1) << GPIP_GPADC_FIFO_OVERRUN_CLR_POS)
#define GPIP_GPADC_FIFO_OVERRUN_CLR_UMSK   (~(((1U << GPIP_GPADC_FIFO_OVERRUN_CLR_LEN) - 1) << GPIP_GPADC_FIFO_OVERRUN_CLR_POS))
#define GPIP_GPADC_FIFO_UNDERRUN_CLR       GPIP_GPADC_FIFO_UNDERRUN_CLR
#define GPIP_GPADC_FIFO_UNDERRUN_CLR_POS   (10U)
#define GPIP_GPADC_FIFO_UNDERRUN_CLR_LEN   (1U)
#define GPIP_GPADC_FIFO_UNDERRUN_CLR_MSK   (((1U << GPIP_GPADC_FIFO_UNDERRUN_CLR_LEN) - 1) << GPIP_GPADC_FIFO_UNDERRUN_CLR_POS)
#define GPIP_GPADC_FIFO_UNDERRUN_CLR_UMSK  (~(((1U << GPIP_GPADC_FIFO_UNDERRUN_CLR_LEN) - 1) << GPIP_GPADC_FIFO_UNDERRUN_CLR_POS))
#define GPIP_GPADC_RDY_MASK                GPIP_GPADC_RDY_MASK
#define GPIP_GPADC_RDY_MASK_POS            (12U)
#define GPIP_GPADC_RDY_MASK_LEN            (1U)
#define GPIP_GPADC_RDY_MASK_MSK            (((1U << GPIP_GPADC_RDY_MASK_LEN) - 1) << GPIP_GPADC_RDY_MASK_POS)
#define GPIP_GPADC_RDY_MASK_UMSK           (~(((1U << GPIP_GPADC_RDY_MASK_LEN) - 1) << GPIP_GPADC_RDY_MASK_POS))
#define GPIP_GPADC_FIFO_OVERRUN_MASK       GPIP_GPADC_FIFO_OVERRUN_MASK
#define GPIP_GPADC_FIFO_OVERRUN_MASK_POS   (13U)
#define GPIP_GPADC_FIFO_OVERRUN_MASK_LEN   (1U)
#define GPIP_GPADC_FIFO_OVERRUN_MASK_MSK   (((1U << GPIP_GPADC_FIFO_OVERRUN_MASK_LEN) - 1) << GPIP_GPADC_FIFO_OVERRUN_MASK_POS)
#define GPIP_GPADC_FIFO_OVERRUN_MASK_UMSK  (~(((1U << GPIP_GPADC_FIFO_OVERRUN_MASK_LEN) - 1) << GPIP_GPADC_FIFO_OVERRUN_MASK_POS))
#define GPIP_GPADC_FIFO_UNDERRUN_MASK      GPIP_GPADC_FIFO_UNDERRUN_MASK
#define GPIP_GPADC_FIFO_UNDERRUN_MASK_POS  (14U)
#define GPIP_GPADC_FIFO_UNDERRUN_MASK_LEN  (1U)
#define GPIP_GPADC_FIFO_UNDERRUN_MASK_MSK  (((1U << GPIP_GPADC_FIFO_UNDERRUN_MASK_LEN) - 1) << GPIP_GPADC_FIFO_UNDERRUN_MASK_POS)
#define GPIP_GPADC_FIFO_UNDERRUN_MASK_UMSK (~(((1U << GPIP_GPADC_FIFO_UNDERRUN_MASK_LEN) - 1) << GPIP_GPADC_FIFO_UNDERRUN_MASK_POS))
#define GPIP_GPADC_FIFO_RDY_MASK           GPIP_GPADC_FIFO_RDY_MASK
#define GPIP_GPADC_FIFO_RDY_MASK_POS       (15U)
#define GPIP_GPADC_FIFO_RDY_MASK_LEN       (1U)
#define GPIP_GPADC_FIFO_RDY_MASK_MSK       (((1U << GPIP_GPADC_FIFO_RDY_MASK_LEN) - 1) << GPIP_GPADC_FIFO_RDY_MASK_POS)
#define GPIP_GPADC_FIFO_RDY_MASK_UMSK      (~(((1U << GPIP_GPADC_FIFO_RDY_MASK_LEN) - 1) << GPIP_GPADC_FIFO_RDY_MASK_POS))
#define GPIP_GPADC_FIFO_DATA_COUNT         GPIP_GPADC_FIFO_DATA_COUNT
#define GPIP_GPADC_FIFO_DATA_COUNT_POS     (16U)
#define GPIP_GPADC_FIFO_DATA_COUNT_LEN     (6U)
#define GPIP_GPADC_FIFO_DATA_COUNT_MSK     (((1U << GPIP_GPADC_FIFO_DATA_COUNT_LEN) - 1) << GPIP_GPADC_FIFO_DATA_COUNT_POS)
#define GPIP_GPADC_FIFO_DATA_COUNT_UMSK    (~(((1U << GPIP_GPADC_FIFO_DATA_COUNT_LEN) - 1) << GPIP_GPADC_FIFO_DATA_COUNT_POS))
#define GPIP_GPADC_FIFO_THL                GPIP_GPADC_FIFO_THL
#define GPIP_GPADC_FIFO_THL_POS            (22U)
#define GPIP_GPADC_FIFO_THL_LEN            (2U)
#define GPIP_GPADC_FIFO_THL_MSK            (((1U << GPIP_GPADC_FIFO_THL_LEN) - 1) << GPIP_GPADC_FIFO_THL_POS)
#define GPIP_GPADC_FIFO_THL_UMSK           (~(((1U << GPIP_GPADC_FIFO_THL_LEN) - 1) << GPIP_GPADC_FIFO_THL_POS))

/* 0x4 : gpadc_dma_rdata */
#define GPIP_GPADC_DMA_RDATA_OFFSET (0x4)
#define GPIP_GPADC_DMA_RDATA        GPIP_GPADC_DMA_RDATA
#define GPIP_GPADC_DMA_RDATA_POS    (0U)
#define GPIP_GPADC_DMA_RDATA_LEN    (26U)
#define GPIP_GPADC_DMA_RDATA_MSK    (((1U << GPIP_GPADC_DMA_RDATA_LEN) - 1) << GPIP_GPADC_DMA_RDATA_POS)
#define GPIP_GPADC_DMA_RDATA_UMSK   (~(((1U << GPIP_GPADC_DMA_RDATA_LEN) - 1) << GPIP_GPADC_DMA_RDATA_POS))

struct gpip_reg {
    /* 0x0 : gpadc_config */
    union {
        struct {
            uint32_t gpadc_dma_en             : 1; /* [    0],        r/w,        0x0 */
            uint32_t gpadc_fifo_clr           : 1; /* [    1],        w1c,        0x0 */
            uint32_t gpadc_fifo_ne            : 1; /* [    2],          r,        0x0 */
            uint32_t gpadc_fifo_full          : 1; /* [    3],          r,        0x0 */
            uint32_t gpadc_rdy                : 1; /* [    4],          r,        0x0 */
            uint32_t gpadc_fifo_overrun       : 1; /* [    5],          r,        0x0 */
            uint32_t gpadc_fifo_underrun      : 1; /* [    6],          r,        0x0 */
            uint32_t gpadc_fifo_rdy           : 1; /* [    7],          r,        0x0 */
            uint32_t gpadc_rdy_clr            : 1; /* [    8],        w1c,        0x0 */
            uint32_t gpadc_fifo_overrun_clr   : 1; /* [    9],        w1c,        0x0 */
            uint32_t gpadc_fifo_underrun_clr  : 1; /* [   10],        w1c,        0x0 */
            uint32_t reserved_11              : 1; /* [   11],       rsvd,        0x0 */
            uint32_t gpadc_rdy_mask           : 1; /* [   12],        r/w,        0x0 */
            uint32_t gpadc_fifo_overrun_mask  : 1; /* [   13],        r/w,        0x0 */
            uint32_t gpadc_fifo_underrun_mask : 1; /* [   14],        r/w,        0x0 */
            uint32_t gpadc_fifo_rdy_mask      : 1; /* [   15],        r/w,        0x1 */
            uint32_t gpadc_fifo_data_count    : 6; /* [21:16],          r,        0x0 */
            uint32_t gpadc_fifo_thl           : 2; /* [23:22],        r/w,        0x0 */
            uint32_t rsvd_31_24               : 8; /* [31:24],       rsvd,        0x0 */
        } BF;
        uint32_t WORD;
    } gpadc_config;

    /* 0x4 : gpadc_dma_rdata */
    union {
        struct {
            uint32_t gpadc_dma_rdata : 26; /* [25: 0],          r,        0x0 */
            uint32_t rsvd_31_26      : 6;  /* [31:26],       rsvd,        0x0 */
        } BF;
        uint32_t WORD;
    } gpadc_dma_rdata;
};

typedef volatile struct gpip_reg gpip_reg_t;

#endif /* __GPIP_REG_H__ */