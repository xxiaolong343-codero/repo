# C 编码规范

**版本：** v1.0
**日期：** 2026-06-17
**适用范围：** 智能燃气表嵌入式软件开发

---

## 1. 文件组织

### 1.1 文件命名

| 文件类型 | 规则 | 示例 |
|----------|------|------|
| 头文件 | 模块名.h（全小写） | `metering.h`, `valve_control.h` |
| 源文件 | 模块名.c（全小写） | `metering.c`, `valve_control.c` |
| 测试文件 | test_模块名.c | `test_metering.c` |

**禁止：** 文件名含空格、中文、大写字母。

### 1.2 头文件结构

```c
/**
 * @file metering.h
 * @brief 计量组件公开 API
 */
#ifndef METERING_H    /* 防重复包含：模块名_H */
#define METERING_H

/* 1. 标准库头文件 */
#include <stdint.h>
#include <stdbool.h>

/* 2. 项目头文件 */
#include "hal_common.h"

/* 3. 其他模块头文件（如有依赖） */
#include "metering_params.h"

/* 4. 类型定义 */
typedef enum { ... } MeteringState_t;

/* 5. 公开函数声明 */
void Metering_Init(void);

#endif /* METERING_H */
```

### 1.3 源文件结构

```c
/**
 * @file metering.c
 * @brief 计量组件实现
 */

/* 1. 对应头文件（最先） */
#include "metering.h"

/* 2. 标准库头文件 */
#include <string.h>

/* 3. 项目头文件 */
#include "TaskManager.h"

/* 4. 私有宏定义 */
#define RING_BUF_SIZE  16

/* 5. 私有类型定义 */
typedef struct { ... } MeteringState_t;

/* 6. 私有变量 */
static MeteringState_t s_state;

/* 7. 私有函数（static） */
static void process_sample(uint16_t Uc);

/* 8. 公开函数 */
void Metering_Init(void) { ... }
void Metering_Process(void) { ... }
```

### 1.4 文件行数限制

| 类型 | 最大行数 | 超过时的处理 |
|------|----------|--------------|
| 头文件 | 100 行 | 考虑拆分类型定义到单独文件 |
| 源文件 | 500 行 | 拆分为多个源文件，保持头文件统一 |

---

## 2. 命名规范

### 2.1 基本原则

- **有意义的名称**：拒绝 `a`, `b`, `tmp`, `data`
- **一致性**：同类事物命名风格一致
- **避免缩写**：除非行业通用（如 `ADC`, `PWM`, `RTC`）

### 2.2 模块命名

| 类别 | 规则 | 示例 |
|------|------|------|
| 模块名 | 全小写，单词间下划线 | `metering`, `valve_control`, `power_mgmt` |

### 2.3 函数命名

| 类别 | 规则 | 示例 |
|------|------|------|
| 公开函数 | 模块名前缀 + 动词 + 名词（大驼峰动词） | `Metering_Init()`, `ValveControl_Open()` |
| 私有函数 | `static` + 小写 + 描述 | `static void process_sample()` |

### 2.4 类型命名

| 类别 | 规则 | 示例 |
|------|------|------|
| 结构体 | 模块名前缀 + 大驼峰 + `_t` | `MeteringParams_t`, `ValveState_t` |
| 枚举 | 模块名前缀 + 大驼峰 + `_t` | `MeteringAlarm_t`, `SleepLevel_t` |
| 枚举值 | 模块名前缀 + 大驼峰 + `_t` | `METERING_ALARM_TOO_SMALL` |

### 2.5 变量命名

| 类别 | 规则 | 示例 |
|------|------|------|
| 全局变量 | `g_` 前缀 + 模块名 + 描述 | `g_sched`, `g_metering_params` |
| 静态变量（文件内） | `s_` 前缀 + 描述 | `s_state`, `s_ring_buf` |
| 局部变量 | 小写 + 描述性 | `sample_count`, `flow_rate` |
| 循环变量 | `i`, `j`, `k`（仅限循环内） | `for (int i = 0; ...)` |

### 2.6 宏与常量命名

| 类别 | 规则 | 示例 |
|------|------|------|
| 宏定义 | 全大写 + 下划线 | `MAX_SUB_PER_BASE`, `TIMER_HANDLE_INVALID` |
| 枚举常量 | 全大写 + 下划线 | `METERING_ALARM_TOO_SMALL` |

---

## 3. 代码格式

### 3.1 缩进与行宽

| 项目 | 规则 |
|------|------|
| 缩进 | 4 空格，**禁用 Tab** |
| 行宽 | 120 字符（超过则换行） |
| 函数间空行 | 1-2 行 |

