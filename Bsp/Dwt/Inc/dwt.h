/*
 * @Author: Hengyang Jiang
 * @Date: 2024-12-17 12:40:39
 * @LastEditors: Hengyang Jiang
 * @LastEditTime: 2024-12-17 13:12:30
 * @Description: dwt.h
 *
 * Copyright (c) 2024 by https://github.com/Nolan-Jon, All Rights Reserved.
 */
#ifndef __DWT__H__
#define __DWT__H__
#include "stdint.h"
#include "main.h"
#include "rtt.h"
void dwt_init(uint32_t cpu_freq_hz);
void dwt_delay_us(uint16_t _dwt_delay_time);
#endif //!__DWT__H__
