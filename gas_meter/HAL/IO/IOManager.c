/**
 * @file IOManager.c
 * @brief 集中式 GPIO 管理器实现 — 引脚占用追踪与冲突检测
 *
 * 本文件实现 GPIO 的集中管理：
 * - 使用位图追踪 32 个引脚的占用状态
 * - 使用模式数组区分输入/输出模式
 * - IO_Request 双重申请自动拒绝
 * - IO_Release 恢复引脚为安全状态（输入+上拉）
 *
 * 设计依据：规格文档 §7 — IO 管理设计
 */
#include "IOManager.h"
#include "hal_gpio.h"
#include <string.h>

/* ---- 引脚占用追踪 ---- */
static uint8_t s_owned[32];     /**< 位图：bit[i]=1 表示引脚 i 已被占用 */
static uint8_t s_pin_mode[32];  /**< 模式：0=空闲, 1=输入, 2=输出 */

/* ============================================================
 *  公开 API
 * ============================================================ */

void IOManager_Init(void)
{
    memset(s_owned, 0, sizeof(s_owned));
    memset(s_pin_mode, 0, sizeof(s_pin_mode));

    /* 初始化所有已使用引脚（不标记为占用，组件需主动 IO_Request） */
    for (uint16_t i = 0; i < g_usedPinCount; i++) {
        const IOPinConfig_t *cfg = &g_usedPins[i];
        HAL_GPIO_Init(cfg->pin, cfg->mode, cfg->pull);
    }

    /* 初始化所有未使用引脚为安全状态（输入 + 上拉） */
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

    /* 检查是否已被占用 */
    if (BIT_GET(s_owned[byte_idx], bit_idx)) {
        return HAL_ERROR;  /* 引脚已被其他组件占用 */
    }

    /* 标记为占用 */
    BIT_SET(s_owned[byte_idx], bit_idx);
    s_pin_mode[pin] = (mode == IO_MODE_OUTPUT) ? 2 : 1;

    /* 配置硬件模式 */
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

    /* 恢复为安全状态：输入 + 上拉 */
    HAL_GPIO_Init(pin, HAL_GPIO_MODE_INPUT, HAL_GPIO_PULL_UP);
}

HAL_Status_t IO_Write(uint8_t pin, uint8_t level)
{
    if (pin >= 32) {
        return HAL_ERROR;
    }

    uint8_t byte_idx = pin / 8;
    uint8_t bit_idx  = pin % 8;

    /* 检查引脚是否已被申请 */
    if (!BIT_GET(s_owned[byte_idx], bit_idx)) {
        return HAL_ERROR;  /* 引脚未被申请 */
    }
    /* 检查模式是否为输出 */
    if (s_pin_mode[pin] != 2) {
        return HAL_ERROR;  /* 引脚模式不是输出 */
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
