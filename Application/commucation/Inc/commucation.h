/*
 * @Author: Hengyang Jiang
 * @Date: 2024-12-13 14:38:32
 * @LastEditors: Hengyang Jiang
 * @LastEditTime: 2024-12-13 15:20:43
 * @Description: commucation.h
 *
 * Copyright (c) 2024 by https://github.com/Nolan-Jon, All Rights Reserved.
 */
#ifndef __COMMUCATION__H__
#define __COMMUCATION__H__
#include "FreeRTOSConfig.h"
#define COMMUCATION_TASK_STACK (configMINIMAL_STACK_SIZE * 2)
#define COMMUCATION_TASK_PRIORITY (configMAX_PRIORITIES - 2)

void commucation_task(void *pvParameters);
#endif //!__COMMUCATION__H__
