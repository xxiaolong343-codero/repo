/**
 * @file hal_gpio.h
 * @brief GPIO hardware abstraction interface
 */
#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include "hal_common.h"

/* ---- GPIO modes ---- */
typedef enum {
    HAL_GPIO_MODE_INPUT       = 0,
    HAL_GPIO_MODE_OUTPUT_PP   = 1,   /* Push-pull */
    HAL_GPIO_MODE_OUTPUT_OD   = 2,   /* Open-drain */
    HAL_GPIO_MODE_ALT_FUNC    = 3
} HAL_GPIO_Mode_t;

/* ---- GPIO pull config ---- */
typedef enum {
    HAL_GPIO_PULL_NONE  = 0,
    HAL_GPIO_PULL_UP    = 1,
    HAL_GPIO_PULL_DOWN  = 2
} HAL_GPIO_Pull_t;

/* ---- HAL GPIO API — implemented by platform driver ---- */
void     HAL_GPIO_Init(uint8_t pin, HAL_GPIO_Mode_t mode, HAL_GPIO_Pull_t pull);
void     HAL_GPIO_Write(uint8_t pin, uint8_t level);
uint8_t  HAL_GPIO_Read(uint8_t pin);
void     HAL_GPIO_Toggle(uint8_t pin);

#endif /* HAL_GPIO_H */
