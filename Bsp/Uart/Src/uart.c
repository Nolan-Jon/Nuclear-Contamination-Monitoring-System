/*
 * @Author: Hengyang Jiang
 * @Date: 2024-12-12 21:33:51
 * @LastEditors: Hengyang Jiang
 * @LastEditTime: 2024-12-12 22:31:28
 * @Description: uart.c
 *
 * Copyright (c) 2024 by https://github.com/Nolan-Jon, All Rights Reserved.
 */
#include "uart.h"
#include "rtt.h"
static UART_InstanceHandle uart_instance_array[DEVICE_UART_NUM] = {NULL}; /* 挂载串口实例句柄 */
/**
 * @description:
 * @return {*}
 */
UART_InstanceHandle Y_uart_create_instance(uint8_t instance_num,                       /* 串口实例编号 */
                                           uint8_t recv_buffer_size,                   /* 串口接受一包数据大小 */
                                           UART_HandleTypeDef *uartHandle,             /* 串口实例对应的设备句柄 */
                                           uart_recv_decode uart_recv_decode_callback) /* 解析回调函数 */
{
    /* 检测串口编号是否非法 */
    if ((instance_num != IDX_UART_INSTANCE_1) || (instance_num != IDX_UART_INSTANCE_3))
    {
        while (1)
        {
            LOGERROR("[uart_create]The Serial Port Number Is Illegal!");
        }
    }
    /* 检测是否已经注册过 */
    
}