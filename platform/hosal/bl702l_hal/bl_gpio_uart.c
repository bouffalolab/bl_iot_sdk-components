#include "bl_gpio_uart.h"
#include "bl_gpio.h"
#include "bl_irq.h"
#include "bl702l_glb.h"
#include "bl702l_timer.h"
#include "hosal_gpio.h"


static uint8_t uart_tx_ok[GPIO_UART_TX_NUM] = {0};
static uint8_t uart_tx_pin[GPIO_UART_TX_NUM] = {0};
static uint32_t uart_tx_bit_dur[GPIO_UART_TX_NUM] = {0};  // unit: 0.5us


int bl_gpio_uart_tx_init(uint8_t id, uint8_t tx_pin, uint32_t baudrate)
{
    if(id >= GPIO_UART_TX_NUM){
        return -1;
    }
    
    if(tx_pin > 31){
        uart_tx_ok[id] = 0;
        return -2;
    }
    
    if(baudrate == 0 || baudrate > 2000000){
        uart_tx_ok[id] = 0;
        return -3;
    }
    
    uart_tx_ok[id] = 1;
    uart_tx_pin[id] = tx_pin;
    uart_tx_bit_dur[id] = (2000000 * 10 / baudrate + 5) / 10;
    
    // here must set output level before output enable, otherwise will output low by default when output enable, which causes a start bit
    bl_gpio_output_set(tx_pin, 1);
    bl_gpio_enable_output(tx_pin, 0, 0);
    
    return 0;
}

//run this function in ram to avoid execution speed variation due to cache
ATTR_TCM_SECTION
static __attribute__((noinline)) void bl_gpio_uart_send_byte_do(uint32_t gpio_output[], uint32_t bit_dur)
{
    uint32_t tick_low;
    int i;
    
    tick_low = *(volatile uint32_t *)0x0200BFF8;
    while(*(volatile uint32_t *)0x0200BFF8 == tick_low);
    tick_low += 1;
    
    for(i=0; i<10; i++){
        BL_WR_WORD(GLB_BASE + GLB_GPIO_OUTPUT_OFFSET, gpio_output[i]);
        
        while(*(volatile uint32_t *)0x0200BFF8 < tick_low + bit_dur);
        tick_low += bit_dur;
    }
}

int bl_gpio_uart_send_byte(uint8_t id, uint8_t data)
{
    uint32_t gpio_output_old;
    uint32_t gpio_output_new[10];
    int i;
    
    if(id >= GPIO_UART_TX_NUM){
        return -1;
    }
    
    if(!uart_tx_ok[id]){
        return -2;
    }
    
    unsigned long mstatus_tmp;
    mstatus_tmp = read_csr(mstatus);
    clear_csr(mstatus, MSTATUS_MIE);
    
    gpio_output_old = BL_RD_WORD(GLB_BASE + GLB_GPIO_OUTPUT_OFFSET);
    
    gpio_output_new[0] = gpio_output_old & ~(0x1 << uart_tx_pin[id]);
    
    for(i=1; i<=8; i++){
        if(data & 0x01){
            gpio_output_new[i] = gpio_output_old | (0x1 << uart_tx_pin[id]);
        }else{
            gpio_output_new[i] = gpio_output_old & ~(0x1 << uart_tx_pin[id]);
        }
        
        data >>= 1;
    }
    
    gpio_output_new[9] = gpio_output_old | (0x1 << uart_tx_pin[id]);
    
    bl_gpio_uart_send_byte_do(gpio_output_new, uart_tx_bit_dur[id]);
    
    write_csr(mstatus, mstatus_tmp);
    
    return 0;
}

int bl_gpio_uart_send_data(uint8_t id, uint8_t *data, uint32_t len)
{
    if(id >= GPIO_UART_TX_NUM){
        return -1;
    }
    
    if(!uart_tx_ok[id]){
        return -2;
    }
    
    while(len--){
        bl_gpio_uart_send_byte(id, *data++);
    }
    
    return 0;
}


static uint8_t uart_rx_pin = 0;
static uint32_t uart_rx_bit_dur = 0;  // unit: 0.5us
static uint8_t *uart_rx_fifo = NULL;
static uint32_t uart_rx_fifo_size = 0;
static uint32_t uart_rx_fifo_wptr = 0;
static uint32_t uart_rx_fifo_rptr = 0;
static uint8_t uart_rx_fifo_wloop = 0;
static uint8_t uart_rx_fifo_rloop = 0;


