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
#define _ZERO_WITH_COMMA_1 0,
#define _second_arg(__ignored, val, ...) val
#define _is_enabled(x)                __is_enabled(x)
#define __is_enabled(val)             ___is_enabled(_ZERO_WITH_COMMA_##val)
#define ___is_enabled(junk_or_comma)  _second_arg(junk_or_comma 1, 0)
#define IS_ENABLED(config)            _is_enabled(config)
#undef CONFIG_ANTI_ROLLBACK
#define CONFIG_APP_ANTI_ROLLBACK_VER 0
#undef CONFIG_BFLB_LOG
#define CONFIG_BFLB_MTD 1
#define CONFIG_COMX "COM5"
#undef CONFIG_COREDUMP
#undef CONFIG_COREDUMP
#define CONFIG_DEBUG 1
#undef CONFIG_EASYFLASH4
#define CONFIG_FREERTOS 1
#define CONFIG_LITTLEFS 1
#define CONFIG_LMAC154 1
#define CONFIG_MBEDTLS 1
#define CONFIG_MBEDTLS_AES_USE_HW 1
#define CONFIG_MBEDTLS_BIGNUM_USE_HW 1
#define CONFIG_MBEDTLS_ECC_USE_HW 1
#undef CONFIG_MBEDTLS_SELF_TEST
#define CONFIG_MBEDTLS_SHA1_USE_HW 1
#define CONFIG_MBEDTLS_SHA256_USE_HW 1
#define CONFIG_MBEDTLS_SHA512_USE_HW 1
#undef CONFIG_NEWLIB
#undef CONFIG_NEWLIB_STANDARD
#define CONFIG_PARTITION 1
#define CONFIG_RF 1
#define CONFIG_ROMAPI 1
#define CONFIG_THREAD 1
#define CONFIG_TLSF 1
#undef CONFIG_VSNPRINTF_FLOAT
#undef CONFIG_VSNPRINTF_FLOAT_EX
#undef CONFIG_VSNPRINTF_LONG_LONG
#define CONFIG_VSNPRINTF_NANO 1
#undef CONFIG_XTAL_POWER_TYPE_ACTIVE
