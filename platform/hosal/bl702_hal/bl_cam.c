#include <string.h>
#include <stdio.h>
#include <bl702_glb.h>
#include <bl702_gpio.h>
#include <bl702_cam.h>
#include <bl702_mjpeg.h>
#include <cam_reg.h>
#include <mjpeg_reg.h>
#include <FreeRTOS.h>
#include <event_groups.h>

#include "bl_cam.h"

static CAM_CFG_Type camera_dvp;
static MJPEG_CFG_Type mjpeg_engine;
static EventGroupHandle_t camera_event;
static const rt_camera_desc *m_desc;

uint32_t *buffer_cam = NULL;    // need pvPortMalloc
uint32_t buffer_cam_size;       // CAMERA_BUFFER_SIZE_WHEN_MJPEG
uint32_t *buffer_psram = NULL;  // need pvPortMallocPsram
uint32_t buffer_psram_size = 1024*1024;

#define CAMERA_RESOLUTION_X            m_desc->width
#define CAMERA_RESOLUTION_Y            m_desc->height
#define CAMERA_FRAME_SIZE              (CAMERA_RESOLUTION_X * 2 * CAMERA_RESOLUTION_Y)  // YUV422, 16 bits per pixel
#define MJPEG_READ_ADDR                (uint32_t)buffer_cam
#define MJPEG_READ_SIZE                2                                                // one for CAM and one for MJPEG in turn
#define CAMERA_BUFFER_SIZE_WHEN_MJPEG  (CAMERA_RESOLUTION_X * 2 * 8 * MJPEG_READ_SIZE)
#define MJPEG_WRITE_ADDR               (uint32_t)buffer_psram//(MJPEG_READ_ADDR + CAMERA_BUFFER_SIZE_WHEN_MJPEG)
#define MJPEG_WRITE_SIZE               buffer_psram_size//(buffer_cam_size - CAMERA_BUFFER_SIZE_WHEN_MJPEG)

#define CROP_WIDTH                     120
#define CROP_HEIGHT                    120
#define CROP_FRAME_SIZE                (CROP_WIDTH * 2 * CROP_HEIGHT)      // YUV422, 16 bits per pixel
#define CROP_FRAME_CNT                 2                                   // make sure CROP_FRAME_CNT >= 2
#define CROP_MEM_ADDR                  (uint32_t)buffer_psram//(uint32_t)buffer_cam
#define CROP_MEM_SIZE                  (CROP_FRAME_SIZE * CROP_FRAME_CNT)  // make sure CROP_MEM_SIZE <= buffer_cam_size

#define MJPEG_EVENT_FRAME_INT          (1 << 0)
#define MJPEG_EVENT_FRAME_ERROR        (1 << 1)

#define CAMERA_EVENT_FRAME_INT         (1 << 0)
#define CAMERA_EVENT_FRAME_ERROR       (1 << 1)

