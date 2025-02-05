#include "bl_timer.h"
#include "bl_wdt.h"
#include "bl_irq.h"
#include <bl702l_timer.h>
#include <bl702l_glb.h>


#define MTIMER_TICKS_PER_US     (2)
static uint64_t timer_tick_now(void)
{
    uint32_t tick_low, tick_high, tick_tmp;
    uint64_t tick64;

    do {
        tick_high = *(volatile uint32_t*)0x0200BFFC;
        tick_low = *(volatile uint32_t*)0x0200BFF8;

        tick_tmp = *(volatile uint32_t*)0x0200BFFC;//make sure no overflow
    } while (tick_high != tick_tmp);

    tick64 = (((uint64_t)tick_high) << 32) | tick_low;
    return tick64;
}

uint32_t bl_timer_now_us(void)
{
    return timer_tick_now() / MTIMER_TICKS_PER_US;
}

uint64_t bl_timer_now_us64(void)
{
    return timer_tick_now() / MTIMER_TICKS_PER_US;
}

void bl_timer_delay_us(uint32_t us)
{
    uint64_t tick_now, tick_start;
    uint64_t ticks, diff;

    tick_start = timer_tick_now();
    ticks = us * MTIMER_TICKS_PER_US;

    do {
        tick_now = timer_tick_now();
        diff = tick_now - tick_start;
    } while (diff < ticks);
}


static bl_timer_cb_t bl_timer_callback[BL_TIMER_CH_NUM] = {NULL};
static uint32_t bl_timer_cmp_val[BL_TIMER_CH_NUM] = {0};
static uint32_t bl_timer_cnt_val = 0;
static uint32_t bl_timer_overflow_cnt = 0;


static void bl_timer_irq(void)
{
    uint8_t ch;
    bl_timer_cb_t cb;
    
    for(ch = 0; ch < BL_TIMER_CH_NUM; ch++){
        if(TIMER_GetMatchStatus(TIMER0_ID, TIMER_CH1, (TIMER_Comp_ID_Type)ch)){
            TIMER_IntMask(TIMER0_ID, TIMER_CH1, (TIMER_INT_Type)ch, MASK);
            TIMER_ClearIntStatus(TIMER0_ID, TIMER_CH1, (TIMER_Comp_ID_Type)ch);
            
            if(bl_timer_callback[ch]){
                cb = bl_timer_callback[ch];
                bl_timer_callback[ch] = NULL;
                cb();
            }
        }
    }
    
    if(TIMER_GetMatchStatus(TIMER0_ID, TIMER_CH1, TIMER_COMP_ID_2)){
        TIMER_ClearIntStatus(TIMER0_ID, TIMER_CH1, TIMER_COMP_ID_2);
        bl_timer_overflow_cnt++;
    }
}

static void bl_timer_cfg(uint32_t init_time)
{
    TIMER_CFG_Type timerCfg = 
    {
        TIMER_CH1,                           /* timer channel 1 */
        TIMER_CLKSRC_XTAL,                   /* timer clock source: XTAL32M clock */
        TIMER_PRELOAD_TRIG_NONE,             /* no preload source, just free run */
        TIMER_COUNT_PRELOAD,                 /* preload */
        31,                                  /* timer clock division value */
        0xFFFFFFFF,                          /* match value 0 */
        0xFFFFFFFF,                          /* match value 1 */
        0xFFFFFFFF,                          /* match value 2 */
        init_time,                           /* preload value */
    };
    
    // Timer reset here will reset all timer channels, which is not expected
    //GLB_AHB_Slave1_Reset(BL_AHB_SLAVE1_TMR);
    
    TIMER_Disable(TIMER0_ID, TIMER_CH1);
    TIMER_IntMask(TIMER0_ID, TIMER_CH1, TIMER_INT_ALL, MASK);
    TIMER_IntMask(TIMER0_ID, TIMER_CH1, TIMER_INT_COMP_2, UNMASK);  // for overflow counting
    TIMER_Init(TIMER0_ID, &timerCfg);
    TIMER_Enable(TIMER0_ID, TIMER_CH1);
    
    bl_irq_register(TIMER_CH1_IRQn, bl_timer_irq);
    bl_irq_enable(TIMER_CH1_IRQn);
}


void bl_timer_init(void)
{
    bl_timer_cfg(0);
}

uint32_t bl_timer_get_overflow_cnt(void)
{
    if(TIMER_GetMatchStatus(TIMER0_ID, TIMER_CH1, TIMER_COMP_ID_2)){
        // If this status is set, it means that the timer overflowed during global interrupt disabled
        return bl_timer_overflow_cnt + 1;
    }else{
        return bl_timer_overflow_cnt;
    }
}

