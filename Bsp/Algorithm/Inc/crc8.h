/*
 * @Author: Hengyang Jiang
 * @Date: 2024-12-16 14:39:57
 * @LastEditors: Hengyang Jiang
 * @LastEditTime: 2024-12-16 16:58:28
 * @Description: crc8.h
 * 
 * Copyright (c) 2024 by https://github.com/Nolan-Jon, All Rights Reserved. 
 */
#ifndef __CRC8__H__
#define __CRC8__H__
#define CRC_START_8 0x00
#include "stdint.h"
uint8_t crc_8(const uint8_t *input_str, uint16_t num_bytes);
uint8_t update_crc_8(uint8_t crc, uint8_t val);

#endif  //!__CRC8__H__
