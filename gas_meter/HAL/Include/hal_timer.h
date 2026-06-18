/**
 * @file hal_timer.h
 * @brief 定时器硬件抽象接口 — 三时间基准、低功耗定时器、RTC 闹钟
 *
 * 本文件定义定时器 HAL 层接口：
 * - 三基准硬件定时器初始化（10ms/100ms/1s）
 * - 低功耗定时器（SNOOZE 模式唤醒源）
 * - RTC 闹钟（STOP 模式唤醒源）
 * - 系统 tick 获取（ms 和 μs 精度）
 *
 * 由平台驱动（RL78/HDSC）实现具体寄存器操作。
 */
#ifndef HAL_TIMER_H
#define HAL_TIMER_H

#include "hal_common.h"

/* ---- 三基准定时器 ---- */

/**
 * @brief 初始化 10ms 时间基准定时器
 * @note 用于消抖、阀门监控等短周期需求
 */
void HAL_Timer_InitTimeBase10ms(void);

/**
 * @brief 初始化 100ms 时间基准定时器
 * @note 用于计量采样、LED 闪烁等中周期需求
 */
void HAL_Timer_InitTimeBase100ms(void);

/**
 * @brief 初始化 1s 时间基准定时器
 * @note 用于电源巡检、电池检测等长周期需求
 */
void HAL_Timer_InitTimeBase1s(void);

/* ---- 低功耗定时器 ---- */

/**
 * @brief 启动低功耗定时器（SNOOZE 模式唤醒源）
 * @param interval_ms 唤醒间隔（毫秒）
 */
void HAL_Timer_StartLowPower(uint32_t interval_ms);

/**
 * @brief 停止低功耗定时器
 */
void HAL_Timer_StopLowPower(void);

/* ---- 低功耗模式 ---- */

/**
 * @brief 进入 SNOOZE 模式（轻度睡眠，低功耗定时器运行）
 */
void HAL_StartSnoozeMode(void);

/**
 * @brief 进入 STOP 模式（深度睡眠，仅 RTC 和中断可唤醒）
 */
void HAL_EnterStopMode(void);

/**
 * @brief 触发硬件唤醒（从低功耗模式退出）
 */
void HAL_TriggerWakeup(void);

/* ---- 系统 tick ---- */

/**
 * @brief 获取系统启动以来的毫秒计数
 * @return 毫秒计数器值
 * @note 用于组件内部 tick 差值计时（消抖、长按判断等）
 */
uint32_t HAL_GetTick(void);

/**
 * @brief 获取微秒级时间戳
 * @return 微秒计数器值
 * @note 用于计量流速计算等需要高精度时间的场景
 */
uint32_t HAL_GetUsTick(void);

/* ---- RTC 闹钟 ---- */

/**
 * @brief 设置 RTC 闹钟
 * @param timestamp 闹钟触发时间戳
 */
void HAL_SetRtcAlarm(uint32_t timestamp);

/**
 * @brief 清除 RTC 闹钟
 */
void HAL_ClearRtcAlarm(void);

#endif /* HAL_TIMER_H */
