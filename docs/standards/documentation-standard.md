# 文档规范

**版本：** v1.0
**日期：** 2026-06-17

---

## 1. 文档分类与位置

| 文档类型 | 位置 | 格式 |
|----------|------|------|
| 设计规格 | `docs/superpowers/specs/` | Markdown |
| 实施计划 | `docs/superpowers/plans/` | Markdown |
| 编码规范 | `docs/standards/` | Markdown |
| API 文档 | 头文件内注释 | Doxygen 风格 |
| README | 项目根目录 | `README.md` |
| CHANGELOG | 项目根目录 | `CHANGELOG.md` |

---

## 2. 文档命名规范

| 类型 | 命名格式 | 示例 |
|------|----------|------|
| 设计规格 | `YYYY-MM-DD-<主题>-design.md` | `2026-04-13-gas-meter-design.md` |
| 实施计划 | `YYYY-MM-DD-<主题>.md` | `2026-06-17-gas-meter-foundation.md` |
| 规范文档 | `<主题>.md` | `coding-standard.md` |

---

## 3. API 注释格式（Doxygen 风格）

### 3.1 文件注释

```c
/**
 * @file metering.h
 * @brief 计量组件公开 API
 *
 * @version 1.0
 * @date 2026-06-17
 * @author 作者姓名
 */
```

### 3.2 函数注释

```c
/**
 * @brief 初始化计量组件
 *
 * 初始化环形缓冲区，设置默认参数，准备光学采样。
 * 调用后组件处于空闲状态，等待脉冲中断触发。
 *
 * @note 必须在调用其他 Metering 函数之前调用。
 *
 * @param params 可选参数指针，NULL 时使用默认值
 * @return HAL_OK 成功，HAL_ERROR 失败
 */
HAL_Status_t Metering_Init(const MeteringParams_t *params);
```

### 3.3 类型注释

```c
/**
 * @brief 计量采样异常类型
 */
typedef enum {
    METERING_ALARM_NONE = 0,         /**< 无异常 */
    METERING_ALARM_TOO_SMALL,        /**< 采样过小 */
    METERING_ALARM_TOO_LARGE,        /**< 采样过大 */
    METERING_ALARM_BASIC_RELATION    /**< 基本关系异常 */
} MeteringAlarm_t;
```

### 3.4 结构体注释

```c
/**
 * @brief 计量参数配置
 */
typedef struct {
    uint16_t env_ref_standard;   /**< 环境光参考标准值（mV） */
    uint16_t env_min_allowed;    /**< 允许最小环境光（mV） */
    uint8_t  judgment_hysteresis;/**< 判决回差（mV） */
} MeteringParams_t;
```

---

## 4. README 格式

项目根目录 README.md 应包含：

```markdown
# 项目名称

简介（一句话描述项目）

## 功能特性

- 功能 1
- 功能 2

## 目录结构

简要说明主要目录

## 构建说明

如何编译（RL78 / HDSC）

## 测试说明

如何运行单元测试

## 文档索引

指向设计文档、规范文档的链接

## 联系方式

负责人联系方式
```

---

## 5. CHANGELOG 格式

```markdown
# CHANGELOG

## v1.0.0 - 2026-06-17

### 新增
- 计量组件光学检测算法
- 阀门控制状态机

### 修复
- 采样异常误触发问题

### 变更
- 重构 TaskManager 调度器

### 已知问题
- 无
```

---

## 修订历史

| 版本 | 日期 | 描述 |
|------|------|------|
| v1.0 | 2026-06-17 | 初始版本 |