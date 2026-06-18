/**
 * @file TaskManager.h
 * @brief Task scheduler — request-driven + FIFO + three time bases
 */
#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#include "hal_common.h"

/* ---- Time bases ---- */
typedef enum {
    TIMEBASE_10MS  = 0,   /* 10ms tick  — debounce, valve monitor */
    TIMEBASE_100MS = 1,   /* 100ms tick — metering, LED */
    TIMEBASE_1S    = 2,   /* 1s tick    — power, battery, upload delay */
    TIMEBASE_COUNT = 3
} TimeBase_t;

/* ---- Sleep levels ---- */
typedef enum {
    SLEEP_LEVEL_RUN    = 0,   /* Full speed */
    SLEEP_LEVEL_SNOOZE = 1,   /* Low-power timer running */
    SLEEP_LEVEL_STOP   = 2    /* Only RTC + interrupts */
} SleepLevel_t;

/* ---- Timer subscription handle ---- */
#define TIMER_HANDLE_INVALID  0xFF

/* ---- Subscription pool size per time base ---- */
#define MAX_SUB_PER_BASE  8

/* ---- Core API ---- */
void     TaskManager_Init(void);
void     TaskManager_Run(void);             /* Never returns */
void     TaskManager_Request(uint8_t task_id);  /* ISR-safe */

/* ---- Config API ---- */
void     TaskManager_EnableTask(uint8_t task_id);
void     TaskManager_DisableTask(uint8_t task_id);
bool     TaskManager_IsEnabled(uint8_t task_id);

/* ---- Timer API ---- */
uint8_t  Timer_Subscribe(TimeBase_t base, uint8_t task_id,
                         uint16_t divider, bool oneshot);
void     Timer_Unsubscribe(TimeBase_t base, uint8_t handle);
void     Timer_Resubscribe(TimeBase_t base, uint8_t handle,
                           uint16_t new_divider);

/* ---- Sleep query (for testing) ---- */
SleepLevel_t TaskManager_GetSleepLevel(void);

#endif /* TASK_MANAGER_H */
