/**
 * @file TaskManager.c
 * @brief Stub — full implementation in Task 3
 */
#include "TaskManager.h"

void TaskManager_Init(void)  {}
void TaskManager_Run(void)   { while(1) {} }
void TaskManager_Request(uint8_t task_id) { (void)task_id; }
void TaskManager_EnableTask(uint8_t task_id)  { (void)task_id; }
void TaskManager_DisableTask(uint8_t task_id) { (void)task_id; }
bool TaskManager_IsEnabled(uint8_t task_id)   { (void)task_id; return false; }
uint8_t Timer_Subscribe(TimeBase_t base, uint8_t task_id,
                        uint16_t divider, bool oneshot) {
    (void)base; (void)task_id; (void)divider; (void)oneshot;
    return TIMER_HANDLE_INVALID;
}
void Timer_Unsubscribe(TimeBase_t base, uint8_t handle) {
    (void)base; (void)handle;
}
void Timer_Resubscribe(TimeBase_t base, uint8_t handle,
                       uint16_t new_divider) {
    (void)base; (void)handle; (void)new_divider;
}
SleepLevel_t TaskManager_GetSleepLevel(void) { return SLEEP_LEVEL_RUN; }
