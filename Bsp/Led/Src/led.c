/*
 * @Author: Hengyang Jiang
 * @Date: 2024-12-17 14:54:59
 * @LastEditors: Hengyang Jiang
 * @LastEditTime: 2024-12-18 20:29:58
 * @Description: led.c
 *               板载一个RGB灯,无其他可配置LED灯,RGB配置有三个引脚R:PD14/G:PD13/B:PD15
 *               通过控制R/G/B产生不同的取值,进而控制最终显示的颜色
 *               使用16色表示实际的RGB颜色,通过宏定义的方式实现,通过位运算取出R/G/B三个值
 *
 * Copyright (c) 2024 by https://github.com/Nolan-Jon, All Rights Reserved.
 */
#include "FreeRTOS.h" /* 这里并不需要FreeRTOS.h这个文件,但是task.h必须在FreeRTOS后面 */
#include "task.h"
#include "led.h"
#include "string.h"
#include "rtt.h"
#include "stdlib.h"
#include "portable.h"
static LED_InstanceHandle led_instance_array[INSTANCE_LED_NUM] = {NULL};
static uint8_t WS2812B_LAMP_DATA[WS2812B_DATA_LENGTH] = {0};
/**
 * @description: 初始化led灯
 * @return {*}
 */
void led_init(void)
{
    /* PD13/PD14/PD15链接TIM4的2/3/4通道 */
    /* 定时器生成PWM信号并不需要开启定时器中断 */
    /* 开启PWM */
    /* 在RTOS之前调用初始化 */
    /* 初始时将比较值设置为0 */
    __HAL_TIM_SET_COMPARE(&htim4, TIM_R_CHANNEL, 0);
    __HAL_TIM_SET_COMPARE(&htim4, TIM_G_CHANNEL, 0);
    __HAL_TIM_SET_COMPARE(&htim4, TIM_B_CHANNEL, 0);
    HAL_TIM_PWM_Start(&htim4, TIM_R_CHANNEL); /* 使能通道2 */
    HAL_TIM_PWM_Start(&htim4, TIM_G_CHANNEL); /* 使能通道3 */
    HAL_TIM_PWM_Start(&htim4, TIM_B_CHANNEL); /* 使能通道4 */
}
/**
 * @description: 私有函数,硬件上打开LED灯
 * @param {LED_InstanceHandle} led_instance_handle
 * @return {*}
 */
static void open_led(LED_InstanceHandle led_instance_handle)
{
    __HAL_TIM_SET_COMPARE(&htim4, TIM_R_CHANNEL, led_instance_handle->R_channel);
    __HAL_TIM_SET_COMPARE(&htim4, TIM_G_CHANNEL, led_instance_handle->G_channel);
    __HAL_TIM_SET_COMPARE(&htim4, TIM_B_CHANNEL, led_instance_handle->B_channel);
}

/**
 * @description: 私有函数,硬件上关闭LED灯
 * @return {*}
 */
static void close_led(void)
{
    __HAL_TIM_SET_COMPARE(&htim4, TIM_R_CHANNEL, 0);
    __HAL_TIM_SET_COMPARE(&htim4, TIM_G_CHANNEL, 0);
    __HAL_TIM_SET_COMPARE(&htim4, TIM_B_CHANNEL, 0);
}
/**
 * @description: 该函数作为LED的控制函数,是守护实例的回调函数:每隔200ms调用一次该函数
 *               调用该函数时需要进入临界区
 * @return {*}
 */
