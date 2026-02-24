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
#include "bl_dma_uart.h"
#include "bl_irq.h"
#include "hosal_dma.h"
#include "ring_buffer.h"


#define UART_NUM                   (2)
#define UART_SIG_NUM               (UART_NUM * 4)
#define UART_ID_MAX                (UART_NUM - 1)


typedef struct
{
    uint8_t uart_id;
    uint8_t uart_tx_pin;
    uint8_t uart_rx_pin;
    uint8_t uart_tx_sig;
    uint8_t uart_rx_sig;
    uint8_t uart_tx_dma_enable;
    uint32_t uart_baudrate;
    uint8_t *uart_rx_buf;
    uint16_t uart_rx_buf_size;
    bl_dma_uart_callback_t uart_tx_event;
    bl_dma_uart_callback_t uart_rx_event;
    
    DMA_LLI_Ctrl_Type uart_rx_queue[2];
    
    hosal_dma_chan_t uart_tx_dma_ch;
    hosal_dma_chan_t uart_rx_dma_ch;
    
    uint8_t *uartBuf;
    Ring_Buffer_Type uartRB;
    
    uint32_t read_idx;
    uint32_t read_lli_cnt;
}bl_dma_uart_param_t;


static const uint32_t uart_base[UART_NUM] = {UART0_BASE, UART1_BASE};
static const DMA_Periph_Req_Type uart_tx_dma_req[UART_NUM] = {DMA_REQ_UART0_TX, DMA_REQ_UART1_TX};
static const DMA_Periph_Req_Type uart_rx_dma_req[UART_NUM] = {DMA_REQ_UART0_RX, DMA_REQ_UART1_RX};
static bl_dma_uart_param_t *p_dma_uart_param[UART_NUM] = {NULL, NULL};


static void uart_rx_process(bl_dma_uart_param_t *param)
{
    uint32_t write_idx;
    uint32_t write_lli_cnt;
    uint32_t read_idx;
    uint32_t read_lli_cnt;
    
    write_idx = DMA_LLI_Get_Dstaddr(param->uart_rx_dma_ch) - (uint32_t)param->uart_rx_buf;
    write_lli_cnt = DMA_LLI_Get_Counter(param->uart_rx_dma_ch);
    read_idx = param->read_idx;
    read_lli_cnt = param->read_lli_cnt;
    
    if(write_idx > read_idx){
        Ring_Buffer_Write(&param->uartRB, param->uart_rx_buf + read_idx, write_idx - read_idx);
    }else{
        if(write_idx == read_idx && write_lli_cnt == read_lli_cnt){
            // no data available
        }else{
            Ring_Buffer_Write(&param->uartRB, param->uart_rx_buf + read_idx, param->uart_rx_buf_size - read_idx);
            Ring_Buffer_Write(&param->uartRB, param->uart_rx_buf, write_idx);
        }
    }
    
    param->read_idx = write_idx;
    param->read_lli_cnt = write_lli_cnt;
}


static void UART_RTO_Callback(void *ctx)
{
    uint8_t uart_id = (uint8_t)(uint32_t)ctx;
    bl_dma_uart_param_t *param = p_dma_uart_param[uart_id];
    
    UART_IntClear(uart_id, UART_INT_RTO);
    
    uart_rx_process(param);
    
    if(param->uart_rx_event){
        param->uart_rx_event(uart_id);
    }
}

static void UART_RX_DMA_Callback(void *p_arg, uint32_t flag)
{
    uint8_t uart_id = (uint8_t)(uint32_t)p_arg;
    bl_dma_uart_param_t *param = p_dma_uart_param[uart_id];
    
    if(flag == HOSAL_DMA_INT_TRANS_COMPLETE){
        uart_rx_process(param);
        
        if(param->uart_rx_event){
            param->uart_rx_event(uart_id);
        }
    }
}

static void UART_TX_DMA_Callback(void *p_arg, uint32_t flag)
{
    uint8_t uart_id = (uint8_t)(uint32_t)p_arg;
    bl_dma_uart_param_t *param = p_dma_uart_param[uart_id];
    
    if(flag == HOSAL_DMA_INT_TRANS_COMPLETE){
        if(param->uart_tx_event){
            param->uart_tx_event(uart_id);
        }
    }
}


