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
#ifndef __BFLB_MFD_BL702L_H

#include <stdbool.h>
#include <stdlib.h>
#include <bl702l.h>
#include <bl702l_ef_ctrl.h>
#include <bl702l_sec_eng.h>
#include <bl702l_romdriver.h>

#include <softcrc.h>
#include <hal_boot2.h>

#define MFD_XIP_BASE BL702L_FLASH_XIP_BASE
#define MFD_XIP_END BL702L_FLASH_XIP_END
#define BOOT2_PARTITION_ADDR_ACTIVE hal_boot2_partition_addr_active
#define MFD_RUNNING_MEMORY_CHECK() 

#define SEC_ENG_REQUEST_AES_ACCESS() Sec_Eng_Group0_Request_AES_Access()
#define SEC_ENG_RELEASE_AES_ACCESS() Sec_Eng_Group0_Release_AES_Access()
#define SF_CTRL_GET_FLASH_IMAGE_OFFSET() RomDriver_SF_Ctrl_Get_Flash_Image_Offset()

#endif
