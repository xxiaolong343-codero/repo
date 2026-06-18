/**
 * @file hal_wdg.h
 * @brief 看门狗硬件抽象接口 — 初始化、喂狗
 *
 * 本文件定义看门狗定时器的 HAL 层接口：
 * - 独立硬件看门狗，防止程序跑飞
 * - 不参与正常任务调度
 *
 * 由平台驱动（RL78/HDSC）实现具体寄存器操作。
 */
#ifndef HAL_WDG_H
#define HAL_WDG_H

#include "hal_common.h"

/**
 * @brief 初始化看门狗定时器
 * @param timeout_ms 超时时间（毫秒），超时后触发系统复位
 */
void HAL_WDG_Init(uint32_t timeout_ms);

/**
 * @brief 喂狗（复位看门狗计数器）
 * @note 必须在主循环中定期调用，频率 > timeout_ms
 */
void HAL_WDG_Feed(void);

#endif /* HAL_WDG_H */
