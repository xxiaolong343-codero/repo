/**
 * @file test_task_config.c
 * @brief Unit tests for task configuration persistence to E2
 */
#include "unity.h"
#include "TaskConfig.h"
#include "TaskManager.h"
#include <string.h>

/* Simulated E2 storage */
static uint8_t s_e2_buf[8];
static bool    s_e2_write_called;

/* Mock E2 interface */
HAL_Status_t E2_Read(uint32_t addr, uint8_t *buf, uint16_t len)
{
    (void)addr;
    memcpy(buf, s_e2_buf, len);
    return HAL_OK;
}

HAL_Status_t E2_Write(uint32_t addr, const uint8_t *buf, uint16_t len)
{
    (void)addr;
    memcpy(s_e2_buf, buf, len);
    s_e2_write_called = true;
    return HAL_OK;
}

void setUp(void)
{
    memset(s_e2_buf, 0xFF, sizeof(s_e2_buf));
    s_e2_write_called = false;
    TaskManager_Init();
}

void tearDown(void) {}

void test_SaveConfig_ShouldWriteToE2(void)
{
    TaskConfig_Save();
    TEST_ASSERT_TRUE(s_e2_write_called);
}

void test_LoadConfig_ShouldRestoreEnabledState(void)
{
    /* Disable some tasks, save, then re-init */
    TaskManager_DisableTask(TASK_BILLING);
    TaskManager_DisableTask(TASK_COMM);
    TaskConfig_Save();

    /* Re-init (should load from E2) */
    TaskManager_Init();
    TaskConfig_Load();

    TEST_ASSERT_FALSE(TaskManager_IsEnabled(TASK_BILLING));
    TEST_ASSERT_FALSE(TaskManager_IsEnabled(TASK_COMM));
    /* Core task should still be enabled */
    TEST_ASSERT_TRUE(TaskManager_IsEnabled(TASK_METERING));
}

void test_LoadConfig_BadCRC_ShouldUseDefaults(void)
{
    /* Corrupt E2 data */
    memset(s_e2_buf, 0xAA, sizeof(s_e2_buf));
    TaskConfig_Load();
    /* Should fall back to defaults — all core tasks enabled */
    TEST_ASSERT_TRUE(TaskManager_IsEnabled(TASK_METERING));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_SaveConfig_ShouldWriteToE2);
    RUN_TEST(test_LoadConfig_ShouldRestoreEnabledState);
    RUN_TEST(test_LoadConfig_BadCRC_ShouldUseDefaults);
    return UNITY_END();
}
