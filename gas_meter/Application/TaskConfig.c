/**
 * @file TaskConfig.c
 * @brief Persist task enable/disable state to E2 with CRC
 *
 * Format: [version:1B][bitmap:3B][crc8:1B] = 5 bytes total
 * spec §4.10
 */
#include "TaskConfig.h"
#include "TaskManager.h"
#include <string.h>

/* ---- E2 address for task config ---- */
#define TASK_CONFIG_E2_ADDR  0x0000

/* ---- CRC-8 (Dallas/Maxim) ---- */
static uint8_t CalcCrc8(const uint8_t *data, uint16_t len)
{
    uint8_t crc = 0;
    for (uint16_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if ((crc & 0x01) != 0) {
                crc = (uint8_t)((crc >> 1) ^ 0x8C);
            } else {
                crc = (uint8_t)(crc >> 1);
            }
        }
    }
    return crc;
}

/* ---- External E2 interface (HAL) ---- */
extern HAL_Status_t E2_Read(uint32_t addr, uint8_t *buf, uint16_t len);
extern HAL_Status_t E2_Write(uint32_t addr, const uint8_t *buf, uint16_t len);

void TaskConfig_Save(void)
{
    uint8_t buf[5];
    buf[0] = TASK_CONFIG_VERSION;

    /* Pack task enabled bitmap into 3 bytes */
    buf[1] = 0;
    buf[2] = 0;
    buf[3] = 0;
    for (uint8_t i = 0; i < TASK_COUNT && i < 24; i++) {
        if (TaskManager_IsEnabled(i)) {
            buf[1 + i / 8] |= (uint8_t)(1 << (i % 8));
        }
    }

    buf[4] = CalcCrc8(buf, 4);
    E2_Write(TASK_CONFIG_E2_ADDR, buf, 5);
}

void TaskConfig_Load(void)
{
    uint8_t buf[5];

    if (E2_Read(TASK_CONFIG_E2_ADDR, buf, 5) != HAL_OK) {
        return;  /* Use current defaults */
    }

    /* Verify CRC */
    if (CalcCrc8(buf, 4) != buf[4]) {
        return;  /* CRC mismatch — keep current defaults */
    }

    if (buf[0] != TASK_CONFIG_VERSION) {
        return;  /* Version mismatch */
    }

    /* Restore task enabled state */
    for (uint8_t i = 0; i < TASK_COUNT && i < 24; i++) {
        bool enabled = (bool)((buf[1 + i / 8] >> (i % 8)) & 0x01);
        if (enabled) {
            TaskManager_EnableTask(i);
        } else {
            TaskManager_DisableTask(i);
        }
    }
}
