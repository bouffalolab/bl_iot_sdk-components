/**
 * @file scaling_down_int.h
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

#ifndef __SCALING_DOWN_INT_H__
#define __SCALING_DOWN_INT_H__

#include "misc.h"
/* 320x240 need more than 90K of memory space */
#define W_DATA (320)
#define H_DATA (240)

/* pic rgb565 gray sha256sum value */
// clang-format off
static const uint8_t sha256sum_pic320x240[] = {
0x6b, 0xd0, 0x36, 0x3c, 0x73, 0xea, 0x78, 0x55,
0xe1, 0x52, 0x7d, 0x06, 0x36, 0xbd, 0xd0, 0xff,
0x16, 0xc6, 0xfc, 0x5c, 0xce, 0x18, 0xda, 0x16,
0x67, 0x37, 0x87, 0x5e, 0xc0, 0xb6, 0x09, 0xc9,
};

static const uint8_t sha256sum_pic640x480[] = {
0x4d, 0xa4, 0x58, 0x24, 0x27, 0x6d, 0x89, 0xfd,
0x31, 0x16, 0xd0, 0x9b, 0x44, 0x18, 0xa6, 0xab,
0x7e, 0x7f, 0xba, 0xeb, 0x72, 0x3b, 0x83, 0x4d,
0x4d, 0xfd, 0x58, 0x3a, 0x75, 0x35, 0x14, 0xe1,
};
// clang-format on

void test_pic_init(uint32_t memory_addr);
void Y_to_rgb565_gray(uint8_t *y_400, uint16_t *gray, uint32_t len);
void resize_image_test(uint8_t *src_8, int8_t *dst_resized, int8_t *buffer) __attribute__((optimize("O2")));

#endif /* __SCALING_DOWN_INT_H__ */