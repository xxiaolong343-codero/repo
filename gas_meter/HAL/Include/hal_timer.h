/**
 * @file hal_timer.h
 * @brief Timer hardware abstraction interface
 */
#ifndef HAL_TIMER_H
#define HAL_TIMER_H

#include "hal_common.h"

/* ---- HAL Timer API ---- */
void HAL_Timer_InitTimeBase10ms(void);    /* Start 10ms time base */
void HAL_Timer_InitTimeBase100ms(void);   /* Start 100ms time base */
void HAL_Timer_InitTimeBase1s(void);      /* Start 1s time base */

void HAL_Timer_StartLowPower(uint32_t interval_ms);  /* SNOOZE wakeup */
void HAL_Timer_StopLowPower(void);

void HAL_StartSnoozeMode(void);
void HAL_EnterStopMode(void);
void HAL_TriggerWakeup(void);

uint32_t HAL_GetTick(void);   /* ms since boot, for component internal timing */
uint32_t HAL_GetUsTick(void); /* microsecond tick for metering timestamps */

void HAL_SetRtcAlarm(uint32_t timestamp);
void HAL_ClearRtcAlarm(void);

#endif /* HAL_TIMER_H */
