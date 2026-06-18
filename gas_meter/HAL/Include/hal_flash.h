/**
 * @file hal_flash.h
 * @brief Flash / EEPROM hardware abstraction interface
 */
#ifndef HAL_FLASH_H
#define HAL_FLASH_H

#include "hal_common.h"

HAL_Status_t HAL_Flash_Read(uint32_t addr, uint8_t *buf, uint16_t len);
HAL_Status_t HAL_Flash_Write(uint32_t addr, const uint8_t *buf, uint16_t len);
HAL_Status_t HAL_Flash_Erase(uint32_t addr, uint16_t len);

#endif /* HAL_FLASH_H */
