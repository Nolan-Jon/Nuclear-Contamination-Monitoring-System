/*
 * @Author: Hengyang Jiang
 * @Date: 2024-12-18 10:39:36
 * @LastEditors: Hengyang Jiang
 * @LastEditTime: 2024-12-20 13:52:02
 * @Description: daemon.c
 *               该文件实现守护实例的创建操作
 *               注意事项:通过实现一个软件定时器作为守护进程,守护进程维护这些守护实例
 *               守护进程对应的软件定时器的执行周期是50ms(测试中ing...)
 *
 * Copyright (c) 2024 by https://github.com/Nolan-Jon, All Rights Reserved.
 */

#include "daemon.h"
#include "stdlib.h"
#include "rtt.h"
#include "led.h"
#include "portable.h"

/* 创建守护实例的挂载队列 */
static Daemon_InstanceHandle daemon_instance_array[INSTANCE_DAEMON_CNT] = {NULL};
static uint8_t daemon_instance_count = 0; /* 当前的daemon实例数量 */
/**
 * @description: 创建守护实例进程
 * @param {void} *owner_instance_handle
 * @param {uint8_t} owner_type
 * @param {uint8_t} owner_id:从0开始
 * @param {uint16_t} reload_count
 * @param {daemon_timeout_callback} callback
 * @return {*}
 */
Daemon_InstanceHandle Y_daemon_create_instance(void *owner_instance_handle, uint8_t owner_type, uint8_t owner_id, uint16_t reload_count, daemon_timeout_callback callback)
{
    /* 不需要对编号进行检测 */
    /* 进入临界区,确保创建串口实例的过程是线程安全的,不要在检测串口是否合法之前进入临界区,这样会影响其他任务的调度,使整个系统卡死 */
    taskENTER_CRITICAL();
    Daemon_InstanceHandle daemon_instance_handle = (Daemon_InstanceHandle)pvPortMalloc(sizeof(Daemon_InstanceDef));
    if (daemon_instance_handle == NULL)
    {
        LOGERROR("[daemon_create]Daemon Instance Create Failed,Stack Overflow!\r\n");
        /* 退出临界区 */
        taskEXIT_CRITICAL();
        return NULL;
    }
    daemon_instance_handle->owner_type = owner_type;
    daemon_instance_handle->owner_idx = owner_id;
    daemon_instance_handle->reload_count = reload_count;
    daemon_instance_handle->count = reload_count;
    daemon_instance_handle->callback = callback;
    /* 使用的时候需要进行强制类型转换 */
    daemon_instance_handle->owner_instance_handle = owner_instance_handle;

    /* 挂载到队列 */
    daemon_instance_array[daemon_instance_count++] = daemon_instance_handle;

    /* 退出临界区 */
    taskEXIT_CRITICAL();

    return daemon_instance_handle;
}
/**
 * @description: 软件定时器Deamon的回调函数
 * @param {TimerHandle_t} xTimer
 * @return {*}
 */
void deamon_timer_callback(TimerHandle_t xTimer)
{
    /* 遍历deamon队列,执行deamon实例的回调函数 */
    taskENTER_CRITICAL();
    /* 调用该函数前需要使用ws2812b_config_color函数配置RGB灯带 */
    /* 最好将配置函数放到ws2812b初始化函数中 */
    /* 后续可以设计运行中更改颜色的接口 */
    ws2812b_breath();
    taskEXIT_CRITICAL();
    for (uint8_t i = 0; i < daemon_instance_count; i++)
    {
        /* 判断是否计数到重载值:0触发 */
        if (daemon_instance_array[i]->count == 0)
        {
            /* 计数达到重载值 */
            /* 是否需要进入临界区 */
            daemon_instance_array[i]->callback((void *)daemon_instance_array[i]);
        }
        else
        {
            /* 递减计数值 */
            daemon_instance_array[i]->count--;
        }
    }
}
