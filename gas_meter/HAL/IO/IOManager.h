/**
 * @file IOManager.h
 * @brief Centralized GPIO request/release/read/write — prevents pin conflicts
 */
#ifndef IO_MANAGER_H
#define IO_MANAGER_H

#include "hal_common.h"
#include "IOConfig.h"

void          IOManager_Init(void);
HAL_Status_t  IO_Request(uint8_t pin, IOMode_t mode);
void          IO_Release(uint8_t pin);
HAL_Status_t  IO_Write(uint8_t pin, uint8_t level);
uint8_t       IO_Read(uint8_t pin);
void          IO_Toggle(uint8_t pin);

#endif /* IO_MANAGER_H */