static uint8_t uart_sig(uint8_t pin)
{
    uint32_t tmpVal = BL_RD_REG(GLB_BASE, GLB_PARM);
    uint8_t swapSet = BL_GET_REG_BITS_VAL(tmpVal, GLB_UART_SWAP_SET);
    uint8_t sig = pin % UART_SIG_NUM;

    if (swapSet & (1 << pin / UART_SIG_NUM)) {
        if (sig < UART_SIG_NUM / 2) {
            sig += UART_SIG_NUM / 2;
        } else {
            sig -= UART_SIG_NUM / 2;
        }
    }

    return sig;
}

static void gpio_init(uint8_t uart_id, bl_dma_uart_param_t *param)
{
    GLB_GPIO_Type pinList[] = {param->uart_tx_pin, param->uart_rx_pin};
    GLB_GPIO_Func_Init(GPIO_FUN_UART, pinList, sizeof(pinList)/sizeof(pinList[0]));
    
    GLB_UART_Fun_Sel(param->uart_tx_sig, GLB_UART_SIG_FUN_UART0_TXD + 4 * uart_id);
    GLB_UART_Fun_Sel(param->uart_rx_sig, GLB_UART_SIG_FUN_UART0_RXD + 4 * uart_id);
}

static void uart_init(uint8_t uart_id, bl_dma_uart_param_t *param)
{
    UART_CFG_Type uartCfg = {
        0,                         /* UART clock */
        param->uart_baudrate,      /* baudrate */
        UART_DATABITS_8,           /* data bits */
        UART_STOPBITS_1,           /* stop bits */
        UART_PARITY_NONE,          /* parity */
        DISABLE,                   /* Disable auto flow control */
        DISABLE,                   /* Disable rx input de-glitch function */
        DISABLE,                   /* Disable RTS output SW control mode */
        DISABLE,                   /* Disable tx output SW control mode */
        DISABLE,                   /* Disable tx lin mode */
        DISABLE,                   /* Disable rx lin mode */
        0,                         /* Tx break bit count for lin mode */
        UART_LSB_FIRST,            /* UART each data byte is send out LSB-first */
    };
    
    UART_FifoCfg_Type fifoCfg = {
        0,                         /* TX FIFO threshold */
        0,                         /* RX FIFO threshold */
        param->uart_tx_dma_enable, /* Enable tx dma req/ack interface */
        ENABLE,                    /* Enable rx dma req/ack interface */
    };
    
    GLB_Set_UART_CLK(1, HBN_UART_CLK_FCLK, 0);
    uartCfg.uartClk = SystemCoreClockGet();
    
    UART_DeInit(uart_id);
    UART_Init(uart_id, &uartCfg);
    UART_FifoConfig(uart_id, &fifoCfg);
    UART_TxFreeRun(uart_id, ENABLE);
    UART_SetRxTimeoutValue(uart_id, 80);
    UART_IntMask(uart_id, UART_INT_ALL, MASK);
    UART_IntMask(uart_id, UART_INT_RTO, UNMASK);
    UART_Enable(uart_id, UART_TXRX);
    
    bl_irq_register_with_ctx(UART0_IRQn + uart_id, UART_RTO_Callback, (void *)(uint32_t)uart_id);
    bl_irq_enable(UART0_IRQn + uart_id);
}

