/*
 * @Author: Hengyang Jiang
 * @Date: 2024-12-13 14:38:45
 * @LastEditors: Hengyang Jiang
 * @LastEditTime: 2024-12-16 15:41:34
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
TaskHandle_t commucation_task_handle;
UART_InstanceHandle commucation_uart_handle;
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
    if ((message == NULL) || (size <= 2))
    {
        return FALSE; /* false */
    }
    ucExpected = crc_8(message, size);
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
    uint8_t ucExpected = 0; /* 期望值 */
    if ((message == NULL) || (size <= 2))
    {
        return FALSE; /* false */
    }
    ucExpected = crc_16(message, size);
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
        message->sof = PROTOCOL_HEAD_CMD;
        /* 检验帧头crc8 */
        /* 帧头4字节 */
        if (crc8_check(rx_buffer, 4))
        {
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
static void commucation_message_decode(uint8_t *buffer, uint16_t frame_length)
{
    /* 现在先简单配置一下 */
    LOGWARNING("%s\n\r", buffer);
    
}
/**
 * @description: 将数据打包成一帧,随后通过待选串口进行发送
 * @return {*}
 */
static void commucation_message_send(uint8_t idx,               /* 执行发送操作的串口实例的id */
                                     uint16_t flags_register,   /* 16bit的寄存器 */
                                     float *tx_data,            /* 待发送的float数据 */
                                     uint8_t float_length,      /* float数据的长度 */
                                     uint8_t *tx_buffer,        /* 待发送数据缓存buffer */
                                     uint16_t *tx_frame_length) /* 待发送的数据帧长度 */
{
    /* 待补充 */
}
/**
 * @description: 上位机通信任务
 * @param {void} *pvParameters
 * @return {*}
 */
void commucation_task(void *pvParameters)
{
    /* 任务配置区 */
    /* 创建串口实例,负责接受上位机的消息 */
    /* 串口实例本质上靠DMA中断处理,因此不属于任务体系,可以考虑作为硬件系统任务处理 */
    commucation_uart_handle = Y_uart_create_instance(IDX_OF_UART_DEVICE_3, COMMUCATION_PROTOCOL_FRAME_SIZE, &huart3, commucation_message_decode);
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
