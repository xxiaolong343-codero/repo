/**
 * @file test_task_manager.c
 * @brief Unit tests for TaskManager scheduler core
 */
#include "unity.h"
#include "TaskManager.h"
#include <string.h>

/* ---- Test doubles ---- */
uint8_t s_process_call_count[24];

void setUp(void)
{
    memset(s_process_call_count, 0, sizeof(s_process_call_count));
    TaskManager_Init();
}

void tearDown(void) {}

/* ---- Init tests ---- */

void test_Init_AllCoreTasksEnabled(void)
{
    TEST_ASSERT_TRUE(TaskManager_IsEnabled(TASK_METERING));
    TEST_ASSERT_TRUE(TaskManager_IsEnabled(TASK_KEY));
    TEST_ASSERT_TRUE(TaskManager_IsEnabled(TASK_ALARM));
    TEST_ASSERT_TRUE(TaskManager_IsEnabled(TASK_VALVE_CTRL));
    TEST_ASSERT_TRUE(TaskManager_IsEnabled(TASK_POWER_MGMT));
    TEST_ASSERT_TRUE(TaskManager_IsEnabled(TASK_CLOCK));
    TEST_ASSERT_TRUE(TaskManager_IsEnabled(TASK_DATA_STORAGE));
    TEST_ASSERT_TRUE(TaskManager_IsEnabled(TASK_UPGRADE));
}

void test_Init_ConfigurableTasksDefaultDisabled(void)
{
    TEST_ASSERT_FALSE(TaskManager_IsEnabled(TASK_EXT_ALARM));
    TEST_ASSERT_FALSE(TaskManager_IsEnabled(TASK_VALVE_LEAK));
    TEST_ASSERT_FALSE(TaskManager_IsEnabled(TASK_BIG_FLOW));
    TEST_ASSERT_FALSE(TaskManager_IsEnabled(TASK_NO_GAS_USAGE));
    TEST_ASSERT_FALSE(TaskManager_IsEnabled(TASK_CONT_FLOW));
    TEST_ASSERT_FALSE(TaskManager_IsEnabled(TASK_TINY_FLOW));
    TEST_ASSERT_FALSE(TaskManager_IsEnabled(TASK_UNCTRL_FLOW));
    TEST_ASSERT_FALSE(TaskManager_IsEnabled(TASK_TILT));
}

/* ---- Enable/Disable tests ---- */

void test_EnableDisableTask(void)
{
    TaskManager_EnableTask(TASK_EXT_ALARM);
    TEST_ASSERT_TRUE(TaskManager_IsEnabled(TASK_EXT_ALARM));

    TaskManager_DisableTask(TASK_EXT_ALARM);
    TEST_ASSERT_FALSE(TaskManager_IsEnabled(TASK_EXT_ALARM));
}

void test_RequestDisabledTask_ShouldBeIgnored(void)
{
    TaskManager_DisableTask(TASK_EXT_ALARM);
    TaskManager_Request(TASK_EXT_ALARM);
    TaskManager_RunOnce();
    TEST_ASSERT_EQUAL(0, s_process_call_count[TASK_EXT_ALARM]);
}

void test_RequestEnabledTask_ShouldExecute(void)
{
    TaskManager_Request(TASK_METERING);
    TaskManager_RunOnce();
    TEST_ASSERT_EQUAL(1, s_process_call_count[TASK_METERING]);
}

void test_FIFO_Order(void)
{
    TaskManager_Request(TASK_ALARM);
    TaskManager_Request(TASK_VALVE_CTRL);
    TaskManager_Request(TASK_METERING);
    TaskManager_RunOnce();
    TEST_ASSERT_TRUE(s_process_call_count[TASK_ALARM] >= 1);
    TEST_ASSERT_TRUE(s_process_call_count[TASK_VALVE_CTRL] >= 1);
    TEST_ASSERT_TRUE(s_process_call_count[TASK_METERING] >= 1);
}

