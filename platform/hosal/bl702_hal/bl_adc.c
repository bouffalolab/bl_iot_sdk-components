#include <bl702_glb.h>
#include <bl702_adc.h>
#include <bl702_dma.h>
#include <bl_adc.h>
#include <bl_irq.h>
#include <bl_efuse.h>
#include <hosal_dma.h>


#define OFFSET_GAINCAL_REFRESH     1  // will only apply for external channel under single_ended mode

#define SKIP_SAMPLE_CNT            4
#define ADC_SAMPLE_CNT             64
#define VBAT_SAMPLE_CNT            64
#define TSEN_SAMPLE_CNT            16


typedef struct bl_adc_common_cfg
{
    uint8_t vbat_en;
    uint8_t tsen_en;
    uint8_t diff_en;
    uint8_t dma_en;
    uint8_t pos_ch;
    uint8_t neg_ch;
}bl_adc_common_cfg_t;


#if OFFSET_GAINCAL_REFRESH != 0
static struct {
    int16_t rsvd;
    int16_t init;
    int16_t os2;   // diff
    uint16_t os1;  // single_ended
    float coe;
}adc_new_param;
#endif

static uint8_t adc_single_ended = 0;
static int16_t tsen_refcode = 0;


static void adc_gpio_init(uint8_t ch)
{
    GLB_GPIO_Type adcPinList[] = {8, 15, 17, 11, 12, 14, 7, 9, 18, 19, 20, 21};
    GLB_GPIO_Cfg_Type gpioCfg;
    
    gpioCfg.gpioFun = GPIO_FUN_ANALOG;
    gpioCfg.gpioMode = GPIO_MODE_ANALOG;
    gpioCfg.pullType = GPIO_PULL_NONE;
    gpioCfg.drive = 0;
    gpioCfg.smtCtrl = 0;
    gpioCfg.gpioPin = adcPinList[ch];
    GLB_GPIO_Init(&gpioCfg);
}


static void bl_adc_common_init(bl_adc_common_cfg_t *cfg)
{
    ADC_CFG_Type adcCfg = {
        .v18Sel = ADC_V18_SEL_1P82V,                      /*!< ADC 1.8V select */
        .v11Sel = ADC_V11_SEL_1P1V,                       /*!< ADC 1.1V select */
        .clkDiv = ADC_CLK_DIV_32,                         /*!< Clock divider */
        .gain1 = ADC_PGA_GAIN_1,                          /*!< PGA gain 1 */
        .gain2 = ADC_PGA_GAIN_1,                          /*!< PGA gain 2 */
        .chopMode = ADC_CHOP_MOD_AZ_PGA_ON,               /*!< ADC chop mode select */
        .biasSel = ADC_BIAS_SEL_MAIN_BANDGAP,             /*!< ADC current form main bandgap or aon bandgap */
        .vcm = ADC_PGA_VCM_1P2V,                          /*!< ADC VCM value */
        .vref = ADC_VREF_3P2V,                            /*!< ADC voltage reference */
        .inputMode = ADC_INPUT_SINGLE_END,                /*!< ADC input signal type */
        .resWidth = ADC_DATA_WIDTH_16_WITH_256_AVERAGE,   /*!< ADC resolution and oversample rate */
        .offsetCalibEn = 0,                               /*!< Offset calibration enable */
        .offsetCalibVal = 0,                              /*!< Offset calibration value */
    };
    
    ADC_FIFO_Cfg_Type adcFifoCfg = {
        .fifoThreshold = ADC_FIFO_THRESHOLD_1,
        .dmaEn = DISABLE,
    };
    
    if(cfg->tsen_en){
        adcCfg.chopMode = ADC_CHOP_MOD_AZ_ON;
        adcCfg.vref = ADC_VREF_2P0V;
    }
    
    if(cfg->diff_en){
        adcCfg.inputMode = ADC_INPUT_DIFF;
    }
    
    if(cfg->dma_en){
        adcFifoCfg.dmaEn = ENABLE;
    }
    
    // Set ADC clock to 32M
    GLB_Set_ADC_CLK(ENABLE, GLB_ADC_CLK_XCLK, 0);
    
    // Initialize ADC
    ADC_Disable();
    ADC_Enable();
    ADC_Reset();
    ADC_Init(&adcCfg);
    ADC_FIFO_Cfg(&adcFifoCfg);
    
    // Configure VBAT
    if(cfg->vbat_en){
        ADC_Vbat_Enable();
    }else{
        ADC_Vbat_Disable();
    }
    
    // Configure TSEN
    if(cfg->tsen_en){
        ADC_Tsen_Init(ADC_TSEN_MOD_INTERNAL_DIODE);
    }else{
        ADC_Tsen_Disable();
    }
    
    // Configure channel
    if(cfg->diff_en){
        ADC_Channel_Config(cfg->pos_ch, cfg->neg_ch, ENABLE);
    }else{
        ADC_Channel_Config(cfg->pos_ch, ADC_CHAN_GND, ENABLE);
    }
}

