# 测试规范

**版本：** v1.0
**日期：** 2026-06-17

---

## 1. 测试层级

| 层级 | 目标 | 工具 | 覆盖要求 |
|------|------|------|----------|
| 单元测试 | 验证单个函数/模块行为 | Unity + CMock | 核心模块 ≥ 80% |
| 集成测试 | 验证模块间交互 | 自定义框架 | 关键场景覆盖 |
| 系统测试 | 验证整机功能 | 目标板验证 | 发布前全量 |
| 硬件测试 | 验证外设驱动 | 示波器/逻辑分析仪 | HAL 层验证 |

---

## 2. 单元测试规范

### 2.1 测试文件命名

- 文件名：`test_<模块名>.c`
- 位置：`gas_meter/Test/`

### 2.2 测试函数命名

```c
void test_<场景描述>(void);
```

示例：
```c
void test_Init_ShouldZeroCumulative(void);
void test_PulseTrigger_ShouldIncrementCount(void);
void test_SampleTooSmall_ShouldTriggerAlarm(void);
```

### 2.3 测试结构

```c
/**
 * @file test_metering.c
 * @brief Unit tests for Metering component
 */
#include "unity.h"
#include "metering.h"

void setUp(void)    { Metering_Init(); }
void tearDown(void) { /* 清理 */ }

void test_Init_ShouldZeroCumulative(void)
{
    TEST_ASSERT_EQUAL(0, Metering_GetCumulativeVolume());
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_Init_ShouldZeroCumulative);
    return UNITY_END();
}
```

### 2.4 断言使用

| Unity 断言 | 用途 |
|------------|------|
| `TEST_ASSERT_EQUAL(expected, actual)` | 值相等 |
| `TEST_ASSERT_TRUE(condition)` | 条件为真 |
| `TEST_ASSERT_FALSE(condition)` | 条件为假 |
| `TEST_ASSERT_NULL(ptr)` | 指针为空 |
| `TEST_ASSERT_NOT_NULL(ptr)` | 指针非空 |

---

## 3. Mock 和 Stub

### 3.1 何时使用 Mock

- 测试组件依赖 HAL 层时
- 测试组件依赖其他组件时

### 3.2 Mock 示例

```c
/* Mock HAL GPIO */
void HAL_GPIO_Write(uint8_t pin, uint8_t level)
{
    /* 记录调用，供测试验证 */
    mock_gpio_write_called = true;
    mock_gpio_pin = pin;
    mock_gpio_level = level;
}
```

---

## 4. 测试覆盖率

### 4.1 核心模块覆盖率要求

| 模块 | 覆盖率要求 |
|------|------------|
| Metering | ≥ 80% |
| ValveControl | ≥ 80% |
| 安全检测组件（7个） | ≥ 80% |

### 4.2 其他模块

- 建议 ≥ 60%

---

## 5. 测试场景设计

### 5.1 三类场景

每个函数应测试：

| 场景类型 | 示例 |
|----------|------|
| 正常场景 | 参数在有效范围内 |
| 边界场景 | 参数为 0、最大值、边界值 |
| 异常场景 | 参数为 NULL、非法值 |

### 5.2 场景命名约定

```c
void test_<动作>_<预期结果>(void);

/* 示例 */
void test_RequestPin_ShouldReturnOK(void);           /* 正常 */
void test_RequestPin_ShouldRejectDoubleRequest(void); /* 异常 */
void test_ZeroBalance_ShouldTriggerValveClose(void);   /* 边界 */
```

---

## 修订历史

| 版本 | 日期 | 描述 |
|------|------|------|
| v1.0 | 2026-06-17 | 初始版本 |