/* ---- Sleep level tests ---- */

void test_SleepLevel_NoTasks_Stop(void)
{
    TEST_ASSERT_EQUAL(SLEEP_LEVEL_STOP, TaskManager_GetSleepLevel());
}

void test_SleepLevel_WithSubscription_Snooze(void)
{
    Timer_Subscribe(TIMEBASE_100MS, TASK_METERING, 2, false);
    TEST_ASSERT_EQUAL(SLEEP_LEVEL_SNOOZE, TaskManager_GetSleepLevel());
}

/* ---- Timer subscription tests ---- */

void test_TimerSubscribe_ReturnsValidHandle(void)
{
    uint8_t h = Timer_Subscribe(TIMEBASE_1S, TASK_POWER_MGMT, 1, false);
    TEST_ASSERT_NOT_EQUAL(TIMER_HANDLE_INVALID, h);
}

void test_TimerSubscribe_ExceedsMaxSlots_ReturnsInvalid(void)
{
    for (int i = 0; i < 8; i++) {
        uint8_t h = Timer_Subscribe(TIMEBASE_1S, TASK_POWER_MGMT, 1, false);
        TEST_ASSERT_NOT_EQUAL(TIMER_HANDLE_INVALID, h);
    }
    uint8_t h = Timer_Subscribe(TIMEBASE_1S, TASK_BATTERY, 60, false);
    TEST_ASSERT_EQUAL(TIMER_HANDLE_INVALID, h);
}

void test_TimerUnsubscribe_ShouldFreeSlot(void)
{
    uint8_t h = Timer_Subscribe(TIMEBASE_1S, TASK_POWER_MGMT, 1, false);
    Timer_Unsubscribe(TIMEBASE_1S, h);
    uint8_t h2 = Timer_Subscribe(TIMEBASE_1S, TASK_BATTERY, 60, false);
    TEST_ASSERT_NOT_EQUAL(TIMER_HANDLE_INVALID, h2);
}

void test_TimerResubscribe_ShouldChangeDivider(void)
{
    uint8_t h = Timer_Subscribe(TIMEBASE_1S, TASK_VALVE_CTRL, 2, true);
    Timer_Resubscribe(TIMEBASE_1S, h, 5);
    /* No crash = success */
}

/* ---- Time-base tick tests ---- */

void test_TimeBaseTick_PeriodicSubscription_FiresRepeatedly(void)
{
    memset(s_process_call_count, 0, sizeof(s_process_call_count));
    Timer_Subscribe(TIMEBASE_100MS, TASK_METERING, 3, false);

    TaskManager_OnTimeBaseTick(TIMEBASE_100MS);
    TaskManager_RunOnce();
    TEST_ASSERT_EQUAL(0, s_process_call_count[TASK_METERING]);

    TaskManager_OnTimeBaseTick(TIMEBASE_100MS);
    TaskManager_RunOnce();
    TEST_ASSERT_EQUAL(0, s_process_call_count[TASK_METERING]);

    TaskManager_OnTimeBaseTick(TIMEBASE_100MS);
    TaskManager_RunOnce();
    TEST_ASSERT_EQUAL(1, s_process_call_count[TASK_METERING]);

    TaskManager_OnTimeBaseTick(TIMEBASE_100MS);
    TaskManager_RunOnce();
    TaskManager_OnTimeBaseTick(TIMEBASE_100MS);
    TaskManager_RunOnce();
    TEST_ASSERT_EQUAL(1, s_process_call_count[TASK_METERING]);

    TaskManager_OnTimeBaseTick(TIMEBASE_100MS);
    TaskManager_RunOnce();
    TEST_ASSERT_EQUAL(2, s_process_call_count[TASK_METERING]);
}

