/*
 * @Author: Hengyang Jiang
 * @Date: 2024-12-17 12:40:28
 * @LastEditors: Hengyang Jiang
 * @LastEditTime: 2024-12-17 15:41:53
 * @Description: dwt.c
 *               CM3和CM4内核有DWT组件,用于跟踪、监控Debug信息,DWT内部有一个"32位"的寄存器CYCCNT,HAL库并没有进行封装,需要使用寄存器直接访问
 *               CYCCNT用来对CPU进行滴答计数,对于168MHz的CPU来说:1s会产生168M个滴答,每产生一个滴答,CYCCNT+1
 *               CYCCNT从0开始一直累加到0xFFFF_FFFF需要2^32 / 168000000 = 25.565s
 *               通常利用CYCCNT的计数性质实现微秒级延时,STM32F407的误差大致在+0.25us
 *               使用建议: 适用于微秒级延时,毫秒级及以上请使用Hal库实现的延时函数或RTOS提供的延时函数
 *               注意: 当前文件提供的代码是阻塞式实现,会持续占用CPU,但遵循上述使用建议,对系统的影响极小,并且系统任务不会影响延时精确度
 *
 *
 * Copyright (c) 2024 by https://github.com/Nolan-Jon, All Rights Reserved.
 */
#include "dwt.h"

/* 静态区变量,用于存储CPU频率相关数据 */
static uint32_t CPU_FREQ_HZ;   /* CPU频率(执行1s需要的CPU节拍数) */
static uint32_t TIME_US_COUNT; /* 执行1us需要的CPU节拍数 */
/**
 * @description: 初始化DWT组件
 * @param {uint32_t} cpu_freq_hz:CPU的频率,例如168表示CPU频率是168MHz
 * @return {*}
 */
void dwt_init(uint32_t cpu_freq_hz)
{
    /* 使能DWT外设 */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

    /* 将DWT->CYCCNT寄存器清零 */
    DWT->CYCCNT = (uint32_t)0;

    /* 使能DWT->CYCCNT寄存器 */
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    /* 获取CPU频率 */
    CPU_FREQ_HZ = cpu_freq_hz * 1000000;

    /* 获取执行1us需要的CPU节拍数 */
    TIME_US_COUNT = CPU_FREQ_HZ / 1000000;
}
/**
 * @description: dwt实现微秒级延时
 * @param {uint16_t} _dwt_delay_time
 * @return {*}
 */
void dwt_delay_us(uint16_t _dwt_delay_time)
{

    uint32_t time_start = DWT->CYCCNT;                         /* 进入时的CYCCNT计数值 */
    uint32_t time_delay_cnt = _dwt_delay_time * TIME_US_COUNT; /* 本次延时需要的节拍数 */
    uint32_t time_cnt = 0;                                     /* 实际节拍数 */

    while (time_cnt < time_delay_cnt)
    {
        /* 计算已经产生了多少节拍数 */
        time_cnt = DWT->CYCCNT - time_start;
        /* 无符号数减法注意事项:*/
        /* 若A > B, A - B = C, 此时C是正确节拍数 */
        /* 若A < B, A - B = C, 此时C也是正确节拍数, C = 0xFFFF_FFFF - B + A + 1*/
        /* 严格要求CYCCNT只产生一次溢出,因此严格要求dwt的延时时间,尽量不要过度到毫秒级 */
    }
}
