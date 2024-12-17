/*
 * @Author: Hengyang Jiang
 * @Date: 2024-12-13 14:38:45
 * @LastEditors: Hengyang Jiang
 * @LastEditTime: 2024-12-17 22:09:23
 * @Description: commucation.c 上位机通信文件
 *
 * Copyright (c) 2024 by https://github.com/Nolan-Jon, All Rights Reserved.
 */

#include "commucation.h"
#include "FreeRTOS.h"
#include "task.h"
#include "usart.h"
#include "rtt.h"
#include "crc8.h"
#include "crc16.h"
#include "string.h"
#ifdef TEST_LED_RGB
#include "led.h"
#endif // TEST_LED_RGB
TaskHandle_t commucation_task_handle;
UART_InstanceHandle commucation_uart_handle;
Commucation_ProtocolHandle message_handle; /* 用于装载通信协议解码后的数据,只创建一次 */
#ifdef TEST_LED_RGB
LED_InstanceHandle commucation_led_instance_handle;
#endif // TEST_LED_RGB
/**
 * @description: 获取crc8校验码
 * @param {uint8_t} *message:输入字符串
 * @param {uint16_t} size:字符串字节数
 * @return {*}
 */
uint8_t get_crc8_check_code(uint8_t *message, uint16_t size)
{
    return crc_8(message, size);
}
/**
 * @description: 检验crc8数据段
 * @param {uint8_t} *message:输入字符串
 * @param {uint16_t} size:字符串字节数
 * @return {*}
 */
uint8_t crc8_check(uint8_t *message, uint16_t size)
{
    uint8_t ucExpected = 0; /* 期望值 */
    /* 问题:为什么需要<=2,,不能计算一个字节数据的crc吗? */
    if ((message == NULL) || (size <= 2))
    {
        return FALSE; /* false */
    }
    ucExpected = crc_8(message, size - 1);
    return (ucExpected == message[size - 1]); /* true or false */
}
/**
 * @description: 获取crc16校验码
 * @param {uint8_t} *message:输入字符串
 * @param {uint32_t} size:字符串字节数
 * @return {*}
 */
uint8_t get_crc16_check_code(uint8_t *message, uint32_t size)
{
    return crc_16(message, size);
}
/**
 * @description: 检验crc16数据段
 * @param {uint8_t} *message:输入字符串
 * @param {uint32_t} size:字符串字节数
 * @return {*}
 */
uint8_t crc16_check(uint8_t *message, uint32_t size)
{
    /* 注意,期望值是16字节 */
    uint16_t ucExpected = 0; /* 期望值 */
    if ((message == NULL) || (size <= 2))
    {
        return FALSE; /* false */
    }
    /* 计算除了帧尾字节外的其他字节的crc16 */
    ucExpected = crc_16(message, size - 2);
    return (((ucExpected & 0xff) == message[size - 2]) && (((ucExpected >> 8) & 0xff) == message[size - 1])); /* true or false */
}
/**
 * @description: 检验数据帧头
 * @param {Commucation_ProtocolHandle} message:数据帧句柄
 * @param {uint8_t*} rx_buffer:接收buffer
 * @return {*}
 */
uint8_t protocol_head_check(Commucation_ProtocolHandle message, uint8_t *rx_buffer)
{
    /* Commucation_ProtocolDef结构体只需要申请一次空间 */
    /* 如果检验通过,将rx_buffer的数据装载到message句柄对应的结构体 */
    if (rx_buffer[0] == PROTOCOL_HEAD_CMD)
    {
#if !defined __EASY_PRINT_TEST && defined __COMMUCATION_PROTOCOL_TEST_DATA
        LOGWARNING("TEST Pass Protocol Head CMD.\r\n");
#endif //!__EASY_PRINT_TEST && __COMMUCATION_PROTOCOL_TEST_DATA
        message->sof = PROTOCOL_HEAD_CMD;
        /* 检验帧头crc8 */
        /* 帧头4字节 */
        if (crc8_check(rx_buffer, 4))
        {
#if !defined __EASY_PRINT_TEST && defined __COMMUCATION_PROTOCOL_TEST_DATA
            LOGWARNING("TEST Pass crc8 Check.\r\n");
#endif                                                                 //!__EASY_PRINT_TEST && __COMMUCATION_PROTOCOL_TEST_DATA
            message->data_length = (rx_buffer[2] << 8) | rx_buffer[1]; /* 先发低字节,后发高字节 */
            message->crc_check = rx_buffer[3];
            message->cmd_id = (rx_buffer[5] << 8) | rx_buffer[4]; /* 先发低字节,后发高字节 */
            /* 不需要装载数据以及帧尾crc,因为数据是不定长的 */
            return TRUE;
        }
    }
    return FALSE;
}
/**
 * @description: 接收解码函数
 * @param {uint8_t} *buffer
 * @param {uint16_t} pack_num
 * @return {*}
 */
