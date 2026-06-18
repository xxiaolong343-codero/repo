/**
 * @file hal_uart.h
 * @brief UART 硬件抽象接口 — 初始化、发送、接收回调
 *
 * 本文件定义 UART 的 HAL 层接口，用于：
 * - 红外通信（本地配置/调试）
 * - 调试串口日志输出
 * - NB-IoT 模组 AT 指令通信
 *
 * 由平台驱动（RL78/HDSC）实现具体寄存器操作。
 */
#ifndef HAL_UART_H
#define HAL_UART_H

#include "hal_common.h"

/**
 * @brief UART 接收字节回调类型
 * @param byte 接收到的字节
 */
typedef void (*HAL_UART_RxCallback_t)(uint8_t byte);

/**
 * @brief 初始化 UART 端口
 * @param port     端口号
 * @param baudrate 波特率
 */
void HAL_UART_Init(uint8_t port, uint32_t baudrate);

/**
 * @brief 通过 UART 发送数据
 * @param port 端口号
 * @param data 数据缓冲区指针
 * @param len  发送字节数
 */
void HAL_UART_Send(uint8_t port, const uint8_t *data, uint16_t len);

/**
 * @brief 注册 UART 接收回调
 * @param port 端口号
 * @param cb   回调函数指针
 */
void HAL_UART_RegisterRxCallback(uint8_t port, HAL_UART_RxCallback_t cb);

#endif /* HAL_UART_H */
