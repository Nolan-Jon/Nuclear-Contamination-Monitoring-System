/*
 * @Author: Hengyang Jiang
 * @Date: 2024-12-12 15:54:05
 * @LastEditors: Hengyang Jiang
 * @LastEditTime: 2024-12-17 09:57:54
 * @Description:
 *
 * Copyright (c) 2024 by https://github.com/Nolan-Jon, All Rights Reserved.
 */
#include "rtt.h"
#include "stdarg.h"
#include "stdlib.h"
#include "stdio.h"
/**
 * @description: 初始化rtt
 * @return {*}
 */
void rtt_log_init(void)
{
    SEGGER_RTT_Init();
    /* 设置输出的Terminal编号恒定为0 */
    SEGGER_RTT_SetTerminal(0);
    rtt_print_log("RTT Init Successfully!\r\n");
}
/**
 * @description: 通过SEGGER RTT打印日志,支持格式化输出,不支持浮点数格式化
 * @param {char *} sFormat
 * @return {*}
 */
int rtt_print_log(const char *sFormat, ...)
{
    // /* 该函数输出的Terminal编号恒定为0 */
    // SEGGER_RTT_SetTerminal(0);

    int r;
    va_list ParamList;

    va_start(ParamList, sFormat);
    r = SEGGER_RTT_vprintf(RTT_BUFFER_INDEX, sFormat, &ParamList);
    va_end(ParamList);
    return r;
}
/**
 * @description: 将浮点值格式化成字符串
 * @param {char} *str:转换后的字符串
 * @param {float} va:浮点值
 * @return {*}
 */
void rtt_float_to_str(char *str, float va)
{
    int flag = va < 0;
    int head = (int)va;
    int point = (int)((va - head) * 1000);
    head = abs(head);
    point = abs(point);
    if (flag)
        sprintf(str, "-%d.%d", head, point);
    else
        sprintf(str, "%d.%d", head, point);
}
/**
 * @description: 将字符串转换成对应的十六进制进行打印:一行16字节数据
 * @param {uint8_t*} buf:字符串指针
 * @param {int} len:打印长度
 * @return {*}
 */
void rtt_str_to_hex(const uint8_t* buf, int len)
{
    int i, c;

    while ((int)len > 0)
    {
        SEGGER_RTT_printf(0, "%08X: ", buf);

        for (i = 0; i < 16; i++)
        {
            if (i < (int)len)
            {
                SEGGER_RTT_printf(0, "%02X ", buf[i] & 0xFF);
            }
            else
            {
                SEGGER_RTT_printf(0, "   ");
            }

            // if (i == 7)
            // {
            //     SEGGER_RTT_printf(0, " ");
            // }
        }

        SEGGER_RTT_printf(0, " |");

        for (i = 0; i < 16; i++)
        {
            if (i < (int)len)
            {
                c = buf[i] & 0xFF;

                if ((c < 0x20) || (c >= 0x7F))
                {
                    c = '.';
                }
            }
            else
            {
                c = ' ';
            }

            SEGGER_RTT_printf(0, "%c", c);
        }

        SEGGER_RTT_printf(0, "|\n");
        len -= 16;
        buf += 16;
    }
}
