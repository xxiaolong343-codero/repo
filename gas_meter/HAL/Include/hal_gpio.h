/**
 * @file hal_gpio.h
 * @brief GPIO 硬件抽象接口 — 模式/上下拉配置、读写、翻转
 *
 * 本文件定义 GPIO 的 HAL 层统一接口，由平台驱动（RL78/HDSC）实现。
 * 组件通过 IOManager 间接使用，不直接调用 HAL_GPIO_* 函数。
 */
#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include "hal_common.h"

/* ---- GPIO 模式 ---- */
/**
 * @brief GPIO 硬件模式枚举
 */
typedef enum {
    HAL_GPIO_MODE_INPUT       = 0,   /**< 输入模式 */
    HAL_GPIO_MODE_OUTPUT_PP   = 1,   /**< 推挽输出 */
    HAL_GPIO_MODE_OUTPUT_OD   = 2,   /**< 开漏输出 */
    HAL_GPIO_MODE_ALT_FUNC    = 3    /**< 复用功能 */
} HAL_GPIO_Mode_t;

/* ---- GPIO 上下拉配置 ---- */
/**
 * @brief GPIO 上下拉配置枚举
 */
typedef enum {
    HAL_GPIO_PULL_NONE  = 0,   /**< 无上下拉 */
    HAL_GPIO_PULL_UP    = 1,   /**< 上拉 */
    HAL_GPIO_PULL_DOWN  = 2    /**< 下拉 */
} HAL_GPIO_Pull_t;

/* ---- HAL GPIO API（由平台驱动实现） ---- */

/**
 * @brief 初始化 GPIO 引脚的模式和上下拉
 * @param pin  引脚编号
 * @param mode 硬件模式（输入/推挽/开漏/复用）
 * @param pull 上下拉配置
 */
void     HAL_GPIO_Init(uint8_t pin, HAL_GPIO_Mode_t mode, HAL_GPIO_Pull_t pull);

/**
 * @brief 写 GPIO 输出电平
 * @param pin   引脚编号
 * @param level 0=低电平, 1=高电平
 */
void     HAL_GPIO_Write(uint8_t pin, uint8_t level);

/**
 * @brief 读 GPIO 输入电平
 * @param pin 引脚编号
 * @return 当前电平（0 或 1）
 */
uint8_t  HAL_GPIO_Read(uint8_t pin);

/**
 * @brief 翻转 GPIO 输出电平
 * @param pin 引脚编号
 */
void     HAL_GPIO_Toggle(uint8_t pin);

#endif /* HAL_GPIO_H */
