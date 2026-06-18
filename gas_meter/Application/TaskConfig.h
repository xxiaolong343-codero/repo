/**
 * @file TaskConfig.h
 * @brief 任务配置持久化到 E2 — 使能位图的保存与加载
 *
 * 本文件提供任务使能状态的持久化接口：
 * - TaskConfig_Save(): 将当前任务使能位图写入 E2（含 CRC-8 校验）
 * - TaskConfig_Load(): 从 E2 读取并恢复任务使能状态
 *
 * 数据格式：[version:1B][bitmap:3B][crc8:1B] = 5 bytes
 * 设计依据：规格文档 §4.10
 */
#ifndef TASK_CONFIG_H
#define TASK_CONFIG_H

#include "hal_common.h"

#define TASK_CONFIG_VERSION  1   /**< 配置结构版本号 */

/**
 * @brief 保存当前任务使能位图到 E2
 *
 * 将所有 24 个任务的 enabled 状态打包为 3 字节位图，
 * 计算 CRC-8 校验值，写入 E2 配置区。
 *
 * @note 写入失败不阻塞系统运行
 */
void TaskConfig_Save(void);

/**
 * @brief 从 E2 加载任务使能位图并恢复状态
 *
 * 从 E2 读取配置数据，校验 CRC-8 和版本号。
 * 校验失败时保持当前默认使能状态不变。
 *
 * @note 校验失败时使用默认配置，不触发报警（由上层决定是否报警）
 */
void TaskConfig_Load(void);

#endif /* TASK_CONFIG_H */
