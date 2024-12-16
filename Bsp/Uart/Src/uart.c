/*
 * @Author: Hengyang Jiang
 * @Date: 2024-12-12 21:33:51
 * @LastEditors: Hengyang Jiang
 * @LastEditTime: 2024-12-16 14:04:48
 * @Description: uart.c
 *
 * Copyright (c) 2024 by https://github.com/Nolan-Jon, All Rights Reserved.
 */
#include "FreeRTOS.h" /* 这里并不需要FreeRTOS.h这个文件,但是task.h必须在FreeRTOS后面 */
#include "task.h"
#include "rtt.h"
#include "uart.h"
#include "string.h"
#include "portable.h"
static UART_InstanceHandle uart_instance_array[DEVICE_UART_NUM] = {NULL}; /* 挂载串口实例句柄 */
/**
 * @description: 成功创建串口实例后,会启动串口服务,主要用于DMA接受
 * @param {UART_InstanceHandle} uart_instance_handle
 * @return {*}
 */
static void uart_service_start(UART_InstanceHandle uart_instance_handle)
{

    /* 注意,虽然使用DMA进行数据传输,但是串口空闲中断依赖UART全局中断,注意开启UART全局中断*/
    HAL_UARTEx_ReceiveToIdle_DMA(uart_instance_handle->uartHandle, uart_instance_handle->recv_buffer, uart_instance_handle->recv_buffer_size);
    /* 该函数会调用UART_Start_Receive_DMA开启串口的DMA接收,该函数会配置三种回调函数,同时开启DMA中断 */
    /* DMA传输完成回调函数、DMA半传输完成回调函数、DMA传输错误回调函数 */
    /* 这里只需要DMA传输完成回调函数UART_DMAReceiveCplt */
    /* 由于设置了标志HAL_UART_RECEPTION_TOIDLE */
    /* 因此传输完成中断和IDLE中断都作为Rx Event,即调用HAL_UARTEx_RxEventCallback函数 */
    /* 实际上半传输中断也会调用HAL_UARTEx_RxEventCallback,因此需要关闭半传输中断(当然也可以进行标志位判断,但关闭中断能提高系统实时性) */
    __HAL_DMA_DISABLE_IT(uart_instance_handle->uartHandle->hdmarx, DMA_IT_HT);
}
/**
 * @description: 发生DMA传输完成中断和IDLE中断后,会调用该回调函数,注意每个串口设备都会调用该函数,因此内部要做区分
 * @param {UART_HandleTypeDef} *huart
 * @param {uint16_t} Size 本次接收到的数据包数量:接收到的一帧数据的大小
 * @return {*}
 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    /* 寻找发生中断的串口 */
    uint8_t idx = 0;
    for (; idx < DEVICE_UART_NUM; idx++)
    {
        /* 找到发生中断的串口对应的串口实例 */
        if (huart == uart_instance_array[idx]->uartHandle)
        {
            break;
        }
    }
    /* 调用内部解析函数,不同的串口实例有不同的解析回调函数 */
    if (uart_instance_array[idx]->uart_recv_decode_callback != NULL)
    {
        /* 可以考虑将size作为解析函数的参数 */
        uart_instance_array[idx]->uart_recv_decode_callback(uart_instance_array[idx]->recv_buffer, Size);
        /* 清除接收缓冲区中的数据 */
        memset(uart_instance_array[idx]->recv_buffer, 0, Size);
    }
    /* 串口没有解析回调函数,但这并不影响系统 */
    else
    {
        LOGERROR("[uart_deocde]UART %d Has No Callback!", idx);
    }
    /* 完成一次接收后,需要重新注册串口的DMA接收服务 */
    uart_service_start(uart_instance_array[idx]);
}
/**
 * @description: 串口传输错误回调函数,常见错误:奇偶校验错误/溢出/栈异常,重启串口DMA服务或执行其他操作
 * @param {UART_HandleTypeDef} *huart
 * @return {*}
 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    /* 找到出错误的串口 */
    uint8_t idx = 0;
    for (; idx < DEVICE_UART_NUM; idx++)
    {
        /* 找到发生错误的串口实例 */
        if (huart == uart_instance_array[idx]->uartHandle)
        {
            break;
        }
    }
    LOGERROR("[uart_rx]UART %d Has an Error!", idx);
    uart_service_start(uart_instance_array[idx]);
}
/**
 * @description: 应用层函数,创建串口实例
 * @return {*}
 */
UART_InstanceHandle Y_uart_create_instance(uint8_t instance_num,                       /* 串口实例编号,支持0 ~ 1 */
                                           uint16_t recv_buffer_size,                  /* 串口接受一包数据大小 */
                                           UART_HandleTypeDef *uartHandle,             /* 串口实例对应的设备句柄 */
                                           uart_recv_decode uart_recv_decode_callback) /* 解析回调函数 */
{
    /* 检测串口编号是否非法 */
    if (instance_num > DEVICE_UART_NUM)
    {
        while (1)
        {
            LOGERROR("[uart_create]The UART Number Is Illegal!");
        }
    }
    /* 检测是否已经注册过 */
    if (uart_instance_array[instance_num] == NULL)
    {
        while (1)
        {
            LOGERROR("[uart_create]UART Instance Already Created!");
        }
    }
    /* 进入临界区,确保创建串口实例的过程是线程安全的,不要在检测串口是否合法之前进入临界区,这样会影响其他任务的调度,使整个系统卡死 */
    taskENTER_CRITICAL();
    /* 使用线程安全的内存分配函数 */
    UART_InstanceHandle uart_instance_handle = (UART_InstanceHandle)pvPortMalloc(sizeof(UART_InstanceDef));

    uart_instance_handle->uartHandle = uartHandle;
    uart_instance_handle->idx = instance_num;
    uart_instance_handle->recv_buffer_size = recv_buffer_size;
    /* 不需要分配BUFFER,每个结构体在创建的时候自带BUFFER */
    uart_instance_handle->uart_recv_decode_callback = uart_recv_decode_callback;

    /* 将创建好的串口实例挂载到串口实例队列中 */
    uart_instance_array[instance_num] = uart_instance_handle;

    /* 注册串口服务 */
    uart_service_start(uart_instance_handle);
    /* 退出临界区 */
    taskEXIT_CRITICAL();

    return uart_instance_handle;
}