#define JPG_HEAD_LENGTH_422            605
static const uint8_t jpgHeadBuf[JPG_HEAD_LENGTH_422] = {
    0xFF, 0xD8, 0xFF, 0xDB, 0x00, 0x43, 0x00, 0x10, 0x0B, 0x0C, 0x0E, 0x0C, 0x0A, 0x10, 0x0E, 0x0D,
    0x0E, 0x12, 0x11, 0x10, 0x13, 0x18, 0x28, 0x1A, 0x18, 0x16, 0x16, 0x18, 0x31, 0x23, 0x25, 0x1D,
    0x28, 0x3A, 0x33, 0x3D, 0x3C, 0x39, 0x33, 0x38, 0x37, 0x40, 0x48, 0x5C, 0x4E, 0x40, 0x44, 0x57,
    0x45, 0x37, 0x38, 0x50, 0x6D, 0x51, 0x57, 0x5F, 0x62, 0x67, 0x68, 0x67, 0x3E, 0x4D, 0x71, 0x79,
    0x70, 0x64, 0x78, 0x5C, 0x65, 0x67, 0x63, 0xFF, 0xDB, 0x00, 0x43, 0x01, 0x11, 0x12, 0x12, 0x18,
    0x15, 0x18, 0x2F, 0x1A, 0x1A, 0x2F, 0x63, 0x42, 0x38, 0x42, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63,
    0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63,
    0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63,
    0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0xFF, 0xC0, 0x00, 0x11,
    0x08, 0x01, 0xE0, 0x01, 0xE0, 0x03, 0x01, 0x21, 0x00, 0x02, 0x11, 0x01, 0x03, 0x11, 0x01, 0xFF,
    0xC4, 0x00, 0x1F, 0x00, 0x00, 0x01, 0x05, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
    0xFF, 0xC4, 0x00, 0xB5, 0x10, 0x00, 0x02, 0x01, 0x03, 0x03, 0x02, 0x04, 0x03, 0x05, 0x05, 0x04,
    0x04, 0x00, 0x00, 0x01, 0x7D, 0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12, 0x21, 0x31, 0x41,
    0x06, 0x13, 0x51, 0x61, 0x07, 0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xA1, 0x08, 0x23, 0x42, 0xB1,
    0xC1, 0x15, 0x52, 0xD1, 0xF0, 0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0A, 0x16, 0x17, 0x18, 0x19,
    0x1A, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x43, 0x44,
    0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x63, 0x64,
    0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x83, 0x84,
    0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0xA2,
    0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9,
    0xBA, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,
    0xD8, 0xD9, 0xDA, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xF1, 0xF2, 0xF3,
    0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFF, 0xC4, 0x00, 0x1F, 0x01, 0x00, 0x03, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03,
    0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0xFF, 0xC4, 0x00, 0xB5, 0x11, 0x00, 0x02, 0x01,
    0x02, 0x04, 0x04, 0x03, 0x04, 0x07, 0x05, 0x04, 0x04, 0x00, 0x01, 0x02, 0x77, 0x00, 0x01, 0x02,
    0x03, 0x11, 0x04, 0x05, 0x21, 0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71, 0x13, 0x22, 0x32,
    0x81, 0x08, 0x14, 0x42, 0x91, 0xA1, 0xB1, 0xC1, 0x09, 0x23, 0x33, 0x52, 0xF0, 0x15, 0x62, 0x72,
    0xD1, 0x0A, 0x16, 0x24, 0x34, 0xE1, 0x25, 0xF1, 0x17, 0x18, 0x19, 0x1A, 0x26, 0x27, 0x28, 0x29,
    0X2A, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x53,
    0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x73,
    0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A,
    0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8,
    0xA9, 0xAA, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6,
    0xC7, 0xC8, 0xC9, 0xCA, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xE2, 0xE3, 0xE4,
    0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFF,
    0xDA, 0x00, 0x0C, 0x03, 0x01, 0x00, 0x02, 0x11, 0x03, 0x11, 0x00, 0x3F, 0x00
};

uint8_t jpeg_quality = 50;

int bl_cam_config_update(uint8_t quality)
{
    jpeg_quality = quality;
    printf("[MJPEG] Using quality %u\r\n",
            jpeg_quality
    );

    return 0;
}

int bl_cam_config_get(uint8_t *quality, uint16_t *width, uint16_t *height)
{
    if (quality){
        *quality = jpeg_quality;
    }
    if (width){
        *width = m_desc->width;
    }
    if (height){
        *height = m_desc->height;
    }

    return 0;
}

