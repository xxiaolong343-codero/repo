/**
 * @file TaskManager.c
 * @brief Request-driven task scheduler with FIFO queue and three time bases
 *
 * Design: spec §4 — TaskManager调度器设计
 */
#include "TaskManager.h"
#include <string.h>

/* ============================================================
 *  Internal data structures
 * ============================================================ */

/* Task descriptor */
typedef struct {
    uint8_t  task_id;
    bool     enabled;
    void   (*process)(void);
} TaskDesc_t;

/* Timer subscription slot */
typedef struct {
    bool     active;
    bool     oneshot;
    uint8_t  task_id;
    uint16_t divider;    /* Current countdown */
    uint16_t reload;     /* Reload value (periodic) */
} TimerSub_t;

/* FIFO ready queue */
typedef struct {
    uint8_t items[TASK_COUNT];
    uint8_t head;
    uint8_t tail;
    uint8_t count;
} ReadyQueue_t;

/* Scheduler state */
typedef struct {
    volatile uint32_t request_bitmap;
    ReadyQueue_t      ready_queue;
    TaskDesc_t        tasks[TASK_COUNT];
    TimerSub_t        subs[TIMEBASE_COUNT][MAX_SUB_PER_BASE];
    volatile SleepLevel_t sleep_level;
} Scheduler_t;

/* ============================================================
 *  Static instance
 * ============================================================ */
static Scheduler_t g_sched;

/* ============================================================
 *  Forward declarations for task process functions
 *  (implemented by each component, linked at build time)
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
 *  Default task enable table (spec §3.2/§3.3)
 * ============================================================ */
static const bool kDefaultEnabled[TASK_COUNT] = {
    /* 0  METERING     */ true,
    /* 1  KEY          */ true,
    /* 2  ALARM        */ true,
    /* 3  VALVE_CTRL   */ true,
    /* 4  HMI          */ true,
    /* 5  BILLING      */ true,   /* Mode decides, default true */
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

/* Process function table */
static void (*const kProcessFunc[TASK_COUNT])(void) = {
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
 *  Ready queue helpers
 * ============================================================ */
static void Queue_Push(ReadyQueue_t *q, uint8_t task_id)
{
    if (q->count >= TASK_COUNT) {
        return;  /* Overflow guard */
    }
    q->items[q->tail] = task_id;
    q->tail = (uint8_t)((q->tail + 1) % TASK_COUNT);
    q->count++;
}

static bool Queue_Pop(ReadyQueue_t *q, uint8_t *task_id)
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
 *  Collect requests from bitmap into FIFO queue
 * ============================================================ */
static void CollectRequests(void)
{
    uint32_t bitmap;

    /* Atomic read-and-clear */
    uint8_t saved_ie = HAL_DisableInterrupts();
    bitmap = g_sched.request_bitmap;
    g_sched.request_bitmap = 0;
    HAL_RestoreInterrupts(saved_ie);

    while (bitmap != 0) {
        uint8_t task_id = (uint8_t)__builtin_ctz(bitmap);
        Queue_Push(&g_sched.ready_queue, task_id);
        bitmap &= bitmap - 1;  /* Clear lowest set bit */
    }
}

/* ============================================================
 *  Sleep level decision (spec §4.9.2)
 * ============================================================ */
static SleepLevel_t DecideSleepLevel(void)
{
    if (g_sched.ready_queue.count > 0) {
        return SLEEP_LEVEL_RUN;
    }

    bool has_short = false;
    bool has_long  = false;

    for (uint8_t i = 0; i < MAX_SUB_PER_BASE; i++) {
        if (g_sched.subs[TIMEBASE_10MS][i].active)  has_short = true;
        if (g_sched.subs[TIMEBASE_100MS][i].active) has_short = true;
        if (g_sched.subs[TIMEBASE_1S][i].active)    has_long  = true;
    }

    if (!has_short && !has_long) {
        return SLEEP_LEVEL_STOP;
    }
    if (has_short) {
        return SLEEP_LEVEL_SNOOZE;
    }
    return SLEEP_LEVEL_STOP;
}

/* ============================================================
 *  Public API
 * ============================================================ */

void TaskManager_Init(void)
{
    memset(&g_sched, 0, sizeof(g_sched));
    g_sched.sleep_level = SLEEP_LEVEL_RUN;

    /* Initialize task descriptors */
    for (uint8_t i = 0; i < TASK_COUNT; i++) {
        g_sched.tasks[i].task_id = i;
        g_sched.tasks[i].enabled = kDefaultEnabled[i];
        g_sched.tasks[i].process = kProcessFunc[i];
    }
}

void TaskManager_Request(uint8_t task_id)
{
    if (task_id >= TASK_COUNT) {
        return;
    }
    if (!g_sched.tasks[task_id].enabled) {
        return;
    }

    /* Bit set is atomic on RL78; for portability use bitmap */
    g_sched.request_bitmap |= (1UL << task_id);

    if (g_sched.sleep_level != SLEEP_LEVEL_RUN) {
        /* On real hardware: HAL_TriggerWakeup(); */
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

    /* Unsubscribe all subscriptions for this task */
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

/* ---- Timer subscription ---- */

uint8_t Timer_Subscribe(TimeBase_t base, uint8_t task_id,
                        uint16_t divider, bool oneshot)
{
    if (base >= TIMEBASE_COUNT || task_id >= TASK_COUNT || divider == 0) {
        return TIMER_HANDLE_INVALID;
    }

    uint8_t saved_ie = HAL_DisableInterrupts();

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
    return TIMER_HANDLE_INVALID;
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
    return DecideSleepLevel();
}

/* ---- Single iteration (for testing) ---- */
void TaskManager_RunOnce(void)
{
    CollectRequests();

    uint8_t task_id;
    while (Queue_Pop(&g_sched.ready_queue, &task_id)) {
        if (g_sched.tasks[task_id].enabled && g_sched.tasks[task_id].process) {
            g_sched.tasks[task_id].process();
        }
    }
}

/* ---- Main loop (production) ---- */
void TaskManager_Run(void)
{
    while (1) {
        CollectRequests();

        /* Execute all queued tasks */
        uint8_t task_id;
        while (Queue_Pop(&g_sched.ready_queue, &task_id)) {
            if (g_sched.tasks[task_id].enabled &&
                g_sched.tasks[task_id].process) {
                g_sched.tasks[task_id].process();
            }
        }

        /* Sleep decision */
        SleepLevel_t level = DecideSleepLevel();
        g_sched.sleep_level = level;

        switch (level) {
        case SLEEP_LEVEL_RUN:
            break;
        case SLEEP_LEVEL_SNOOZE:
            /* HAL_StartLowPowerTimer(...); HAL_EnterSnoozeMode(); */
            break;
        case SLEEP_LEVEL_STOP:
            /* HAL_EnterStopMode(); */
            break;
        default:
            break;
        }

        g_sched.sleep_level = SLEEP_LEVEL_RUN;
    }
}

/* ============================================================
 *  Time-base ISR handlers (called from hardware ISR)
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
            continue;
        }

        p[i].divider--;
        if (p[i].divider == 0) {
            TaskManager_Request(p[i].task_id);

            if (p[i].oneshot) {
                p[i].active = false;
            } else {
                p[i].divider = p[i].reload;
            }
        }
    }
}
