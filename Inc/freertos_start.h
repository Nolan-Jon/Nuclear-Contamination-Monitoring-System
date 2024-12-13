/*
 * @Author: Hengyang Jiang
 * @Date: 2024-12-13 13:39:11
 * @LastEditors: Hengyang Jiang
 * @LastEditTime: 2024-12-13 17:13:12
 * @Description: freertos_start.h
 *
 * Copyright (c) 2024 by https://github.com/Nolan-Jon, All Rights Reserved.
 */
#ifndef __FREERTOS_START__H__
#define __FREERTOS_START__H__
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#define START_TASK_STACK (configMINIMAL_STACK_SIZE * 2) /* 栈大小:configMINIMAL_STACK_SIZE x 2 x 4Byte*/
#define START_TASK_PRIORITY (configMAX_PRIORITIES - 1)        /* 最大优先级 */
void freertos_start(void);
#endif //!__FREERTOS_START__H__
