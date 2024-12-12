/*
 * @Author: Hengyang Jiang
 * @Date: 2024-12-12 15:54:18
 * @LastEditors: Hengyang Jiang
 * @LastEditTime: 2024-12-12 22:34:58
 * @Description:
 *
 * Copyright (c) 2024 by https://github.com/Nolan-Jon, All Rights Reserved.
 */
#ifndef __RTT__H__
#define __RTT__H__

#include "SEGGER_RTT.h"

#define RTT_BUFFER_INDEX 0         // RTT Viewer的终端编号为0
#define RTT_INFO_BUFFER_INDEX 1    // RTT Viewer的终端编号为1
#define RTT_WARNING_BUFFER_INDEX 2 // RTT Viewer的终端编号为2
#define RTT_ERROR_BUFFER_INDEX 3   // RTT Viewer的终端编号为3
void rtt_log_init(void);
int rtt_print_log(const char *sFormat, ...);
void rtt_float_to_str(char *str, float va);

/**
 * @description: 日志功能原型,供下面的LOGI,LOGW,LOGE等使用
 * @return {*}
 */
#define LOG_PROTO(bufferidx, type, color, format, ...)                       \
        SEGGER_RTT_printf(bufferidx, "  %s%s" format "\r\n%s", \
                          color,                                  \
                          type,                                   \
                          ##__VA_ARGS__,                          \
                          RTT_CTRL_RESET)
// information level
#define LOGINFO(format, ...) LOG_PROTO(RTT_INFO_BUFFER_INDEX, "I:", RTT_CTRL_TEXT_BRIGHT_GREEN, format, ##__VA_ARGS__)
// warning level
#define LOGWARNING(format, ...) LOG_PROTO(RTT_WARNING_BUFFER_INDEX, "W:", RTT_CTRL_TEXT_BRIGHT_YELLOW, format, ##__VA_ARGS__)
// error level
#define LOGERROR(format, ...) LOG_PROTO(RTT_ERROR_BUFFER_INDEX, "E:", RTT_CTRL_TEXT_BRIGHT_RED, format, ##__VA_ARGS__)

#endif //!__RTT__H__
