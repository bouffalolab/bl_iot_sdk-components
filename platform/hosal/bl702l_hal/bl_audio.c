#include "bl_audio.h"
#include "bl_irq.h"
#include "hosal_dma.h"


static int8_t audio_dma_ch = -1;
static uint16_t pcm_frame_size;
static int16_t *pcm_frame_buf[2];
static bl_audio_callback_t pcm_frame_event;
static int pcm_frame_idx;


static void audio_dma_callback(void *p_arg, uint32_t flag)
{
    int idx = pcm_frame_idx;
    
    //BL_WR_REG(DMA_BASE, DMA_INTTCCLEAR, 1U << audio_dma_ch);
    
    pcm_frame_idx = !pcm_frame_idx;
    DMA_Channel_Update_DstMemcfg(DMA0_ID, audio_dma_ch, (uint32_t)pcm_frame_buf[pcm_frame_idx], pcm_frame_size);
    DMA_Channel_Enable(DMA0_ID, audio_dma_ch);
    
    if(pcm_frame_event != NULL){
        pcm_frame_event(idx);
    }
}


static void audio_amic_gpio_init(bl_audio_amic_cfg_t *cfg)
{
    GLB_GPIO_Type audioPinList[] = {2, 3, 7, 8, 10, 11, 14, 15};
    GLB_GPIO_Cfg_Type gpioCfg;
    
    gpioCfg.gpioFun = GPIO_FUN_ANALOG;
    gpioCfg.gpioMode = GPIO_MODE_ANALOG;
    gpioCfg.pullType = GPIO_PULL_NONE;
    gpioCfg.drive = 0;
    gpioCfg.smtCtrl = 0;
    
    gpioCfg.gpioPin = audioPinList[cfg->amic_pos_ch];
    GLB_GPIO_Init(&gpioCfg);
    
    if(!cfg->amic_single_ended){
        gpioCfg.gpioPin = audioPinList[cfg->amic_neg_ch];
        GLB_GPIO_Init(&gpioCfg);
    }
}

static void audio_pdm_gpio_init(bl_audio_pdm_cfg_t *cfg)
{
    GLB_GPIO_Cfg_Type gpioCfg;
    
    gpioCfg.gpioFun = GPIO_FUN_PDM;
    gpioCfg.gpioMode = GPIO_MODE_AF;
    gpioCfg.pullType = GPIO_PULL_NONE;
    gpioCfg.drive = 1;
    gpioCfg.smtCtrl = 1;
    
    gpioCfg.gpioPin = cfg->pdm_clk_pin;
    GLB_GPIO_Init(&gpioCfg);
    
    gpioCfg.gpioPin = cfg->pdm_in_pin;
    GLB_GPIO_Init(&gpioCfg);
}


