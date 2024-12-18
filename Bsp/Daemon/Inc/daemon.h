/*
 * @Author: Hengyang Jiang
 * @Date: 2024-12-18 10:40:04
 * @LastEditors: Hengyang Jiang
 * @LastEditTime: 2024-12-18 15:10:10
 * @Description: daemon.h
 *
 * Copyright (c) 2024 by https://github.com/Nolan-Jon, All Rights Reserved.
 */
#ifndef __DAEMON__H__
#define __DAEMON__H__
#include "FreeRTOS.h" /* 这里并不需要FreeRTOS.h这个文件,但是task.h必须在FreeRTOS后面 */
#include "task.h"
#include "stdint.h"
#include "timers.h"
#define INSTANCE_DAEMON_CNT 10   /* daemon实例最大数量 */
#define DAEMON_OWNER_TYPE_LED 1  /* LED类型 */
#define DAEMON_OWNER_TYPE_BEEP 2 /* BEEP类型 */
#define DAEMON_OWNER_TYPE_RC 3   /* Remote Control类型 */

/* 重载值选择 */
#define DAEMON_RELOAD_LED   4 /* LED类型的守护实例重载值:守护进程每50ms执行一次,而LED的回调函数要求200ms调用一次 */

typedef void (*daemon_timeout_callback)(void *);
typedef struct
{
    /* data */
    uint8_t owner_type;               /* 拥有者的类型 */
    uint8_t owner_idx;                /* 拥有者的编号 */
    uint16_t reload_count;            /* daemon维护的计数重载值:计数值从重载值减到0会触发超时回调函数 */
    uint16_t count;                   /* 计数值:从上到下计数 */
    daemon_timeout_callback callback; /* 守护超时回调函数 */
    void *owner_instance_handle;      /* 拥有者的实例句柄(需要强制转换成所需类型) */
} Daemon_InstanceDef;
typedef Daemon_InstanceDef *Daemon_InstanceHandle;

Daemon_InstanceHandle Y_daemon_create_instance(void *owner_instance_handle, uint8_t owner_type, uint8_t owner_id, uint16_t reload_count, daemon_timeout_callback callback);
void deamon_timer_callback(TimerHandle_t xTimer);
#endif //!__DAEMON__H__