static int cam_init_gpio(void)
{
    GLB_GPIO_Cfg_Type cfg;

#if 0
    cfg.drive=0;
    cfg.smtCtrl=1;
    cfg.gpioPin = GLB_GPIO_PIN_0;
    cfg.gpioFun = GPIO0_FUN_PIX_CLK;
    cfg.gpioMode = GPIO_MODE_INPUT;
    cfg.pullType = GPIO_PULL_NONE;
    GLB_GPIO_Init(&cfg);
#endif

    cfg.drive=0;
    cfg.smtCtrl=1;
    cfg.gpioPin = GLB_GPIO_PIN_1;
    cfg.gpioFun = GPIO1_FUN_FRAME_VLD;
    cfg.gpioMode = GPIO_MODE_INPUT;
    cfg.pullType = GPIO_PULL_NONE;
    GLB_GPIO_Init(&cfg);

    cfg.drive=0;
    cfg.smtCtrl=1;
    cfg.gpioPin = GLB_GPIO_PIN_2;
    cfg.gpioFun = GPIO2_FUN_LINE_VLD;
    cfg.gpioMode = GPIO_MODE_INPUT;
    cfg.pullType = GPIO_PULL_NONE;
    GLB_GPIO_Init(&cfg);

    cfg.drive=0;
    cfg.smtCtrl=1;
    cfg.gpioPin = GLB_GPIO_PIN_3;
    cfg.gpioFun = GPIO3_FUN_PIX_DAT0;
    cfg.gpioMode = GPIO_MODE_INPUT;
    cfg.pullType = GPIO_PULL_NONE;
    GLB_GPIO_Init(&cfg);

    cfg.drive=0;
    cfg.smtCtrl=1;
    cfg.gpioPin = GLB_GPIO_PIN_4;
    cfg.gpioFun = GPIO4_FUN_PIX_DAT1;
    cfg.gpioMode = GPIO_MODE_INPUT;
    cfg.pullType = GPIO_PULL_NONE;
    GLB_GPIO_Init(&cfg);

    cfg.drive=0;
    cfg.smtCtrl=1;
    cfg.gpioPin = GLB_GPIO_PIN_5;
    cfg.gpioFun = GPIO5_FUN_PIX_DAT2;
    cfg.gpioMode = GPIO_MODE_INPUT;
    cfg.pullType = GPIO_PULL_NONE;
    GLB_GPIO_Init(&cfg);

    cfg.drive=0;
    cfg.smtCtrl=1;
    cfg.gpioPin = GLB_GPIO_PIN_6;
    cfg.gpioFun = GPIO6_FUN_PIX_DAT3;
    cfg.gpioMode = GPIO_MODE_INPUT;
    cfg.pullType = GPIO_PULL_NONE;
    GLB_GPIO_Init(&cfg);

    cfg.drive=0;
    cfg.smtCtrl=1;
    cfg.gpioPin = GLB_GPIO_PIN_12;
    cfg.gpioFun = GPIO12_FUN_PIX_DAT4;
    cfg.gpioMode = GPIO_MODE_INPUT;
    cfg.pullType = GPIO_PULL_NONE;
    GLB_GPIO_Init(&cfg);

    cfg.drive=0;
    cfg.smtCtrl=1;
    cfg.gpioPin = GLB_GPIO_PIN_29;
    cfg.gpioFun = GPIO29_FUN_PIX_DAT5;
    cfg.gpioMode = GPIO_MODE_INPUT;
    cfg.pullType = GPIO_PULL_NONE;
    GLB_GPIO_Init(&cfg);

    cfg.drive=0;
    cfg.smtCtrl=1;
    cfg.gpioPin = GLB_GPIO_PIN_30;
    cfg.gpioFun = GPIO30_FUN_PIX_DAT6;
    cfg.gpioMode = GPIO_MODE_INPUT;
    cfg.pullType = GPIO_PULL_NONE;
    GLB_GPIO_Init(&cfg);

    cfg.drive=0;
    cfg.smtCtrl=1;
    cfg.gpioPin = GLB_GPIO_PIN_31;
    cfg.gpioFun = GPIO31_FUN_PIX_DAT7;
    cfg.gpioMode = GPIO_MODE_INPUT;
    cfg.pullType = GPIO_PULL_NONE;
    GLB_GPIO_Init(&cfg);

    return 0;
}

static int dvp_init(int useMjpeg, int frm_vld_high)
{
    memset(&camera_dvp, 0, sizeof(camera_dvp));

    if (useMjpeg) {
        camera_dvp.swMode = CAM_SW_MODE_AUTO;
//        camera_dvp.swIntCnt = 0;
        camera_dvp.frameMode = CAM_INTERLEAVE_MODE;
        camera_dvp.yuvMode = CAM_YUV422;
        camera_dvp.linePol = CAM_LINE_ACTIVE_POLARITY_HIGH;
        camera_dvp.framePol = frm_vld_high;
        camera_dvp.camSensorMode = CAM_SENSOR_MODE_V_AND_H;
        camera_dvp.burstType = CAM_BURST_TYPE_INCR16;
        camera_dvp.waitCount = 0x40;
        camera_dvp.memStart0 = MJPEG_READ_ADDR;
        camera_dvp.memSize0 = CAMERA_BUFFER_SIZE_WHEN_MJPEG;
        camera_dvp.frameSize0 = CAMERA_FRAME_SIZE;
        camera_dvp.memStart1 = 0;
        camera_dvp.memSize1 = 0;
        camera_dvp.frameSize1 = 0;
    } else {
        CAM_Hsync_Crop(m_desc->width-CROP_WIDTH, m_desc->width+CROP_WIDTH);
        CAM_Vsync_Crop((m_desc->height-CROP_HEIGHT)/2, (m_desc->height+CROP_HEIGHT)/2);

        camera_dvp.swMode = CAM_SW_MODE_MANUAL;
//        camera_dvp.swIntCnt = 0;
        camera_dvp.frameMode = CAM_INTERLEAVE_MODE;
        camera_dvp.yuvMode = CAM_YUV422;
        camera_dvp.linePol = CAM_LINE_ACTIVE_POLARITY_HIGH;
        camera_dvp.framePol = frm_vld_high;
        camera_dvp.camSensorMode = CAM_SENSOR_MODE_V_AND_H;
        camera_dvp.burstType = CAM_BURST_TYPE_INCR16;
        camera_dvp.waitCount = 0x40;
        camera_dvp.memStart0 = CROP_MEM_ADDR;
        camera_dvp.memSize0 = CROP_MEM_SIZE;
        camera_dvp.frameSize0 = CROP_FRAME_SIZE;
        camera_dvp.memStart1 = 0;
        camera_dvp.memSize1 = 0;
        camera_dvp.frameSize1 = 0;
    }

    CAM_Init(&camera_dvp);
    CAM_Enable();

    return 0;
}