static void dma_rx_init(uint8_t uart_id, bl_dma_uart_param_t *param)
{
    DMA_LLI_Cfg_Type lliCfg = {
        .dir = DMA_TRNS_P2M,
        .srcPeriph = uart_rx_dma_req[uart_id],
        .dstPeriph = DMA_REQ_NONE,
    };
    
    struct DMA_Control_Reg dmaCtrlRegVal = {
        .TransferSize = param->uart_rx_buf_size / 2,
        .SBSize = DMA_BURST_SIZE_1,
        .dst_min_mode = DISABLE,
        .DBSize = DMA_BURST_SIZE_1,
        .dst_add_mode = DISABLE,
        .SWidth = DMA_TRNS_WIDTH_8BITS,
        .DWidth = DMA_TRNS_WIDTH_8BITS,
        .fix_cnt = 0,
        .SI = DMA_PINC_DISABLE,
        .DI = DMA_MINC_ENABLE,
        .Prot = 0,
        .I = 1,
    };
    
    param->uart_rx_queue[0].srcDmaAddr = uart_base[uart_id] + 0x8C;
    param->uart_rx_queue[0].destDmaAddr = (uint32_t)&param->uart_rx_buf[0];
    param->uart_rx_queue[0].nextLLI = (uint32_t)&param->uart_rx_queue[1];
    param->uart_rx_queue[0].dmaCtrl = dmaCtrlRegVal;
    
    param->uart_rx_queue[1].srcDmaAddr = uart_base[uart_id] + 0x8C;
    param->uart_rx_queue[1].destDmaAddr = (uint32_t)&param->uart_rx_buf[param->uart_rx_buf_size / 2];
    param->uart_rx_queue[1].nextLLI = (uint32_t)&param->uart_rx_queue[0];
    param->uart_rx_queue[1].dmaCtrl = dmaCtrlRegVal;
    
    DMA_LLI_Init(param->uart_rx_dma_ch, &lliCfg);
    DMA_LLI_Update(param->uart_rx_dma_ch, (uint32_t)&param->uart_rx_queue[0]);
    DMA_Channel_Enable(param->uart_rx_dma_ch);
}

static void dma_tx_init(uint8_t uart_id, bl_dma_uart_param_t *param)
{
    DMA_Channel_Cfg_Type dmaTxCfg = {
        0,                         /* Source address of DMA transfer */
        uart_base[uart_id] + 0x88, /* Destination address of DMA transfer */
        0,                         /* Transfer length, 0~4095, this is burst count */
        DMA_TRNS_M2P,              /* Transfer dir control. 0: Memory to Memory, 1: Memory to peripheral, 2: Peripheral to memory */
        param->uart_tx_dma_ch,     /* Channel select 0-7 */
        DMA_TRNS_WIDTH_8BITS,      /* Transfer width. 0: 8  bits, 1: 16  bits, 2: 32  bits */
        DMA_TRNS_WIDTH_8BITS,      /* Transfer width. 0: 8  bits, 1: 16  bits, 2: 32  bits */
        DMA_BURST_SIZE_1,          /* Number of data items for burst transaction length. Each item width is as same as tansfer width. 0: 1 item, 1: 4 items, 2: 8 items, 3: 16 items */
        DMA_BURST_SIZE_1,          /* Number of data items for burst transaction length. Each item width is as same as tansfer width. 0: 1 item, 1: 4 items, 2: 8 items, 3: 16 items */
        DISABLE,
        DISABLE,
        0,
        DMA_MINC_ENABLE,           /* Source address increment. 0: No change, 1: Increment */
        DMA_PINC_DISABLE,          /* Destination address increment. 0: No change, 1: Increment */
        DMA_REQ_NONE,              /* Source peripheral select */
        uart_tx_dma_req[uart_id],  /* Destination peripheral select */
    };
    
    DMA_Channel_Init(&dmaTxCfg);
}

static void dma_init(uint8_t uart_id, bl_dma_uart_param_t *param)
{
    dma_rx_init(uart_id, param);
    if(param->uart_tx_dma_enable){
        dma_tx_init(uart_id, param);
    }
}


