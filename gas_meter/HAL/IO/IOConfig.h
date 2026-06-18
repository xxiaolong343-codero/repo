/**
 * @file IOConfig.h
 * @brief IO 引脚配置表类型定义 — 已使用/未使用引脚声明
 *
 * 本文件定义：
 * - IOMode_t: IO 管理器使用的输入/输出模式枚举
 * - IOPinConfig_t: 单个引脚的配置条目（引脚号 + 硬件模式 + 上下拉）
 * - 板级配置表的外部声明（由 Bsp/ 目录中的 IOConfig.c 实现）
 *
 * 设计依据：规格文档 §7.3 — IO 配置表
 */
#ifndef IO_CONFIG_H
#define IO_CONFIG_H

#include "hal_common.h"
#include "hal_gpio.h"

/* ---- IO 管理器使用的模式 ---- */
/**
 * @brief IO 模式枚举 — 用于 IO_Request() 参数
 */
typedef enum {
    IO_MODE_INPUT  = 0,   /**< 输入模式 */
    IO_MODE_OUTPUT = 1    /**< 输出模式 */
} IOMode_t;

/* ---- 引脚配置条目 ---- */
/**
 * @brief 单个引脚的配置条目 — 定义在 Bsp/IOConfig.c 中
 */
typedef struct {
    uint8_t         pin;   /**< 引脚编号（IOPin_t 枚举值） */
    HAL_GPIO_Mode_t mode;  /**< 硬件 GPIO 模式（输入/推挽/开漏/复用） */
    HAL_GPIO_Pull_t pull;  /**< 上下拉配置（无/上拉/下拉） */
} IOPinConfig_t;

/* ---- 板级配置表（由 Bsp/ 中的 IOConfig.c 定义） ---- */
extern const IOPinConfig_t g_usedPins[];      /**< 已使用引脚配置表 */
extern const uint16_t      g_usedPinCount;    /**< 已使用引脚数量 */
extern const IOPinConfig_t g_unusedPins[];    /**< 未使用引脚配置表（安全状态） */
extern const uint16_t      g_unusedPinCount;  /**< 未使用引脚数量 */

#endif /* IO_CONFIG_H */