int bl_cam_frame_wait(void)
{
    int ret;
    EventBits_t xResult;

    if ((ret = MJPEG_Get_Frame_Count())) {
        return ret;
    }

    xResult = xEventGroupWaitBits(
            camera_event,
            CAMERA_EVENT_FRAME_INT,
            pdTRUE,
            pdFAIL,
            10
    );
    if (xResult & CAMERA_EVENT_FRAME_INT){
        return 0;
    }

    return -1;
}

int bl_cam_yuv_frame_wait(void)
{
    int ret;
    EventBits_t xResult;

    if ((ret = CAM_Get_Frame_Count_0())) {
        return ret;
    }

    xResult = xEventGroupWaitBits(
            camera_event,
            CAMERA_EVENT_FRAME_INT,
            pdTRUE,
            pdFAIL,
            10
    );
    if (xResult & CAMERA_EVENT_FRAME_INT){
        return 0;
    }

    return -1;
}

void bl_cam_frame_edge_drop(void)
{
    // TODO
}

int bl_cam_frame_pop(void)
{
    MJPEG_Pop_Frame();
    return 0;
}

int bl_cam_yuv_frame_pop(void)
{
    CAM_Interleave_Pop_Frame();
    return 0;
}

int bl_cam_frame_pop_old(void)
{
    while (MJPEG_Get_Frame_Count() > 1) {
        MJPEG_Pop_Frame();
    }
    return 0;
}

int bl_cam_frame_get(uint32_t *frames, uint8_t **ptr1, uint32_t *len1, uint8_t **ptr2, uint32_t *len2)
{
    MJPEG_Frame_Info info;

    MJPEG_Get_Frame_Info(&info);
    *frames = info.validFrames;
    *ptr1 = (uint8_t *)(info.curFrameAddr);
    *len1 = info.curFrameBytes;
    if (*frames) {
        memcpy(*ptr1, jpgHeadBuf, JPG_HEAD_LENGTH_422);

        *ptr2 = NULL;
        *len2 = 0;
        return 0;
    }

    return -1;
}

int bl_cam_yuv_frame_get(uint32_t *frames, uint8_t **ptr1, uint32_t *len1, uint8_t **ptr2, uint32_t *len2)
{
    CAM_Interleave_Frame_Info info;

    CAM_Interleave_Get_Frame_Info(&info);
    *frames = info.validFrames;
    *ptr1 = (uint8_t *)(info.curFrameAddr);
    *len1 = info.curFrameBytes;
    if (*frames) {
        *ptr2 = NULL;
        *len2 = 0;
        return 0;
    }

    return -1;
}

int bl_cam_frame_fifo_get(uint32_t *frames, uint8_t **ptr1, uint32_t *len1, uint8_t **ptr2, uint32_t *len2)
{
    // TODO
    return 0;
}

static int mjpeg_init()
{
    memset(&mjpeg_engine, 0, sizeof(mjpeg_engine));

    mjpeg_engine.burst = MJPEG_BURST_INCR16;
    mjpeg_engine.quality = jpeg_quality;
    mjpeg_engine.yuv = MJPEG_YUV422_INTERLEAVE;
    mjpeg_engine.waitCount = 0x400;
    mjpeg_engine.bufferMjpeg = MJPEG_WRITE_ADDR;
    mjpeg_engine.sizeMjpeg = MJPEG_WRITE_SIZE;
    mjpeg_engine.bufferCamYY = MJPEG_READ_ADDR;
    mjpeg_engine.sizeCamYY = MJPEG_READ_SIZE;
    mjpeg_engine.bufferCamUV = 0;
    mjpeg_engine.sizeCamUV = 0;
    mjpeg_engine.resolutionX = CAMERA_RESOLUTION_X;
    mjpeg_engine.resolutionY = CAMERA_RESOLUTION_Y;
    mjpeg_engine.bitOrderEnable = ENABLE;
    mjpeg_engine.evenOrderEnable = ENABLE;
    mjpeg_engine.swapModeEnable = DISABLE;
    mjpeg_engine.overStopEnable = ENABLE;
    mjpeg_engine.reflectDmy = DISABLE;
    mjpeg_engine.verticalDmy = DISABLE;
    mjpeg_engine.horizationalDmy = DISABLE;

    MJPEG_Init(&mjpeg_engine);
    MJPEG_Packet_Type packetCfg = { .frameHead = JPG_HEAD_LENGTH_422 };
    MJPEG_Packet_Config(&packetCfg);
    MJPEG_Set_YUYV_Order_Interleave(1,0,3,2);
    MJPEG_Enable();

    return 0;
}

