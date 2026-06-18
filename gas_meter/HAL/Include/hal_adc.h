/**
 * @file hal_adc.h
 * @brief ADC 硬件抽象接口 — 初始化、启动、读取、回调注册
 *
 * 本文件定义 ADC 的 HAL 层接口，用于：
 * - 计量组件的红外光/环境光采样（Le, Lc）
 * - 电源组件的电压检测（主电、备电、SPC）
 *
 * 由平台驱动（RL78/HDSC）实现具体寄存器操作。
 */
#ifndef HAL_ADC_H
#define HAL_ADC_H

#include "hal_common.h"

/**
 * @brief ADC 转换完成回调类型
 * @param channel ADC 通道号
 * @param value   转换结果（12 位）
 */
typedef void (*HAL_ADC_Callback_t)(uint8_t channel, uint16_t value);

/**
 * @brief 初始化 ADC 模块
 */
void HAL_ADC_Init(void);

/**
 * @brief 启动指定通道的 ADC 转换
 * @param channel ADC 通道号
 */
void HAL_ADC_Start(uint8_t channel);

/**
 * @brief 读取指定通道的 ADC 转换结果
 * @param channel ADC 通道号
 * @return 12 位转换结果
 */
uint16_t HAL_ADC_Read(uint8_t channel);

/**
 * @brief 注册 ADC 转换完成回调
 * @param channel ADC 通道号
 * @param cb      回调函数指针
 */
void HAL_ADC_RegisterCallback(uint8_t channel, HAL_ADC_Callback_t cb);

#endif /* HAL_ADC_H */
