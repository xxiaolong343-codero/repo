/**
 * @file IOManager.c
 * @brief Centralized GPIO manager — request, release, conflict detection
 */
#include "IOManager.h"
#include "hal_gpio.h"
#include <string.h>

/* Track which pins are currently owned */
static uint8_t s_owned[32];   /* Bit i = 1 if pin i is owned */
/* 0 = free, 1 = owned as input, 2 = owned as output */
static uint8_t s_pin_mode[32];

void IOManager_Init(void)
{
    memset(s_owned, 0, sizeof(s_owned));
    memset(s_pin_mode, 0, sizeof(s_pin_mode));

    /* Initialize all used pins from config table */
    for (uint16_t i = 0; i < g_usedPinCount; i++) {
        const IOPinConfig_t *cfg = &g_usedPins[i];
        HAL_GPIO_Init(cfg->pin, cfg->mode, cfg->pull);
        /* Don't mark as owned — components must IO_Request() */
    }

    /* Initialize unused pins to safe state (input + pull) */
    for (uint16_t i = 0; i < g_unusedPinCount; i++) {
        const IOPinConfig_t *cfg = &g_unusedPins[i];
        HAL_GPIO_Init(cfg->pin, cfg->mode, cfg->pull);
    }
}

HAL_Status_t IO_Request(uint8_t pin, IOMode_t mode)
{
    if (pin >= 32) {
        return HAL_ERROR;
    }

    uint8_t byte_idx = pin / 8;
    uint8_t bit_idx  = pin % 8;

    /* Already owned? Reject */
    if (BIT_GET(s_owned[byte_idx], bit_idx)) {
        return HAL_ERROR;
    }

    /* Mark as owned */
    BIT_SET(s_owned[byte_idx], bit_idx);
    s_pin_mode[pin] = (mode == IO_MODE_OUTPUT) ? 2 : 1;

    /* Configure hardware */
    HAL_GPIO_Mode_t hw_mode = (mode == IO_MODE_OUTPUT)
                            ? HAL_GPIO_MODE_OUTPUT_PP
                            : HAL_GPIO_MODE_INPUT;
    HAL_GPIO_Init(pin, hw_mode, HAL_GPIO_PULL_NONE);

    return HAL_OK;
}

void IO_Release(uint8_t pin)
{
    if (pin >= 32) {
        return;
    }

    uint8_t byte_idx = pin / 8;
    uint8_t bit_idx  = pin % 8;

    BIT_CLR(s_owned[byte_idx], bit_idx);
    s_pin_mode[pin] = 0;

    /* Return to safe input state */
    HAL_GPIO_Init(pin, HAL_GPIO_MODE_INPUT, HAL_GPIO_PULL_UP);
}

HAL_Status_t IO_Write(uint8_t pin, uint8_t level)
{
    if (pin >= 32) {
        return HAL_ERROR;
    }

    uint8_t byte_idx = pin / 8;
    uint8_t bit_idx  = pin % 8;

    if (!BIT_GET(s_owned[byte_idx], bit_idx)) {
        return HAL_ERROR;
    }
    if (s_pin_mode[pin] != 2) {
        return HAL_ERROR;  /* Not output */
    }

    HAL_GPIO_Write(pin, level);
    return HAL_OK;
}

uint8_t IO_Read(uint8_t pin)
{
    if (pin >= 32) {
        return 0;
    }
    return HAL_GPIO_Read(pin);
}

void IO_Toggle(uint8_t pin)
{
    if (pin >= 32) {
        return;
    }
    HAL_GPIO_Toggle(pin);
}