static void audio_amic_adc_init(bl_audio_amic_cfg_t *cfg)
{
    AUADC_ADC_AnalogCfg_Type auadc_adc_cfg = {
        .adc_edge_mode = AUADC_ADC_PGA_CLK_FALLING_SENT,            /* ADC output data clock edge, 0:falling edge sent,rising edge recieve, 1:rising edge sent,falling edge recieve */
        .adc_clock_phase_invert_en = DISABLE,                       /* adc clock phase invert */
        .pga_opmic_bias_cur = AUADC_ADC_PGA_BIAS_4_UA,              /* PGA_OPMIC bias current control */
        .pga_opaaf_bias_cur = AUADC_ADC_PGA_BIAS_4_UA,              /* PGA_OPAAF bias current control */
        .pga_chopper = AUADC_ADC_PGA_OPMIC_ON_OPAAF_ON,             /* control chopper for opmic&opaaf */
        .pga_hf_res = AUADC_ADC_PGA_HF_480K,                        /* PGA high pass filter Res for AC-coupled mode */
        .pga_chopper_en = ENABLE,                                   /* PGA chopper control, ENABLE or DISABLE */
        .pga_chopper_freq = AUADC_ADC_PGA_CHOPPER_FREQ_128K,        /* PGA chopper frequency control @Fs=2048k */
        //.pga_chopper_clk_source = AUADC_ADC_PGA_CLOCK_ADC,          /* PGA chopper clock source selection (not used) */
        
        .sdm_lowpower_en = DISABLE,                                 /* SDM lowpower funciton en, ENABLE: 0.6 of disable */
        .sdm_bias_cur = AUADC_ADC_SDM_BIAS_5_UA,                    /* SDM bias current control */
        .sdm_i_first_num = AUADC_ADC_SDM_I_FIRST_NUM_48,            /* op number control for first integrator in SDM */
        .sdm_i_sec_num = AUADC_ADC_SDM_I_SEC_NUM_24,                /* op number control for second integrator in SDM */
        .dem_en = ENABLE,                                           /* dem function control, ENABLE or DISABLE */
        .sdm_qg = AUADC_ADC_SDM_QG_DIV12,                           /* quantizer gain control for SDM */
        .sdm_dither_en = ENABLE,                                    /* dither control, ENABLE or DISABLE */
        .sdm_dither_level = AUADC_ADC_SDM_DL_LSB_DIV7_5,            /* dither level control for SDM */
        .sdm_dither_order = AUADC_ADC_SDM_ORDER_0,                  /* dither order control for SDM */
        
        .pga_circuit_en = ENABLE,                                   /* PGA related circuit enable */
        .sdm_circuit_en = ENABLE,                                   /* SDM related circuit enable */
        .pga_positive_en = ENABLE,                                  /* DISABLE, look into each channel will see high impedance, ENABLE:one of eight channel will be choose */
        .pga_negative_en = ENABLE,                                  /* DISABLE, look into each channel will see high impedance, ENABLE:one of eight channel will be choose */
        .pga_posi_ch = cfg->amic_pos_ch,                            /* Positive channel selection, connected to PGA positive terminal */
        .pga_nega_ch = cfg->amic_neg_ch,                            /* Negative channel selection, connected to PGA negative terminal */
        .pga_coupled_mode = cfg->amic_single_ended,                 /* PGA mode configuration */
        .pga_gain = AUADC_ADC_PGA_GAIN_30DB,                        /* audio mode enable, audio filter is on when set to high */
        .adc_mode = AUADC_ADC_FILT_MODE_AUDIO,                      /* adc work mode, audio-mode or measuring-mode */
        .audio_osr = AUADC_ADC_Audio_OSR_128,                       /* audio osr configuration */
        .measure_filter_mode = AUADC_ADC_MEASURE_FILTER_FILE_SINC3, /* digital dicimation filter selection when in measuring mode */
        .measure_rate = AUADC_ADC_MEASURE_RATE_SPS_2_5,             /* audadc ouput data rate selection when configured to measuring mode */
    };
    
    AUADC_ADC_Config(&auadc_adc_cfg);
}

static void audio_auadc_init(int auadc_src, void *cfg)
{
    AUADC_Cfg_Type auadc_cfg = {
        AUADC_CLK_16K_HZ,    /* sample rate */
        ENABLE,
        DISABLE,
        DISABLE,
        DISABLE,
        auadc_src,
        AUADC_PDM_LEFT,
    };
    
    AUADC_FifoCfg_Type auadc_fifo_cfg = {
        AUADC_RES_16_BITS,
        AUADC_FIFO_AILGN_MSB_AT_BIT15,
        0,
        AUADC_DRQ_EQUEL_TO_IRQ,
        ENABLE,
    };
    
    GLB_PER_Clock_UnGate(GLB_AHB_CLOCK_AUDIO);
    GLB_Power_On_DLL(GLB_DLL_XTAL_32M);
    GLB_Enable_DLL_Clk(GLB_DLL_CLK_2P032M);
    GLB_Set_AUDIO_CLK(0, 1, GLB_AUDIO_CLK_SRC_2P032M, 4);
    
    AUADC_Disable();
    AUADC_Enable();
    
    if(auadc_src == AUADC_SOURCE_ANALOG){
        audio_amic_adc_init((bl_audio_amic_cfg_t *)cfg);
    }
    
    AUADC_Init(&auadc_cfg);
    AUADC_FifoInit(&auadc_fifo_cfg);
}


static void audio_dma_init(void)
{
    DMA_Channel_Cfg_Type dmaChCfg = {
        0x4000AD88,                /* Source address of DMA transfer */
        0,                         /* Destination address of DMA transfer */
        0,                         /* Transfer length, 0~4095, this is burst count */
        DMA_TRNS_P2M,              /* Transfer dir control. 0: Memory to Memory, 1: Memory to peripheral, 2: Peripheral to memory */
        audio_dma_ch,              /* Channel select 0-7 */
        DMA_TRNS_WIDTH_16BITS,     /* Transfer width. 0: 8  bits, 1: 16  bits, 2: 32  bits */
        DMA_TRNS_WIDTH_16BITS,     /* Transfer width. 0: 8  bits, 1: 16  bits, 2: 32  bits */
        DMA_BURST_SIZE_1,          /* Number of data items for burst transaction length. Each item width is as same as tansfer width. 0: 1 item, 1: 4 items, 2: 8 items, 3: 16 items */
        DMA_BURST_SIZE_1,          /* Number of data items for burst transaction length. Each item width is as same as tansfer width. 0: 1 item, 1: 4 items, 2: 8 items, 3: 16 items */
        DISABLE,
        DISABLE,
        0,
        DMA_PINC_DISABLE,          /* Source address increment. 0: No change, 1: Increment */
        DMA_MINC_ENABLE,           /* Destination address increment. 0: No change, 1: Increment */
        DMA_REQ_AUADC,             /* Source peripheral select */
        DMA_REQ_NONE,              /* Destination peripheral select */
    };
    
    GLB_PER_Clock_UnGate(GLB_AHB_CLOCK_DMA_0);
    DMA_Enable(DMA0_ID);
    bl_irq_enable(DMA_ALL_IRQn);
    
    DMA_Channel_Init(DMA0_ID, &dmaChCfg);
    hosal_dma_irq_callback_set(audio_dma_ch, audio_dma_callback, NULL);
}