int bl_dma_uart_init(bl_dma_uart_cfg_t *cfg)
{
    hosal_dma_chan_t uart_tx_dma_ch = -1;
    hosal_dma_chan_t uart_rx_dma_ch = -1;
    bl_dma_uart_param_t *param;
    
    if(cfg == NULL){
        return -1;
    }
    
    if(cfg->uart_id > UART_ID_MAX){
        return -2;
    }
    
    // already initialized
    if(p_dma_uart_param[cfg->uart_id] != NULL){
        return -3;
    }
    
    if(cfg->uart_tx_pin > 31 || cfg->uart_rx_pin > 31){
        return -4;
    }
    
    if(uart_sig(cfg->uart_tx_pin) == uart_sig(cfg->uart_rx_pin)){
        return -4;
    }
    
    if(cfg->uart_rx_buf == NULL){
        return -5;
    }
    
    if(cfg->uart_rx_buf_size > 4095 * 2 || cfg->uart_rx_buf_size < 2){
        return -5;
    }
    
    hosal_dma_init();
    
    uart_rx_dma_ch = hosal_dma_chan_request(0);
    if(uart_rx_dma_ch == -1){
        return -6;
    }
    
    if(cfg->uart_tx_dma_enable){
        uart_tx_dma_ch = hosal_dma_chan_request(0);
        if(uart_tx_dma_ch == -1){
            hosal_dma_chan_release(uart_rx_dma_ch);
            return -6;
        }
    }
    
    hosal_dma_irq_callback_set(uart_rx_dma_ch, UART_RX_DMA_Callback, (void *)(uint32_t)cfg->uart_id);
    if(cfg->uart_tx_dma_enable){
        hosal_dma_irq_callback_set(uart_tx_dma_ch, UART_TX_DMA_Callback, (void *)(uint32_t)cfg->uart_id);
    }
    
    param = p_dma_uart_param[cfg->uart_id] = malloc(sizeof(bl_dma_uart_param_t));
    
    param->uart_id = cfg->uart_id;
    param->uart_tx_pin = cfg->uart_tx_pin;
    param->uart_rx_pin = cfg->uart_rx_pin;
    param->uart_tx_sig = uart_sig(cfg->uart_tx_pin);
    param->uart_rx_sig = uart_sig(cfg->uart_rx_pin);
    param->uart_tx_dma_enable = cfg->uart_tx_dma_enable;
    param->uart_baudrate = cfg->uart_baudrate;
    param->uart_rx_buf = cfg->uart_rx_buf;
    param->uart_rx_buf_size = cfg->uart_rx_buf_size;
    param->uart_tx_event = cfg->uart_tx_event;
    param->uart_rx_event = cfg->uart_rx_event;
    
    param->uart_tx_dma_ch = uart_tx_dma_ch;
    param->uart_rx_dma_ch = uart_rx_dma_ch;
    
    param->uartBuf = malloc(param->uart_rx_buf_size);
    Ring_Buffer_Init(&param->uartRB, param->uartBuf, param->uart_rx_buf_size, NULL, NULL);
    
    param->read_idx = 0;
    param->read_lli_cnt = 0;
    
    gpio_init(param->uart_id, param);
    dma_init(param->uart_id, param);
    uart_init(param->uart_id, param);
    
    return 0;
}

int bl_dma_uart_change_baudrate(uint8_t uart_id, uint32_t baudrate)
{
    uint32_t clk;
    uint32_t divisor;
    uint32_t fraction;
    
    if(uart_id > UART_ID_MAX){
        return -1;
    }
    
    if(p_dma_uart_param[uart_id] == NULL){
        return -2;
    }
    
    clk = SystemCoreClockGet();
    divisor = clk / baudrate;
    fraction = clk * 10 / baudrate % 10;
    if(fraction < 5){
        divisor--;
    }
    
    while(UART_GetTxBusBusyStatus(uart_id) && UART_GetRxBusBusyStatus(uart_id));
    
    UART_Disable(uart_id, UART_TXRX);
    BL_WR_REG(uart_base[uart_id], UART_BIT_PRD, (divisor << 16) | divisor);
    UART_Enable(uart_id, UART_TXRX);
    
    return 0;
}

uint16_t bl_dma_uart_get_rx_count(uint8_t uart_id)
{
    bl_dma_uart_param_t *param;
    
    if(uart_id > UART_ID_MAX){
        return 0;
    }
    
    if(p_dma_uart_param[uart_id] == NULL){
        return 0;
    }
    
    param = p_dma_uart_param[uart_id];
    
    return Ring_Buffer_Get_Length(&param->uartRB);
}

