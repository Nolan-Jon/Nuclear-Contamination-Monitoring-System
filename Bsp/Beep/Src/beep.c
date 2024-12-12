/*
 * @Author: Hengyang Jiang
 * @Date: 2024-12-12 18:57:27
 * @LastEditors: Hengyang Jiang
 * @LastEditTime: 2024-12-12 19:38:33
 * @Description: beep.c
 * 
 * Copyright (c) 2024 by https://github.com/Nolan-Jon, All Rights Reserved. 
 */
#include "main.h"
#include "beep.h"
/**
 * @description: 初始化无源蜂鸣器,注意开发板上的PD11不是定时器通道
 * @return {*}
 */
void beep_init(void)
{
    HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_RESET);
}
/**
 * @description: 
 * @return {*}
 */
void beep_start(void)
{
    HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_SET);
}
/**
 * @description: 
 * @return {*}
 */
void beep_stop(void)
{
    HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_RESET);
}