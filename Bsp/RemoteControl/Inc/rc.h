/*
 * @Author: Hengyang Jiang
 * @Date: 2024-12-20 12:22:31
 * @LastEditors: Hengyang Jiang
 * @LastEditTime: 2024-12-20 18:16:02
 * @Description: rc.h
 *
 * Copyright (c) 2024 by https://github.com/Nolan-Jon, All Rights Reserved.
 */
#ifndef __RC__H__
#define __RC__H__
#include "stdint.h"
#include "uart.h"
#include "daemon.h"
/* 遥控器通道解码图 */
/**
 * 云卓T10的SBUS接收数据:
 *                     一帧SBUS协议共25字节,SBUS[0]表示帧头
 *                     SBUS[1] ~ SBUS[22]共22字节,对应16个通道,每个通道11bit
 *                     SBUS[23]表示遥控器的通讯状态:在线或离线
 * SBUS协议的数据值与通道的对应关系:
 *                               ch_1 = sbus[2]的低3bit + sbus[1]的高8bit(只有8bit)
 *                               ch_2 = sbus[3]的低6bit + sbus[2]的高5bit
 *                               ch_3 = sbus[5]的低1bit + sbus[4]的8bit + sbus[3]的高2bit
 *                               ch_4 = sbus[6]的低4bit + sbus[5]的高7bit
 *                               ch_5 = sbus[7]的低7bit + sbus[6]的高4bit
 *                               .................
 *
 *
 *           | |                                                                          | |
 *           | |                                                                          | |
 *           | |                                                                          | |
 *           | |                                                                          | |
 *           | |                                                                          | |
 *           | |                                                                          | |
 *           | |                                                                          | |
 *           | |                                                                          | |
 *     —— —— —— —— —— —— —— —— —— —— —— —— —— —— —— —— —— —— —— —— —— —— —— —— —— —— —— —— —— —— ——
 *    |   ||| <E>                                                                        <F> |||   |
 *    |                                                                                            |
 *    |      ------- <C>                                                               <D> ------- |
 *    |                                                                                            |
 *    |             +                                                             +                |
 *    |            +++                                                           +++               |
 *    |           +++++                                                         +++++              |
 *    |         +++++++++   <Y1>                                       <Y2>   +++++++++            |
 *    |           +++++                                                         +++++              |
 *    |            +++                                                           +++               |
 *    |             +                                                             +                |
 *    |                                                                                            |
 *    |            <X1>                                                          <X2>              |
 *    |                                                                                            |
 *    |                                                                                            |
 *    |                                            |                                               |
 *    |                      (A)                 __|__                 (B)                         |
 *    |                                                                                            |
 *    |                                                                                            |
 *     —— —— —— —— —— —— —— —— —— —— —— —— —— —— —— —— —— —— —— —— —— —— —— —— —— —— —— —— —— —— ——
 * 通道值分析:
 * CH1: 控制X2; 中间值[1002], 向左取值[282, 1002), 向右取值(1002, 1722]
 * CH2: 控制Y2; 中间值[1002], 向上取值(1002, 282], 向下取值(1002, 1722]
 * CH3: 控制Y1; 中间值[1002], 向上取值(1002, 1722], 向下取值(1022, 282]
 * CH4: 控制X1; 中间值[1002], 向左取值(1002, 282], 向右取值(1002, 1722]
 * CH5: 控制E,最左值[282], 中间值[1002], 最右值[1722]
 * CH6: 控制F,最左值[282], 中间值[1002], 最右值[1722]
 * CH7: 控制按键A,非选中值[282], 选中值[1722]
 * CH8: 控制按键B,非选中值[282], 选中值[1722]
 * CH9: 控制滚轮C,最左值[282], 最右值[1722]
 * CH10:控制滚轮D,最左值[282], 最右值[1722]
 * CH11:未使用
 * CH12:未使用
 * CH13:未使用
 * CH14:未使用
 * CH15:未使用
 * CH16:未使用
 */
typedef struct
{
    /* data */
    uint8_t enable_flag;                         /* 遥控器使能标志:0失能,1使能 */
    uint8_t state_flag;                          /* 遥控器状态标志:在线或离线,0离线,1在线 */
    uint16_t CH_1;                               /* 通道1 */
    uint16_t CH_2;                               /* 通道2 */
    uint16_t CH_3;                               /* 通道3 */
    uint16_t CH_4;                               /* 通道4 */
    uint16_t CH_5;                               /* 通道5 */
    uint16_t CH_6;                               /* 通道6 */
    uint16_t CH_7;                               /* 通道7 */
    uint16_t CH_8;                               /* 通道8 */
    uint16_t CH_9;                               /* 通道9 */
    uint16_t CH_10;                              /* 通道10 */
    UART_InstanceHandle rc_uart_instance_handle; /* 对应的串口实例 */
    Daemon_InstanceHandle rc_daemon_instance;    /* 对应的守护对象实例 */
    /* 应创建一个BEEP对象:当遥控器离线的时候产生蜂鸣声 */
} RemoteCR_InstanceDef;
typedef RemoteCR_InstanceDef *RemoteCR_InstanceHandle;
#define REMOTECR_PROTOCOL_FRAME_SIZE 25 /* 遵循SBUS协议:一帧数据25字节;是否需要将缓冲区放大一点 */
RemoteCR_InstanceHandle Y_rc_create_instance(void);
#endif //!__RC__H__
