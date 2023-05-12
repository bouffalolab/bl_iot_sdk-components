/**
 * @file scaling_down_int.c
 * @brief
 *
 * Copyright (c) 2022 Bouffalolab team
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 */

#include "scaling_down_int.h"

static int scaling_down_horizontal_direction(unsigned char *raw, int width, int height, unsigned char *dst_raw, int dst_width, int dst_height, int ratio_x, int ratio_y, unsigned char *linebuff) __attribute__((optimize("O2")));
static int scaling_down_vertical_direction(unsigned char *raw, int width, int height, unsigned char *dst_raw, int dst_width, int dst_height, int ratio_x, int ratio_y, unsigned char *linebuff) __attribute__((optimize("O2")));

/**
 * @brief horizontal direction scaling down
 *
 * @param raw raw data ptr
 * @param width raw width
 * @param height raw height
 * @param dst_raw dst data ptr
 * @param dst_width dst width
 * @param dst_height dst height
 * @param ratio_x x direction ratio
 * @param ratio_y y direction ratio
 * @param linebuff line tmp buffer
 * @return int
 */
static int scaling_down_horizontal_direction(unsigned char *raw, int width, int height, unsigned char *dst_raw, int dst_width, int dst_height, int ratio_x, int ratio_y, unsigned char *linebuff)
{
    int i = 0;
    int j = 0;
    int k = 0;
    int x0 = 0;
    int x1 = 0;
    int x2 = 0;
    int x3 = 0;
    int dx0 = 0;
    int dx1 = 0;
    int dx2 = 0;
    int dx3 = 0;
    int pixel = 0;
    int pixel1 = 0;
    int pixel2 = 0;
    int sum_ratio_x = 0;

    for (j = 0; j < height; j++) {
        sum_ratio_x = 0;
        for (i = 0; i < dst_width; i++) {
            x0 = (sum_ratio_x + 128) >> 8;
            x1 = x0 - 1;
            x2 = x0 + 1;
            x3 = x2 + 1;
            if (x1 <= 0) {
                x1 = 0;
            }
            if (x2 >= width) {
                x2 = width - 1;
            }
            if (x3 >= width) {
                x3 = width - 1;
            }
            dx0 = (sum_ratio_x + 128) - x0 * 256;
            dx1 = 256 + dx0;
            dx2 = 256 - dx0;
            dx3 = 256 + dx2;
            sum_ratio_x = sum_ratio_x + ratio_x;
            pixel1 = (dx2 * raw[j * width + x0] + dx0 * raw[j * width + x2]) >> 8;
            if (ratio_x > 256 && ratio_x <= 512) {
                pixel = pixel1;
            } else {
                pixel2 = ((dx3 * raw[j * width + x1] + dx1 * raw[j * width + x3]) * 342) >> 18;
                if (ratio_x == 1024) {
                    pixel = (136 * pixel1 + 120 * pixel2) >> 8;
                } else {
                    pixel = (200 * pixel1 + 56 * pixel2) >> 8;
                }
            }

            if (pixel > 255) {
                pixel = 255;
            }
            k = j * dst_width + i;
            *(linebuff + k) = pixel;
        }
    }
    return 0;
}
/**
 * @brief vertical direction scaling down
 *
 * @param raw raw data ptr
 * @param width raw width
 * @param height raw height
 * @param dst_raw dst data ptr
 * @param dst_width dst width
 * @param dst_height dst height
 * @param ratio_x x direction ratio
 * @param ratio_y y direction ratio
 * @param linebuff line tmp buffer
 * @return int
 */