static void bl_adc_common_get_result(int cnt, uint16_t *udata, int16_t *sdata, float *volt)
{
    uint32_t val;
    ADC_Result_Type result;
    uint32_t sum_unsigned = 0;
    int32_t sum_signed = 0;
    float sum_volt = 0;
    
    ADC_Start();
    while(ADC_Get_FIFO_Count() < SKIP_SAMPLE_CNT);
    ADC_FIFO_Clear();
    
    for(int i=0; i<cnt; i++){
        while(ADC_Get_FIFO_Count() == 0);
        val = ADC_Read_FIFO();
        if(udata){
            sum_unsigned += (uint16_t)(val & 0xFFFF);
        }
        if(sdata){
            sum_signed += (int16_t)(val & 0xFFFF);
        }
        if(volt){
            ADC_Parse_Result(&val, 1, &result);
            sum_volt += result.volt;
        }
    }
    
    ADC_Stop();
    ADC_FIFO_Clear();
    
    // Avoid calling ADC_Start after ADC_Stop without at least 1us delay
    arch_delay_us(10);
    
    if(udata){
        *udata = sum_unsigned / cnt;
    }
    if(sdata){
        *sdata = sum_signed / cnt;
    }
    if(volt){
        *volt = sum_volt / cnt;
    }
}

static void bl_adc_common_get_temperature(uint16_t v0, uint16_t v1, int16_t *temperature)
{
    int16_t vdelta = v0 - v1;
    
    if(temperature){
        *temperature = (int16_t)((vdelta - tsen_refcode) / 7.753);
    }
}


#if OFFSET_GAINCAL_REFRESH != 0
static void bl_adc_refresh_param(void)
{
    int16_t os2;
    uint16_t os1;
    float coe;
    
    if(adc_new_param.init){
        return;
    }
    
    bl_adc_common_cfg_t os2_cfg = {
        .vbat_en = 1,
        .tsen_en = 0,
        .diff_en = 1,
        .dma_en = 0,
        .pos_ch = ADC_CHAN_VABT_HALF,
        .neg_ch = ADC_CHAN_VABT_HALF,
    };
    
    bl_adc_common_cfg_t os1_cfg = {
        .vbat_en = 0,
        .tsen_en = 0,
        .diff_en = 0,
        .dma_en = 0,
        .pos_ch = ADC_CHAN_GND,
        .neg_ch = ADC_CHAN_GND,
    };
    
    bl_efuse_read_adc_gain_coe(&coe);
    
    bl_adc_common_init(&os2_cfg);
    bl_adc_common_get_result(5, NULL, &os2, NULL);
    
    os2 *= 2;
    coe -= (float)os2 / 40960;
    
    bl_adc_common_init(&os1_cfg);
    bl_adc_common_get_result(5, &os1, NULL, NULL);
    
    adc_new_param.os2 = os2;
    adc_new_param.os1 = os1;
    adc_new_param.coe = coe;
    adc_new_param.init = 1;
}

static void bl_adc_get_result_with_new_param(int cnt, uint16_t *data, float *volt)
{
    int16_t os2 = adc_new_param.os2;
    uint16_t os1 = adc_new_param.os1;
    float coe = adc_new_param.coe;
    uint16_t udata;
    float new_data;
    
    bl_adc_common_get_result(cnt, &udata, NULL, NULL);
    
    if(udata < os1){
        if(data) *data = 0;
        if(volt) *volt = 0;
        return;
    }
    
    if(os2 < 0){
        if(udata < 1.5 * os1){
            new_data = (udata - os1) / coe;
        }else if(udata >= 1.5 * os1 - os2){
            new_data = (udata - os2) / coe;
        }else{
            new_data = udata / coe;
        }
    }else{
        if(udata < os1 + os2){
            new_data = (udata - os1) / coe;
        }else{
            new_data = (udata - os2) / coe;
        }
    }
    
    if(data) *data = (uint16_t)new_data;
    if(volt) *volt = new_data * 3.2 / 65536;
}
#endif


