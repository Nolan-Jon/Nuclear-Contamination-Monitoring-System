/*
 * @Author: Hengyang Jiang
 * @Date: 2024-12-13 13:38:48
 * @LastEditors: Hengyang Jiang
 * @LastEditTime: 2024-12-20 14:19:03
 * @Description: freertos_start.c
 *
 * Copyright (c) 2024 by https://github.com/Nolan-Jon, All Rights Reserved.
 */
#include "freertos_start.h"
#include "commucation.h"
#include "chassis.h"
#include "rtt.h"
#include "daemon.h"
TaskHandle_t start_task_handle;
extern TaskHandle_t commucation_task_handle;
extern TaskHandle_t chassis_task_handle;
TimerHandle_t daemon_timer_handle;
void start_task(void *pvParameters)
{
    /* 启动任务运行的时候调度器已经启动,由于需要创建其他任务,为了避免错误,需要设置临界区 */
    LOGINFO("Enter start task\r\n");
    /* 进入临界区 */
    taskENTER_CRITICAL();
    /* 创建应用级任务 */
    xTaskCreate(commucation_task, "com", COMMUCATION_TASK_STACK, NULL, COMMUCATION_TASK_PRIORITY, &commucation_task_handle);
    xTaskCreate(chassis_task, "chassis", CHASSIS_TASK_STACK, NULL, CHASSIS_TASK_PRIORITY, &chassis_task_handle);
    /* 创建软件定时器Daemon */
    daemon_timer_handle = xTimerCreate("Daemon", DAEMON_TIMER_PERIOD_TICKS, pdTRUE, (void *)1, deamon_timer_callback); /* pdTRUE循环执行,pdFALSE单次执行*/
    /* 由于在调度器启动之前开启Timer,函数第二个参数将被忽略 */
    xTimerStart(daemon_timer_handle, 0);
    /* 删除启动任务自身 */
    vTaskDelete(NULL);
    taskEXIT_CRITICAL();
    /* 退出临界区 */
}
/**
 * @description: 启动freertos,内部创建启动任务
 * @return {*}
 */
void freertos_start(void)
{
    BaseType_t res = 0;
    /* 创建启动任务 */
    LOGINFO("start freertos\r\n");
    res = xTaskCreate(start_task, "start_task", START_TASK_STACK, NULL, START_TASK_PRIORITY, &start_task_handle);
    if (res == pdPASS)
    {
        LOGINFO("start task create successful.\r\n");
    }
    else
    {
        LOGINFO("start task create fail.\r\n");
    }
    /* 启动调度器 */
    vTaskStartScheduler();
}
