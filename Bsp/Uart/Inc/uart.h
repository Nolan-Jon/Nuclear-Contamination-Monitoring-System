/*
 * @Author: Hengyang Jiang
 * @Date: 2024-12-12 21:34:06
 * @LastEditors: Hengyang Jiang
 * @LastEditTime: 2024-12-17 20:24:00
 * @Description: uart.h
 *
 * Copyright (c) 2024 by https://github.com/Nolan-Jon, All Rights Reserved.
 */
#ifndef __UART__H__
#define __UART__H__
#include "stdint.h"
#include "usart.h"
/* 创建串口实例结构体,把每个串口都当成一个实例对象,即一个串口设备对应一个串口对象 */
/* 串口是独占的点对点通信,不存在多个设备同时占用一个串口的情况 */
#define DEVICE_UART_NUM 2            /* 开发板上只有两个可用串口,从0开始算 */
#define UART_RECEIVE_BUFFER_SIZE 256 /* 接受缓冲区的大小*/
#define IDX_OF_UART_DEVICE_1 0       /* 串口0对应的编号*/
#define IDX_OF_UART_DEVICE_3 1       /* 串口3对应的编号*/

typedef void (*uart_recv_decode)(uint8_t *buffer, uint16_t Size); /* 解析接受串口数据的函数指针,做结构体的回调函数*/

typedef struct
{
    /* data */
    uint8_t idx;                                   /* 串口实例的编号,对于该开发板来讲,编号只有1和3 */
    uint8_t recv_buffer[UART_RECEIVE_BUFFER_SIZE]; /* 接收缓冲区 */
    uint16_t recv_buffer_size;                     /* 接收一包数据的大小:数据帧大小 */
    UART_HandleTypeDef *uartHandle;                /* 每个UART实例对应的设备句柄 */
    uart_recv_decode uart_recv_decode_callback;    /* 解析协议回调函数 */
} UART_InstanceDef;
typedef UART_InstanceDef *UART_InstanceHandle;

UART_InstanceHandle Y_uart_create_instance(uint8_t, uint16_t, UART_HandleTypeDef *, uart_recv_decode);

#endif //!__UART__H__
