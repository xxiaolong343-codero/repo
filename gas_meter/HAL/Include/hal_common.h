/**
 * @file hal_common.h
 * @brief 公共 HAL 类型定义、平台检测宏、位操作辅助宏
 *
 * 本文件提供：
 * - 编译期平台检测（RL78 / HDSC / Host）
 * - 通用返回码枚举（HAL_Status_t）
 * - 中断开关辅助函数（Host 模式为 inline stub）
 * - 位操作辅助宏（参数加括号，整体加括号）
 */
#ifndef HAL_COMMON_H
#define HAL_COMMON_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* ---- 平台检测 ---- */
#if defined(__CCRL__) || defined(__RL78__)
    #define PLATFORM_RL78  1     /**< 瑞萨 RL78 平台（CS+ 编译器） */
    #define PLATFORM_HDSC  0     /**< 华大芯片平台（非当前） */
#elif defined(__CC_ARM) || defined(__ARMCC_VERSION) || defined(__KEIL__)
    #define PLATFORM_RL78  0     /**< 瑞萨 RL78 平台（非当前） */
    #define PLATFORM_HDSC  1     /**< 华大芯片平台（Keil MDK 编译器） */
#else
    #define PLATFORM_RL78  0     /**< 瑞萨 RL78 平台（非当前） */
    #define PLATFORM_HDSC  0     /**< 华大芯片平台（非当前） */
    #define PLATFORM_HOST  1     /**< Host 模式（单元测试 / 仿真） */
#endif

/* ---- 通用返回码 ---- */
typedef enum {
    HAL_OK       = 0,    /**< 操作成功 */
    HAL_ERROR    = -1,   /**< 一般错误 */
    HAL_BUSY     = -2,   /**< 资源忙碌 */
    HAL_TIMEOUT  = -3    /**< 操作超时 */
} HAL_Status_t;

/* ---- 中断开关辅助（平台相关） ---- */
#if PLATFORM_HOST
    /**
     * @brief 关闭中断（Host 模式 stub，直接返回 0）
     * @return 中断保存值（Host 模式下无意义）
     */
    static inline uint8_t HAL_DisableInterrupts(void) { return 0; }

    /**
     * @brief 恢复中断（Host 模式 stub，无操作）
     * @param saved_ie 之前保存的中断状态值
     */
    static inline void HAL_RestoreInterrupts(uint8_t saved_ie) { (void)saved_ie; }
#else
    /**
     * @brief 关闭中断（平台 HAL 实现）
     * @return 中断保存值，用于后续 HAL_RestoreInterrupts
     */
    uint8_t HAL_DisableInterrupts(void);

    /**
     * @brief 恢复中断到之前保存的状态
     * @param saved_ie HAL_DisableInterrupts 返回的保存值
     */
    void HAL_RestoreInterrupts(uint8_t saved_ie);
#endif

/* ---- 位操作辅助宏 ---- */
/**
 * @brief 设置寄存器中指定位
 * @param reg 目标寄存器变量
 * @param bit 位编号（0起始）
 * @note 参数和整体均加括号，符合 MISRA-C Rule 20.4 / 20.7
 */
#define BIT_SET(reg, bit)     ((reg) |=  (1UL << (bit)))

/**
 * @brief 清除寄存器中指定位
 * @param reg 目标寄存器变量
 * @param bit 位编号（0起始）
 */
#define BIT_CLR(reg, bit)     ((reg) &= ~(1UL << (bit)))

/**
 * @brief 读取寄存器中指定位的值
 * @param reg 目标寄存器变量
 * @param bit 位编号（0起始）
 * @return 该位的值（0 或 1）
 */
#define BIT_GET(reg, bit)     (((reg) >> (bit)) & 1UL)

/**
 * @brief 翻转寄存器中指定位
 * @param reg 目标寄存器变量
 * @param bit 位编号（0起始）
 */
#define BIT_TOGGLE(reg, bit)  ((reg) ^=  (1UL << (bit)))

#endif /* HAL_COMMON_H */