static int scaling_down_vertical_direction(unsigned char *raw, int width, int height, unsigned char *dst_raw, int dst_width, int dst_height, int ratio_x, int ratio_y, unsigned char *linebuff)
{
    int i = 0;
    int j = 0;
    int k = 0;
    int y0 = 0;
    int y1 = 0;
    int y2 = 0;
    int y3 = 0;
    int dy0 = 0;
    int dy1 = 0;
    int dy2 = 0;
    int dy3 = 0;
    int pixel = 0;
    int pixel1 = 0;
    int pixel2 = 0;
    int sum_ratio_y = 0;

    for (j = 0; j < dst_height; j++) {
        for (i = 0; i < dst_width; i++) {
            y0 = (sum_ratio_y + 128) >> 8;
            y1 = y0 - 1;
            y2 = y0 + 1;
            y3 = y2 + 1;

            if (y1 <= 0) {
                y1 = 0;
            }

            if (y2 >= height) {
                y2 = height - 1;
            }

            if (y3 >= height) {
                y3 = height - 1;
            }
            dy0 = (sum_ratio_y + 128) - y0 * 256;
            dy1 = 256 + dy0;
            dy2 = 256 - dy0;
            dy3 = 256 + dy2;
            pixel1 = (dy2 * (*(linebuff + y0 * dst_width + i)) + dy0 * (*(linebuff + y2 * dst_width + i))) >> 8;

            if (ratio_y > 256 && ratio_y <= 512) {
                pixel = pixel1;
            } else {
                pixel2 = ((dy3 * (*(linebuff + y1 * dst_width + i)) + dy1 * (*(linebuff + y3 * dst_width + i))) * 342) >> 18;
                if (ratio_y == 1024) {
                    pixel = (136 * pixel1 + 120 * pixel2) >> 8;
                } else {
                    pixel = (200 * pixel1 + 56 * pixel2) >> 8;
                }
            }

            if (pixel > 255) {
                pixel = 255;
            }

            k = dst_width * j + i;
            dst_raw[k] = pixel;
        }
        sum_ratio_y = sum_ratio_y + ratio_y;
    }
    return 0;
}
/**
 * @brief reszise a picture to 50x50 size
 *
 * @param src_8 src picture addr
 * @param dst_resized dst picture addr
 * @param buffer temporary buf during operation
 */
void resize_image_test(uint8_t *src_8, int8_t *dst_resized, int8_t *buffer)
{
    int width = W_DATA;
    int height = H_DATA;
    int dst_width = 50;                                                                    //if dst img change, register for software to set, shadow, AP need to access
    int dst_height = 50;                                                                   //if dst img change, register for software to set, shadow, AP need to access
    int ratio_x = (W_DATA / 50 * 256); /****Software need to calculate as 640/50*256****/  //if dst img change, register for software to set, shadow, AP need to access
    int ratio_y = (H_DATA / 50 * 256); /****Software need to calculate as 480/50*256 ****/ //if dst img change, register for software to set, shadow, AP need to access

    scaling_down_horizontal_direction(src_8, width, height, (uint8_t *)dst_resized, dst_width, dst_height, ratio_x, ratio_y, (uint8_t *)buffer);
    scaling_down_vertical_direction(src_8, width, height, (uint8_t *)dst_resized, dst_width, dst_height, ratio_x, ratio_y, (uint8_t *)buffer);
}

/**
 * @brief draw a picture, picture size W_DATA x H_DATA
 *
 * @param memory_addr draw start addr
 */
void test_pic_init(uint32_t memory_addr)
{
    uint32_t i;
    uint8_t val;

    for (i = 0; i < W_DATA * H_DATA; i++) {
        *((volatile uint8_t *)((uintptr_t)memory_addr + i)) = i;
    }

    for (i = 0; i < W_DATA * H_DATA; i++) {
        val = *((volatile uint8_t *)((uintptr_t)memory_addr + i));

        if ((uint8_t)i != val) {
            // MSG("test8 init fail!\r\n");
            // MSG("addr = 0x%08X, val = 0x%08X\r\n", (memory_addr + i), *((volatile uint8_t *)(memory_addr + i)));
            while (1) {
            }
        }
    }
}
/**
 * @brief Convert YUV400 to grayscale image
 *
 * @param y_400 YUV400 image ptr
 * @param gray gray image ptr
 * @param len image size
 */
void Y_to_rgb565_gray(uint8_t *y_400, uint16_t *gray, uint32_t len)
{
    uint8_t tmp = 0;

    for (uint32_t i = 0; i < len; i++) {
        tmp = y_400[i] >> 3;
        gray[i] = tmp;
        gray[i] = gray[i] << 5;
        gray[i] += tmp;
        gray[i] = gray[i] << 6;
        gray[i] += tmp;
    }
}