static void commucation_message_decode(uint8_t *rx_buffer, uint16_t frame_length)
{
#ifdef __EASY_PRINT_TEST
    rtt_str_to_hex(rx_buffer, frame_length);
#else
    /* 申请在静态区,只创建一次,避免反复申请 */
    static uint16_t frame_error_count = 0; /* 错误帧计数 */

    if (protocol_head_check(message_handle, rx_buffer))
    {
        /* 通过帧头校验 */
        if (crc16_check(rx_buffer, frame_length))
        {
            /* 通过crc16校验 */
#if !defined __EASY_PRINT_TEST && defined __COMMUCATION_PROTOCOL_TEST_DATA
            LOGWARNING("Pass crc16 Check.\r\n");
#endif //!__EASY_PRINT_TEST && __COMMUCATION_PROTOCOL_TEST_DATA
            /* 整帧数据通过crc16检验 */
            /* 获取16位寄存器的值 */
            message_handle->flags_register = (rx_buffer[7] << 8) | rx_buffer[6];
            memcpy(message_handle->float_data, rx_buffer + 8, message_handle->data_length - 2);
#ifdef __COMMUCATION_PROTOCOL_TEST_DATA
            /* cmd_id解码测试 */
            // 正确编码:00 10
            uint8_t cmd_id_high = message_handle->cmd_id >> 8;
            uint8_t cmd_id_low = message_handle->cmd_id;
            rtt_str_to_hex(&cmd_id_high, 1);
            rtt_str_to_hex(&cmd_id_low, 1);
            /* float数据解码测试 */
            // 正确编码:EB 56 B7 3F AE 6E 67 43 A4 70 15 41 2A E9 F6 42
            rtt_str_to_hex(message_handle->float_data, message_handle->data_length - 2);
            /* 测试数据由函数generate_test_data生成 */
            // A5 12 00 74 10 00 55 FE EB 56 B7 3F AE 6E 67 43 A4 70 15 41 2A E9 F6 42 75 71
            // 浮点数据:1.43234/231.43234/9.34/123.4554
#endif //__COMMUCATION_PROTOCOL_TEST_DATA
        }
#if !defined __EASY_PRINT_TEST && defined __COMMUCATION_PROTOCOL_TEST_DATA
        else
        {
            LOGWARNING("Can not Pass crc16 Check.\r\n");
        }
#endif //!__EASY_PRINT_TEST && __COMMUCATION_PROTOCOL_TEST_DATA
    }
    else
    {
        frame_error_count++;
        LOGWARNING("Receive Error Frame, accumulate [%d] times.\r\n", frame_error_count);
    }
#endif //__EASY_PRINT_TEST
}
#ifndef __EASY_PRINT_TEST
#ifdef __COMMUCATION_PROTOCOL_TEST_DATA
/**
 * @description: 通过LOG的形式生成测试数据帧,该函数不允许在RTOS中调用
 * @return {*}
 */
