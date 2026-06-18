/**
 * @file TaskManager.c
 * @brief 请求驱动的任务调度器实现 — FIFO 就绪队列 + 三时间基准 + 分级睡眠
 *
 * 本文件实现任务调度器的全部核心逻辑，包括：
 * - 请求位图 → FIFO 就绪队列的收集机制
 * - 三基准定时器订阅/退订/重订阅管理
 * - RUN / SNOOZE / STOP 三级睡眠决策
 * - 原子 request_bitmap 读-清零（中断安全）
 * - 时间基准 tick 处理（oneshot 自动退订）
 *
 * 设计依据：规格文档 §4 — TaskManager 调度器设计
 */
#include "TaskManager.h"
#include <string.h>

/* ============================================================
 *  内部数据结构
 * ============================================================ */

/**
 * @brief 任务描述符 — 单个任务的全部管理信息
 */
typedef struct {
    uint8_t  task_id;           /**< 任务ID（对应 TaskId_t 枚举） */
    bool     enabled;           /**< 配置使能状态（true=可被请求调度） */
    void   (*process)(void);    /**< 任务处理函数指针（组件实现） */
} TaskDesc_t;

/**
 * @brief 定时器订阅槽 — 单个任务在某个时间基准上的订阅信息
 */
typedef struct {
    bool     active;            /**< 是否激活（未被占用时可重新分配） */
    bool     oneshot;           /**< true=一次性（到期自动退订），false=周期性 */
    uint8_t  task_id;           /**< 到期时 Request 的目标任务ID */
    uint16_t divider;           /**< 当前剩余 tick 数（减到 0 触发） */
    uint16_t reload;            /**< 重载值（周期性订阅使用，oneshot 不使用） */
} TimerSub_t;

/**
 * @brief FIFO 就绪队列 — 被请求但尚未执行的任务 ID 列表
 */
typedef struct {
    uint8_t items[TASK_COUNT];  /**< 队列元素（任务ID数组，固定大小无动态分配） */
    uint8_t head;               /**< 队头索引（pop 位置） */
    uint8_t tail;               /**< 队尾索引（push 位置） */
    uint8_t count;              /**< 当前队列元素数 */
} ReadyQueue_t;

/**
 * @brief 调度器总状态 — 包含所有运行时数据
 */
typedef struct {
    volatile uint32_t request_bitmap;  /**< 请求位图（ISR 原子写，主循环原子读-清零） */
    ReadyQueue_t      ready_queue;     /**< FIFO 就绪队列 */
    TaskDesc_t        tasks[TASK_COUNT];   /**< 任务描述符表（索引即 task_id） */
    TimerSub_t        subs[TIMEBASE_COUNT][MAX_SUB_PER_BASE];  /**< 三基准 × 8 槽订阅表 */
    volatile SleepLevel_t sleep_level;  /**< 当前睡眠级别 */
} Scheduler_t;

/* ============================================================
 *  静态实例
 * ============================================================ */

/** @brief 调度器全局单例 */
static Scheduler_t g_sched;

/* ============================================================
 *  任务处理函数前向声明（由各组件实现，链接时解析）
 * ============================================================ */
extern void Metering_Process(void);
extern void Key_Process(void);
extern void Alarm_Process(void);
extern void ValveControl_Process(void);
extern void HMI_Process(void);
extern void Billing_Process(void);
extern void Communication_Process(void);
extern void Infrared_Process(void);
extern void DataStorage_Process(void);
extern void E2Storage_Process(void);
extern void EventLog_Process(void);
extern void UsageLog_Process(void);
extern void ExtAlarm_Process(void);
extern void ValveLeak_Process(void);
extern void BigFlow_Process(void);
extern void NoGasUsage_Process(void);
extern void ContFlow_Process(void);
extern void TinyFlow_Process(void);
extern void UnctrlFlow_Process(void);
extern void Tilt_Process(void);
extern void Upgrade_Process(void);
extern void PowerMgmt_Process(void);
extern void Battery_Process(void);
extern void Clock_Process(void);

