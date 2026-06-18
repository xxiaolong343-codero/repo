/**
 * @file test_io_manager.c
 * @brief Unit tests for IOManager — request, release, read, write, conflict
 */
#include "unity.h"
#include "IOManager.h"
#include "IOConfig.h"
#include <string.h>

/* ---- Test doubles for HAL GPIO ---- */
static uint8_t gpio_levels[32];
static uint8_t gpio_mode[32];   /* 0=free, 1=input, 2=output */

/* Override HAL_GPIO_Write/Read in host test build */
void HAL_GPIO_Write(uint8_t pin, uint8_t level) {
    gpio_levels[pin] = level;
}
uint8_t HAL_GPIO_Read(uint8_t pin) {
    return gpio_levels[pin];
}
void HAL_GPIO_SetMode(uint8_t pin, uint8_t mode) {
    gpio_mode[pin] = mode;
}

void setUp(void)
{
    memset(gpio_levels, 0, sizeof(gpio_levels));
    memset(gpio_mode, 0, sizeof(gpio_mode));
    IOManager_Init();
}

void tearDown(void) {}

/* ---- Tests ---- */

void test_RequestPin_ShouldReturnOK(void)
{
    HAL_Status_t result = IO_Request(PIN_LED_RUN, IO_MODE_OUTPUT);
    TEST_ASSERT_EQUAL(HAL_OK, result);
}

void test_RequestPin_ShouldRejectDoubleRequest(void)
{
    IO_Request(PIN_LED_RUN, IO_MODE_OUTPUT);
    HAL_Status_t result = IO_Request(PIN_LED_RUN, IO_MODE_OUTPUT);
    TEST_ASSERT_EQUAL(HAL_ERROR, result);
}

void test_ReleasePin_ShouldAllowReRequest(void)
{
    IO_Request(PIN_LED_RUN, IO_MODE_OUTPUT);
    IO_Release(PIN_LED_RUN);
    HAL_Status_t result = IO_Request(PIN_LED_RUN, IO_MODE_INPUT);
    TEST_ASSERT_EQUAL(HAL_OK, result);
}

void test_WriteRead_ShouldReflectLevel(void)
{
    IO_Request(PIN_LED_RUN, IO_MODE_OUTPUT);
    IO_Write(PIN_LED_RUN, 1);
    TEST_ASSERT_EQUAL(1, IO_Read(PIN_LED_RUN));
    IO_Write(PIN_LED_RUN, 0);
    TEST_ASSERT_EQUAL(0, IO_Read(PIN_LED_RUN));
}

void test_Toggle_ShouldFlipLevel(void)
{
    IO_Request(PIN_LED_RUN, IO_MODE_OUTPUT);
    IO_Write(PIN_LED_RUN, 0);
    IO_Toggle(PIN_LED_RUN);
    TEST_ASSERT_EQUAL(1, IO_Read(PIN_LED_RUN));
    IO_Toggle(PIN_LED_RUN);
    TEST_ASSERT_EQUAL(0, IO_Read(PIN_LED_RUN));
}

void test_WriteUnrequestedPin_ShouldReturnError(void)
{
    HAL_Status_t result = IO_Write(PIN_LED_RUN, 1);
    TEST_ASSERT_EQUAL(HAL_ERROR, result);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_RequestPin_ShouldReturnOK);
    RUN_TEST(test_RequestPin_ShouldRejectDoubleRequest);
    RUN_TEST(test_ReleasePin_ShouldAllowReRequest);
    RUN_TEST(test_WriteRead_ShouldReflectLevel);
    RUN_TEST(test_Toggle_ShouldFlipLevel);
    RUN_TEST(test_WriteUnrequestedPin_ShouldReturnError);
    return UNITY_END();
}
