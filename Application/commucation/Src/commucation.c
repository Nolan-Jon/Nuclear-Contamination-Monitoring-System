/*
 * @Author: Hengyang Jiang
 * @Date: 2024-12-13 14:38:45
 * @LastEditors: Hengyang Jiang
 * @LastEditTime: 2024-12-13 17:27:58
 * @Description: commucation.c 上位机通信文件
 * 
 * Copyright (c) 2024 by https://github.com/Nolan-Jon, All Rights Reserved. 
 */

#include "commucation.h"
#include "FreeRTOS.h"
#include "task.h"
#include "uart.h"
#include "usart.h"
#include "rtt.h"
TaskHandle_t commucation_task_handle;
UART_InstanceHandle commucation_uart_handle;
/**
 * @description: 解码函数
 * @param {uint8_t} *buffer
 * @param {uint16_t} pack_num
 * @return {*}
 */
static void commucation_uart_decode(uint8_t *buffer, uint16_t pack_num)
{
    /* 现在先简单配置一下 */
    LOGWARNING("%s\n\r", buffer);
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
    commucation_uart_handle = Y_uart_create_instance(IDX_OF_UART_DEVICE_3, sizeof(uint8_t), &huart3, commucation_uart_decode);
    uint32_t heart_count = 0;
    TickType_t xLastWakeTime = 0;
    const TickType_t xDelay1ms = pdMS_TO_TICKS(1);
    xLastWakeTime = xTaskGetTickCount();
    while (1)
    {
        /* 每隔5s打印一次心跳包 */
        LOGINFO("Heart %d\n\r", heart_count++);
        xTaskDelayUntil(&xLastWakeTime, xDelay1ms * 1000);
    }
}