/* ============================================================
 *  默认任务使能表（规格 §3.2 / §3.3）
 *  核心组件始终开启，可配置组件默认关闭
 * ============================================================ */
static const bool k_default_enabled[TASK_COUNT] = {
    /* 0  METERING     */ true,
    /* 1  KEY          */ true,
    /* 2  ALARM        */ true,
    /* 3  VALVE_CTRL   */ true,
    /* 4  HMI          */ true,
    /* 5  BILLING      */ true,   /* 模式决定，默认开启 */
    /* 6  COMM         */ true,
    /* 7  INFRARED     */ true,
    /* 8  DATA_STORAGE */ true,
    /* 9  E2_STORAGE   */ true,
    /* 10 EVENT_LOG    */ true,
    /* 11 USAGE_LOG    */ true,
    /* 12 EXT_ALARM    */ false,
    /* 13 VALVE_LEAK   */ false,
    /* 14 BIG_FLOW     */ false,
    /* 15 NO_GAS_USAGE */ false,
    /* 16 CONT_FLOW    */ false,
    /* 17 TINY_FLOW    */ false,
    /* 18 UNCTRL_FLOW  */ false,
    /* 19 TILT         */ false,
    /* 20 UPGRADE      */ true,
    /* 21 POWER_MGMT   */ true,
    /* 22 BATTERY      */ true,
    /* 23 CLOCK        */ true
};

/**
 * @brief 任务处理函数表 — 按 task_id 索引
 *
 * 初始化和运行时不修改，为 const 以避免意外覆盖。
 * 所有 24 个组件通过 extern 前向声明链接。
 */
static void (*const k_process_func[TASK_COUNT])(void) = {
    Metering_Process,   Key_Process,         Alarm_Process,
    ValveControl_Process, HMI_Process,        Billing_Process,
    Communication_Process, Infrared_Process,  DataStorage_Process,
    E2Storage_Process,  EventLog_Process,    UsageLog_Process,
    ExtAlarm_Process,   ValveLeak_Process,   BigFlow_Process,
    NoGasUsage_Process, ContFlow_Process,    TinyFlow_Process,
    UnctrlFlow_Process, Tilt_Process,        Upgrade_Process,
    PowerMgmt_Process,  Battery_Process,     Clock_Process
};

/* ============================================================
 *  就绪队列内部操作
 * ============================================================ */

/**
 * @brief 将任务 ID 推入 FIFO 就绪队列尾部
 * @param q       就绪队列指针
 * @param task_id 任务 ID
 * @note 队列满时会丢弃（保护机制，正常情况下 TASK_COUNT 足够）
 */
static void queue_push(ReadyQueue_t *q, uint8_t task_id)
{
    if (q->count >= TASK_COUNT) {
        return;  /* 队列已满，丢弃（防御性编程） */
    }
    q->items[q->tail] = task_id;
    q->tail = (uint8_t)((q->tail + 1) % TASK_COUNT);
    q->count++;
}

/**
 * @brief 从 FIFO 就绪队列头部取出任务 ID
 * @param q       就绪队列指针
 * @param task_id 输出参数，接收取出的任务 ID
 * @return true 成功取出，false 队列为空
 */
static bool queue_pop(ReadyQueue_t *q, uint8_t *task_id)
{
    if (q->count == 0) {
        return false;
    }
    *task_id = q->items[q->head];
    q->head = (uint8_t)((q->head + 1) % TASK_COUNT);
    q->count--;
    return true;
}

/* ============================================================
 *  请求收集
 * ============================================================ */

/**
 * @brief 从请求位图收集请求到就绪队列（原子读-清零）
 *
 * 关中断保护读写之间的竞态窗口，保证不丢失 ISR 设置的请求位。
 * 使用 __builtin_ctz 按位提取，逐位清空，O(设置位数) 复杂度。
 */
static void collect_requests(void)
{
    uint32_t bitmap;

    /* 原子读-清零：关中断保证读和清零之间不丢 ISR 请求 */
    uint8_t saved_ie = HAL_DisableInterrupts();
    bitmap = g_sched.request_bitmap;
    g_sched.request_bitmap = 0;
    HAL_RestoreInterrupts(saved_ie);

    while (bitmap != 0) {
        uint8_t task_id = (uint8_t)__builtin_ctz(bitmap);
        queue_push(&g_sched.ready_queue, task_id);
        bitmap &= bitmap - 1;  /* 清除最低设置位 */
    }
}