uint16_t bl_dma_uart_read(uint8_t uart_id, uint8_t *data, uint16_t len)
{
    bl_dma_uart_param_t *param;
    uint16_t cnt;
    
    if(uart_id > UART_ID_MAX){
        return 0;
    }
    
    if(p_dma_uart_param[uart_id] == NULL){
        return 0;
    }
    
    if(!data || !len){
        return 0;
    }
    
    param = p_dma_uart_param[uart_id];
    
    cnt = Ring_Buffer_Get_Length(&param->uartRB);
    if(cnt < len){
        len = cnt;
    }
    
    Ring_Buffer_Read(&param->uartRB, data, len);
    
    return len;
}

int bl_dma_uart_write(uint8_t uart_id, uint8_t *data, uint16_t len, uint8_t wait_tx_done)
{
    bl_dma_uart_param_t *param;
    
    if(uart_id > UART_ID_MAX){
        return -1;
    }
    
    if(p_dma_uart_param[uart_id] == NULL){
        return -2;
    }
    
    if(!data || !len){
        return -3;
    }
    
    param = p_dma_uart_param[uart_id];
    
    if(param->uart_tx_dma_enable){
        while(DMA_Channel_Is_Busy(param->uart_tx_dma_ch));
        DMA_Channel_Update_SrcMemcfg(param->uart_tx_dma_ch, (uint32_t)data, len);
        DMA_Channel_Enable(param->uart_tx_dma_ch);
        
        if(wait_tx_done){
            while(DMA_Channel_Is_Busy(param->uart_tx_dma_ch));
        }
    }else{
        UART_SendData(uart_id, data, len);
        
        if(wait_tx_done){
            while(UART_GetTxBusBusyStatus(uart_id));
        }
    }
    
    return 0;
}

int bl_dma_uart_deinit(uint8_t uart_id)
{
    bl_dma_uart_param_t *param;
    
    if(uart_id > UART_ID_MAX){
        return -1;
    }
    
    if(p_dma_uart_param[uart_id] == NULL){
        return -2;
    }
    
    param = p_dma_uart_param[uart_id];
    
    UART_DeInit(uart_id);
    
    DMA_Channel_Disable(param->uart_rx_dma_ch);
    hosal_dma_chan_release(param->uart_rx_dma_ch);
    
    if(param->uart_tx_dma_enable){
        DMA_Channel_Disable(param->uart_tx_dma_ch);
        hosal_dma_chan_release(param->uart_tx_dma_ch);
    }
    
    GLB_GPIO_Type pinList[] = {param->uart_tx_pin, param->uart_rx_pin};
    GLB_GPIO_Func_Init(GPIO_FUN_GPIO, pinList, sizeof(pinList)/sizeof(pinList[0]));
    
    GLB_UART_Fun_Sel(param->uart_tx_sig, 0);
    GLB_UART_Fun_Sel(param->uart_rx_sig, 0);
    
    free(param->uartBuf);
    free(param);
    
    p_dma_uart_param[uart_id] = NULL;
    
    return 0;
}


#if 0
// Do not enable CLI if you want to use dma uart function on UART0
// After bl_dma_uart_init with uart_tx_dma_enable=1, do not call UART API like UART_SendData
// After bl_dma_uart_init with uart_tx_dma_enable=1 on UART0, do not call printf/puts/putchar unless CONFIG_GPIO_SIM_PRINT is set
void bl_dma_uart_test(void)
{
    uint8_t rx_buf[32];
    uint8_t tx_buf[32];
    uint16_t len;
    int ret;
    
    bl_dma_uart_cfg_t cfg = {
        .uart_id = 0,
        .uart_tx_pin = 14,
        .uart_rx_pin = 15,
        .uart_tx_dma_enable = 0,
        .uart_baudrate = 2000000,
        .uart_rx_buf = rx_buf,
        .uart_rx_buf_size = sizeof(rx_buf),
        .uart_tx_event = NULL,
        .uart_rx_event = NULL,
    };
    
    ret = bl_dma_uart_init(&cfg);
    if(cfg.uart_tx_dma_enable == 0){
        printf("bl_dma_uart_init: %d\r\n", ret);
    }
    
    while(1){
        len = bl_dma_uart_get_rx_count(cfg.uart_id);
        if(len){
            bl_dma_uart_read(cfg.uart_id, tx_buf, len);
            bl_dma_uart_write(cfg.uart_id, tx_buf, len, 1);
        }
    }
}
#endif