void led_daemon_control_callback(void *daemon_instance_handle)
{
    /* 拿取对应的Daemon实例句柄 */
    Daemon_InstanceHandle h_daemon = (Daemon_InstanceHandle)daemon_instance_handle;
    /* 拿取对应的LED实例句柄 */
    LED_InstanceHandle h_led = (LED_InstanceHandle)(h_daemon->owner_instance_handle);
    /* 执行"喂狗"操作 */
    h_daemon->count = h_daemon->reload_count; /* 重新从重载值向下计数 */
    /* LED相关操作 */
    if (h_led->enable_flag == LED_ENABLE)
    {

        if (h_led->all_flash_cnt <= 0) /* 不闪烁 */
        {
            h_led->enable_flag = LED_DIABLE;
            return;
        }
        else /* 闪烁 */
        {
            /* 闪烁周期是400ms */
            h_led->state ? close_led() : open_led(h_led);
            /* 状态取反 */
            h_led->state ^= 0x01;
            h_led->all_flash_cnt--;
        }
    }
    return;
}
/**
 * @description: 创建led灯实例对象
 * @param {uint8_t} idx
 * @param {uint32_t} color
 * @return {*}
 */
LED_InstanceHandle Y_led_creat_instance(uint8_t idx, uint32_t color)
{
    /* 检测编号合法性 */
    if (idx >= INSTANCE_LED_NUM)
    {
        while (1)
        {
            LOGERROR("[led_create]The LED Number Is Illegal!");
        }
    }
    /* 检测实例是否存在 */
    if (led_instance_array[idx] != NULL)
    {
        while (1)
        {
            LOGERROR("[led_create]LED Instance Already Created!");
        }
    }
    /* 进入临界区,确保创建串口实例的过程是线程安全的,不要在检测串口是否合法之前进入临界区,这样会影响其他任务的调度,使整个系统卡死 */
    taskENTER_CRITICAL();
    /* 使用线程安全的内存分配函数 */
    LED_InstanceHandle led_instance_handle = (LED_InstanceHandle)pvPortMalloc(sizeof(LED_InstanceDef));
    if (led_instance_handle == NULL)
    {
        LOGERROR("[led_create]LED Instance Create Failed,Stack Overflow!\r\n");
        /* 退出临界区 */
        taskEXIT_CRITICAL();
        return NULL;
    }
    led_instance_handle->idx = idx;
    led_instance_handle->color = color;
    led_instance_handle->all_flash_cnt = 0;        /* 初始化为0 */
    led_instance_handle->enable_flag = LED_DIABLE; /* 初始时LED失能 */
    led_instance_handle->state = LED_STATE_STOP;   /* 初始时LED熄灭 */

    /* 解析RGB值:解析出RBG通道值对应的定时器比较值 */
    uint8_t R = color >> 16; /* 通道取值范围:0 ~ 255 */
    uint8_t G = color >> 8;
    uint8_t B = color;
    /* TIM4的自动重载值是255 * 5,因此通道的每一份取值对应的计数值为255 * 5 / 255 = 5 */
    /* 配置:小于比较值的时候,低电平;大于比较值的时候,高电平 */
    /* 注意LED灯低电平点亮,需要在配置定时器PWM的时候电平翻转 */
    led_instance_handle->R_channel = R * 5;
    led_instance_handle->G_channel = G * 5;
    led_instance_handle->B_channel = B * 5;

    /* 注册守护对象 */
    led_instance_handle->led_daemon = Y_daemon_create_instance((void *)led_instance_handle, DAEMON_OWNER_TYPE_LED, idx, DAEMON_RELOAD_LED, led_daemon_control_callback);
    /* 退出临界区 */
    taskEXIT_CRITICAL();

    return led_instance_handle;
}
/**
 * @description: 开启LED灯,任务中调用需要进入临界区
 *               若其他任务同样执行LED灯闪烁操作,当前执行的操作将被替换掉
 *               不支持常亮:后续可提供支持
 * @param {LED_InstanceHandle} led_instance_handle
 * @param {uint16_t} all_flash_cnt
 * @return {*}
 */
void led_start(LED_InstanceHandle led_instance_handle, uint16_t all_flash_cnt)
{
    /* 开启当前LED实例 */
    led_instance_handle->all_flash_cnt = all_flash_cnt;
    led_instance_handle->flash_cnt = 0;
    led_instance_handle->enable_flag = LED_ENABLE; /* LED处于运行态 */
}

