/*
 * @Author: Hengyang Jiang
 * @Date: 2024-12-12 15:54:05
 * @LastEditors: Hengyang Jiang
 * @LastEditTime: 2024-12-16 11:49:00
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
 * @param {char} *str
 * @param {float} va
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
