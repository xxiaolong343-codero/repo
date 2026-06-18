/**
 * @file IOConfig.c
 * @brief HDSC board IO configuration table
 */
#include "IOConfig.h"

const IOPinConfig_t g_usedPins[] = {
    /* pin,                mode,                   pull */
    { PIN_METER_PULSE,    HAL_GPIO_MODE_INPUT,    HAL_GPIO_PULL_UP   },
    { PIN_VALVE_CTRL_A,   HAL_GPIO_MODE_OUTPUT_PP, HAL_GPIO_PULL_NONE },
    { PIN_VALVE_CTRL_B,   HAL_GPIO_MODE_OUTPUT_PP, HAL_GPIO_PULL_NONE },
    { PIN_LED_RUN,        HAL_GPIO_MODE_OUTPUT_PP, HAL_GPIO_PULL_NONE },
    { PIN_KEY_MENU,       HAL_GPIO_MODE_INPUT,    HAL_GPIO_PULL_UP   },
    /* ... HDSC-specific pin assignments ... */
};
const uint16_t g_usedPinCount = sizeof(g_usedPins) / sizeof(g_usedPins[0]);

const IOPinConfig_t g_unusedPins[] = {
    /* All unlisted HDSC pins → input + pull-up for safety */
};
const uint16_t g_unusedPinCount = sizeof(g_unusedPins) / sizeof(g_unusedPins[0]);