/*--------------------------------------------------ws2812b驱动接口--------------------------------------------------*/
/**
 * @description: WS2812B相关代码,目标:做成呼吸灯
 *               无法通过占空比的方式实现呼吸效果,因此选择RGB色盘实现呼吸效果
 *               启用TIM1的Channel4,一个方波的频率是800KHz,即1.25us
 *               TIM1的重载值是210,1/3即70,同时TIM1设置为PWM模式1,由于DMA向TIM1发送比较值,因此我们需要配置比较值
 *               例如,在当前配置下向TIM1发送数据70,即将比较值设置为70,表示产生一个占空比为1/3的频率为800KHz的方波,"持续一个周期"
 *               0码:占空比为1/3的800KHz的方波,对应的数据0x46,即70
 *               1码:占空比为2/3的800KHz的方波,对应的数据0x8C,即140
 *               从上面的分析可以看出发送ws2812b协议的1码需要实际一字节数据,一个灯珠需要24码数据,即24字节数据,这是一个不小的体量
 * @return {*}
 */
void ws2812b_init(void)
{
    LOGINFO("-------Use WS2812B.\r\n");
    ws2812b_close_lamp();
    ws2812b_load_data();
}
/**
 * @description: 开启DMA,向定时器写入比较值,即将数据写入到ws2812b中
 * @return {*}
 */
void ws2812b_load_data(void)
{
    HAL_TIM_PWM_Start_DMA(&htim1, TIM_CHANNEL_4, (uint32_t *)WS2812B_LAMP_DATA, WS2812B_DATA_LENGTH);
}
/**
 * @description: 关闭所有的灯珠
 * @return {*}
 */
void ws2812b_close_lamp(void)
{
    uint16_t i = 0;
    /* 写入#000000(黑色) */
    for (; i < LAMP_NUM * 24; i++)
    {
        WS2812B_LAMP_DATA[i] = WS_0;
    }

    /* 写入复位脉冲 */
    for (i = LAMP_NUM * 24; i < WS2812B_DATA_LENGTH; i++)
    {
        WS2812B_LAMP_DATA[i] = 0; /* 全0复位脉冲 */
    }
}
/**
 * @description: 向数组中写入颜色数据
 * @param {uint32_t} color
 * @return {*}
 */
void ws2812b_write_color(uint32_t color)
{
    uint8_t decode_data[24] = {0};
    uint8_t ws_R_channel = color >> 16;
    uint8_t ws_G_channel = color >> 8;
    uint8_t ws_B_channel = color;

    /* 解码G */
    for (uint8_t i = 0; i < 8; i++)
    {
        decode_data[i] = ((ws_G_channel & 0x80) ? WS_1 : WS_0);
        ws_G_channel <<= 1;
    }
    /* 解码R */
    for (uint8_t i = 0; i < 8; i++)
    {
        decode_data[i + 8] = ((ws_R_channel & 0x80) ? WS_1 : WS_0);
        ws_R_channel <<= 1;
    }
    /* 解码B */
    for (uint8_t i = 0; i < 8; i++)
    {
        decode_data[i + 16] = ((ws_B_channel & 0x80) ? WS_1 : WS_0);
        ws_B_channel <<= 1;
    }
    
    /* 写入数组 */
    for (uint16_t i = 0; i < LAMP_NUM; i++)
    {
        for (uint8_t j = 0; j < 24; j++)
        {
            WS2812B_LAMP_DATA[24 * i + j] = decode_data[j];
        }
    }

    /* 写入复位脉冲 */
    for (uint16_t i = LAMP_NUM * 24; i < WS2812B_DATA_LENGTH; i++)
    {
        WS2812B_LAMP_DATA[i] = 0;
    }
}
