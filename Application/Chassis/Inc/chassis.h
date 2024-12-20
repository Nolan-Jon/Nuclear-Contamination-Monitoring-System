/*
 * @Author: Hengyang Jiang
 * @Date: 2024-12-20 14:01:41
 * @LastEditors: Hengyang Jiang
 * @LastEditTime: 2024-12-20 14:49:43
 * @Description: chassis.h
 *
 * Copyright (c) 2024 by https://github.com/Nolan-Jon, All Rights Reserved.
 */
#ifndef __CHASSIS__H__
#define __CHASSIS__H__
#include "stdint.h"
#include "rc.h"
#define CHASSIS_TASK_STACK (configMINIMAL_STACK_SIZE * 3)
#define CHASSIS_TASK_PRIORITY (configMAX_PRIORITIES - 3) /* 当前优先级为2 */
void chassis_task(void *pvParameters);
#endif //!__CHASSIS__H__
