/**
 * @file bl606p_cks.c
 * @brief
 *
 * Copyright (c) 2021 Bouffalolab team
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
#include "bl702l_cks.h"
#include "cks_reg.h"

uint16_t CKS_Compute(uint8_t *data, uint32_t len)
{
    uint16_t checksum = 0;
    CKS_Reset();
    CKS_Set_Big_Endian();

    for (uint32_t i = 0; i < len; i++) {
        CKS_Data_In(*((uint8_t *)(data + i)));
    }

    checksum = (uint16_t)CKS_Data_Out();
    return checksum;
}
