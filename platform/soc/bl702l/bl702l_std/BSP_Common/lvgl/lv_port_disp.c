/**
 * @file lv_port_disp_templ.c
 *
 */

/*Copy this file as "lv_port_disp.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"
#include "lv_port_disp.h"
#include "hal_spi.h"
#include "hal_dma.h"
#include "../lcd/lcd.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

static void disp_init(void);
static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);
#if LV_USE_GPU
static void gpu_blend(lv_disp_drv_t *disp_drv, lv_color_t *dest, const lv_color_t *src, uint32_t length, lv_opa_t opa);
static void gpu_fill(lv_disp_drv_t *disp_drv, lv_color_t *dest_buf, lv_coord_t dest_width,
                     const lv_area_t *fill_area, lv_color_t color);
#endif

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
lv_disp_drv_t disp_drv; /*Descriptor of a display driver*/
static volatile lv_disp_drv_t *p_disp_drv_cb = NULL;

void lv_port_disp_init(void)
{
    /*-------------------------
     * Initialize your display
     * -----------------------*/
    disp_init();

    /*-----------------------------
     * Create a buffer for drawing
     *----------------------------*/

    /* LVGL requires a buffer where it internally draws the widgets.
     * Later this buffer will passed your display drivers `flush_cb` to copy its content to your display.
     * The buffer has to be greater than 1 display row
     *
     * There are three buffering configurations:
     * 1. Create ONE buffer with some rows:
     *      LVGL will draw the display's content here and writes it to your display
     *
     * 2. Create TWO buffer with some rows:
     *      LVGL will draw the display's content to a buffer and writes it your display.
     *      You should use DMA to write the buffer's content to the display.
     *      It will enable LVGL to draw the next part of the screen to the other buffer while
     *      the data is being sent form the first buffer. It makes rendering and flushing parallel.
     *
     * 3. Create TWO screen-sized buffer:
     *      Similar to 2) but the buffer have to be screen sized. When LVGL is ready it will give the
     *      whole frame to display. This way you only need to change the frame buffer's address instead of
     *      copying the pixels.
     * */

    /* Example for 1) */
    // static lv_disp_buf_t draw_buf_dsc;
    // static lv_color_t draw_buf_1[LCD_W * 30]  ATTR_EALIGN(64);       /*A buffer for 10 rows*/
    // lv_disp_buf_init(&draw_buf_dsc, draw_buf_1, NULL, LCD_W * 30);   /*Initialize the display buffer*/

/* MCU LCD Common interface */
#if (LCD_INTERFACE_TYPE == LCD_INTERFACE_DBI) || (LCD_INTERFACE_TYPE == LCD_INTERFACE_SPI)
    // /* Example for 2) */
    static lv_disp_buf_t draw_buf_dsc;
    static lv_color_t draw_buf_1[LCD_W * LCD_H / 8]  ATTR_EALIGN(64);           /*A buffer for 10 rows*/
    static lv_color_t draw_buf_2[LCD_W * LCD_H / 8]  ATTR_EALIGN(64);           /*An other buffer for 10 rows*/
    lv_disp_buf_init(&draw_buf_dsc, draw_buf_1, draw_buf_2, LCD_W * LCD_H / 8); /*Initialize the display buffer*/

/* RGB LCD Common interface,  */
#elif (LCD_INTERFACE_TYPE == LCD_INTERFACE_DPI) || (LCD_INTERFACE_TYPE == LCD_INTERFACE_DSI_VIDIO)
    /* Example for 3) */
    static lv_disp_buf_t draw_buf_dsc;
    static lv_color_t draw_buf_1[LCD_W * LCD_H]  ATTR_EALIGN(64);            /*A screen sized buffer*/
    static lv_color_t draw_buf_2[LCD_W * LCD_H]  ATTR_EALIGN(64);            /*An other screen sized buffer*/
    lv_disp_buf_init(&draw_buf_dsc, draw_buf_1, draw_buf_2, LCD_W * LCD_H);  /*Initialize the display buffer*/
    /* init MIPI-DPI */
    lcd_init(&(draw_buf_1->full));
#if defined(BL808)
    #include "bl808_dvp_tsrc.h"
    #include "bl808_isp_misc.h"
#endif
    void dpi_screen_switch_callback(void);
    ISP_MISC_Int_Callback_Install(ISP_MISC_INT_DISPLAY,&dpi_screen_switch_callback);
    ISP_MISC_Int_Mask(ISP_MISC_INT_DISPLAY,UNMASK);
#endif
    /*-----------------------------------
     * Register the display in LVGL
     *----------------------------------*/

    lv_disp_drv_init(&disp_drv); /*Basic initialization*/

    /*Set up the functions to access to your display*/

    /*Set the resolution of the display*/
    disp_drv.hor_res = LCD_W;
    disp_drv.ver_res = LCD_H;

    /* hardware rotation */
/* MCU LCD Common interface */
#if (LCD_INTERFACE_TYPE == LCD_INTERFACE_DBI) || (LCD_INTERFACE_TYPE == LCD_INTERFACE_SPI)
    disp_drv.sw_rotate = 0;

/* RGB LCD Common interface,  */
#elif (LCD_INTERFACE_TYPE == LCD_INTERFACE_DPI) || (LCD_INTERFACE_TYPE == LCD_INTERFACE_DSI_VIDIO)
    disp_drv.sw_rotate = 1;
#endif

    /*  rotation */
    disp_drv.rotated = LV_DISP_ROT_NONE;

    /*Used to copy the buffer's content to the display*/
    disp_drv.flush_cb = disp_flush;

    /*Set a display buffer*/
    disp_drv.buffer = &draw_buf_dsc;

#if LV_USE_GPU
    /*Optionally add functions to access the GPU. (Only in buffered mode, LV_VDB_SIZE != 0)*/

    /*Blend two color array using opacity*/
    disp_drv.gpu_blend_cb = gpu_blend;

    /*Fill a memory array with a color*/
    disp_drv.gpu_fill_cb = gpu_fill;
#endif

    /*Finally register the driver*/
    lv_disp_drv_register(&disp_drv);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
/* MCU LCD Common interface */
#if (LCD_INTERFACE_TYPE == LCD_INTERFACE_DBI) || (LCD_INTERFACE_TYPE == LCD_INTERFACE_SPI)
void flush_callback(struct device *dev, void *args, uint32_t size, uint32_t event)
{
    while (lcd_draw_is_busy()){
    };
    if(p_disp_drv_cb != NULL){
        lv_disp_flush_ready((lv_disp_drv_t *)p_disp_drv_cb);
        p_disp_drv_cb = NULL;
    }
}
/* RGB LCD Common interface,  */
#elif (LCD_INTERFACE_TYPE == LCD_INTERFACE_DPI) || (LCD_INTERFACE_TYPE == LCD_INTERFACE_DSI_VIDIO)

void dpi_screen_switch_callback(void)
{
    if(p_disp_drv_cb != NULL){
        lv_disp_flush_ready((lv_disp_drv_t *)p_disp_drv_cb);
        p_disp_drv_cb = NULL;
    }
}

#endif

/* Initialize your display and the required peripherals. */
/* MCU LCD Common interface */
#if (LCD_INTERFACE_TYPE == LCD_INTERFACE_DBI) || (LCD_INTERFACE_TYPE == LCD_INTERFACE_SPI)

void disp_init(void)
{
    lcd_init();

    lcd_clear(LCD_COLOR_RGB(0x00,0X00,0X00));

    if(lcd_dev_ifs_dma != NULL ){
        device_set_callback(lcd_dev_ifs_dma, flush_callback);
        device_control(lcd_dev_ifs_dma, DEVICE_CTRL_SET_INT, NULL);
    }
}

/* RGB LCD Common interface,  */
#elif (LCD_INTERFACE_TYPE == LCD_INTERFACE_DPI) || (LCD_INTERFACE_TYPE == LCD_INTERFACE_DSI_VIDIO)

void disp_init(void)
{
    /* Move to the lv_port_disp_init */
}

#endif

/* Flush the content of the internal buffer the specific area on the display
 * You can use DMA or any hardware acceleration to do this operation in the background but
 * 'lv_disp_flush_ready()' has to be called when finished. */
/* MCU LCD Common interface */
#if (LCD_INTERFACE_TYPE == LCD_INTERFACE_DBI) || (LCD_INTERFACE_TYPE == LCD_INTERFACE_SPI)

static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    static uint8_t rotated_dir = 0;
    if (rotated_dir != disp_drv->rotated) {
        rotated_dir = disp_drv->rotated;
        lcd_set_dir(rotated_dir, 0);
    }
    p_disp_drv_cb = disp_drv;
    lcd_draw_picture_nonblocking(area->x1, area->y1, area->x2, area->y2, (lcd_color_t *)color_p);
}


