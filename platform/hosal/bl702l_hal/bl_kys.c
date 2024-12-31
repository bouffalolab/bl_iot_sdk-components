#include "bl_kys.h"
#include "bl_irq.h"


static uint8_t bl_kys_sw_ghost_det_en = 1;

static bl_kys_cb_t bl_kys_callback = NULL;

static uint8_t last_state[8];
static uint8_t curr_state[8];


void bl_kys_update_ghost_det(bl_kys_result_t *result, uint8_t col_num)
{
    uint8_t state[8];
    uint8_t common;
    int c1, c2;
    int i;

    for(i=0; i<sizeof(state)/sizeof(state[0]); i++){
        state[i] = 0;
    }

    for(i=0; i<result->key_num; i++){
        state[result->col_idx[i]] |= 0x1 << result->row_idx[i];
    }

    for(c1=0; c1<col_num; c1++){
        if(!state[c1]){
            continue;
        }

        for(c2=c1+1; c2<col_num; c2++){
            common = state[c1] & state[c2];
            if(common & (common - 1)){
                result->ghost_det = 1;
                return;
            }
        }
    }
}

void bl_kys_get_result(bl_kys_result_t *result)
{
    uint32_t tmpVal;
    uint8_t row_num;
    uint8_t col_num;
    uint8_t key_num;
    uint8_t key_code;
    int i;

    tmpVal = BL_RD_REG(KYS_BASE, KYS_KS_CTRL);
    row_num = BL_GET_REG_BITS_VAL(tmpVal, KYS_ROW_NUM) + 1;
    col_num = BL_GET_REG_BITS_VAL(tmpVal, KYS_COL_NUM) + 1;

    tmpVal = BL_RD_REG(KYS_BASE, KYS_KEYFIFO_IDX);
    key_num = (tmpVal >> 16) & 0x0F;

    if(result){
        tmpVal = BL_RD_REG(KYS_BASE, KYS_KS_INT_STS);
        result->ghost_det = (tmpVal >> 12) & 0x01;
        result->fifo_full = (tmpVal >> 8) & 0x01;

        result->key_num = key_num;
        for(i=0; i<result->key_num; i++){
            key_code = BL_RD_REG(KYS_BASE, KYS_KEYFIFO_VALUE) & 0x7F;
            result->key_code[i] = key_code;
            result->row_idx[i] = key_code % row_num;
            result->col_idx[i] = key_code / row_num;
        }
    }

    // Disable kys to perform one-shot mode, will clear interrupt status and key fifo
    KYS_Disable();

    if(key_num > 0){
        // Enable ks_done, fifo_full, ghost interrupt
        BL_WR_REG(KYS_BASE, KYS_KS_INT_EN, (0x1<<7)|(0x1<<8)|(0x1<<12));
    }else{
        // Enable fifo_nonempty, fifo_full, ghost interrupt
        BL_WR_REG(KYS_BASE, KYS_KS_INT_EN, (0x1<<11)|(0x1<<8)|(0x1<<12));
    }

    // Patch for hardware ghost key detection
    if(result && bl_kys_sw_ghost_det_en){
        if(result->ghost_det == 0 && result->key_num >= 4){
            bl_kys_update_ghost_det(result, col_num);
        }
    }
}

static void bl_kys_irq(void)
{
    bl_kys_result_t result;

    bl_kys_get_result(&result);

    if(bl_kys_callback){
        bl_kys_callback(&result);
    }
}

