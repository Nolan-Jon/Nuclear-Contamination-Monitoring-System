/*
 * @Author: Hengyang Jiang
 * @Date: 2024-12-12 15:54:18
 * @LastEditors: Hengyang Jiang
 * @LastEditTime: 2024-12-13 17:00:14
 * @Description:
 *
 * Copyright (c) 2024 by https://github.com/Nolan-Jon, All Rights Reserved.
 */
#ifndef __RTT__H__
#define __RTT__H__

#include "SEGGER_RTT.h"

#define RTT_BUFFER_INDEX 0         // Buffer index 0表示输出到终端
void rtt_log_init(void);
int rtt_print_log(const char *sFormat, ...);
void rtt_float_to_str(char *str, float va);
void rtt_str_to_hex(const uint8_t* buf, int len);
/**
 * @description: 日志功能原型,供下面的LOGI,LOGW,LOGE等使用
 * @return {*}
 */
#define LOG_PROTO(type, color, format, ...)                       \
        SEGGER_RTT_printf(RTT_BUFFER_INDEX, "  %s%s" format "\r\n%s", \
                          color,                                  \
                          type,                                   \
                          ##__VA_ARGS__,                          \
                          RTT_CTRL_RESET)
/* 输出LOG前可以使用SEGGER_RTT_SetTerminal选择终端编号 */
// information level
#define LOGINFO(format, ...) LOG_PROTO("I:", RTT_CTRL_TEXT_BRIGHT_GREEN, format, ##__VA_ARGS__)
// warning level
#define LOGWARNING(format, ...) LOG_PROTO("W:", RTT_CTRL_TEXT_BRIGHT_YELLOW, format, ##__VA_ARGS__)
// error level
#define LOGERROR(format, ...) LOG_PROTO("E:", RTT_CTRL_TEXT_BRIGHT_RED, format, ##__VA_ARGS__)

#endif //!__RTT__H__
