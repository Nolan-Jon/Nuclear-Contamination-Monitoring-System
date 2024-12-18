/*
 * @Author: Hengyang Jiang
 * @Date: 2024-12-17 14:54:44
 * @LastEditors: Hengyang Jiang
 * @LastEditTime: 2024-12-18 10:34:18
 * @Description: led.h
 *               RGB颜色参考:https://tool.oschina.net/commons?type=3
 *
 * Copyright (c) 2024 by https://github.com/Nolan-Jon, All Rights Reserved.
 */
#ifndef __LED__H__
#define __LED__H__
#include "stdint.h"
#include "gpio.h"
#include "tim.h"

/* Color Define */
#define Turquoise1 ((uint32_t)0x00F5FF)
#define DarkGreen ((uint32_t)0x006400)
#define DarkGoldenrod1 ((uint32_t)0xFFB90F)
#define Firebrick ((uint32_t)0xB22222)
#define MediumPurple1 ((uint32_t)0xAB82FF)

#define INSTANCE_LED_NUM 5 /* 最多允许创建5个Led实例对象 */
#define LED_ENABLE 1       /* LED实例使能 */
#define LED_DISABLE 0      /* LED实例失能 */
#define LED_STATE_RUN 1    /* LED状态为运行 */
#define LED_STATE_STOP 2   /* LED状态为停止 */

#define TIM_R_CHANNEL TIM_CHANNEL_3
#define TIM_G_CHANNEL TIM_CHANNEL_2
#define TIM_B_CHANNEL TIM_CHANNEL_4
/* RGB结构体定义 */
typedef struct
{
    /* data */
    uint8_t idx;            /* 编号:范围0 ~ 4 */
    uint8_t enable_flag;    /* RGB实例对象使能或禁止 */
    uint8_t state_flag;     /* RGB实例对象当前的状态 */
    uint16_t light_time;    /* 总点亮时间:单位200ms */
    uint16_t stop_time;     /* 熄灭时间:单位200ms */
    uint16_t all_flash_cnt; /* 期望闪烁次数 */
    uint16_t flash_cnt;     /* 实际闪烁次数 */
    uint32_t color;         /* RGB灯需要显示的颜色 */
    uint16_t R_channel;     /* 通道R:取值为定时器的比较值 */
    uint16_t G_channel;     /* 通道G:取值为定时器的比较值 */
    uint16_t B_channel;     /* 通道B:取值为定时器的比较值 */
} LED_InstanceDef;
typedef LED_InstanceDef *LED_InstanceHandle;

void led_init(void);
LED_InstanceHandle Y_led_creat_instance(uint8_t idx, uint32_t color);
void led_start(LED_InstanceHandle led_instance_handle, uint16_t light_time, uint16_t stop_time, uint16_t all_flash_cnt);
#endif //!__LED__H__