void generate_test_data(uint16_t cmd_id,         /* 命令码 */
                        uint16_t flags_register) /* 16位寄存器 */
{
    uint8_t *tx_buffer = (uint8_t *)malloc(sizeof(uint8_t) * PROTOCOL_FRAME_LENGTH_MAX); /* 待发送的数据帧 */
    uint8_t float_length = 4;                                                            /* 发送4个float数据 */
    float *float_data = (float *)malloc(sizeof(float) * float_length);                   /* 待发送的float数据 */
    float_data[0] = 1.43234;
    float_data[1] = 231.43234;
    float_data[2] = 9.34;
    float_data[3] = 123.4554;
    uint16_t frame_tail;
    uint16_t data_length;  /* 数据段长度:寄存器值 + float数据 */
    uint16_t frame_length; /* 数据帧长度,用于打印 */

    data_length = float_length * 4 + 2; /* 一个float数据是4字节,转换成uint8_t类型需要乘4 */
    /* 帧头部分 */
    /* sof */
    tx_buffer[0] = PROTOCOL_HEAD_CMD;
    /* data_length */
    tx_buffer[1] = data_length;      /* 先发低字节 */
    tx_buffer[2] = data_length >> 8; /* 后发高字节 */
    /* crc_check */
    tx_buffer[3] = crc_8(tx_buffer, 3); /* 生成帧头的8位宽crc */
    /* 命令码部分 */
    /* cmd_id */
    tx_buffer[4] = cmd_id;      /* 先发低字节 */
    tx_buffer[5] = cmd_id >> 8; /* 后发高字节 */
    /* 数据段部分 */
    /* flags_register */
    tx_buffer[6] = flags_register;      /* 先发低字节 */
    tx_buffer[7] = flags_register >> 8; /* 后发高字节 */
    /* data */
    for (uint8_t i = 0; i < 4 * float_length; i++)
    {
        /* 从第9字节开始发送数据 */
        /* 一个float需要执行4次循环才能发送完毕 */
        /* (uint8_t *)(&float_data[i / 4])表示将第i / 4个float类型的指针强制转换成uint8_t类型的指针 */
        /* [i % 4]表示转换之后按照1字节的大小将float数据取出,每个float取4次 */
        tx_buffer[i + 8] = ((uint8_t *)(&float_data[i / 4]))[i % 4];
    }

    /* 整包校验部分 */
    /* frame_tail */
    frame_tail = crc_16(tx_buffer, data_length + 6);
    tx_buffer[data_length + 6] = frame_tail;      /* 先发低字节 */
    tx_buffer[data_length + 7] = frame_tail >> 8; /* 后发高字节 */

    /* 计算数据帧长度 */
    frame_length = data_length + 8;
    /* 循环打印四次 */
    for (uint8_t i = 0; i < 4; i++)
    {
        rtt_str_to_hex(tx_buffer, frame_length);
    }
}
#endif //__COMMUCATION_PROTOCOL_TEST_DATA
#endif //!__EASY_PRINT_TEST
/**
 * @description: 将数据打包成一帧,随后通过待选串口进行发送
 * @return {*}
 */
// static void commucation_message_send(uint8_t idx,               /* 执行发送操作的串口实例的id */
//                                      uint16_t flags_register,   /* 16bit的寄存器 */
//                                      float *tx_data,            /* 待发送的float数据 */
//                                      uint8_t float_length,      /* float数据的长度 */
//                                      uint8_t *tx_buffer,        /* 待发送数据缓存buffer */
//                                      uint16_t *tx_frame_length) /* 待发送的数据帧长度 */
// {
//     /* 待补充 */
// }
/**
 * @description: 上位机通信任务
 * @param {void} *pvParameters
 * @return {*}
 */
void commucation_task(void *pvParameters)
{
    /* 任务配置区 */
    /* 创建通信协议解码结构体 */
    /* 注意,增大该任务的栈空间 */
    message_handle = (Commucation_ProtocolHandle)pvPortMalloc(sizeof(Commucation_ProtocolDef));
    /* 创建串口实例,负责接受上位机的消息 */
    /* 串口实例本质上靠DMA中断处理,因此不属于任务体系,可以考虑作为硬件系统任务处理 */
    commucation_uart_handle = Y_uart_create_instance(IDX_OF_UART_DEVICE_3, COMMUCATION_PROTOCOL_FRAME_SIZE, &huart3, commucation_message_decode);
#ifdef TEST_LED_RGB
    /* LED测试 */
    commucation_led_instance_handle = Y_led_creat_instance(0, Turquoise1);
    /* 注意更改参数 */
    led_start(commucation_led_instance_handle, 0, 0, 0);
#endif // TEST_LED_RGB
    uint32_t heart_count = 0;
    TickType_t xLastWakeTime = 0;
    /* 创建1ms的tick计数值 */
    const TickType_t xDelay1ms = pdMS_TO_TICKS(1);
    xLastWakeTime = xTaskGetTickCount();
    while (1)
    {
        /* 每隔5s打印一次心跳包 */
        LOGINFO("Heart %d\n\r", heart_count++);
        xTaskDelayUntil(&xLastWakeTime, xDelay1ms * 1000);
    }
}