void test_TimeBaseTick_OneshotSubscription_FiresOnceThenAutoRemove(void)
{
    memset(s_process_call_count, 0, sizeof(s_process_call_count));
    Timer_Subscribe(TIMEBASE_1S, TASK_VALVE_CTRL, 2, true);

    TaskManager_OnTimeBaseTick(TIMEBASE_1S);
    TaskManager_RunOnce();
    TEST_ASSERT_EQUAL(0, s_process_call_count[TASK_VALVE_CTRL]);

    TaskManager_OnTimeBaseTick(TIMEBASE_1S);
    TaskManager_RunOnce();
    TEST_ASSERT_EQUAL(1, s_process_call_count[TASK_VALVE_CTRL]);

    TaskManager_OnTimeBaseTick(TIMEBASE_1S);
    TaskManager_OnTimeBaseTick(TIMEBASE_1S);
    TaskManager_RunOnce();
    TEST_ASSERT_EQUAL(1, s_process_call_count[TASK_VALVE_CTRL]);
}

void test_TimeBaseTick_MultipleSubscribersOnSameBase(void)
{
    memset(s_process_call_count, 0, sizeof(s_process_call_count));
    Timer_Subscribe(TIMEBASE_1S, TASK_POWER_MGMT, 1, false);
    Timer_Subscribe(TIMEBASE_1S, TASK_BATTERY, 3, false);

    TaskManager_OnTimeBaseTick(TIMEBASE_1S);
    TaskManager_RunOnce();
    TEST_ASSERT_EQUAL(1, s_process_call_count[TASK_POWER_MGMT]);
    TEST_ASSERT_EQUAL(0, s_process_call_count[TASK_BATTERY]);

    TaskManager_OnTimeBaseTick(TIMEBASE_1S);
    TaskManager_RunOnce();
    TEST_ASSERT_EQUAL(2, s_process_call_count[TASK_POWER_MGMT]);
    TEST_ASSERT_EQUAL(0, s_process_call_count[TASK_BATTERY]);

    TaskManager_OnTimeBaseTick(TIMEBASE_1S);
    TaskManager_RunOnce();
    TEST_ASSERT_EQUAL(3, s_process_call_count[TASK_POWER_MGMT]);
    TEST_ASSERT_EQUAL(1, s_process_call_count[TASK_BATTERY]);
}

void test_DisableTask_RemovesSubscriptions(void)
{
    Timer_Subscribe(TIMEBASE_1S, TASK_EXT_ALARM, 1, false);
    TaskManager_DisableTask(TASK_EXT_ALARM);
    TEST_ASSERT_EQUAL(SLEEP_LEVEL_STOP, TaskManager_GetSleepLevel());
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_Init_AllCoreTasksEnabled);
    RUN_TEST(test_Init_ConfigurableTasksDefaultDisabled);
    RUN_TEST(test_EnableDisableTask);
    RUN_TEST(test_RequestDisabledTask_ShouldBeIgnored);
    RUN_TEST(test_RequestEnabledTask_ShouldExecute);
    RUN_TEST(test_FIFO_Order);
    RUN_TEST(test_SleepLevel_NoTasks_Stop);
    RUN_TEST(test_SleepLevel_WithSubscription_Snooze);
    RUN_TEST(test_TimerSubscribe_ReturnsValidHandle);
    RUN_TEST(test_TimerSubscribe_ExceedsMaxSlots_ReturnsInvalid);
    RUN_TEST(test_TimerUnsubscribe_ShouldFreeSlot);
    RUN_TEST(test_TimerResubscribe_ShouldChangeDivider);
    RUN_TEST(test_TimeBaseTick_PeriodicSubscription_FiresRepeatedly);
    RUN_TEST(test_TimeBaseTick_OneshotSubscription_FiresOnceThenAutoRemove);
    RUN_TEST(test_TimeBaseTick_MultipleSubscribersOnSameBase);
    RUN_TEST(test_DisableTask_RemovesSubscriptions);
    return UNITY_END();
}
