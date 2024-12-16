/*
 * @Author: Hengyang Jiang
 * @Date: 2024-12-16 14:39:44
 * @LastEditors: Hengyang Jiang
 * @LastEditTime: 2024-12-16 16:58:44
 * @Description: crc16.c
 * 
 * Copyright (c) 2024 by https://github.com/Nolan-Jon, All Rights Reserved. 
 */
#include "crc16.h"
#include "stdlib.h"

static uint8_t crc_tab16_init = 0;
static uint16_t crc_tab16[256];
/**
 * @description: 计算输入字符串的16位宽crc
 * @param {uint8_t} *input_str:输入字符串
 * @param {uint16_t} num_bytes:字符串字节数
 * @return {*}
 */
uint16_t crc_16(const uint8_t *input_str, uint16_t num_bytes)
{
    uint16_t crc;
    const uint8_t *ptr;
    uint16_t a;
    if (!crc_tab16_init)
        init_crc16_tab();
    crc = CRC_START_16;
    ptr = input_str;
    if (ptr != NULL)
        for (a = 0; a < num_bytes; a++)
        {
            crc = (crc >> 8) ^ crc_tab16[(crc ^ (uint16_t)*ptr++) & 0x00FF];
        }
    return crc;
}
/**
 * @description: 计算输入字符串的16位宽modbus crc
 * @param {uint8_t} *input_str:输入字符串
 * @param {uint16_t} num_bytes:字符串字节数
 * @return {*}
 */
uint16_t crc_modbus(const uint8_t *input_str, uint16_t num_bytes)
{
    uint16_t crc;
    const uint8_t *ptr;
    uint16_t a;

    if (!crc_tab16_init)
        init_crc16_tab();

    crc = CRC_START_MODBUS;
    ptr = input_str;
    if (ptr != NULL)
        for (a = 0; a < num_bytes; a++)
        {

            crc = (crc >> 8) ^ crc_tab16[(crc ^ (uint16_t)*ptr++) & 0x00FF];
        }
    return crc;
}

/*
 * uint16_t update_crc_16( uint16_t crc, unsigned char c );
 *
 *函数update_crc_16()根据
 *前一个循环冗余校验值和下一个要检查的数据字节。
 */
/**
 * @description: 计算数据新的实际crc值
 * @param {uint16_t} crc:上一个crc值
 * @param {uint8_t} c:数据字节
 * @return {*}
 */
uint16_t update_crc_16(uint16_t crc, uint8_t c)
{
    if (!crc_tab16_init)
        init_crc16_tab();
    return (crc >> 8) ^ crc_tab16[(crc ^ (uint16_t)c) & 0x00FF];
}
/**
 * @description: 查找表首次由init_crc16_tab()例程计算
 * @return {*}
 */
void init_crc16_tab(void)
{
    uint16_t i;
    uint16_t j;
    uint16_t crc;
    uint16_t c;
    for (i = 0; i < 256; i++)
    {
        crc = 0;
        c = i;
        for (j = 0; j < 8; j++)
        {
            if ((crc ^ c) & 0x0001)
                crc = (crc >> 1) ^ CRC_POLY_16;
            else
                crc = crc >> 1;
            c = c >> 1;
        }
        crc_tab16[i] = crc;
    }
    crc_tab16_init = 1;
}