uint32_t bl_timer_get_current_time(void)
{
    // TIMER_GetCounterValue does not work as expected
    //return TIMER_GetCounterValue(TIMER_CH1);
    return BL_RD_WORD(TIMER_BASE + TIMER_TCR2_OFFSET + TIMER_CH1 * 4);
}

uint32_t bl_timer_get_remaining_time(uint8_t ch)
{
    uint32_t cnt;
    uint32_t cmp;
    uint32_t delta;
    
    if(ch >= BL_TIMER_CH_NUM){
        return 0;
    }
    
    // always get current time first to avoid the effects of interrupt
    cnt = bl_timer_get_current_time();
    
    if(bl_timer_callback[ch]){
        cmp = TIMER_GetCompValue(TIMER0_ID, TIMER_CH1, (TIMER_Comp_ID_Type)ch);
        delta = cmp - cnt;
        
        //printf("cmp: %lu, cnt: %lu, delta: %lu\r\n", cmp, cnt, delta);
        
        return delta;
    }else{
        return 0;
    }
}

void bl_timer_start(uint8_t ch, uint32_t target_time, bl_timer_cb_t cb)
{
    if(ch >= BL_TIMER_CH_NUM){
        return;
    }
    
    bl_timer_callback[ch] = cb;
    
    TIMER_SetCompValue(TIMER0_ID, TIMER_CH1, (TIMER_Comp_ID_Type)ch, target_time);
    TIMER_IntMask(TIMER0_ID, TIMER_CH1, (TIMER_INT_Type)ch, UNMASK);
}

void* bl_timer_stop(uint8_t ch)
{
    bl_timer_cb_t cb;
    
    if(ch >= BL_TIMER_CH_NUM){
        return NULL;
    }
    
    cb = bl_timer_callback[ch];
    bl_timer_callback[ch] = NULL;
    
    TIMER_IntMask(TIMER0_ID, TIMER_CH1, (TIMER_INT_Type)ch, MASK);
    TIMER_ClearIntStatus(TIMER0_ID, TIMER_CH1, (TIMER_Comp_ID_Type)ch);
    
    return cb;
}

void bl_timer_store(void)
{
    bl_timer_store_time();
    bl_timer_store_events();
}

void bl_timer_store_time(void)
{
    bl_timer_cnt_val = bl_timer_get_current_time();
}

void bl_timer_store_events(void)
{
    uint8_t ch;
    
    for(ch = 0; ch < BL_TIMER_CH_NUM; ch++){
        if(bl_timer_callback[ch]){
            bl_timer_cmp_val[ch] = TIMER_GetCompValue(TIMER0_ID, TIMER_CH1, (TIMER_Comp_ID_Type)ch);
        }
    }
}

void bl_timer_restore(uint32_t jump_time, uint8_t run_expired)
{
    bl_timer_restore_time(jump_time);
    bl_timer_restore_events(run_expired);
}

void bl_timer_restore_time(uint32_t jump_time)
{
    //GLB_AHB_Slave1_Reset(BL_AHB_SLAVE1_TMR);
    //bl_wdt_restore();
    
    bl_timer_cfg(bl_timer_cnt_val + jump_time);
}

void bl_timer_restore_events(uint8_t run_expired)
{
    uint8_t ch;
    bl_timer_cb_t expired_cb[BL_TIMER_CH_NUM];
    uint32_t jump_time = bl_timer_get_current_time() - bl_timer_cnt_val;
    
    for(ch = 0; ch < BL_TIMER_CH_NUM; ch++){
        expired_cb[ch] = NULL;
        if(bl_timer_callback[ch]){
            if(jump_time < bl_timer_cmp_val[ch] - bl_timer_cnt_val){
                TIMER_SetCompValue(TIMER0_ID, TIMER_CH1, (TIMER_Comp_ID_Type)ch, bl_timer_cmp_val[ch]);
                TIMER_IntMask(TIMER0_ID, TIMER_CH1, (TIMER_INT_Type)ch, UNMASK);
            }else{
                expired_cb[ch] = bl_timer_callback[ch];
                bl_timer_callback[ch] = NULL;
            }
        }
    }
    
    if(run_expired){
        for(ch = 0; ch < BL_TIMER_CH_NUM; ch++){
            if(expired_cb[ch]){
                expired_cb[ch]();
            }
        }
    }
}