/* ============================================================
 *  睡眠级别决策
 *  信息源：就绪队列、订阅表、RTC 闹钟（未来集成）
 *  spec §4.9.2
 * ============================================================ */

/**
 * @brief 决策当前应进入的睡眠级别
 *
 * 决策逻辑：
 *   1. 就绪队列不空 → RUN
 *   2. 有短周期活跃订阅(10ms/100ms) → SNOOZE
 *   3. 仅有长周期订阅(1s)或无订阅 → STOP
 *
 * @return 睡眠级别（RUN / SNOOZE / STOP）
 */
static SleepLevel_t decide_sleep_level(void)
{
    /* 有被请求的任务 → RUN */
    if (g_sched.ready_queue.count > 0) {
        return SLEEP_LEVEL_RUN;
    }

    bool has_short = false;
    bool has_long  = false;

    /* 检查三个基准的订阅表 */
    for (uint8_t i = 0; i < MAX_SUB_PER_BASE; i++) {
        if (g_sched.subs[TIMEBASE_10MS][i].active)  has_short = true;
        if (g_sched.subs[TIMEBASE_100MS][i].active) has_short = true;
        if (g_sched.subs[TIMEBASE_1S][i].active)    has_long  = true;
    }

    if (!has_short && !has_long) {
        return SLEEP_LEVEL_STOP;  /* 无任何定时需求 → STOP */
    }

    /* 有短周期订阅 → SNOOZE；仅长周期 → STOP */
    if (has_short) {
        return SLEEP_LEVEL_SNOOZE;
    }
    return SLEEP_LEVEL_STOP;
}

/* ============================================================
 *  公开 API
 * ============================================================ */

void TaskManager_Init(void)
{
    memset(&g_sched, 0, sizeof(g_sched));
    g_sched.sleep_level = SLEEP_LEVEL_RUN;

    /* 从默认使能表初始化任务描述符 */
    for (uint8_t i = 0; i < TASK_COUNT; i++) {
        g_sched.tasks[i].task_id = i;
        g_sched.tasks[i].enabled = k_default_enabled[i];
        g_sched.tasks[i].process = k_process_func[i];
    }
}

void TaskManager_Request(uint8_t task_id)
{
    if (task_id >= TASK_COUNT) {
        return;
    }
    if (!g_sched.tasks[task_id].enabled) {
        return;  /* 未使能的任务忽略请求 */
    }

    /* 位设置操作在 RL78 上为原子，无需关中断 */
    g_sched.request_bitmap |= (1UL << task_id);

    if (g_sched.sleep_level != SLEEP_LEVEL_RUN) {
        /* 实际硬件：调用 HAL_TriggerWakeup() 唤醒 MCU */
    }
}

void TaskManager_EnableTask(uint8_t task_id)
{
    if (task_id >= TASK_COUNT) {
        return;
    }
    g_sched.tasks[task_id].enabled = true;
}

void TaskManager_DisableTask(uint8_t task_id)
{
    if (task_id >= TASK_COUNT) {
        return;
    }
    g_sched.tasks[task_id].enabled = false;

    /* 退订该任务的所有订阅 */
    for (uint8_t b = 0; b < TIMEBASE_COUNT; b++) {
        for (uint8_t i = 0; i < MAX_SUB_PER_BASE; i++) {
            if (g_sched.subs[b][i].active &&
                g_sched.subs[b][i].task_id == task_id) {
                g_sched.subs[b][i].active = false;
            }
        }
    }
}

bool TaskManager_IsEnabled(uint8_t task_id)
{
    if (task_id >= TASK_COUNT) {
        return false;
    }
    return g_sched.tasks[task_id].enabled;
}

/* ---- 定时器订阅 ---- */