int bl_adc_init(uint8_t single_ended, uint8_t pos_ch, uint8_t neg_ch)
{
    if(pos_ch > 11){
        return -1;
    }
    
    if(!single_ended){
        if(neg_ch > 11){
            return -1;
        }
    }
    
    adc_gpio_init(pos_ch);
    
    if(!single_ended){
        adc_gpio_init(neg_ch);
    }
    
    adc_single_ended = single_ended;
    
#if OFFSET_GAINCAL_REFRESH != 0
    if(adc_single_ended){
        bl_adc_refresh_param();
    }
#endif
    
    bl_adc_common_cfg_t cfg = {
        .vbat_en = 0,
        .tsen_en = 0,
        .diff_en = !single_ended,
        .dma_en = 0,
        .pos_ch = pos_ch,
        .neg_ch = neg_ch,
    };
    
    bl_adc_common_init(&cfg);
    
    return 0;
}

int bl_adc_get_conversion_result(void)
{
    uint16_t udata;
    int16_t sdata;
    
#if OFFSET_GAINCAL_REFRESH != 0
    if(adc_single_ended){
        bl_adc_get_result_with_new_param(ADC_SAMPLE_CNT, &udata, NULL);
        return udata;
    }
#endif
    
    if(adc_single_ended){
        bl_adc_common_get_result(ADC_SAMPLE_CNT, &udata, NULL, NULL);
        return udata;
    }else{
        bl_adc_common_get_result(ADC_SAMPLE_CNT, NULL, &sdata, NULL);
        return sdata;
    }
}

float bl_adc_get_val(void)
{
    float volt;
    
#if OFFSET_GAINCAL_REFRESH != 0
    if(adc_single_ended){
        bl_adc_get_result_with_new_param(ADC_SAMPLE_CNT, NULL, &volt);
        return volt;
    }
#endif
    
    bl_adc_common_get_result(ADC_SAMPLE_CNT, NULL, NULL, &volt);
    
    return volt;
}

int bl_adc_disable(void)
{
    ADC_Disable();
    
    return 0;
}


int bl_adc_vbat_init(void)
{
    bl_adc_common_cfg_t cfg = {
        .vbat_en = 1,
        .tsen_en = 0,
        .diff_en = 0,
        .dma_en = 0,
        .pos_ch = ADC_CHAN_VABT_HALF,
        .neg_ch = ADC_CHAN_GND,
    };
    
    bl_adc_common_init(&cfg);
    
    return 0;
}

float bl_adc_vbat_get_val(void)
{
    float volt;
    
    bl_adc_common_get_result(VBAT_SAMPLE_CNT, NULL, NULL, &volt);
    
    return volt * 2;
}


int bl_adc_tsen_init(void)
{
    bl_adc_common_cfg_t cfg = {
        .vbat_en = 0,
        .tsen_en = 1,
        .diff_en = 0,
        .dma_en = 0,
        .pos_ch = ADC_CHAN_TSEN_P,
        .neg_ch = ADC_CHAN_GND,
    };
    
    bl_adc_common_init(&cfg);
    
    // Get TSEN ref code
    return bl_efuse_read_tsen_refcode(&tsen_refcode);
}

int16_t bl_adc_tsen_get_val(void)
{
    uint16_t v0, v1;
    int16_t temperature;
    
    // Get V0
    ADC_SET_TSVBE_LOW();
    bl_adc_common_get_result(TSEN_SAMPLE_CNT, &v0, NULL, NULL);
    
    // Get V1
    ADC_SET_TSVBE_HIGH();
    bl_adc_common_get_result(TSEN_SAMPLE_CNT, &v1, NULL, NULL);
    
    // Get temperature
    bl_adc_common_get_temperature(v0, v1, &temperature);
    
    return temperature;
}


static int8_t tsen_dma_ch = -1;
static bl_adc_tsen_callback_t tsen_event;
static uint16_t tsen_buf[SKIP_SAMPLE_CNT + TSEN_SAMPLE_CNT];
static volatile int tsen_fsm = 0;