int bl_audio_amic_init(bl_audio_amic_cfg_t *cfg)
{
    if(cfg == NULL){
        return -1;
    }
    
    if(cfg->amic_single_ended > 1){
        return -1;
    }
    
    if(cfg->amic_pos_ch > 7){
        return -1;
    }
    
    if(!cfg->amic_single_ended){
        if(cfg->amic_neg_ch > 7){
            return -1;
        }
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
    
    if(audio_dma_ch < 0){
        hosal_dma_init();
        
        audio_dma_ch = hosal_dma_chan_request(0);
        if(audio_dma_ch < 0){
            return -1;
        }
    }
    
    audio_amic_gpio_init(cfg);
    audio_auadc_init(AUADC_SOURCE_ANALOG, cfg);
    audio_dma_init();
    
    pcm_frame_size = cfg->pcm_frame_size;
    pcm_frame_buf[0] = cfg->pcm_frame_buf[0];
    pcm_frame_buf[1] = cfg->pcm_frame_buf[1];
    pcm_frame_event = cfg->pcm_frame_event;
    
    return 0;
}

int bl_audio_pdm_init(bl_audio_pdm_cfg_t *cfg)
{
    if(cfg == NULL){
        return -1;
    }
    
    if(cfg->pdm_clk_pin != 0 && cfg->pdm_clk_pin != 2 && cfg->pdm_clk_pin != 8){
        return -1;
    }
    
    if(cfg->pdm_in_pin != 1 && cfg->pdm_in_pin != 3 && cfg->pdm_in_pin != 7){
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
    
    if(audio_dma_ch < 0){
        hosal_dma_init();
        
        audio_dma_ch = hosal_dma_chan_request(0);
        if(audio_dma_ch < 0){
            return -1;
        }
    }
    
    audio_pdm_gpio_init(cfg);
    audio_auadc_init(AUADC_SOURCE_PDM, NULL);
    audio_dma_init();
    
    pcm_frame_size = cfg->pcm_frame_size;
    pcm_frame_buf[0] = cfg->pcm_frame_buf[0];
    pcm_frame_buf[1] = cfg->pcm_frame_buf[1];
    pcm_frame_event = cfg->pcm_frame_event;
    
    return 0;
}

int bl_audio_start(void)
{
    if(audio_dma_ch < 0){
        return -1;
    }
    
    pcm_frame_idx = 0;
    DMA_Channel_Update_DstMemcfg(DMA0_ID, audio_dma_ch, (uint32_t)pcm_frame_buf[pcm_frame_idx], pcm_frame_size);
    DMA_Channel_Enable(DMA0_ID, audio_dma_ch);
    
    AUADC_Start();
    
    return 0;
}

int bl_audio_stop(void)
{
    if(audio_dma_ch < 0){
        return -1;
    }
    
    DMA_Channel_Disable(DMA0_ID, audio_dma_ch);
    
    AUADC_Stop();
    AUADC_FifoClear();
    
    return 0;
}

void bl_audio_set_high_pass_filter(int k1, int k2)
{
    uint32_t tmpVal;
    
    tmpVal = BL_RD_REG(AUADC_BASE, AUADC_PDM_ADC_1);
    
    if(k1 == 0){
        tmpVal = BL_SET_REG_BITS_VAL(tmpVal, AUADC_ADC_0_K1_EN, 0);
    }else{
        tmpVal = BL_SET_REG_BITS_VAL(tmpVal, AUADC_ADC_0_K1_EN, 1);
        tmpVal = BL_SET_REG_BITS_VAL(tmpVal, AUADC_ADC_0_K1, k1);
    }
    
    if(k2 == 0){
        tmpVal = BL_SET_REG_BITS_VAL(tmpVal, AUADC_ADC_0_K2_EN, 0);
    }else{
        tmpVal = BL_SET_REG_BITS_VAL(tmpVal, AUADC_ADC_0_K2_EN, 1);
        tmpVal = BL_SET_REG_BITS_VAL(tmpVal, AUADC_ADC_0_K2, k2);
    }
    
    BL_WR_REG(AUADC_BASE, AUADC_PDM_ADC_1, tmpVal);
}

void bl_audio_set_analog_gain(int gain_db)
{
    uint32_t tmpVal;
    
    if(gain_db < 6){
        gain_db = 6;
    }
    
    if(gain_db > 42){
        gain_db = 42;
    }
    
    tmpVal = BL_RD_REG(AUADC_BASE, AUADC_AUDADC_CMD);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, AUADC_AUDADC_PGA_GAIN, gain_db / 3);
    BL_WR_REG(AUADC_BASE, AUADC_AUDADC_CMD, tmpVal);
}

void bl_audio_set_digital_gain(float gain_db)
{
    uint32_t tmpVal;
    
    if(gain_db < -95.5){
        gain_db = -95.5;
    }
    
    if(gain_db > 18){
        gain_db = 18;
    }
    
    tmpVal = BL_RD_REG(AUADC_BASE, AUADC_PDM_ADC_S0);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, AUADC_ADC_S0_VOLUME, (int)(gain_db * 2) & 0x1FF);
    BL_WR_REG(AUADC_BASE, AUADC_PDM_ADC_S0, tmpVal);
}

