/*
 * @Author: Hengyang Jiang
 * @Date: 2024-12-20 14:01:26
 * @LastEditors: Hengyang Jiang
 * @LastEditTime: 2024-12-20 14:10:50
 * @Description: chassis.c
 *               该文件用于chassis任务,涉及的底层文件包括:rc.c/......
 *               当前任务执行频率为200Hz:周期5ms,可以尝试将周期调整到10ms
 *
 * Copyright (c) 2024 by https://github.com/Nolan-Jon, All Rights Reserved.
 */
#include "FreeRTOS.h"
#include "task.h"
#include "portable.h"
#include "chassis.h"
#include "rtt.h"
TaskHandle_t chassis_task_handle;
RemoteCR_InstanceHandle remote_control_instance_handle;
/**
 * @description: 底盘任务
 * @param {void} *pvParameters
 * @return {*}
 */
void chassis_task(void *pvParameters)
{
    /* 任务配置区 */
    /* 创建遥控器实例 */
    remote_control_instance_handle = Y_rc_create_instance();
    if (remote_control_instance_handle == NULL)
    {
        while (1)
        {
            LOGERROR("[chassis_task]produces a null pointer!\r\n");
        }
    }
    remote_control_instance_handle->enable_flag = 1; /* 使能遥控器 */
    TickType_t xLastWakeTime = 0;
    /* 创建1ms的tick计数值 */
    const TickType_t xDelay1ms = pdMS_TO_TICKS(1);
    xLastWakeTime = xTaskGetTickCount();
    while (1)
    {
        xTaskDelayUntil(&xLastWakeTime, xDelay1ms * 5);
    }
}
