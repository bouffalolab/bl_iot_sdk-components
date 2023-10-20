#include "bl_dma_uart.h"
#include "bl_irq.h"
#include "hosal_dma.h"
#include "ring_buffer.h"


static uint8_t uart_ok = 0;
static hosal_dma_chan_t uart_tx_dma_ch = -1;
static hosal_dma_chan_t uart_rx_dma_ch = -1;
static uint8_t uart_id = 0;
static uint8_t uart_tx_pin = 0;
static uint8_t uart_rx_pin = 0;
static uint32_t uart_base = 0;
static DMA_Periph_Req_Type uart_tx_dma_req = DMA_REQ_NONE;
static DMA_Periph_Req_Type uart_rx_dma_req = DMA_REQ_NONE;
static uint8_t *uart_rx_buf = NULL;
static uint16_t uart_rx_buf_size = 0;
static bl_dma_uart_callback_t uart_tx_event = NULL;
static bl_dma_uart_callback_t uart_rx_event = NULL;
static DMA_LLI_Ctrl_Type uart_rx_queue[2] __attribute((aligned(4)));

static uint8_t *uartBuf = NULL;
static Ring_Buffer_Type uartRB;
static uint32_t read_idx = 0;
static uint32_t read_lli_cnt = 0;


static void uart_rx_process(void)
{
    uint32_t write_idx;
    uint32_t write_lli_cnt;
    
    write_idx = DMA_LLI_Get_Dstaddr(DMA0_ID, uart_rx_dma_ch) - (uint32_t)uart_rx_buf;
    write_lli_cnt = DMA_LLI_Get_Counter(DMA0_ID, uart_rx_dma_ch);
    
    if(write_idx > read_idx){
        Ring_Buffer_Write(&uartRB, uart_rx_buf + read_idx, write_idx - read_idx);
    }else{
        if(write_idx == read_idx && write_lli_cnt == read_lli_cnt){
            // no data available
        }else{
            Ring_Buffer_Write(&uartRB, uart_rx_buf + read_idx, uart_rx_buf_size - read_idx);
            Ring_Buffer_Write(&uartRB, uart_rx_buf, write_idx);
        }
    }
    
    read_idx = write_idx;
    read_lli_cnt = write_lli_cnt;
}


static void UART_RTO_Callback(void)
{
    UART_IntClear(uart_id, UART_INT_RTO);
    
    uart_rx_process();
    
    if(uart_rx_event){
        uart_rx_event();
    }
}

static void UART_RX_DMA_Callback(void *p_arg, uint32_t flag)
{
    if(flag == HOSAL_DMA_INT_TRANS_COMPLETE){
        uart_rx_process();
        
        if(uart_rx_event){
            uart_rx_event();
        }
    }
}

static void UART_TX_DMA_Callback(void *p_arg, uint32_t flag)
{
    if(flag == HOSAL_DMA_INT_TRANS_COMPLETE){
        if(uart_tx_event){
            uart_tx_event();
        }
    }
}


static void gpio_init(bl_dma_uart_cfg_t *cfg)
{
    GLB_GPIO_Type pinList[] = {cfg->uart_tx_pin, cfg->uart_rx_pin};
    GLB_GPIO_Func_Init(GPIO_FUN_UART, pinList, sizeof(pinList)/sizeof(pinList[0]));
    
    GLB_UART_Fun_Sel(cfg->uart_tx_pin % 4, GLB_UART_SIG_FUN_UART0_TXD + 4 * cfg->uart_id);
    GLB_UART_Fun_Sel(cfg->uart_rx_pin % 4, GLB_UART_SIG_FUN_UART0_RXD + 4 * cfg->uart_id);
}