uint8_t Timer_Subscribe(TimeBase_t base, uint8_t task_id,
                        uint16_t divider, bool oneshot)
{
    /* 参数校验 */
    if (base >= TIMEBASE_COUNT || task_id >= TASK_COUNT || divider == 0) {
        return TIMER_HANDLE_INVALID;
    }

    uint8_t saved_ie = HAL_DisableInterrupts();

    /* 在指定基准的订阅表中查找空闲槽位 */
    for (uint8_t i = 0; i < MAX_SUB_PER_BASE; i++) {
        TimerSub_t *p = &g_sched.subs[base][i];
        if (!p->active) {
            p->active   = true;
            p->oneshot  = oneshot;
            p->task_id  = task_id;
            p->divider  = divider;
            p->reload   = divider;
            HAL_RestoreInterrupts(saved_ie);
            return i;
        }
    }

    HAL_RestoreInterrupts(saved_ie);
    return TIMER_HANDLE_INVALID;  /* 该基准已满 */
}

void Timer_Unsubscribe(TimeBase_t base, uint8_t handle)
{
    if (base >= TIMEBASE_COUNT || handle >= MAX_SUB_PER_BASE) {
        return;
    }
    g_sched.subs[base][handle].active = false;
}

void Timer_Resubscribe(TimeBase_t base, uint8_t handle, uint16_t new_divider)
{
    if (base >= TIMEBASE_COUNT || handle >= MAX_SUB_PER_BASE) {
        return;
    }
    if (new_divider == 0) {
        return;
    }
    TimerSub_t *p = &g_sched.subs[base][handle];
    if (!p->active) {
        return;
    }
    p->divider = new_divider;
    p->reload  = new_divider;
}

SleepLevel_t TaskManager_GetSleepLevel(void)
{
    return decide_sleep_level();
}

/* ---- 测试：单次迭代 ---- */
void TaskManager_RunOnce(void)
{
    collect_requests();

    uint8_t task_id;
    while (queue_pop(&g_sched.ready_queue, &task_id)) {
        if (g_sched.tasks[task_id].enabled && g_sched.tasks[task_id].process) {
            g_sched.tasks[task_id].process();
        }
    }
}

/* ---- 生产：主循环 ---- */
void TaskManager_Run(void)
{
    while (1) {
        collect_requests();

        /* 执行就绪队列中所有被请求的任务（FIFO 顺序） */
        uint8_t task_id;
        while (queue_pop(&g_sched.ready_queue, &task_id)) {
            if (g_sched.tasks[task_id].enabled &&
                g_sched.tasks[task_id].process) {
                g_sched.tasks[task_id].process();
            }
        }

        /* 决策睡眠级别并进入低功耗模式 */
        SleepLevel_t level = decide_sleep_level();
        g_sched.sleep_level = level;

        switch (level) {
        case SLEEP_LEVEL_RUN:
            break;  /* 有任务待执行，保持运行 */
        case SLEEP_LEVEL_SNOOZE:
            /* TODO: 实际硬件：HAL_StartLowPowerTimer(...); HAL_EnterSnoozeMode(); */
            break;
        case SLEEP_LEVEL_STOP:
            /* TODO: 实际硬件：HAL_EnterStopMode(); */
            break;
        default:
            break;
        }

        g_sched.sleep_level = SLEEP_LEVEL_RUN;
    }
}

/* ============================================================
 *  时间基准 tick 处理（由平台 HAL 定时器 ISR 调用）
 *  spec §4.7.3
 * ============================================================ */
void TaskManager_OnTimeBaseTick(TimeBase_t base)
{
    if (base >= TIMEBASE_COUNT) {
        return;
    }

    TimerSub_t *p = g_sched.subs[base];
    for (uint8_t i = 0; i < MAX_SUB_PER_BASE; i++) {
        if (!p[i].active) {
            continue;  /* 跳过空闲槽位 */
        }

        p[i].divider--;
        if (p[i].divider == 0) {
            /* 订阅到期：请求调度对应任务 */
            TaskManager_Request(p[i].task_id);

            if (p[i].oneshot) {
                p[i].active = false;        /* 一次性：自动退订 */
            } else {
                p[i].divider = p[i].reload; /* 周期性：重载分频数 */
            }
        }
    }
}
