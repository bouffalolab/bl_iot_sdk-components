#include "bl_gpio_uart.h"
#include "bl_gpio.h"
#include "bl_irq.h"
#include "bl702l_glb.h"


static uint8_t uart_ok = 0;
static uint8_t uart_tx_pin = 0;
static uint32_t uart_bit_dur = 0;  // unit: 0.5us


int bl_gpio_uart_init(uint8_t tx_pin, uint32_t baudrate)
{
    if(tx_pin > 31){
        uart_ok = 0;
        return -1;
    }
    
    if(baudrate == 0 || baudrate > 2000000){
        uart_ok = 0;
        return -2;
    }
    
    uart_ok = 1;
    uart_tx_pin = tx_pin;
    uart_bit_dur = lround(2000000 / (float)baudrate);
    
    bl_gpio_enable_output(uart_tx_pin, 0, 0);
    bl_gpio_output_set(uart_tx_pin, 1);
    
    return 0;
}

int bl_gpio_uart_send_byte(uint8_t data)
{
    uint32_t gpio_output_old;
    uint32_t gpio_output_new[10];
    uint32_t tick_low;
    int i;
    
    if(!uart_ok){
        return -1;
    }
    
    unsigned long mstatus_tmp;
    mstatus_tmp = read_csr(mstatus);
    clear_csr(mstatus, MSTATUS_MIE);
    
    gpio_output_old = BL_RD_WORD(GLB_BASE + GLB_GPIO_OUTPUT_OFFSET);
    
    gpio_output_new[0] = gpio_output_old & ~(0x1 << uart_tx_pin);
    
    for(i=1; i<=8; i++){
        if(data & 0x01){
            gpio_output_new[i] = gpio_output_old | (0x1 << uart_tx_pin);
        }else{
            gpio_output_new[i] = gpio_output_old & ~(0x1 << uart_tx_pin);
        }
        
        data >>= 1;
    }
    
    gpio_output_new[9] = gpio_output_old | (0x1 << uart_tx_pin);
    
    tick_low = *(volatile uint32_t *)0x0200BFF8;
    while(*(volatile uint32_t *)0x0200BFF8 == tick_low);
    tick_low += 1;
    
    for(i=0; i<10; i++){
        BL_WR_WORD(GLB_BASE + GLB_GPIO_OUTPUT_OFFSET, gpio_output_new[i]);
        
        while(*(volatile uint32_t *)0x0200BFF8 < tick_low + uart_bit_dur);
        tick_low += uart_bit_dur;
    }
    
    write_csr(mstatus, mstatus_tmp);
    
    return 0;
}

int bl_gpio_uart_send_data(uint8_t *data, uint32_t len)
{
    if(!uart_ok){
        return -1;
    }
    
    while(len--){
        bl_gpio_uart_send_byte(*data++);
    }
    
    return 0;
}
