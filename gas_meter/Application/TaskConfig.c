/**
 * @file TaskConfig.c
 * @brief 任务配置持久化到 E2 的实现 — CRC-8 保护的位图读写
 *
 * 本文件实现任务使能状态的持久化：
 * - calc_crc8(): Dallas/Maxim CRC-8 计算
 * - TaskConfig_Save(): 打包位图 → 计算校验 → 写入 E2
 * - TaskConfig_Load(): 读取 E2 → 校验通过 → 恢复状态
 *
 * 数据格式：[version:1B][bitmap:3B][crc8:1B] = 5 bytes total
 * 设计依据：规格文档 §4.10
 */
#include "TaskConfig.h"
#include "TaskManager.h"
#include <string.h>

/* ---- E2 配置区地址 ---- */
#define TASK_CONFIG_E2_ADDR  0x0000   /**< E2 中任务配置的起始地址 */

/* ---- CRC-8 计算 ---- */

/**
 * @brief 计算 CRC-8 校验值（Dallas/Maxim 多项式 0x8C）
 *
 * @param data 数据缓冲区指针
 * @param len  数据长度（字节）
 * @return 8 位 CRC 校验值
 */
static uint8_t calc_crc8(const uint8_t *data, uint16_t len)
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

/* ---- 外部 E2 接口（HAL 层提供） ---- */
extern HAL_Status_t E2_Read(uint32_t addr, uint8_t *buf, uint16_t len);
extern HAL_Status_t E2_Write(uint32_t addr, const uint8_t *buf, uint16_t len);

/* ============================================================
 *  公开 API
 * ============================================================ */

void TaskConfig_Save(void)
{
    uint8_t buf[5];          /**< 配置缓冲区：[version][bitmap0][bitmap1][bitmap2][crc] */
    buf[0] = TASK_CONFIG_VERSION;

    /* 打包任务使能位图（3 字节 × 8 位 = 24 位） */
    buf[1] = 0;
    buf[2] = 0;
    buf[3] = 0;
    for (uint8_t i = 0; i < TASK_COUNT && i < 24; i++) {
        if (TaskManager_IsEnabled(i)) {
            buf[1 + i / 8] |= (uint8_t)(1 << (i % 8));
        }
    }

    /* CRC-8 校验（覆盖 version + bitmap，不含 CRC 自身） */
    buf[4] = calc_crc8(buf, 4);
    E2_Write(TASK_CONFIG_E2_ADDR, buf, 5);
}

void TaskConfig_Load(void)
{
    uint8_t buf[5];

    /* 读取配置数据 */
    if (E2_Read(TASK_CONFIG_E2_ADDR, buf, 5) != HAL_OK) {
        return;  /* 读取失败，保持当前默认配置 */
    }

    /* 校验 CRC-8 */
    if (calc_crc8(buf, 4) != buf[4]) {
        return;  /* CRC 不匹配，保持当前默认配置 */
    }

    /* 校验版本号 */
    if (buf[0] != TASK_CONFIG_VERSION) {
        return;  /* 版本不匹配，保持当前默认配置 */
    }

    /* 恢复任务使能状态 */
    for (uint8_t i = 0; i < TASK_COUNT && i < 24; i++) {
        bool enabled = (bool)((buf[1 + i / 8] >> (i % 8)) & 0x01);
        if (enabled) {
            TaskManager_EnableTask(i);
        } else {
            TaskManager_DisableTask(i);
        }
    }
}