void bl_kys_gpio_init(uint8_t row_num, uint8_t col_num, uint8_t row_pins[], uint8_t col_pins[])
{
    GLB_GPIO_Cfg_Type gpioCfg;
    int i;

    gpioCfg.gpioFun = GPIO_FUN_KEY_SCAN_ROW;
    gpioCfg.gpioMode = GPIO_MODE_AF;
    gpioCfg.pullType = GPIO_PULL_UP;
    gpioCfg.drive = 1;
    gpioCfg.smtCtrl = 1;
    for(i=0; i<row_num; i++){
        gpioCfg.gpioPin = row_pins[i];
        GLB_GPIO_Init(&gpioCfg);
    }

    gpioCfg.gpioFun = GPIO_FUN_KEY_SCAN_COL;
    gpioCfg.gpioMode = GPIO_MODE_AF;
    gpioCfg.pullType = GPIO_PULL_NONE;
    gpioCfg.drive = 1;
    gpioCfg.smtCtrl = 1;
    for(i=0; i<col_num; i++){
        gpioCfg.gpioPin = col_pins[i];
        GLB_GPIO_Init(&gpioCfg);
    }
}

void bl_kys_matrix_init(uint8_t row_num, uint8_t col_num, uint8_t row_pins[], uint8_t col_pins[], uint8_t row_det_cnt)
{
    uint32_t row_sel[2] = {0, 0};
    uint32_t col_sel[2] = {0, 0};
    int i;

    for(i=0; i<row_num; i++){
        row_sel[i/4] |= (uint32_t)row_pins[i] << (8 * (i%4));
    }

    for(i=row_num; i<8; i++){
        row_sel[i/4] |= 4 << (8 * (i%4));  // use GPIO4 as row padding
    }

    for(i=0; i<col_num; i++){
        col_sel[i/4] |= (uint32_t)col_pins[i] << (8 * (i%4));
    }

    BL_WR_REG(PDS_BASE, PDS_KYS_ROW_I_GPIO_SEL0, row_sel[0] | (row_det_cnt << 29));
    BL_WR_REG(PDS_BASE, PDS_KYS_ROW_I_GPIO_SEL1, row_sel[1]);
    BL_WR_REG(PDS_BASE, PDS_KYS_COL_O_GPIO_SEL0, col_sel[0]);
    BL_WR_REG(PDS_BASE, PDS_KYS_COL_O_GPIO_SEL1, col_sel[1]);
}


void bl_kys_init(uint8_t row_num, uint8_t col_num, uint8_t row_pins[], uint8_t col_pins[])
{
    KYS_CFG_Type kysCfg = {
        col_num,        /* Col of keyboard,max:8 */
        row_num,        /* Row of keyboard,max:8 */
        0,              /* Idle duration between column scans */
        ENABLE,         /* Fifo mode */
        ENABLE,         /* Enable ghost key event detection */
        DISABLE,        /* Disable deglitch function */
        0,              /* Deglitch count */
        0,
        0,
    };

    // Run KYS at 1M (better no greater than 1M)
    GLB_PER_Clock_UnGate(GLB_AHB_CLOCK_KYS);
    GLB_Set_KYS_CLK(GLB_KYS_CLK_SRC_XCLK, 31);

    KYS_Disable();
    KYS_Init(&kysCfg);

    // Enable ks_done, fifo_full, ghost interrupt
    BL_WR_REG(KYS_BASE, KYS_KS_INT_EN, (0x1<<7)|(0x1<<8)|(0x1<<12));

    bl_kys_gpio_init(row_num, col_num, row_pins, col_pins);
    bl_kys_matrix_init(row_num, col_num, row_pins, col_pins, row_num);

    bl_irq_register(KYS_IRQn, bl_kys_irq);
}

void bl_kys_trigger_poll(bl_kys_result_t *result)
{
    bl_irq_disable(KYS_IRQn);
    bl_irq_pending_clear(KYS_IRQn);

    // Enable ks_done, fifo_full, ghost interrupt
    BL_WR_REG(KYS_BASE, KYS_KS_INT_EN, (0x1<<7)|(0x1<<8)|(0x1<<12));

    KYS_Enable();
    while(!KYS_GetIntStatus(KYS_INT_KS_DONE));

    bl_kys_get_result(result);
}

void bl_kys_register_interrupt_callback(bl_kys_cb_t cb)
{
    bl_kys_callback = cb;
}

void bl_kys_trigger_interrupt(void)
{
    bl_irq_enable(KYS_IRQn);

    KYS_Enable();
}