static void uart_init(bl_dma_uart_cfg_t *cfg)
{
    UART_CFG_Type uartCfg = {
      0,                           /* UART clock */
      cfg->uart_baudrate,          /* baudrate */
      UART_DATABITS_8,             /* data bits */
      UART_STOPBITS_1,             /* stop bits */
      UART_PARITY_NONE,            /* parity */
      DISABLE,                     /* Disable auto flow control */
      DISABLE,                     /* Disable rx input de-glitch function */
      DISABLE,                     /* Disable RTS output SW control mode */
      DISABLE,                     /* Disable tx output SW control mode */
      DISABLE,                     /* Disable tx lin mode */
      DISABLE,                     /* Disable rx lin mode */
      0,                           /* Tx break bit count for lin mode */
      UART_LSB_FIRST,              /* UART each data byte is send out LSB-first */
    };
    
    UART_FifoCfg_Type fifoCfg = {
        0,                         /* TX FIFO threshold */
        0,                         /* RX FIFO threshold */
        ENABLE,                    /* Enable tx dma req/ack interface */
        ENABLE,                    /* Enable rx dma req/ack interface */
    };
    
    GLB_Set_UART_CLK(1, HBN_UART_CLK_XCLK, 0);
    uartCfg.uartClk = 32000000;
    
    UART_DeInit(cfg->uart_id);
    UART_Init(cfg->uart_id, &uartCfg);
    UART_FifoConfig(cfg->uart_id, &fifoCfg);
    UART_TxFreeRun(cfg->uart_id, ENABLE);
    UART_SetRxTimeoutValue(cfg->uart_id, 80);
    UART_IntMask(cfg->uart_id, UART_INT_ALL, MASK);
    UART_IntMask(cfg->uart_id, UART_INT_RTO, UNMASK);
    UART_Enable(cfg->uart_id, UART_TXRX);
    
    bl_irq_register(UART0_IRQn + cfg->uart_id, UART_RTO_Callback);
    bl_irq_enable(UART0_IRQn + cfg->uart_id);
}

