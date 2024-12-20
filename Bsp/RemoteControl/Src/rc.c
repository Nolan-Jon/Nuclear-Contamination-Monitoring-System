/*
 * @Author: Hengyang Jiang
 * @Date: 2024-12-20 12:22:17
 * @LastEditors: Hengyang Jiang
 * @LastEditTime: 2024-12-20 18:17:25
 * @Description: rc.c
 *               使用的遥控器是云卓T10,接收器协议是SBUS
 *               STM32配置如下:
 *               a.波特率:100KBit/s
 *               b.奇偶校验:偶校验
 *               c.数据位:9bit
 *               d.停止位:1bit
 *               c.无硬件流控
 *               e.串口仅做接收模式
 *               f.配置DMA通道,开启串口全局中断和DMA中断
 *
 * Copyright (c) 2024 by https://github.com/Nolan-Jon, All Rights Reserved.
 */
#include "FreeRTOS.h"
#include "task.h"
#include "portable.h"
#include "rc.h"
#include "stdlib.h"
#include "rtt.h"
#include "usart.h"
static uint8_t REMOTERC_INSTANCE_COUNT = 0; /* 用于对遥控器实例进行计数,最多支持一个遥控器实例 */
extern RemoteCR_InstanceHandle remote_control_instance_handle;
/**
 * @description: 解码通道值
 * @param {uint8_t} *frame
 * @return {*}
 */
static void channel_decode(uint8_t *frame)
{
    /* 帧大小要求25字节 */
    /* 该函数被调用的时候不会发生空指针的情况,遥控器实例内存在串口创建前被分配 */
    /* 安全的操作 */
    /* 获取通道值 */
    remote_control_instance_handle->CH_1 = ((int16_t)frame[1] >> 0 | ((int16_t)frame[2] << 8)) & 0x07FF;
    remote_control_instance_handle->CH_2 = ((int16_t)frame[2] >> 3 | ((int16_t)frame[3] << 5)) & 0x07FF;
    remote_control_instance_handle->CH_3 = ((int16_t)frame[3] >> 6 | ((int16_t)frame[4] << 2) | (int16_t)frame[5] << 10) & 0x07FF;
    remote_control_instance_handle->CH_4 = ((int16_t)frame[5] >> 1 | ((int16_t)frame[6] << 7)) & 0x07FF;
    remote_control_instance_handle->CH_5 = ((int16_t)frame[6] >> 4 | ((int16_t)frame[7] << 4)) & 0x07FF;
    remote_control_instance_handle->CH_6 = ((int16_t)frame[7] >> 7 | ((int16_t)frame[8] << 1) | (int16_t)frame[9] << 9) & 0x07FF;
    remote_control_instance_handle->CH_7 = ((int16_t)frame[9] >> 2 | ((int16_t)frame[10] << 6)) & 0x07FF;
    remote_control_instance_handle->CH_8 = ((int16_t)frame[10] >> 5 | ((int16_t)frame[11] << 3)) & 0x07FF;
    remote_control_instance_handle->CH_9 = ((int16_t)frame[12] << 0 | ((int16_t)frame[13] << 8)) & 0x07FF;
    remote_control_instance_handle->CH_10 = ((int16_t)frame[13] >> 3 | ((int16_t)frame[14] << 5)) & 0x07FF;
    /* 获取遥控器状态 */
    remote_control_instance_handle->state_flag = frame[23];
}
/**
 * @description: 遥控器解码回调函数
 * @param {uint8_t} *rx_buffer
 * @param {uint16_t} frame_length
 * @return {*}
 */
static void remote_control_sbus_decode_callback(uint8_t *rx_buffer, uint16_t frame_length)
{
    /* 解码通道值 */
    channel_decode(rx_buffer);
    /* 打印通道值:测试 */
    LOGINFO("CH1:[%d]---CH2:[%d]---CH3:[%d]---CH4:[%d]---CH5:[%d]---CH6:[%d]---CH7:[%d]---CH8:[%d]---CH9:[%d]---CH10:[%d]\r\n",
            remote_control_instance_handle->CH_1, 
            remote_control_instance_handle->CH_2, 
            remote_control_instance_handle->CH_3, 
            remote_control_instance_handle->CH_4,
            remote_control_instance_handle->CH_5, 
            remote_control_instance_handle->CH_6, 
            remote_control_instance_handle->CH_7, 
            remote_control_instance_handle->CH_8, 
            remote_control_instance_handle->CH_9, 
            remote_control_instance_handle->CH_10);
}
/**
 * @description: 遥控器离线回调函数
 * @param {void} *daemon_instance_handle
 * @return {*}
 */
static void remote_control_lost_callback(void *daemon_instance_handle)
{
    /* 拿取对应的Daemon实例句柄 */
    Daemon_InstanceHandle h_daemon = (Daemon_InstanceHandle)daemon_instance_handle;
    /* 拿取对应的遥控器实例句柄 */
    RemoteCR_InstanceHandle h_remote = (RemoteCR_InstanceHandle)(h_daemon->owner_instance_handle);
    /* 先执行喂狗操作 */
    h_daemon->count = h_daemon->reload_count; /* 重新从重载值向下计数 */
    /* 遥控器失能,直接返回就好 */
    if (h_remote->enable_flag == 0)
    {
        return;
    }
    /* 遥控器实例使能状态,离线 */
    if (h_remote->state_flag == 0)
    {
        /* 蜂鸣器操作 */
        //......发声三次
        /* 离线操作 */
        //......清空接收区数据/重启串口服务/打印LOG
        // LOGWARNING("[daemon_remote]RemoteControl Lost Service.\r\n");
    }
    return;
}
/**
 * @description: 创建遥控器实例
 * @return {*}
 */
RemoteCR_InstanceHandle Y_rc_create_instance(void)
{
    /* 已经创建遥控器实例 */
    if (REMOTERC_INSTANCE_COUNT > 0)
    {
        while (1)
        {
            LOGERROR("[remoterc_create]RemoteControl Instance Already Created!");
        }
    }
    /* 进入临界区进行创建操作:注意,由于是在临界区中创建,因此需要确保串口的编号正确,否则会使系统卡死 */
    taskENTER_CRITICAL();
    RemoteCR_InstanceHandle remote_rc_instance_handle = (RemoteCR_InstanceHandle)pvPortMalloc(sizeof(RemoteCR_InstanceDef));
    if (remote_rc_instance_handle == NULL)
    {
        LOGERROR("[remoterc_create]RemoteControl Instance Create Failed,Stack Overflow!\r\n");
        /* 退出临界区 */
        taskEXIT_CRITICAL();
        return NULL;
    }
    remote_rc_instance_handle->enable_flag = 0; /* 初始为失能 */
    remote_rc_instance_handle->state_flag = 0;  /* 初始为离线 */
    remote_rc_instance_handle->rc_uart_instance_handle = Y_uart_create_instance(IDX_OF_UART_DEVICE_5,
                                                                                REMOTECR_PROTOCOL_FRAME_SIZE,
                                                                                &huart5, /* 使用串口5 */
                                                                                remote_control_sbus_decode_callback);

    /* 注册遥控器的守护对象,用于判断是否离线 */
    remote_rc_instance_handle->rc_daemon_instance = Y_daemon_create_instance((void *)remote_rc_instance_handle,
                                                                             DAEMON_OWNER_TYPE_RC,
                                                                             0,
                                                                             DAEMON_RELOD_RC,
                                                                             remote_control_lost_callback);

    /* 退出临界区 */
    taskEXIT_CRITICAL();

    /* 返回实例对象 */
    return remote_rc_instance_handle;
}
