/**
 * @file test_integration.c
 * @brief Integration test — simulates a typical gas meter system cycle
 */
#include "unity.h"
#include "TaskManager.h"
#include "IOManager.h"
#include <string.h>

/* ---- Task process call tracking ---- */
static uint8_t s_calls[24];
#define STUB(name, id) void name(void) { s_calls[id]++; }

STUB(Metering_Process,    TASK_METERING)
STUB(Key_Process,         TASK_KEY)
STUB(Alarm_Process,       TASK_ALARM)
STUB(ValveControl_Process,TASK_VALVE_CTRL)
STUB(HMI_Process,         TASK_HMI)
STUB(Billing_Process,     TASK_BILLING)
STUB(Communication_Process,TASK_COMM)
STUB(Infrared_Process,    TASK_INFRARED)
STUB(DataStorage_Process, TASK_DATA_STORAGE)
STUB(E2Storage_Process,   TASK_E2_STORAGE)
STUB(EventLog_Process,    TASK_EVENT_LOG)
STUB(UsageLog_Process,    TASK_USAGE_LOG)
STUB(ExtAlarm_Process,    TASK_EXT_ALARM)
STUB(ValveLeak_Process,   TASK_VALVE_LEAK)
STUB(BigFlow_Process,     TASK_BIG_FLOW)
STUB(NoGasUsage_Process,  TASK_NO_GAS_USAGE)
STUB(ContFlow_Process,    TASK_CONT_FLOW)
STUB(TinyFlow_Process,    TASK_TINY_FLOW)
STUB(UnctrlFlow_Process,  TASK_UNCTRL_FLOW)
STUB(Tilt_Process,        TASK_TILT)
STUB(Upgrade_Process,     TASK_UPGRADE)
STUB(PowerMgmt_Process,   TASK_POWER_MGMT)
STUB(Battery_Process,     TASK_BATTERY)
STUB(Clock_Process,       TASK_CLOCK)

/* ---- HAL stubs ---- */
void HAL_GPIO_Init(uint8_t pin, uint8_t mode, uint8_t pull) { (void)pin; (void)mode; (void)pull; }
void HAL_GPIO_Write(uint8_t pin, uint8_t level) { (void)pin; (void)level; }
uint8_t HAL_GPIO_Read(uint8_t pin) { (void)pin; return 0; }
void HAL_GPIO_Toggle(uint8_t pin) { (void)pin; }

/* E2 stubs */
HAL_Status_t E2_Read(uint32_t a, uint8_t *b, uint16_t l) { (void)a;(void)b;(void)l; return HAL_OK; }
HAL_Status_t E2_Write(uint32_t a, const uint8_t *b, uint16_t l) { (void)a;(void)b;(void)l; return HAL_OK; }

/* IOConfig stubs */
const IOPinConfig_t g_usedPins[] = {};
const uint16_t g_usedPinCount = 0;
const IOPinConfig_t g_unusedPins[] = {};
const uint16_t g_unusedPinCount = 0;

void setUp(void)    { memset(s_calls, 0, sizeof(s_calls)); TaskManager_Init(); }
void tearDown(void) {}

/* ---- Scenario: Pulse interrupt → Metering → Alarm → ValveControl ---- */
void test_Scenario_PulseTriggersMeteringAlarmValve(void)
{
    TaskManager_Request(TASK_METERING);
    TaskManager_RunOnce();
    TEST_ASSERT_EQUAL(1, s_calls[TASK_METERING]);

    TaskManager_Request(TASK_ALARM);
    TaskManager_RunOnce();
    TEST_ASSERT_EQUAL(1, s_calls[TASK_ALARM]);

    TaskManager_Request(TASK_VALVE_CTRL);
    TaskManager_RunOnce();
    TEST_ASSERT_EQUAL(1, s_calls[TASK_VALVE_CTRL]);
}

/* ---- Scenario: Periodic metering via time-base ---- */
void test_Scenario_PeriodicMeteringViaTimeBase(void)
{
    Timer_Subscribe(TIMEBASE_100MS, TASK_METERING, 2, false);

    TaskManager_OnTimeBaseTick(TIMEBASE_100MS);
    TaskManager_RunOnce();
    TEST_ASSERT_EQUAL(0, s_calls[TASK_METERING]);

    TaskManager_OnTimeBaseTick(TIMEBASE_100MS);
    TaskManager_RunOnce();
    TEST_ASSERT_EQUAL(1, s_calls[TASK_METERING]);

    TaskManager_OnTimeBaseTick(TIMEBASE_100MS);
    TaskManager_RunOnce();
    TEST_ASSERT_EQUAL(1, s_calls[TASK_METERING]);

    TaskManager_OnTimeBaseTick(TIMEBASE_100MS);
    TaskManager_RunOnce();
    TEST_ASSERT_EQUAL(2, s_calls[TASK_METERING]);
}

/* ---- Scenario: Enable/disable safety detection task ---- */
void test_Scenario_EnableDisableExtAlarm(void)
{
    TaskManager_Request(TASK_EXT_ALARM);
    TaskManager_RunOnce();
    TEST_ASSERT_EQUAL(0, s_calls[TASK_EXT_ALARM]);

    TaskManager_EnableTask(TASK_EXT_ALARM);
    TaskManager_Request(TASK_EXT_ALARM);
    TaskManager_RunOnce();
    TEST_ASSERT_EQUAL(1, s_calls[TASK_EXT_ALARM]);

    TaskManager_DisableTask(TASK_EXT_ALARM);
    TaskManager_Request(TASK_EXT_ALARM);
    TaskManager_RunOnce();
    TEST_ASSERT_EQUAL(1, s_calls[TASK_EXT_ALARM]);
}

/* ---- Scenario: Sleep level transitions ---- */
void test_Scenario_SleepLevelTransitions(void)
{
    TEST_ASSERT_EQUAL(SLEEP_LEVEL_STOP, TaskManager_GetSleepLevel());

    uint8_t h = Timer_Subscribe(TIMEBASE_100MS, TASK_METERING, 2, false);
    TEST_ASSERT_EQUAL(SLEEP_LEVEL_SNOOZE, TaskManager_GetSleepLevel());

    TaskManager_Request(TASK_METERING);
    Timer_Unsubscribe(TIMEBASE_100MS, h);
    TaskManager_RunOnce();
    TEST_ASSERT_EQUAL(SLEEP_LEVEL_STOP, TaskManager_GetSleepLevel());
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_Scenario_PulseTriggersMeteringAlarmValve);
    RUN_TEST(test_Scenario_PeriodicMeteringViaTimeBase);
    RUN_TEST(test_Scenario_EnableDisableExtAlarm);
    RUN_TEST(test_Scenario_SleepLevelTransitions);
    return UNITY_END();
}
