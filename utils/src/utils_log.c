#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>

#include "utils_log.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(BL602)
#define ONE_LINE_MAX_NUM        (16)/* for print_buf log length */
#else
#define ONE_LINE_MAX_NUM        (50)/* for print_buf log length */
#endif
#define MODULE_LOG_LOCK_LOCK    /* reserved */
#define MODULE_LOG_LOCK_UNLOCK  /* reserved */

#if defined(BL602)
#define LOGBUF_SIZE      (128)
#elif defined(BL702) || defined(BL702L)
#define LOGBUF_SIZE      (512)
#else
#define LOGBUF_SIZE      (512)
static char log_buf[LOGBUF_SIZE];
#endif

int log_buf_out(const char *file, int line, const void *inbuf, int len, LOG_BUF_OUT_DATA_TYPE_T type)
{
    char *buf = (char *)inbuf;
    char *pbuffer = NULL;

#if defined(BL602) || defined(BL702) || defined(BL702L)
    pbuffer = (char *)pvPortMalloc(LOGBUF_SIZE);
    if(pbuffer == NULL){
        return -1;
    }
#else
    pbuffer = (char *)log_buf;
#endif
    int m = 0, n = 0;
    int j = 0, k = 0, tmp = 0;

    MODULE_LOG_LOCK_LOCK;

    tmp = (LOGBUF_SIZE)/3;/* 数组最大长度 */
    if ((ONE_LINE_MAX_NUM > tmp) || (len < 1))
    {
        MODULE_LOG_LOCK_UNLOCK;
        return -1;
    }

    m = len / ONE_LINE_MAX_NUM;
    n = len % ONE_LINE_MAX_NUM;
    if (n > 0) {
        m++;
    }


    if (n > 0) {
        /* 非整数倍 */
        for (k = 0; k < m; k++) {
            if ((k+1) == m) {
                /* 最后一帧数据 */
                tmp = 0;
                for (j = 0; j < n; j++) {
                    switch (type) {
                        case LOG_BUF_OUT_DATA_TYPE_INT8:
                        {
                            tmp += sprintf(pbuffer + tmp, "%3d ", (int8_t)buf[k*ONE_LINE_MAX_NUM+j]);
                        }
                        break;
                        case LOG_BUF_OUT_DATA_TYPE_UNT8:
                        {
                            tmp += sprintf(pbuffer + tmp, "%3u ", (uint8_t)buf[k*ONE_LINE_MAX_NUM+j]);
                        }
                        break;
                        case LOG_BUF_OUT_DATA_TYPE_HEX:
                        default:
                        {
                            tmp += sprintf(pbuffer + tmp, "%02x ", (uint8_t)buf[k*ONE_LINE_MAX_NUM+j]);
                        }
                    }
                }
                log_buf_pri(file, line, "%.*s\r\n", tmp, pbuffer);
            } else {
                tmp = 0;
                for (j = 0; j < ONE_LINE_MAX_NUM; j++) {
                    switch (type) {
                        case LOG_BUF_OUT_DATA_TYPE_INT8:
                        {
                            tmp += sprintf(pbuffer + tmp, "%3d ", (int8_t)buf[k*ONE_LINE_MAX_NUM+j]);
                        }
                        break;
                        case LOG_BUF_OUT_DATA_TYPE_UNT8:
                        {
                            tmp += sprintf(pbuffer + tmp, "%3u ", (uint8_t)buf[k*ONE_LINE_MAX_NUM+j]);
                        }
                        break;
                        case LOG_BUF_OUT_DATA_TYPE_HEX:
                        default:
                        {
                            tmp += sprintf(pbuffer + tmp, "%02x ", (uint8_t)buf[k*ONE_LINE_MAX_NUM+j]);
                        }
                    }
                }
                log_buf_pri(file, line, "%.*s\r\n", tmp, pbuffer);
            }
        }
    } else {
        /* 整数倍 */
        for (k = 0; k < m; k++) {
            tmp = 0;
            for (j = 0; j < ONE_LINE_MAX_NUM; j++) {
                switch (type) {
                    case LOG_BUF_OUT_DATA_TYPE_INT8:
                    {
                        tmp += sprintf(pbuffer + tmp, "%3d ", (int8_t)buf[k*ONE_LINE_MAX_NUM+j]);
                    }
                    break;
                    case LOG_BUF_OUT_DATA_TYPE_UNT8:
                    {
                        tmp += sprintf(pbuffer + tmp, "%3u ", (uint8_t)buf[k*ONE_LINE_MAX_NUM+j]);
                    }
                    break;
                    case LOG_BUF_OUT_DATA_TYPE_HEX:
                    default:
                    {
                        tmp += sprintf(pbuffer + tmp, "%02x ", (uint8_t)buf[k*ONE_LINE_MAX_NUM+j]);
                    }
                }
            }
            log_buf_pri(file, line, "%.*s\r\n", tmp, pbuffer);
        }
    }

    MODULE_LOG_LOCK_UNLOCK;

#if defined(BL602) || defined(BL702) || defined(BL702L)
    vPortFree(pbuffer);
#endif

    return 0;
}

extern void vprint(const char *fmt, va_list argp);

extern volatile bool sys_log_all_enable;

void bl_printk(const char *format, ...)
{
    va_list args;

    if (sys_log_all_enable) {
        /* args point to the first variable parameter */
        va_start(args, format);

        /* You can add your code under here. */
        vprint(format, args);

        va_end(args);
    }
}

#ifdef __cplusplus
}
#endif