//run this function in ram to avoid execution speed variation due to cache
//ATTR_TCM_SECTION
static void bl_gpio_uart_recv_byte(void *arg)
{
    volatile uint32_t *current_time = (volatile uint32_t *)(TIMER_BASE + 0x2C);
    uint32_t start_time = *(volatile uint32_t *)(TIMER_BASE + 0xC4);
    uint32_t sample_time = start_time + uart_rx_bit_dur + uart_rx_bit_dur / 2;
    uint8_t sample_value;
    uint8_t rx_data;
    int i;
    
    rx_data = 0;
    for(i=0; i<8; i++){
        if(*current_time > sample_time){
            return;
        }
        
        while(*current_time < sample_time);
        sample_value = (BL_RD_WORD(GLB_BASE + GLB_GPIO_INPUT_OFFSET) >> uart_rx_pin) & 0x01;
        rx_data |= sample_value << i;
        
        sample_time += uart_rx_bit_dur;
    }
    
    if(!(uart_rx_fifo_wptr == uart_rx_fifo_rptr && uart_rx_fifo_wloop != uart_rx_fifo_rloop)){
        uart_rx_fifo[uart_rx_fifo_wptr++] = rx_data;
        if(uart_rx_fifo_wptr == uart_rx_fifo_size){
            uart_rx_fifo_wptr = 0;
            uart_rx_fifo_wloop++;
        }
    }
}

int bl_gpio_uart_rx_init(uint8_t rx_pin_1, uint8_t rx_pin_2, uint32_t baudrate, uint8_t *rx_fifo, uint32_t fifo_size)
{
    if(rx_pin_1 > 31 || rx_pin_2 > 31 || rx_pin_1 == rx_pin_2){
        return -1;
    }
    
    if(baudrate == 0 || baudrate > 2000000){
        return -2;
    }
    
    if(fifo_size == 0){
        return -3;
    }
    
    uart_rx_pin = rx_pin_1;
    uart_rx_bit_dur = (2000000 * 10 / baudrate + 5) / 10;
    
    uart_rx_fifo = rx_fifo;
    uart_rx_fifo_size = fifo_size;
    uart_rx_fifo_wptr = 0;
    uart_rx_fifo_rptr = 0;
    uart_rx_fifo_wloop = 0;
    uart_rx_fifo_rloop = 0;
    
    GLB_GPIO_Type pinList[1] = {rx_pin_2};
    GLB_GPIO_Func_Init(GPIO_FUN_CLK_OUT, pinList, sizeof(pinList)/sizeof(GLB_GPIO_Type));
    GLB_Sel_TMR_GPIO_Clock(pinList[0]);
    
    TIMER_CFG_Type timerCfg = {
        TIMER_CH0,
        TIMER_CLKSRC_XTAL,
        TIMER_PRELOAD_TRIG_NONE,
        TIMER_COUNT_FREERUN,
        15,
        0,
        0,
        0,
        0,
    };
    
    TIMER_Disable(TIMER0_ID, TIMER_CH0);
    TIMER_IntMask(TIMER0_ID, TIMER_CH0, TIMER_INT_ALL, MASK);
    TIMER_Init(TIMER0_ID, &timerCfg);
    TIMER_Enable(TIMER0_ID, TIMER_CH0);
    TIMER_GPIOSetPolarity(TIMER0_ID, TIMER_CH0, TIMER_GPIO_NEG);
    TIMER_CH0_SetMeasurePulseWidth(TIMER0_ID, ENABLE);
    
    hosal_gpio_dev_t gpio = {
        uart_rx_pin,
        INPUT_PULL_UP,
        NULL,
    };
    
    hosal_gpio_init(&gpio);
    hosal_gpio_irq_set(&gpio, (hosal_gpio_irq_trigger_t)GLB_GPIO_INT_TRIG_ASYNC_FALLING_EDGE, bl_gpio_uart_recv_byte, NULL);
    
    return 0;
}

uint32_t bl_gpio_uart_get_rx_length(void)
{
    uint32_t cnt;
    
    unsigned long mstatus_tmp;
    mstatus_tmp = read_csr(mstatus);
    clear_csr(mstatus, MSTATUS_MIE);
    
    if(uart_rx_fifo_wloop == uart_rx_fifo_rloop){
        cnt = uart_rx_fifo_wptr - uart_rx_fifo_rptr;
    }else{
        cnt = uart_rx_fifo_size + uart_rx_fifo_wptr - uart_rx_fifo_rptr;
    }
    
    write_csr(mstatus, mstatus_tmp);
    
    return cnt;
}

uint32_t bl_gpio_uart_read_data(uint8_t *data, uint32_t len)
{
    uint32_t cnt;
    
    cnt = bl_gpio_uart_get_rx_length();
    if(cnt > len){
        cnt = len;
    }
    
    len = cnt;
    while(len--){
        *data++ = uart_rx_fifo[uart_rx_fifo_rptr++];
        if(uart_rx_fifo_rptr == uart_rx_fifo_size){
            uart_rx_fifo_rptr = 0;
            uart_rx_fifo_rloop++;
        }
    }
    
    return cnt;
}