static int video_init(int useMjpeg)
{
#if 1
    static char video_init_flag = 0;

    if (0 == video_init_flag) {
        camera_event = xEventGroupCreate();
        video_init_flag = 1;
    }
#else
    if (NULL != camera_event) {
        vEventGroupDelete(camera_event);
    }
    camera_event = xEventGroupCreate();
#endif

    if (buffer_cam == NULL) {
        buffer_cam_size = CAMERA_BUFFER_SIZE_WHEN_MJPEG;
        buffer_cam = pvPortMalloc(buffer_cam_size);
    }

#if defined(CFG_USE_PSRAM)
    if (buffer_psram == NULL) {
        buffer_psram = pvPortMallocPsram(buffer_psram_size);
    }
#endif

    if (useMjpeg) {
        CAM_Disable();
        MJPEG_Disable();
        mjpeg_init();
        dvp_init(1, m_desc->frm_vld_high);
    } else {
        CAM_Disable();
        MJPEG_Disable();
        dvp_init(0, m_desc->frm_vld_high);
    }

    return 0;
}

static void set_camera_desc(const rt_camera_desc *desc)
{
    m_desc = desc;
}

int bl_cam_mjpeg_encoder(uint32_t yuv_addr, uint32_t jpeg_addr, uint32_t *jpeg_size,  uint32_t width, uint32_t height, uint32_t quality)
{
    // TODO
    return 0;
}

int bl_cam_init(int enable_mjpeg, const rt_camera_desc *desc)
{
    cam_init_gpio();
    set_camera_desc(desc);
    video_init(enable_mjpeg);
    return 0;
}

int bl_cam_restart(int enable_mjpeg)
{
    cam_init_gpio();
    video_init(enable_mjpeg);
    return 0;
}

int bl_cam_enable_24MRef(void)
{
    GLB_GPIO_Cfg_Type cfg;

#if 1  // configure pix_clk before cam_ref_clk
    cfg.drive=0;
    cfg.smtCtrl=1;
    cfg.gpioPin = GLB_GPIO_PIN_0;
    cfg.gpioFun = GPIO0_FUN_PIX_CLK;
    cfg.gpioMode = GPIO_MODE_INPUT;
    cfg.pullType = GPIO_PULL_NONE;
    GLB_GPIO_Init(&cfg);
#endif

#if 0
    cfg.drive=1;
    cfg.smtCtrl=1;
    cfg.gpioPin = GLB_GPIO_PIN_10;
    cfg.gpioFun = GPIO10_FUN_CAM_REF_CLK;
    cfg.gpioMode = GPIO_MODE_OUTPUT;
    cfg.pullType = GPIO_PULL_NONE;
    GLB_GPIO_Init(&cfg);
#else
    cfg.drive=1;
    cfg.smtCtrl=1;
    cfg.gpioPin = GLB_GPIO_PIN_9;
    cfg.gpioFun = GPIO9_FUN_CLK_OUT_1;
    cfg.gpioMode = GPIO_MODE_OUTPUT;
    cfg.pullType = GPIO_PULL_NONE;
    GLB_GPIO_Init(&cfg);

    GLB_Set_I2S_CLK(ENABLE, GLB_I2S_OUT_REF_CLK_NONE);
    PDS_Set_Audio_PLL_Freq(AUDIO_PLL_24576000_HZ);
    GLB_Set_Chip_Out_1_CLK_Sel(GLB_CHIP_CLK_OUT_I2S_REF_CLK);
#endif

    GLB_AHB_Slave1_Clock_Gate(DISABLE,0x1D);  // BL_AHB_SLAVE1_CAM
    GLB_AHB_Slave1_Clock_Gate(DISABLE,0x1E);  // BL_AHB_SLAVE1_MJPEG
    GLB_Set_CAM_CLK(ENABLE,GLB_CAM_CLK_DLL96M,3);
    GLB_SWAP_EMAC_CAM_Pin(GLB_EMAC_CAM_PIN_CAM);

    return 0;
}