/* RGB LCD Common interface,  */
#elif (LCD_INTERFACE_TYPE == LCD_INTERFACE_DPI) || (LCD_INTERFACE_TYPE == LCD_INTERFACE_DSI_VIDIO)

static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    lcd_color_t* screen_buffer = lcd_get_screen_using();

    if(screen_buffer == (lcd_color_t *)color_p){
        lv_disp_flush_ready(disp_drv);
        return;
    }

    p_disp_drv_cb = disp_drv;
    lcd_screen_switch((lcd_color_t *)color_p);
    // lv_disp_flush_ready(disp_drv);

}

#endif


/*OPTIONAL: GPU INTERFACE*/
#if LV_USE_GPU

/* If your MCU has hardware accelerator (GPU) then you can use it to blend to memories using opacity
 * It can be used only in buffered mode (LV_VDB_SIZE != 0 in lv_conf.h)*/
static void gpu_blend(lv_disp_drv_t *disp_drv, lv_color_t *dest, const lv_color_t *src, uint32_t length, lv_opa_t opa)
{
    /*It's an example code which should be done by your GPU*/
    for (uint32_t i = 0; i < length; i++) {
        dest[i] = lv_color_mix(dest[i], src[i], opa);
    }

}

/* If your MCU has hardware accelerator (GPU) then you can use it to fill a memory with a color
 * It can be used only in buffered mode (LV_VDB_SIZE != 0 in lv_conf.h)*/
static void gpu_fill(lv_disp_drv_t *disp_drv, lv_color_t *dest_buf, lv_coord_t dest_width,
                     const lv_area_t *fill_area, lv_color_t color)
{
    /*It's an example code which should be done by your GPU*/
    int32_t x, y;
    dest_buf += dest_width * fill_area->y1; /*Go to the first line*/

    for (y = fill_area->y1; y <= fill_area->y2; y++) {
        for (x = fill_area->x1; x <= fill_area->x2; x++) {
            dest_buf[x] = color;
        }

        dest_buf += dest_width; /*Go to the next line*/
    }
}

#endif /*LV_USE_GPU*/

#else /* Enable this file at the top */

/* This dummy typedef exists purely to silence -Wpedantic. */
typedef int keep_pedantic_happy;
#endif