int bl_audio_get_analog_gain(void)
{
    uint32_t tmpVal;
    int pga_gain;
    
    tmpVal = BL_RD_REG(AUADC_BASE, AUADC_AUDADC_CMD);
    pga_gain = BL_GET_REG_BITS_VAL(tmpVal, AUADC_AUDADC_PGA_GAIN);
    
    if(pga_gain < 2){
        pga_gain = 2;
    }
    
    if(pga_gain > 14){
        pga_gain = 14;
    }
    
    return pga_gain * 3;
}

float bl_audio_get_digital_gain(void)
{
    uint32_t tmpVal;
    int volume;
    
    tmpVal = BL_RD_REG(AUADC_BASE, AUADC_PDM_ADC_S0);
    volume = BL_GET_REG_BITS_VAL(tmpVal, AUADC_ADC_S0_VOLUME);
    
    if(volume >> 8){
        volume -= 512;
    }
    
    return (float)volume / 2;
}


#if 0
#define FRAME_SIZE 308
#define FRAME_NUM  240  // 308*240/16000 = 4.62s
static int16_t pcm_buf[2][FRAME_SIZE];

static void audio_callback(int buf_idx)
{
    static uint32_t len = 0;
    
    printf("audio_callback: %d\r\n", buf_idx);
    printf("rawdata start\r\n");
    extern int UART_SendData(uint8_t uartId, uint8_t *data, uint32_t len);
    UART_SendData(0, (uint8_t *)pcm_buf[buf_idx], sizeof(int16_t) * FRAME_SIZE);
    printf("\r\nrawdata end\r\n\r\n");
    len += FRAME_SIZE;
    if(len == FRAME_SIZE * FRAME_NUM){
        bl_audio_stop();
        len = 0;
        printf("Please use customer_app/bl702l_remote_control/audio_parser.py to convert this log file to pcm file.\r\n\r\n");
    }
}

void bl_audio_amic_test(void)
{
    bl_audio_amic_cfg_t cfg;
    cfg.amic_single_ended = 0;
    cfg.amic_pos_ch = 1;  // GPIO3
    cfg.amic_neg_ch = 3;  // GPIO8
    cfg.pcm_frame_size = FRAME_SIZE;
    cfg.pcm_frame_buf[0] = pcm_buf[0];
    cfg.pcm_frame_buf[1] = pcm_buf[1];
    cfg.pcm_frame_event = audio_callback;
    
    bl_audio_amic_init(&cfg);
    bl_audio_start();
}

void bl_audio_pdm_test(void)
{
    bl_audio_pdm_cfg_t cfg;
    cfg.pdm_clk_pin = 8;
    cfg.pdm_in_pin = 3;
    cfg.pcm_frame_size = FRAME_SIZE;
    cfg.pcm_frame_buf[0] = pcm_buf[0];
    cfg.pcm_frame_buf[1] = pcm_buf[1];
    cfg.pcm_frame_event = audio_callback;
    
    bl_audio_pdm_init(&cfg);
    bl_audio_start();
}
#endif
