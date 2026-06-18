/**
 * @file hal_gpio_host.c
 * @brief Host stubs for HAL GPIO — used in unit tests
 */
#include "hal_gpio.h"

static uint8_t s_levels[32];
static HAL_GPIO_Mode_t s_modes[32];

void HAL_GPIO_Init(uint8_t pin, HAL_GPIO_Mode_t mode, HAL_GPIO_Pull_t pull)
{
    (void)pull;
    if (pin < 32) {
        s_modes[pin] = mode;
    }
}

void HAL_GPIO_Write(uint8_t pin, uint8_t level)
{
    if (pin < 32) {
        s_levels[pin] = level;
    }
}

uint8_t HAL_GPIO_Read(uint8_t pin)
{
    if (pin < 32) {
        return s_levels[pin];
    }
    return 0;
}

void HAL_GPIO_Toggle(uint8_t pin)
{
    if (pin < 32) {
        s_levels[pin] = (uint8_t)(!s_levels[pin]);
    }
}
