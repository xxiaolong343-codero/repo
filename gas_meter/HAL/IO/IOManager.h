/**
 * @file IOManager.h
 * @brief 集中式 GPIO 管理器 — 申请/释放/读写/翻转，防止引脚冲突
 *
 * 本文件提供统一的 GPIO 管理接口：
 * - 组件通过 IO_Request() 申请引脚，IO_Release() 释放
 * - IO_Write()/IO_Read()/IO_Toggle() 执行实际 IO 操作
 * - 双重申请自动拒绝，防止多组件同时使用同一引脚
 *
 * 设计依据：规格文档 §7 — IO 管理设计
 */
#ifndef IO_MANAGER_H
#define IO_MANAGER_H

#include "hal_common.h"
#include "IOConfig.h"

/**
 * @brief 初始化 IO 管理器
 *
 * 清零引脚占用表，根据配置表初始化所有已使用和未使用的引脚。
 * 已使用引脚配置为 HAL 层默认状态（不标记为已占用），
 * 未使用引脚配置为输入+上拉的安全状态。
 *
 * @note 必须在所有 IO_Request() 调用之前调用。
 */
void IOManager_Init(void);

/**
 * @brief 申请 GPIO 引脚
 *
 * 检查引脚是否已被占用，未被占用时标记为占用并配置硬件。
 *
 * @param pin  引脚编号（IOPin_t 枚举值）
 * @param mode IO_MODE_INPUT 或 IO_MODE_OUTPUT
 * @return HAL_OK 申请成功，HAL_ERROR 引脚已被占用或 pin 无效
 */
HAL_Status_t IO_Request(uint8_t pin, IOMode_t mode);

/**
 * @brief 释放 GPIO 引脚
 *
 * 取消引脚占用标记，恢复为输入+上拉的安全状态。
 *
 * @param pin 引脚编号
 */
void IO_Release(uint8_t pin);

/**
 * @brief 写 GPIO 电平
 *
 * @param pin   引脚编号
 * @param level 0=低电平, 1=高电平
 * @return HAL_OK 写入成功，HAL_ERROR 引脚未申请或模式不是输出
 */
HAL_Status_t IO_Write(uint8_t pin, uint8_t level);

/**
 * @brief 读 GPIO 电平
 *
 * @param pin 引脚编号
 * @return 当前电平（0 或 1），pin 无效时返回 0
 */
uint8_t IO_Read(uint8_t pin);

/**
 * @brief 翻转 GPIO 电平
 *
 * @param pin 引脚编号
 */
void IO_Toggle(uint8_t pin);

#endif /* IO_MANAGER_H */
