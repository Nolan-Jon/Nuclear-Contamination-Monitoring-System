/*
 * @Author: Hengyang Jiang
 * @Date: 2024-12-16 14:40:03
 * @LastEditors: Hengyang Jiang
 * @LastEditTime: 2024-12-16 14:53:22
 * @Description: crc16.h
 * 
 * Copyright (c) 2024 by https://github.com/Nolan-Jon, All Rights Reserved. 
 */
#ifndef __CRC16__H__
#define __CRC16__H__

#define CRC_START_16 0xFFFF
#define CRC_START_MODBUS 0xFFFF
#define CRC_POLY_16 0xA001

uint16_t crc_16(const uint8_t *input_str, uint16_t num_bytes);
uint16_t crc_modbus(const uint8_t *input_str, uint16_t num_bytes);
uint16_t update_crc_16(uint16_t crc, uint8_t c);
void init_crc16_tab(void);

#endif  //!__CRC16__H__
