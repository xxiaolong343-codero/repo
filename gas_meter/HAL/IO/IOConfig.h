/**
 * @file IOConfig.h
 * @brief IO pin configuration table types
 */
#ifndef IO_CONFIG_H
#define IO_CONFIG_H

#include "hal_common.h"
#include "hal_gpio.h"

/* ---- IO mode for IOManager ---- */
typedef enum {
    IO_MODE_INPUT  = 0,
    IO_MODE_OUTPUT = 1
} IOMode_t;

/* ---- Pin configuration entry — defined in Bsp/IOConfig.c ---- */
typedef struct {
    uint8_t         pin;        /* IOPin_t value */
    HAL_GPIO_Mode_t mode;
    HAL_GPIO_Pull_t pull;
} IOPinConfig_t;

/* ---- Board-specific config tables (defined in Bsp/) ---- */
extern const IOPinConfig_t g_usedPins[];
extern const uint16_t      g_usedPinCount;
extern const IOPinConfig_t g_unusedPins[];
extern const uint16_t      g_unusedPinCount;

#endif /* IO_CONFIG_H */
