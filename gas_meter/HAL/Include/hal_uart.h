/**
 * @file hal_uart.h
 * @brief UART hardware abstraction interface
 */
#ifndef HAL_UART_H
#define HAL_UART_H

#include "hal_common.h"

typedef void (*HAL_UART_RxCallback_t)(uint8_t byte);

void HAL_UART_Init(uint8_t port, uint32_t baudrate);
void HAL_UART_Send(uint8_t port, const uint8_t *data, uint16_t len);
void HAL_UART_RegisterRxCallback(uint8_t port, HAL_UART_RxCallback_t cb);

#endif /* HAL_UART_H */