### 3.2 大括号风格

采用 **K&R 风格**：

```c
/* 函数定义：大括号另起一行 */
void Metering_Init(void)
{
    /* 函数体 */
}

/* if/for/while/switch：大括号紧跟 */
if (condition) {
    action();
} else {
    other();
}

for (int i = 0; i < count; i++) {
    process(i);
}

switch (state) {
case STATE_IDLE:
    handle_idle();
    break;
case STATE_ACTIVE:
    handle_active();
    break;
default:
    handle_error();
    break;
}
```

**单语句可不加括号（但推荐加）：**
```c
if (ptr == NULL) return;
/* 推荐： */
if (ptr == NULL) {
    return;
}
```

### 3.3 空格规则

| 场景 | 规则 | 示例 |
|------|------|------|
| 关键字后 | 加空格 | `if (cond)` 而非 `if(cond)` |
| 运算符两侧 | 加空格 | `x = y + z` 而非 `x=y+z` |
| 逗号后 | 加空格 | `func(a, b, c)` |
| 分号前 | 无空格 | `for (i = 0; i < n; i++)` |
| 函数名与括号间 | 无空格 | `Metering_Init()` 而非 `Metering_Init ()` |

### 3.4 注释风格

| 类型 | 格式 | 用途 |
|------|------|------|
| 单行注释 | `/* 注释内容 */` | 简短说明 |
| 多行注释 | `/* ... */` 每行 | 详细说明 |
| API 文档 | `/** @brief ... */` | 公开函数/类型说明 |
| 行尾注释 | `/* 注释 */` | 简短标注（谨慎使用） |

**API 文档注释示例：**
```c
/**
 * @brief 初始化计量组件
 *
 * 初始化环形缓冲区，设置默认参数，准备光学采样。
 *
 * @note 必须在调用其他 Metering 函数之前调用。
 */
void Metering_Init(void);

/**
 * @brief 获取累积用气量
 * @return 用气量，单位 0.01 m³
 */
uint32_t Metering_GetCumulativeVolume(void);
```

---

## 4. 数据类型与表达式

### 4.1 禁用裸 int

**必须使用 `<stdint.h>` 精确宽度类型：**

| 类型 | 用途 |
|------|------|
| `uint8_t` | 小范围计数、状态值、字节 |
| `uint16_t` | ADC 值、中等范围计数 |
| `uint32_t` | 时间戳、累积量、大计数 |
| `int8_t` | 有符号小范围 |
| `int16_t` | 有符号中等范围 |
| `int32_t` | 有符号大范围、余额 |

**禁止：**
```c
int count;          /* 禁止：不确定大小 */
unsigned long id;   /* 禁止：用 uint32_t */
```

**正确：**
```c
uint8_t count;      /* 明确 8 位 */
uint32_t id;        /* 明确 32 位 */
```

### 4.2 布尔类型

使用 `<stdbool.h>` 的 `bool`：

```c
bool enabled = true;
bool active  = false;
```

**禁止：** 用 `int` 或 `uint8_t` 表示布尔值。

### 4.3 类型转换

**禁止隐式转换，必须显式 cast：**

```c
uint16_t adc = 1024;
uint32_t voltage = (uint32_t)adc * 3300 / 4096;  /* 显式 cast */
```

### 4.4 位操作

**仅用于无符号类型：**

```c
uint32_t bitmap = 0;
bitmap |= (1UL << 5);    /* 设置位 */
bitmap &= ~(1UL << 5);   /* 清除位 */
bool bit = (bitmap >> 5) & 1UL;  /* 读取位 */
```

**禁止：** 对有符号类型进行位操作。

### 4.5 无符号类型算术

MISRA-C 建议避免无符号类型的复杂算术（防止意外溢出/下溢）。

**谨慎：**
```c
uint32_t a = 10, b = 20;
if (a - b > 5) { ... }  /* a - b 会溢出为正数，逻辑错误 */
```

**正确：**
```c
if (a > b + 5) { ... }  /* 不涉及减法 */
```

---

## 5. 函数设计

### 5.1 函数长度

- **≤ 60 行**（超过则拆分）
- 拆分标准：每个子函数做一件事

### 5.2 参数数量

- **≤ 4 个参数**
- 超过则用结构体打包：

```c
/* 禁止 */
void process(uint16_t Le, uint16_t Lc, uint16_t Ue, uint16_t Uc, bool flag);

/* 正确 */
typedef struct {
    uint16_t Le;
    uint16_t Lc;
    uint16_t Ue;
    uint16_t Uc;
    bool     flag;
} SampleData_t;

void process(const SampleData_t *data);
```

### 5.3 参数校验