static void tsen_dma_callback(void *p_arg, uint32_t flag)
{
    static uint16_t v0;
    uint16_t v1;
    uint32_t sum;
    int16_t temperature;
    
    //BL_WR_REG(DMA_BASE, DMA_INTTCCLEAR, 1U << tsen_dma_ch);
    
    switch(tsen_fsm)
    {
        case 1:
            ADC_Stop();
            ADC_FIFO_Clear();
            
            // Get V0
            sum = 0;
            for(int i = SKIP_SAMPLE_CNT; i < SKIP_SAMPLE_CNT + TSEN_SAMPLE_CNT; i++){
                sum += tsen_buf[i];
            }
            v0 = sum / TSEN_SAMPLE_CNT;
            
            DMA_Channel_Update_DstMemcfg(tsen_dma_ch, (uint32_t)tsen_buf, sizeof(tsen_buf)/sizeof(tsen_buf[0]));
            DMA_Channel_Enable(tsen_dma_ch);
            
            ADC_SET_TSVBE_HIGH();
            ADC_Start();
            
            tsen_fsm = 2;
        break;
        
        case 2:
            ADC_Stop();
            ADC_FIFO_Clear();
            
            // Get V1
            sum = 0;
            for(int i = SKIP_SAMPLE_CNT; i < SKIP_SAMPLE_CNT + TSEN_SAMPLE_CNT; i++){
                sum += tsen_buf[i];
            }
            v1 = sum / TSEN_SAMPLE_CNT;
            
            // Get temperature
            bl_adc_common_get_temperature(v0, v1, &temperature);
            
            if(tsen_event != NULL){
                tsen_event(temperature);
            }
            
            tsen_fsm = 0;
        break;
        
        default:
        break;
    }
}

static void tsen_adc_init(void)
{
    bl_adc_common_cfg_t cfg = {
        .vbat_en = 0,
        .tsen_en = 1,
        .diff_en = 0,
        .dma_en = 1,
        .pos_ch = ADC_CHAN_TSEN_P,
        .neg_ch = ADC_CHAN_GND,
    };
    
    bl_adc_common_init(&cfg);
}

static void tsen_dma_init(bl_adc_tsen_cfg_t *cfg)
{
    DMA_Channel_Cfg_Type dmaChCfg = {
        0x40002004,                /* Source address of DMA transfer */
        0,                         /* Destination address of DMA transfer */
        0,                         /* Transfer length, 0~4095, this is burst count */
        DMA_TRNS_P2M,              /* Transfer dir control. 0: Memory to Memory, 1: Memory to peripheral, 2: Peripheral to memory */
        tsen_dma_ch,               /* Channel select 0-7 */
        DMA_TRNS_WIDTH_16BITS,     /* Transfer width. 0: 8  bits, 1: 16  bits, 2: 32  bits */
        DMA_TRNS_WIDTH_16BITS,     /* Transfer width. 0: 8  bits, 1: 16  bits, 2: 32  bits */
        DMA_BURST_SIZE_1,          /* Number of data items for burst transaction length. Each item width is as same as tansfer width. 0: 1 item, 1: 4 items, 2: 8 items, 3: 16 items */
        DMA_BURST_SIZE_1,          /* Number of data items for burst transaction length. Each item width is as same as tansfer width. 0: 1 item, 1: 4 items, 2: 8 items, 3: 16 items */
        DISABLE,
        DISABLE,
        0,
        DMA_PINC_DISABLE,          /* Source address increment. 0: No change, 1: Increment */
        DMA_MINC_ENABLE,           /* Destination address increment. 0: No change, 1: Increment */
        DMA_REQ_GPADC0,            /* Source peripheral select */
        DMA_REQ_NONE,              /* Destination peripheral select */
    };
    
    GLB_PER_Clock_UnGate(GLB_AHB_CLOCK_DMA_0);
    DMA_Enable();
    bl_irq_enable(DMA_ALL_IRQn);
    
    DMA_Channel_Init(&dmaChCfg);
    hosal_dma_irq_callback_set(tsen_dma_ch, tsen_dma_callback, NULL);
}

int bl_adc_tsen_dma_init(bl_adc_tsen_cfg_t *cfg)
{
    if(cfg == NULL){
        return -1;
    }
    
    if(tsen_dma_ch < 0){
        hosal_dma_init();
        
        tsen_dma_ch = hosal_dma_chan_request(0);
        if(tsen_dma_ch < 0){
            return -1;
        }
    }
    
    tsen_adc_init();
    tsen_dma_init(cfg);
    
    tsen_event = cfg->tsen_event;
    
    // Get TSEN ref code
    return bl_efuse_read_tsen_refcode(&tsen_refcode);
}