static void dma_rx_init(void)
{
    DMA_LLI_Cfg_Type lliCfg = {
        .dir = DMA_TRNS_P2M,
        .srcPeriph = uart_rx_dma_req,
        .dstPeriph = DMA_REQ_NONE,
    };
    
    struct DMA_Control_Reg dmaCtrlRegVal = {
        .TransferSize = uart_rx_buf_size / 2,
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
    
    uart_rx_queue[0].srcDmaAddr = uart_base + 0x8C;
    uart_rx_queue[0].destDmaAddr = (uint32_t)&uart_rx_buf[0];;
    uart_rx_queue[0].nextLLI = (uint32_t)&uart_rx_queue[1];
    uart_rx_queue[0].dmaCtrl = dmaCtrlRegVal;
    
    uart_rx_queue[1].srcDmaAddr = uart_base + 0x8C;
    uart_rx_queue[1].destDmaAddr = (uint32_t)&uart_rx_buf[uart_rx_buf_size / 2];
    uart_rx_queue[1].nextLLI = (uint32_t)&uart_rx_queue[0];
    uart_rx_queue[1].dmaCtrl = dmaCtrlRegVal;
    
    DMA_LLI_Init(DMA0_ID, uart_rx_dma_ch, &lliCfg);
    DMA_LLI_Update(DMA0_ID, uart_rx_dma_ch, (uint32_t)&uart_rx_queue[0]);
    DMA_Channel_Enable(DMA0_ID, uart_rx_dma_ch);
}

static void dma_tx_init(void)
{
    DMA_Channel_Cfg_Type dmaTxCfg = {
        0,                         /* Source address of DMA transfer */
        uart_base + 0x88,          /* Destination address of DMA transfer */
        0,                         /* Transfer length, 0~4095, this is burst count */
        DMA_TRNS_M2P,              /* Transfer dir control. 0: Memory to Memory, 1: Memory to peripheral, 2: Peripheral to memory */
        uart_tx_dma_ch,            /* Channel select 0-3 */
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
        uart_tx_dma_req,           /* Destination peripheral select */
    };
    
    DMA_Channel_Init(DMA0_ID, &dmaTxCfg);
}

static void dma_init(void)
{
    dma_rx_init();
    dma_tx_init();
}


int bl_dma_uart_init(bl_dma_uart_cfg_t *cfg)
{
    if(uart_ok){
        return -1;
    }
    
    if(cfg == NULL){
        return -1;
    }
    
    if(cfg->uart_id > 0){
        return -1;
    }
    
    if(cfg->uart_tx_pin > 31){
        return -1;
    }
    
    if(cfg->uart_rx_pin > 31){
        return -1;
    }
    
    if(cfg->uart_rx_buf == NULL){
        return -1;
    }
    
    hosal_dma_init();
    
    if(uart_tx_dma_ch == -1){
        uart_tx_dma_ch = hosal_dma_chan_request(0);
        if(uart_tx_dma_ch == -1){
            return -1;
        }
    }
    
    if(uart_rx_dma_ch == -1){
        uart_rx_dma_ch = hosal_dma_chan_request(0);
        if(uart_rx_dma_ch == -1){
            hosal_dma_chan_release(uart_tx_dma_ch);
            uart_tx_dma_ch = -1;
            return -1;
        }
    }
    
    hosal_dma_irq_callback_set(uart_tx_dma_ch, UART_TX_DMA_Callback, NULL);
    hosal_dma_irq_callback_set(uart_rx_dma_ch, UART_RX_DMA_Callback, NULL);
    
    uart_id = cfg->uart_id;
    uart_tx_pin = cfg->uart_tx_pin;
    uart_rx_pin = cfg->uart_rx_pin;
    uart_base = UART0_BASE;
    uart_tx_dma_req = DMA_REQ_UART0_TX;
    uart_rx_dma_req = DMA_REQ_UART0_RX;
    uart_rx_buf = cfg->uart_rx_buf;
    uart_rx_buf_size = cfg->uart_rx_buf_size & ~1;
    uart_tx_event = cfg->uart_tx_event;
    uart_rx_event = cfg->uart_rx_event;
    uart_ok = 1;
    
    if(uartBuf){
        free(uartBuf);
    }
    uartBuf = malloc(uart_rx_buf_size);
    Ring_Buffer_Init(&uartRB, uartBuf, uart_rx_buf_size, NULL, NULL);
    
    gpio_init(cfg);
    uart_init(cfg);
    dma_init();
    
    return 0;
}

uint16_t bl_dma_uart_get_rx_count(void)
{
    return Ring_Buffer_Get_Length(&uartRB);
}

uint16_t bl_dma_uart_read(uint8_t *data, uint16_t len)
{
    uint16_t cnt;
    
    cnt = Ring_Buffer_Get_Length(&uartRB);
    if(cnt < len){
        len = cnt;
    }
    
    Ring_Buffer_Read(&uartRB, data, len);
    return len;
}

void bl_dma_uart_write(uint8_t *data, uint16_t len)
{
    if(!uart_ok){
        return;
    }
    
    DMA_Channel_Disable(DMA0_ID, uart_tx_dma_ch);
    DMA_Channel_Update_SrcMemcfg(DMA0_ID, uart_tx_dma_ch, (uint32_t)data, len);
    DMA_Channel_Enable(DMA0_ID, uart_tx_dma_ch);
}

void bl_dma_uart_deinit(void)
{
    if(!uart_ok){
        return;
    }
    
    DMA_Channel_Disable(DMA0_ID, uart_tx_dma_ch);
    DMA_Channel_Disable(DMA0_ID, uart_rx_dma_ch);
    
    UART_DeInit(uart_id);
    
    GLB_GPIO_Type pinList[] = {uart_tx_pin, uart_rx_pin};
    GLB_GPIO_Func_Init(GPIO_FUN_GPIO, pinList, sizeof(pinList)/sizeof(pinList[0]));
    
    GLB_UART_Fun_Sel(uart_tx_pin % 4, 0);
    GLB_UART_Fun_Sel(uart_rx_pin % 4, 0);
    
    read_idx = 0;
    read_lli_cnt = 0;
    uart_ok = 0;
}


#if 0
// For BL702L, do not enable CLI if you want to use dma uart function
// After bl_dma_uart_init, do not call printf/puts/putchar/UART_SendData
void bl_dma_uart_test(void)
{
    uint8_t rx_buf[16];
    uint8_t tx_buf[16];
    uint16_t len;
    int ret;
    
    bl_dma_uart_cfg_t cfg = {
        .uart_id = 0,
        .uart_baudrate = 2000000,
        .uart_tx_pin = 14,
        .uart_rx_pin = 15,
        .uart_rx_buf = rx_buf,
        .uart_rx_buf_size = sizeof(rx_buf),
        .uart_tx_event = NULL,
        .uart_rx_event = NULL,
    };
    
    ret = bl_dma_uart_init(&cfg);
    //printf("bl_dma_uart_init: %d\r\n", ret);
    
    while(1){
        len = bl_dma_uart_get_rx_count();
        bl_dma_uart_read(tx_buf, len);
        bl_dma_uart_write(tx_buf, len);
        arch_delay_ms(500);
    }
}
#endif
