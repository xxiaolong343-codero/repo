/**
 * @file hal_timer_rl78.c
 * @brief RL78 Timer driver — TAU channels for three time bases
 *
 * TAU0 → TIMEBASE_10MS  (10ms interval)
 * TAU1 → TIMEBASE_100MS (100ms interval)
 * TAU2 → TIMEBASE_1S    (1s interval)
 * Low-power interval timer → SNOOZE wakeup
 * RTC → STOP mode wakeup
 */
#include "hal_timer.h"
#include "TaskManager.h"
#include "hal_common.h"

#if PLATFORM_RL78

void HAL_Timer_InitTimeBase10ms(void)
{
    /* TODO: Configure TAU0 channel as interval timer, 10ms period */
}

void HAL_Timer_InitTimeBase100ms(void)
{
    /* TODO: Configure TAU1 channel as interval timer, 100ms period */
}

void HAL_Timer_InitTimeBase1s(void)
{
    /* TODO: Configure TAU2 channel as interval timer, 1s period */
}

void HAL_Timer_StartLowPower(uint32_t interval_ms)
{
    (void)interval_ms;
    /* TODO: Configure low-power interval timer (15kHz source) */
}

void HAL_Timer_StopLowPower(void)
{
    /* TODO: Stop low-power interval timer */
}

void HAL_StartSnoozeMode(void)
{
    /* TODO: Enter RL78 SNOOZE mode (HALT with sub-clock) */
}

void HAL_EnterStopMode(void)
{
    /* TODO: Enter RL78 STOP mode */
}

void HAL_TriggerWakeup(void)
{
    /* TODO: Software trigger to exit low-power mode */
}

uint32_t HAL_GetTick(void)
{
    /* TODO: Return ms counter incremented by 1ms TAU interrupt */
    return 0;
}

uint32_t HAL_GetUsTick(void)
{
    /* TODO: Return microsecond counter from TAU capture channel */
    return 0;
}

void HAL_SetRtcAlarm(uint32_t timestamp)
{
    (void)timestamp;
    /* TODO: Set RTC alarm register */
}

void HAL_ClearRtcAlarm(void)
{
    /* TODO: Clear RTC alarm */
}

/* ---- ISR: TAU0 (10ms) ---- */
#pragma interrupt INT_TAU0_0
static void ISR_TAU0_0(void)
{
    TaskManager_OnTimeBaseTick(TIMEBASE_10MS);
}

/* ---- ISR: TAU1 (100ms) ---- */
#pragma interrupt INT_TAU1_0
static void ISR_TAU1_0(void)
{
    TaskManager_OnTimeBaseTick(TIMEBASE_100MS);
}

/* ---- ISR: TAU2 (1s) ---- */
#pragma interrupt INT_TAU2_0
static void ISR_TAU2_0(void)
{
    TaskManager_OnTimeBaseTick(TIMEBASE_1S);
}

#endif /* PLATFORM_RL78 */
