/**
 * @file hal_flash.h
 * @brief Flash / EEPROM 硬件抽象接口 — 读、写、擦除
 *
 * 本文件定义 Flash/EEPROM 的 HAL 层接口，用于：
 * - E2 存储组件的参数读写（CAT24C256 EEPROM）
 * - OTA 升级的固件区写入
 *
 * 由平台驱动（RL78/HDSC）实现具体总线操作（I2C）。
 */
#ifndef HAL_FLASH_H
#define HAL_FLASH_H

#include "hal_common.h"

/**
 * @brief 从 Flash/EEPROM 读取数据
 * @param addr 起始地址
 * @param buf  数据缓冲区指针
 * @param len  读取字节数
 * @return HAL_OK 成功，HAL_ERROR 失败
 */
HAL_Status_t HAL_Flash_Read(uint32_t addr, uint8_t *buf, uint16_t len);

/**
 * @brief 向 Flash/EEPROM 写入数据
 * @param addr 起始地址
 * @param buf  数据缓冲区指针
 * @param len  写入字节数
 * @return HAL_OK 成功，HAL_ERROR 失败
 */
HAL_Status_t HAL_Flash_Write(uint32_t addr, const uint8_t *buf, uint16_t len);

/**
 * @brief 擦除 Flash/EEPROM 指定区域
 * @param addr 起始地址
 * @param len  擦除字节数
 * @return HAL_OK 成功，HAL_ERROR 失败
 */
HAL_Status_t HAL_Flash_Erase(uint32_t addr, uint16_t len);

#endif /* HAL_FLASH_H */