**入口参数先校验（公开函数）：**

```c
void Metering_SetParams(const MeteringParams_t *params)
{
    if (params == NULL) return;  /* 指针校验 */
    if (params->pulse_equivalent == 0) return;  /* 值校验 */
    s_params = *params;
}
```

### 5.4 返回类型

**必须明确声明返回类型（禁止隐式 int）：**

```c
void Metering_Init(void);           /* 无返回 */
uint32_t Metering_GetVolume(void);  /* 有返回 */
```

### 5.5 单一出口原则

MISRA-C 规则 15.5 建议函数单一出口。本项目**允许豁免**，但需减少多 return：

```c
/* 不推荐（多处 return） */
bool check_sample(uint16_t Uc)
{
    if (Uc < 100) return false;
    if (Uc > 4000) return false;
    return true;
}

/* 推荐（单一出口） */
bool check_sample(uint16_t Uc)
{
    bool valid = true;
    if (Uc < 100 || Uc > 4000) {
        valid = false;
    }
    return valid;
}
```

---

## 6. 控制流

### 6.1 禁止 goto

**禁止使用 `goto` 语句。**

如果必须跳出多层循环，使用标志变量：

```c
bool found = false;
for (int i = 0; i < ROWS && !found; i++) {
    for (int j = 0; j < COLS && !found; j++) {
        if (matrix[i][j] == target) {
            found = true;
        }
    }
}
```

### 6.2 switch 必须有 default

```c
switch (state) {
case STATE_IDLE:
    ...
    break;
case STATE_ACTIVE:
    ...
    break;
default:            /* 必须有 default */
    handle_error();
    break;
}
```

### 6.3 禁止递归

**禁止递归调用函数**（嵌入式栈有限 + MISRA-C 规则 17.2）。

### 6.4 循环计数器

`for` 循环计数器不得在循环体内修改：

```c
/* 禁止 */
for (int i = 0; i < 10; i++) {
    i = 5;  /* 禁止修改计数器 */
}
```

### 6.5 break/continue 使用

允许，但需注释理由：

```c
for (int i = 0; i < count; i++) {
    if (data[i] == INVALID) {
        continue;  /* 跳过无效数据 */
    }
    if (data[i] == TERMINATOR) {
        break;     /* 遇到终止符，结束循环 */
    }
    process(data[i]);
}
```

---

## 7. 内存管理

### 7.1 禁止动态分配

本项目**禁止使用 `malloc`/`free`/`calloc`/`realloc`**。

理由：
- 嵌入式环境内存有限（8KB RAM）
- 长期运行可能导致内存碎片
- 防止内存泄漏
- MISRA-C 规则 22.1

**替代方案：** 全静态分配 + 编译期确定大小。

```c
/* 禁止 */
char *buf = malloc(256);

/* 正确 */
static char s_buf[256];
```

### 7.2 全局变量最小化

全局变量应尽可能少。必须使用时：
- 加 `g_` 前缀
- 考虑是否可用 `static` 替代（限制作用域）

---

## 8. 预处理指令

### 8.1 宏定义规则

| 规则 | 示例 |
|------|------|
| 宏名全大写 | `MAX_SUB_PER_BASE` |
| 参数加括号 | `#define MAX(a, b) ((a) > (b) ? (a) : (b))` |
| 整体加括号 | `#define SQUARE(x) ((x) * (x))` |
| 避免副作用 | 禁止 `#define INC(x) (++(x))` |

### 8.2 条件编译

用于平台适配：

```c
#if PLATFORM_RL78
    /* RL78 特定代码 */
#elif PLATFORM_HDSC
    /* HDSC 特定代码 */
#else
    /* Host 模式（单元测试） */
#endif
```

### 8.3 防重复包含

每个头文件必须有：

```c
#ifndef METERING_H
#define METERING_H
...
#endif /* METERING_H */
```

---

## 9. 错误处理

### 9.1 返回码约定

使用 `HAL_Status_t` 作为通用返回码：

```c
typedef enum {
    HAL_OK       = 0,
    HAL_ERROR    = -1,
    HAL_BUSY     = -2,
    HAL_TIMEOUT  = -3
} HAL_Status_t;
```

### 9.2 错误处理模式

```c
HAL_Status_t result = IO_Request(PIN_LED_RUN, IO_MODE_OUTPUT);
if (result != HAL_OK) {
    /* 处理错误：记录日志、报警、返回 */
    return;
}
```

---

## 10. 附录：MISRA-C 2012 合规要求

见 `misra-c-checklist.md`。

---

## 修订历史

| 版本 | 日期 | 描述 |
|------|------|------|
| v1.0 | 2026-06-17 | 初始版本 |