void bl_kys_abort(void)
{
    bl_irq_disable(KYS_IRQn);
    bl_irq_pending_clear(KYS_IRQn);

    KYS_Disable();
}


void bl_kys_update_event(const bl_kys_result_t *result)
{
    int i;

    for(i=0; i<8; i++){
        last_state[i] = curr_state[i];
        curr_state[i] = 0;
    }

    for(i=0; i<result->key_num; i++){
        curr_state[result->row_idx[i]] |= 0x1 << result->col_idx[i];
    }
}

int bl_kys_is_key_event_detected(void)
{
    int i;

    for(i=0; i<8; i++){
        if(last_state[i] ^ curr_state[i]){
            return 1;
        }
    }

    return 0;
}

int bl_kys_get_key_event_by_coordinates(uint8_t row_idx, uint8_t col_idx)
{
    if((last_state[row_idx] ^ curr_state[row_idx]) & (0x1 << col_idx)){
        return curr_state[row_idx] & (0x1 << col_idx) ? KEY_EVENT_PRESS : KEY_EVENT_RELEASE;
    }else{
        return KEY_EVENT_NONE;
    }
}

int bl_kys_get_key_event_by_key_code(uint8_t key_code)
{
    uint32_t tmpVal;
    uint8_t row_num;
    uint8_t row_idx;
    uint8_t col_idx;

    tmpVal = BL_RD_REG(KYS_BASE, KYS_KS_CTRL);
    row_num = BL_GET_REG_BITS_VAL(tmpVal, KYS_ROW_NUM) + 1;

    row_idx = key_code % row_num;
    col_idx = key_code / row_num;

    return bl_kys_get_key_event_by_coordinates(row_idx, col_idx);
}


#if 0
static uint8_t row_pins[] = {31, 30, 10, 25, 24, 23};
static uint8_t col_pins[] = {9, 1, 0, 28, 27, 26};

void bl_kys_poll_test(void)
{
    bl_kys_result_t result;

    bl_kys_init(sizeof(row_pins), sizeof(col_pins), row_pins, col_pins);

    while(1){
        // poll scan mode
        bl_kys_trigger_poll(&result);

        // resut
        printf("\r\nghost_det: %d, fifo_full: %d, key_num: %d\r\n", result.ghost_det, result.fifo_full, result.key_num);
        if(result.key_num > 0){
            for(int i=0; i<result.key_num; i++){
                printf("%d@(%d, %d)  ", result.key_code[i], result.row_idx[i], result.col_idx[i]);
            }
            printf("\r\n");
        }

        // scan interval
        arch_delay_ms(1000);
    }
}

static void bl_kys_interrupt_callback(bl_kys_result_t *result)
{
    int key_event;

    bl_kys_update_event(result);

    if(bl_kys_is_key_event_detected()){
        for(int i=0; i<sizeof(row_pins); i++){
            for(int j=0; j<sizeof(col_pins); j++){
                key_event = bl_kys_get_key_event_by_coordinates(i, j);
                if(key_event != KEY_EVENT_NONE){
                    printf("key(%d, %d) %s\r\n", i, j, key_event == KEY_EVENT_PRESS ? "pressed" : "released");
                }
            }
        }

        for(int k=0; k<sizeof(row_pins)*sizeof(col_pins); k++){
            key_event = bl_kys_get_key_event_by_key_code(k);
            if(key_event != KEY_EVENT_NONE){
                printf("key%d %s\r\n", k, key_event == KEY_EVENT_PRESS ? "pressed" : "released");
            }
        }
    }
}

void bl_kys_interrupt_test(void)
{
    bl_kys_init(sizeof(row_pins), sizeof(col_pins), row_pins, col_pins);
    bl_kys_register_interrupt_callback(bl_kys_interrupt_callback);

    while(1){
        // interrupt scan mode
        bl_kys_trigger_interrupt();

        // scan interval
        arch_delay_ms(100);
    }
}
#endif
