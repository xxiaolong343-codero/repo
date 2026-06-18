/**
 * @file task_stubs.c
 * @brief Stub process functions for all 24 tasks — used in unit tests
 */
#include "TaskManager.h"

/* External arrays defined in test file */
extern uint8_t s_process_call_count[24];

/* Helper macro */
#define STUB(name, id) void name(void) { s_process_call_count[id]++; }

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
