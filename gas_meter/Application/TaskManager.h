/**
 * @file TaskManager.h
 * @brief 任务调度器公开接口 — 请求驱动 + FIFO 队列 + 三时间基准
 *
 * 本文件定义任务调度器的全部公开接口，包括：
 * - 三种时间基准枚举（10ms / 100ms / 1s）
 * - 三级睡眠模式（RUN / SNOOZE / STOP）
 * - 任务请求、使能/禁能、定时器订阅等 API
 *
 * 设计依据：规格文档 §4 — TaskManager 调度器设计
 */
#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#include "hal_common.h"

/* ---- 时间基准 ---- */
/**
 * @brief 时间基准枚举 — 所有定时需求映射到三种固定基准
 */
typedef enum {
    TIMEBASE_10MS  = 0,   /**< 10ms基准 — 消抖、阀门监控 */
    TIMEBASE_100MS = 1,   /**< 100ms基准 — 计量采样、LED闪烁 */
    TIMEBASE_1S    = 2,   /**< 1s基准 — 电源巡检、电池检测、延时上传 */
    TIMEBASE_COUNT = 3    /**< 基准数量（仅用于数组大小和循环边界） */
} TimeBase_t;

/* ---- 睡眠级别 ---- */
/**
 * @brief 睡眠级别枚举 — 无任务时 MCU 进入低功耗模式
 */
typedef enum {
    SLEEP_LEVEL_RUN    = 0,   /**< 全速运行，有任务正在执行 */
    SLEEP_LEVEL_SNOOZE = 1,   /**< 轻度睡眠，低功耗定时器运行（~5μA） */
    SLEEP_LEVEL_STOP   = 2    /**< 深度睡眠，仅 RTC 和中断可唤醒（~0.5μA） */
} SleepLevel_t;

/* ---- 定时器订阅句柄 ---- */
#define TIMER_HANDLE_INVALID  0xFF   /**< 无效的定时器订阅句柄 */

/* ---- 订阅池大小 ---- */
#define MAX_SUB_PER_BASE  8   /**< 每个时间基准最大订阅槽位数 */

/* ---- 核心 API ---- */

/**
 * @brief 初始化任务调度器
 *
 * 清零调度器状态，初始化任务描述符（从默认使能表加载），
 * 设置初始睡眠级别为 RUN。生产环境中还会初始化三基准硬件定时器。
 *
 * @note 必须在调用其他 TaskManager 函数之前调用。
 * @note 必须在 main() 中最先调用（在 TaskManager_Run 之前）。
 */
void TaskManager_Init(void);

/**
 * @brief 调度器主循环（生产环境入口）
 *
 * 无限循环执行：收集请求 → 执行队列任务 → 决策睡眠级别 → 进入睡眠。
 * 此函数永不返回。
 *
 * @note 仅在 main() 中调用，测试环境使用 TaskManager_RunOnce()
 */
void TaskManager_Run(void);

/**
 * @brief 请求调度某任务
 *
 * 将指定任务的请求标志置位，主循环下次迭代时执行其 process()。
 * 对未使能的任务调用此函数会被忽略。
 * 中断安全：可在 ISR 中调用。
 *
 * @param task_id 任务ID（TaskId_t 枚举值，0 ~ TASK_COUNT-1）
 */
void TaskManager_Request(uint8_t task_id);

/* ---- 配置 API ---- */

/**
 * @brief 开启指定任务
 *
 * 将任务设为使能状态，后续 TaskManager_Request 可生效。
 * 同时创建该任务关联的周期性定时器订阅（如有配置）。
 *
 * @param task_id 任务ID
 * @note 持久化到 E2（当前版本未实现）
 */
void TaskManager_EnableTask(uint8_t task_id);

/**
 * @brief 关闭指定任务
 *
 * 将任务设为禁能状态，后续 TaskManager_Request 被忽略。
 * 同时退订该任务的所有定时器订阅。
 *
 * @param task_id 任务ID
 * @note 持久化到 E2（当前版本未实现）
 */
void TaskManager_DisableTask(uint8_t task_id);

/**
 * @brief 查询指定任务是否开启
 * @param task_id 任务ID
 * @return true 任务已开启，false 任务已关闭或 task_id 无效
 */
bool TaskManager_IsEnabled(uint8_t task_id);

/* ---- 定时器 API ---- */

/**
 * @brief 在指定时间基准上订阅定时服务
 *
 * 订阅后，每 divider 个 tick 到期时自动调用
 * TaskManager_Request(task_id)。oneshot 模式到期后自动退订。
 *
 * @param base     时间基准（TIMEBASE_10MS / TIMEBASE_100MS / TIMEBASE_1S）
 * @param task_id  到期时请求调度的任务ID
 * @param divider  分频数（每 divider 个 tick 触发一次，必须 > 0）
 * @param oneshot  true = 一次性（到期自动退订），false = 周期性
 * @return 订阅句柄（0 ~ MAX_SUB_PER_BASE-1），TIMER_HANDLE_INVALID 表示失败
 *
 * @note 中断安全：ISR 中可调用（内部关中断保护订阅表）
 * @note 每个基准最多 MAX_SUB_PER_BASE(8) 个订阅者
 */
uint8_t Timer_Subscribe(TimeBase_t base, uint8_t task_id,
                         uint16_t divider, bool oneshot);

/**
 * @brief 退订定时器订阅
 *
 * 释放订阅槽位，之后该槽位可被重新订阅。
 *
 * @param base   时间基准
 * @param handle 订阅句柄（Timer_Subscribe 返回值）
 */
void Timer_Unsubscribe(TimeBase_t base, uint8_t handle);

/**
 * @brief 修改已订阅定时器的分频数
 *
 * 同时更新 divider 和 reload 值。下次到期时按新分频数触发。
 * 仅对活跃的订阅有效，对未激活的订阅调用无效。
 *
 * @param base         时间基准
 * @param handle       订阅句柄
 * @param new_divider  新分频数（必须 > 0）
 *
 * @note 用于阀门动作切换（开阀2s→关阀200ms）等场景
 */
void Timer_Resubscribe(TimeBase_t base, uint8_t handle,
                        uint16_t new_divider);

/* ---- 查询接口 ---- */

/**
 * @brief 获取当前睡眠级别
 * @return 当前睡眠级别（RUN / SNOOZE / STOP）
 * @note 主要用于测试，生产代码中由调度器内部使用
 */
SleepLevel_t TaskManager_GetSleepLevel(void);

/* ---- 测试支持 ---- */

/**
 * @brief 执行一次调度迭代（仅用于单元测试）
 *
 * 收集请求 → 执行就绪队列中的所有任务 → 返回。
 * 生产环境使用 TaskManager_Run() 的无限循环。
 */
void TaskManager_RunOnce(void);

/**
 * @brief 时间基准 tick 回调（由硬件 ISR 调用）
 *
 * 遍历指定基准的订阅表，将到期订阅对应的任务加入请求位图。
 * oneshot 订阅到期后自动退订，周期性订阅自动重载。
 *
 * @param base 时间基准（TIMEBASE_10MS / TIMEBASE_100MS / TIMEBASE_1S）
 * @note 由平台 HAL 定时器 ISR 调用，不要在主循环中直接调用
 */
void TaskManager_OnTimeBaseTick(TimeBase_t base);

#endif /* TASK_MANAGER_H */