int bl_adc_tsen_dma_trigger(void)
{
    if(tsen_dma_ch < 0){
        return -1;
    }
    
    if(tsen_fsm != 0){
        return -1;
    }
    
    DMA_Channel_Update_DstMemcfg(tsen_dma_ch, (uint32_t)tsen_buf, sizeof(tsen_buf)/sizeof(tsen_buf[0]));
    DMA_Channel_Enable(tsen_dma_ch);
    
    ADC_SET_TSVBE_LOW();
    ADC_Start();
    
    tsen_fsm = 1;
    
    return 0;
}

int bl_adc_tsen_dma_is_busy(void)
{
    return (tsen_fsm != 0);
}


static int8_t adc_dma_ch = -1;
static uint16_t pcm_frame_size;
static int16_t *pcm_frame_buf[2];
static bl_adc_voice_callback_t pcm_frame_event;
static int pcm_frame_idx;

static void voice_dma_callback(void *p_arg, uint32_t flag)
{
    int idx = pcm_frame_idx;
    
    //BL_WR_REG(DMA_BASE, DMA_INTTCCLEAR, 1U << adc_dma_ch);
    
    pcm_frame_idx = !pcm_frame_idx;
    DMA_Channel_Update_DstMemcfg(adc_dma_ch, (uint32_t)pcm_frame_buf[pcm_frame_idx], pcm_frame_size);
    DMA_Channel_Enable(adc_dma_ch);
    
    if(pcm_frame_event != NULL){
        pcm_frame_event(idx);
    }
}

static void voice_gpio_init(bl_adc_voice_cfg_t *cfg)
{
    adc_gpio_init(cfg->adc_pos_ch);
    adc_gpio_init(cfg->adc_neg_ch);
}

static void voice_adc_init(bl_adc_voice_cfg_t *cfg)
{
    // Sample Rate = 15.625kHz (32M / 16 / 128)
    ADC_CFG_Type adcCfg = {
        .v18Sel = ADC_V18_SEL_1P82V,                      /*!< ADC 1.8V select */
        .v11Sel = ADC_V11_SEL_1P1V,                       /*!< ADC 1.1V select */
        .clkDiv = ADC_CLK_DIV_16,                         /*!< Clock divider */
        .gain1 = ADC_PGA_GAIN_8,                          /*!< PGA gain 1 */
        .gain2 = ADC_PGA_GAIN_4,                          /*!< PGA gain 2 */
        .chopMode = ADC_CHOP_MOD_AZ_ON,                   /*!< ADC chop mode select */
        .biasSel = ADC_BIAS_SEL_MAIN_BANDGAP,             /*!< ADC current form main bandgap or aon bandgap */
        .vcm = ADC_PGA_VCM_1P4V,                          /*!< ADC VCM value */
        .vref = ADC_VREF_3P2V,                            /*!< ADC voltage reference */
        .inputMode = ADC_INPUT_DIFF,                      /*!< ADC input differential type */
        .resWidth = ADC_DATA_WIDTH_16_WITH_128_AVERAGE,   /*!< ADC resolution and oversample rate */
        .offsetCalibEn = 0,                               /*!< Offset calibration enable */
        .offsetCalibVal = 0,                              /*!< Offset calibration value */
    };
    
    ADC_FIFO_Cfg_Type adcFifoCfg = {
        .fifoThreshold = ADC_FIFO_THRESHOLD_1,
        .dmaEn = ENABLE,
    };
    
    // Set ADC clock to 32M
    GLB_Set_ADC_CLK(ENABLE, GLB_ADC_CLK_XCLK, 0);
    
    // Initialize ADC
    ADC_Disable();
    ADC_Enable();
    ADC_Reset();
    ADC_Init(&adcCfg);
    ADC_FIFO_Cfg(&adcFifoCfg);
    ADC_Vbat_Disable();
    ADC_Tsen_Disable();
    ADC_Channel_Config(cfg->adc_pos_ch, cfg->adc_neg_ch, ENABLE);
    
    // Set pga_vcmi_en = 1 for mic
    ADC_PGA_Config(1, 1);
}

