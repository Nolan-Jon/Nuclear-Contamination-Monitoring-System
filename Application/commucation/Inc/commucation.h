/*
 * @Author: Hengyang Jiang
 * @Date: 2024-12-13 14:38:32
 * @LastEditors: Hengyang Jiang
 * @LastEditTime: 2024-12-16 17:06:50
 * @Description: commucation.h
 *
 * Copyright (c) 2024 by https://github.com/Nolan-Jon, All Rights Reserved.
 */
#ifndef __COMMUCATION__H__
#define __COMMUCATION__H__
#include "FreeRTOSConfig.h"
#include "uart.h"
#define COMMUCATION_TASK_STACK (configMINIMAL_STACK_SIZE * 2)
#define COMMUCATION_TASK_PRIORITY (configMAX_PRIORITIES - 2)

/* 串口通信协议封装方式 */
/**
1. 通信协议格式

   | 帧头                    | 数据ID         | 数据          | 帧尾                                 |
   | ----------------------- | -------------- | -------------  | :----------------------------------|
   | protocol_header(4-byte) | cmd_id(2-byte) | data (n-byte) | frame_tail(2-byte，CRC16，整包校验) |

2. 帧头详细定义

   | 帧头        | 偏移位置 | 字节大小 | 内容                          |
   | ----------- | -------- | -------- | ----------------------------- |
   | sof(CMD)    | 0        | 1        | 数据帧起始字节，固定值为 0xA5 |
   | data_length | 1        | 2        | 数据帧中 data 的长度          |
   | crc_check   | 3        | 1        | 帧头CRC校验                   |

3. cmd_id 命令码 ID 说明 **(字节偏移 4，字节大小 2)**

   | 命令码         | 数据段长度      | 功能说明 |
   | -------------- | --------------- | -------- |
   | 0x0001(可修改) | 2 byte (16-bit) | 视觉数据 |
   | .......        |                 |          |

4.  数据段data (n-byte)

   | 数据           | 偏移位置 | 字节大小        | 内容                                  |
   | -------------- | -------- | --------------- | ------------------------------------- |
   | flags_register | 6        | 2               | 16位标志置位寄存器                    |
   | float_data     | 8        | 4 * data_length | float数据内容（4 * data_length-byte） |

5. frame_tail(CRC16，整包校验)
*/

#define PROTOCOL_HEAD_CMD 0xA5
#define OFFSET_BYTE 0x08                 /* 串口通信协议中,除了数据段外,其他部分所占字节数 */
#define PROTOCOL_FRAME_LENGTH_MAX (256u) /* 数据帧最大字节数 */
#define PROTOCOL_DATA_LENGTH_MAX (128u)  /* 数据最大字节数 */
/* 用于检验的宏定义 */
#define TRUE 0x01
#define FALSE 0x00
/* 生成测试数据 */
#define __COMMUCATION_PROTOCOL_TEST_DATA
#ifdef __COMMUCATION_PROTOCOL_TEST_DATA
void generate_test_data(uint16_t cmd_id,          /* 命令码 */
                        uint16_t flags_register); /* 16位寄存器 */
#endif
/* 串口通信协议结构体 */
#if defined(__CC_ARM)
#pragma anon_unions /*支持匿名结构体和联合体 */
#endif              //(__CC_ARM)

typedef struct
{
    /* data */
    struct Frame_Head
    {
        /* data */
        uint8_t sof;          /* 0xA5 */
        uint16_t data_length; /* 数据长度,包含尾部CRC校验码 */
        uint8_t crc_check;    /* 帧头CRC校验 */
    }; /* 帧头 */
    uint16_t cmd_id; /* 功能码 */
    struct Fream_Data
    {
        /* data */
        uint16_t flags_register;                      /* 16位标志位寄存器 */
        uint8_t float_data[PROTOCOL_DATA_LENGTH_MAX]; /* 数据 */
    }; /* 数据帧 */
    uint16_t frame_tail; /* 帧尾CRC校验 */
} Commucation_ProtocolDef;
typedef Commucation_ProtocolDef *Commucation_ProtocolHandle;

/* 将数据帧长度设置为buffer的大小,实际上要求一帧数据长度小于buffer大小,这样做是为了避免DMA传输错位 */
/* 同时利用IDLE中断实现不定长数据的传输 */
#define COMMUCATION_PROTOCOL_FRAME_SIZE UART_RECEIVE_BUFFER_SIZE /* 串口通信过程中,一个数据帧的大小 */
void commucation_task(void *pvParameters);
#endif //!__COMMUCATION__H__
