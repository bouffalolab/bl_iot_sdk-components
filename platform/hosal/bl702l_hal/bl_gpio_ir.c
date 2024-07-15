#include "bl_gpio_ir.h"
#include "bl_gpio.h"
#include "bl_irq.h"
#include "bl702l_glb.h"


static uint8_t ir_tx_ok = 0;
static uint8_t ir_tx_pin = 0;


int bl_gpio_ir_tx_init(uint8_t tx_pin)
{
    if(tx_pin > 31){
        ir_tx_ok = 0;
        return -1;
    }
    
    ir_tx_ok = 1;
    ir_tx_pin = tx_pin;
    
    // output high by default
    bl_gpio_output_set(tx_pin, 1);
    bl_gpio_enable_output(tx_pin, 0, 0);
    
    return 0;
}

//run this function in ram to avoid execution speed variation due to cache
//ATTR_TCM_SECTION
static __attribute__((noinline)) void bl_gpio_ir_tx_do(uint16_t time[], uint32_t len, uint32_t gpio_output[])
{
    uint32_t tick_low;
    uint32_t i;
    
    tick_low = *(volatile uint32_t *)0x0200BFF8;
    while(*(volatile uint32_t *)0x0200BFF8 == tick_low);
    tick_low += 1;
    
    for(i=0; i<len; i++){
        BL_WR_WORD(GLB_BASE + GLB_GPIO_OUTPUT_OFFSET, gpio_output[i % 2]);
        
        while(*(volatile uint32_t *)0x0200BFF8 < tick_low + time[i] * 2);
        tick_low += time[i] * 2;
    }
    
    // output high by default
    BL_WR_WORD(GLB_BASE + GLB_GPIO_OUTPUT_OFFSET, gpio_output[1]);
}

int bl_gpio_ir_tx(uint16_t time[], uint32_t len)
{
    uint32_t gpio_output_old;
    uint32_t gpio_output_new[2];
    
    if(!ir_tx_ok){
        return -1;
    }
    
    unsigned long mstatus_tmp;
    mstatus_tmp = read_csr(mstatus);
    clear_csr(mstatus, MSTATUS_MIE);
    
    gpio_output_old = BL_RD_WORD(GLB_BASE + GLB_GPIO_OUTPUT_OFFSET);
    
    gpio_output_new[0] = gpio_output_old & ~(0x1 << ir_tx_pin);
    gpio_output_new[1] = gpio_output_old | (0x1 << ir_tx_pin);
    
    bl_gpio_ir_tx_do(time, len, gpio_output_new);
    
    write_csr(mstatus, mstatus_tmp);
    
    return 0;
}