static void voice_dma_init(bl_adc_voice_cfg_t *cfg)
{
    DMA_Channel_Cfg_Type dmaChCfg = {
        0x40002004,                /* Source address of DMA transfer */
        0,                         /* Destination address of DMA transfer */
        0,                         /* Transfer length, 0~4095, this is burst count */
        DMA_TRNS_P2M,              /* Transfer dir control. 0: Memory to Memory, 1: Memory to peripheral, 2: Peripheral to memory */
        adc_dma_ch,                /* Channel select 0-7 */
        DMA_TRNS_WIDTH_16BITS,     /* Transfer width. 0: 8  bits, 1: 16  bits, 2: 32  bits */
        DMA_TRNS_WIDTH_16BITS,     /* Transfer width. 0: 8  bits, 1: 16  bits, 2: 32  bits */
        DMA_BURST_SIZE_1,          /* Number of data items for burst transaction length. Each item width is as same as tansfer width. 0: 1 item, 1: 4 items, 2: 8 items, 3: 16 items */
        DMA_BURST_SIZE_1,          /* Number of data items for burst transaction length. Each item width is as same as tansfer width. 0: 1 item, 1: 4 items, 2: 8 items, 3: 16 items */
        DISABLE,
        DISABLE,
        0,
        DMA_PINC_DISABLE,          /* Source address increment. 0: No change, 1: Increment */
        DMA_MINC_ENABLE,           /* Destination address increment. 0: No change, 1: Increment */
        DMA_REQ_GPADC0,            /* Source peripheral select */
        DMA_REQ_NONE,              /* Destination peripheral select */
    };
    
    GLB_PER_Clock_UnGate(GLB_AHB_CLOCK_DMA_0);
    DMA_Enable();
    bl_irq_enable(DMA_ALL_IRQn);
    
    DMA_Channel_Init(&dmaChCfg);
    hosal_dma_irq_callback_set(adc_dma_ch, voice_dma_callback, NULL);
}

int bl_adc_voice_init(bl_adc_voice_cfg_t *cfg)
{
    if(cfg == NULL){
        return -1;
    }
    
    if(cfg->adc_pos_ch > 11){
        return -1;
    }
    
    if(cfg->adc_neg_ch > 11){
        return -1;
    }
    
    if(cfg->pcm_frame_size < 1 || cfg->pcm_frame_size > 4095){
        return -1;
    }
    
    if(cfg->pcm_frame_buf[0] == NULL){
        return -1;
    }
    
    if(cfg->pcm_frame_buf[1] == NULL){
        return -1;
    }
    
    if(adc_dma_ch < 0){
        hosal_dma_init();
        
        adc_dma_ch = hosal_dma_chan_request(0);
        if(adc_dma_ch < 0){
            return -1;
        }
    }
    
    voice_gpio_init(cfg);
    voice_adc_init(cfg);
    voice_dma_init(cfg);
    
    pcm_frame_size = cfg->pcm_frame_size;
    pcm_frame_buf[0] = cfg->pcm_frame_buf[0];
    pcm_frame_buf[1] = cfg->pcm_frame_buf[1];
    pcm_frame_event = cfg->pcm_frame_event;
    
    return 0;
}

int bl_adc_voice_start(void)
{
    if(adc_dma_ch < 0){
        return -1;
    }
    
    pcm_frame_idx = 0;
    DMA_Channel_Update_DstMemcfg(adc_dma_ch, (uint32_t)pcm_frame_buf[pcm_frame_idx], pcm_frame_size);
    DMA_Channel_Enable(adc_dma_ch);
    
    ADC_Start();
    
    return 0;
}

int bl_adc_voice_stop(void)
{
    if(adc_dma_ch < 0){
        return -1;
    }
    
    DMA_Channel_Disable(adc_dma_ch);
    
    ADC_Stop();
    ADC_FIFO_Clear();
    
    return 0;
}


#if 0
#define FRAME_SIZE 4000
#define FRAME_NUM  4
static int16_t audio_buf[FRAME_SIZE * FRAME_NUM];
static int16_t pcm_buf[2][FRAME_SIZE];

static void audio_callback(int buf_idx)
{
    static uint32_t len = 0;
    
    printf("audio_callback: %d\r\n", buf_idx);
    memcpy(audio_buf + len, pcm_buf[buf_idx], sizeof(int16_t) * FRAME_SIZE);
    len += FRAME_SIZE;
    if(len == FRAME_SIZE * FRAME_NUM){
        bl_adc_voice_stop();
        len = 0;
        /* Now you can dump audio_buf and play the audio with Cool Edit. */
    }
}

void bl_adc_audio_test(void)
{
    bl_adc_voice_cfg_t cfg;
    cfg.adc_pos_ch = 3;  // GPIO11
    cfg.adc_neg_ch = 7;  // GPIO9
    cfg.pcm_frame_size = FRAME_SIZE;
    cfg.pcm_frame_buf[0] = pcm_buf[0];
    cfg.pcm_frame_buf[1] = pcm_buf[1];
    cfg.pcm_frame_event = audio_callback;
    
    bl_adc_voice_init(&cfg);
    bl_adc_voice_start();
}
#endif
