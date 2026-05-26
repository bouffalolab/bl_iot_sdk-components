/*
 * Copyright (c) 2016-2026 Bouffalolab.
 *
 * This file is part of
 *     *** Bouffalolab Software Dev Kit ***
 *      (see www.bouffalolab.com).
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of Bouffalo Lab nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef __BFLB_MFD_H
#define __BFLB_MFD_H

#include <stdbool.h>
#include <stdlib.h>
#if defined BL616
#include <bl616.h>
#include <bl616_romdriver_e907.h>
#elif defined BL616CL
#include <bl616cl.h>
#include <bl616cl_romdriver_e907.h>
#endif

#include <bflb_boot2.h>
#include <bflb_flash.h>
#include <bflb_sec_aes.h>

#if defined BL616
#define MFD_XIP_BASE BL616_FLASH_XIP_BASE
#define MFD_XIP_END BL616_FLASH_XIP_END
#elif defined BL616CL
#define MFD_XIP_BASE BL616CL_FLASH_XIP_BASE
#define MFD_XIP_END BL616CL_FLASH_XIP_END
#endif
#define SF_CTRL_GET_FLASH_IMAGE_OFFSET() bflb_sf_ctrl_get_flash_image_offset(0, SF_CTRL_FLASH_BANK0)
#define BOOT2_PARTITION_ADDR_ACTIVE bflb_boot2_partition_addr_active
#define MFD_RUNNING_MEMORY_CHECK() 

#endif