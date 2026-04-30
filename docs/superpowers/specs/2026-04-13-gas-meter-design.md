# 智能燃气表嵌入式系统设计

**版本：** v3.1
**日期：** 2026-04-29
**状态：** 已批准

---

## 1. 目标与范围

设计一套支持多硬件平台的智能燃气表嵌入式软件系统，具备计量、报警、远程抄表、阀门控制、现场人机交互等功能，支持 NB-IoT / 4G 双模通信，满足计量认证和防爆认证要求。

### 1.1 核心功能

- 燃气流量计量，支持预付费/后付费
- 泄漏检测与报警，异常用气检测
- 远程抄表（NB-IoT / 4G），现场红外抄表/配置/测试
- 阀门远程控制，支持故障安全
- 阶梯计价，余额管理
- OTA 远程升级，定制 Bootloader 支持安全启动和防回滚
- LCD + LED + 按键现场人机交互
- 电池供电，支持碱电池/锂电池主电 + Supercap/小锂电备电，无中断切换

### 1.2 目标硬件平台

- 瑞萨 RL78 系列（使用 CS+ 编译器）
- 华大芯片（使用 Keil MDK 编译器）

### 1.3 设计约束

- 裸机开发（无 RTOS），满足低功耗需求
- 中型项目，多模块协作
- 需满足计量认证（OIML R31 / GB/T 6968）和防爆认证


## 2. 系统架构

┌─────────────────────────────────────────────────────────────────┐  
│ 应用层 │  
│ ┌───────────────────────────────────────────────────────────┐ │  
│ │ TaskManager │ │  
│ │ - 请求驱动的任务调度 │ │  
│ │ - FIFO执行队列 │ │  
│ │ - 软件定时器池 │ │  
│ │ - 分级睡眠管理（STOP/SNOOZE/RUN） │ │  
│ └───────────────────────────────────────────────────────────┘ │  
├─────────────────────────────────────────────────────────────────┤  
│ 业务组件层（24个） │  
│ ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐ │  
│ │Metering │ │ Key │ │ Alarm │ │ Valve │ │ HMI │ │  
│ └─────────┘ └─────────┘ └─────────┘ └─────────┘ └─────────┘ │  
│ ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐ │  
│ │ Billing │ │ Comm │ │Infrared │ │DataStore│ │E2Storage│ │  
│ └─────────┘ └─────────┘ └─────────┘ └─────────┘ └─────────┘ │  
│ ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐ │  
│ │EventLog │ │UsageLog │ │ExtAlarm │ │ValveLeak│ │ BigFlow │ │  
│ └─────────┘ └─────────┘ └─────────┘ └─────────┘ └─────────┘ │  
│ ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐ │  
│ │NoGasUse │ │ContFlow │ │TinyFlow │ │UnctrlFlow│ │ Tilt │ │  
│ └─────────┘ └─────────┘ └─────────┘ └─────────┘ └─────────┘ │  
│ ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐ │  
│ │ Upgrade │ │PowerMgmt│ │Battery │ │ Clock │ │ Debug │ │  
│ └─────────┘ └─────────┘ └─────────┘ └─────────┘ └─────────┘ │  
├─────────────────────────────────────────────────────────────────┤  
│ 中间件层 │  
│ ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐ │  
│ │ DLMS │ │BillingAlg│ │EventSched│ │DataManager│ │  
│ └─────────┘ └─────────┘ └─────────┘ └─────────┘ │  
├─────────────────────────────────────────────────────────────────┤  
│ 硬件抽象层 (HAL) │  
│ ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐ │  
│ │ UART │ │ GPIO │ │ ADC │ │ Timer │ │ Flash │ │  
│ └─────────┘ └─────────┘ └─────────┘ └─────────┘ └─────────┘ │  
├────────────────────────┬────────────────────────────────────────┤  
│ RL78 驱动 │ 华大芯片驱动 │  
└────────────────────────┴────────────────────────────────────────┘


### 2.1 层级职责

| 层级 | 职责 | 跨平台 |
|------|------|--------|
| 应用层 | 业务逻辑整合，任务调度，模式切换（预付费/后付费） | 是 |
| 业务组件层 | 独立业务模块，接口清晰，24个组件各司其职 | 是 |
| 中间件层 | 协议算法，通用能力（DLMS协议、计价算法、数据管理、定时事件调度） | 是 |
| HAL 层 | 硬件接口统一抽象 | 按芯片实现 |
| 驱动层 | 芯片寄存器操作 | 芯片相关 |

## 3. 组件清单

### 3.1 组件分类说明

根据可配置性和功能特性，24个组件分为三类：

| 分类 | 说明 | 数量 |
|------|------|------|
| **核心组件** | 系统运行基础，不可配置关闭 | 8个 |
| **可配置组件** | 可通过配置使能/禁能 | 15个 |
| **模式相关组件** | 行为随系统模式变化 | 1个 |

### 3.2 核心组件（始终激活）

| 组件 | 职责 | 不可关闭理由 |
|------|------|-------------|
| Metering | 燃气流量计量、红外光采样自学习、脉冲采集、计量校准 | 核心功能，计量认证要求 |
| Key | 按键检测（消抖、状态机）、按键事件处理 | 基本人机交互入口 |
| PowerMgmt | 电源状态管理、主备电切换、SPC管理 | 系统生存基础 |
| Alarm | 统一报警管理（报警码管理、报警状态） | 报警管理中枢 |
| Clock | RTC管理、时间同步、时钟异常检测 | 时间基准 |
| DataStorage | 参数存储、计量数据持久化 | 参数持久化基础 |
| ValveControl | 阀门开关控制、开阀条件检测、阀门锁定 | 安全核心 |
| Debug | 串口日志输出、红外现场调试、生产测试模式 | 调试基础设施 |

### 3.3 可配置组件（运行时开关）

| 组件 | 职责 | 配置依据 | 默认状态 |
|------|------|----------|----------|
| Communication | NB-IoT通信管理、心跳重连、PSM模式 | 定时通信使能等 | 部分功能可关闭 |
| Infrared | 近红外/远红外通信 | 本地通信使能 | 开启 |
| Billing | 阶梯计价、余额管理、费用计算 | 预付费/后付费模式 | 模式决定 |
| EventLog | 事件记录（计量/报警/通信/配置/操作） | 事件记录使能 | 开启 |
| E2Storage | E2存储分区管理、写平衡、磨损均衡 | 存储策略配置 | 开启 |
| UsageLog | 用气日志（天用气+半小时用气） | 日志功能使能 | 开启 |
| ExtAlarm | 外部报警检测 | 功能使能配置 | 关闭 |
| ValveLeak | 阀关走气检测、直通检测 | 功能使能配置 | 关闭 |
| BigFlow | 异常大流量检测 | 功能使能配置 | 关闭 |
| NoGasUsage | 多天不用气检测（一级/二级） | 一级/二级使能配置 | 关闭 |
| ContFlowTimeout | 持续流量超时检测 | 功能使能配置 | 关闭 |
| TinyFlow | 微小流检测 | 功能使能配置 | 关闭 |
| UnctrlFlow | 不受控流量检测 | 功能使能配置 | 关闭 |
| TiltDetection | 倾斜检测、倾斜自检 | 功能使能配置 | 关闭 |
| Upgrade | OTA升级、Bootloader | 升级功能使能 | 开启 |
| BatteryMgmt | 主电类型检测、电量检测 | 电池检测使能 | 开启 |

### 3.4 模式相关组件

| 组件 | 职责 | 模式影响 |
|------|------|----------|
| HMI | LCD显示（5种状态）、LED指示、菜单导航 | 不同模式下显示内容不同 |

**共 24 个业务组件。**

### 3.5 中间件组件清单

| 组件 | 职责 | 跨平台 |
|------|------|--------|
| DLMS | DLMS/COSEM协议栈，抄表数据封装/解析，计量认证 | 是 |
| BillingAlgorithm | 阶梯计价算法（气价/气量阶梯），与Billing组件配合 | 是 |
| EventScheduler | 定时任务调度引擎（天用气日志、半小时用气日志、定时通信） | 是 |
| DataManager | 数据管理（分区抽象、冗余备份策略） | 是 |

## 4. 任务调度器设计

### 4.1 设计目标

任务调度器（TaskManager）是应用层的核心组件，负责协调24个业务组件的执行。设计需满足以下目标：

| 需求 | 说明 |
|------|------|
| **低功耗优先** | 无任务执行时 MCU 进入睡眠，由中断/定时器唤醒 |
| **按需执行** | 任务开启时，仅在被请求时执行一次，无请求不执行 |
| **配置控制** | 任务可开启/关闭，通过远程/本地通信指令配置 |
| **多触发源** | 支持中断、事件、定时器三种触发源 |
| **简单可靠** | 无指针、无动态分配，适合长期无人值守运行 |

### 4.2 核心模型

#### 4.2.1 任务状态

```
任务状态：开启(ENABLED) / 关闭(DISABLED)

开启状态：
  - 可被请求调度
  - 请求一次，执行一次
  - 执行完毕后回到空闲，等待下次请求

关闭状态：
  - 不参与调度
  - 请求被忽略
  - 定时器（如有）被删除
```

#### 4.2.2 调度模型

```
请求一次，执行一次

触发源：
  - 中断：硬件中断（脉冲、按键、RTC、串口等）
  - 事件：组件内部事件（计量完成、报警触发等）
  - 定时器：软件定时器到期

执行顺序：FIFO（先请求先执行）

请求接口：TaskManager_Request(task_id)
```

#### 4.2.3 睡眠策略

分级睡眠，根据当前状态自动选择：

| 睡眠级别 | 电流 | 唤醒源 | 进入条件 |
|---------|------|--------|----------|
| **RUN** | ~3mA | 全部 | 有任务正在执行 |
| **SNOOZE** | ~5μA | 低功耗定时器、中断 | 有短周期定时请求（<1s） |
| **STOP** | ~0.5μA | RTC闹钟、中断 | 无任务执行，无短周期定时 |

### 4.3 架构设计

#### 4.3.1 架构总览

```
┌─────────────────────────────────────────────────────────┐
│                      配置指令                              │
│              (远程/本地通信下发)                             │
│                    │                                      │
│                    ▼                                      │
│           ┌───────────────┐                               │
│           │  任务配置管理   │ ← 开启/关闭任务，持久化到E2      │
│           └───────┬───────┘                               │
│                   │ enable/disable                         │
│                   ▼                                      │
│  ┌──────────────────────────────────────────────────┐    │
│  │              任务调度器 (核心)                       │    │
│  │                                                    │    │
│  │  ┌─────────────┐  ┌──────────────┐                │    │
│  │  │ 请求位图      │  │ FIFO 就绪队列  │                │    │
│  │  │ request_bitmap│  │ ready_queue  │                │    │
│  │  └──────┬──────┘  └──────┬───────┘                │    │
│  │         │                │                         │    │
│  │         ▼                ▼                         │    │
│  │  ┌─────────────────────────────┐                  │    │
│  │  │      执行引擎                 │                  │    │
│  │  │  FIFO 顺序执行被请求的任务     │                  │    │
│  │  └─────────────────────────────┘                  │    │
│  │                                                    │    │
│  │  ┌─────────────┐  ┌──────────────┐                │    │
│  │  │ 软件定时器池   │  │ 睡眠管理器    │                │    │
│  │  │ timer_pool   │  │ sleep_mgr    │                │    │
│  │  └─────────────┘  └──────────────┘                │    │
│  └──────────────────────────────────────────────────┘    │
│                                                           │
│  触发源：                                                  │
│  ┌──────┐ ┌──────┐ ┌──────┐ ┌──────┐ ┌──────────┐       │
│  │中断   │ │事件   │ │定时器 │ │RTC   │ │低功耗定时器│       │
│  └──┬───┘ └──┬───┘ └──┬───┘ └──┬───┘ └────┬─────┘       │
│     └────────┴────────┴────────┴──────────┘              │
│                        │                                  │
│              TaskManager_Request(task_id)                 │
└─────────────────────────────────────────────────────────┘
```

#### 4.3.2 模块职责

| 模块 | 职责 |
|------|------|
| **任务配置管理** | 接收配置指令，开启/关闭任务，持久化到E2 |
| **请求位图** | 中断安全的请求标志，支持原子操作 |
| **FIFO 就绪队列** | 存储被请求的任务ID，保证执行顺序 |
| **执行引擎** | 从就绪队列取出任务，调用其 process() |
| **软件定时器池** | 管理周期性定时请求，到期自动请求调度 |
| **睡眠管理器** | 根据当前状态决定睡眠级别，配置唤醒源 |

### 4.4 数据结构

#### 4.4.1 任务描述符

```c
typedef struct {
    uint8_t task_id;                        // 任务ID（0 ~ TASK_COUNT-1）
    bool enabled;                           // 配置使能
    void (*process)(void);                  // 任务处理函数
} TaskDesc_t;
```

#### 4.4.2 定时器订阅槽

系统提供三种固定时间基准（10ms / 100ms / 1s），任务通过订阅 + 分频来获得定时服务。每个基准维护独立的订阅表：

```c
typedef enum {
    TIMEBASE_10MS  = 0,   // 10ms 基准（消抖、阀门监控）
    TIMEBASE_100MS = 1,   // 100ms 基准（计量、LED闪烁）
    TIMEBASE_1S    = 2,   // 1s 基准（电源巡检、电池检测、延时上传）
    TIMEBASE_COUNT = 3
} TimeBase_t;

// 订阅槽 — 任务在某基准上"每N个tick触发一次"
typedef struct {
    bool    active;           // 是否激活
    bool    oneshot;          // true=一次性（到期自动退订），false=周期性
    uint8_t task_id;          // 到期时 Request(task_id)
    uint16_t divider;         // 当前剩余tick数（减到0触发）
    uint16_t reload;          // 重载值（周期性用）
} TimerSub_t;
```

#### 4.4.3 FIFO 就绪队列

```c
typedef struct {
    uint8_t items[TASK_COUNT];              // 队列存储
    uint8_t head;                           // 队头索引
    uint8_t tail;                           // 队尾索引
    uint8_t count;                          // 当前元素数
} ReadyQueue_t;
```

#### 4.4.4 调度器状态

```c
#define MAX_SUB_PER_BASE  8   // 每个基准最多 8 个订阅者

typedef struct {
    volatile uint32_t request_bitmap;       // 请求位图（中断安全）
    ReadyQueue_t ready_queue;               // FIFO 就绪队列
    TaskDesc_t tasks[TASK_COUNT];           // 任务表
    TimerSub_t subs[TIMEBASE_COUNT][MAX_SUB_PER_BASE];  // 3×8 订阅表
    volatile uint8_t sleep_level;           // 当前睡眠级别
} Scheduler_t;
```

#### 4.4.5 资源估算

| 项目 | 数量 | 单项大小 | 总大小 |
|------|------|----------|--------|
| 任务表 | 24 | ~12B | ~288B |
| 定时器订阅表 | 3×8 | ~8B | ~192B |
| 就绪队列 | 1 | ~26B | ~26B |
| 请求位图 | 1 | 4B | ~4B |
| 其他状态 | — | — | ~20B |
| **总计** | — | — | **~530B RAM** |

### 4.5 核心流程

#### 4.5.1 主循环流程

```
主循环：
  ┌──────────────────────────────────────────────┐
  │  1. 收集请求：request_bitmap → FIFO 就绪队列    │
  │  2. 执行任务：FIFO 顺序执行（定时触发已在ISR完成） │
  │  3. 计算睡眠：扫描订阅表 + RTC最近闹钟           │
  │  4. 进入睡眠：STOP / SNOOZE / 保持 RUN          │
  │  5. 唤醒（中断/定时器/RTC）→ 回到步骤1           │
  └──────────────────────────────────────────────┘
```

#### 4.5.2 请求调度流程

```
TaskManager_Request(task_id)
    │
    ▼
任务是否开启？
    │
    ├── 否 → 直接返回（忽略请求）
    │
    └── 是 → 设置请求位图
              request_bitmap |= (1 << task_id)
                │
                ▼
              当前是否在睡眠？
                │
                ├── 是 → 触发唤醒
                └── 否 → 返回（主循环会处理）
```

#### 4.5.3 执行流程

```
执行引擎：
  │
  ▼
就绪队列是否为空？
  │
  ├── 是 → 结束执行
  │
  └── 否 → 取出队头任务 task_id
              │
              ▼
          调用 tasks[task_id].process()
              │
              ▼
          回到队列检查
```

#### 4.5.4 分级睡眠决策

```
睡眠级别决策：
  │
  ▼
有被请求的任务？
  │
  ├── 是 → RUN（执行任务）
  │
  └── 否 → 有活跃订阅？
              │
              ├── 是，有短周期订阅（TIMEBASE_10MS 或 TIMEBASE_100MS）→ SNOOZE
              │
              ├── 是，仅有 TIMEBASE_1S 长周期 → STOP（RTC闹钟唤醒）
              │
              └── 否 → STOP（仅中断唤醒）
```

### 4.6 接口设计

#### 4.6.1 核心接口

| 接口 | 用途 | 可调用上下文 |
|------|------|-------------|
| `TaskManager_Init()` | 初始化调度器 | main() 启动时 |
| `TaskManager_Run()` | 主循环入口 | main() 主循环 |
| `TaskManager_Request(task_id)` | 请求调度某任务 | 中断/主循环 |

#### 4.6.2 配置接口

| 接口 | 用途 | 可调用上下文 |
|------|------|-------------|
| `TaskManager_EnableTask(task_id)` | 开启任务（持久化到E2） | 主循环 |
| `TaskManager_DisableTask(task_id)` | 关闭任务（持久化到E2） | 主循环 |
| `TaskManager_IsEnabled(task_id)` | 查询任务是否开启 | 主循环 |

#### 4.6.3 定时器接口

| 接口 | 用途 | 可调用上下文 |
|------|------|-------------|
| `Timer_Subscribe(base, task_id, divider, oneshot)` | 在指定时间基准上订阅定时服务 | 主循环/中断 |
| `Timer_Unsubscribe(base, handle)` | 退订，释放订阅槽 | 主循环/中断 |
| `Timer_Resubscribe(base, handle, new_divider)` | 修改分频数（如从200ms改为1s） | 主循环 |

#### 4.6.4 接口详细定义

```c
/**
 * @brief 初始化调度器
 * @note  从E2读取任务配置，恢复各任务使能状态；初始化三个硬件基准定时器
 */
void TaskManager_Init(void);

/**
 * @brief 调度器主循环
 * @note  无线循环，永不返回
 */
void TaskManager_Run(void);

/**
 * @brief 请求调度某任务
 * @param  task_id 任务ID
 * @note   中断安全，可在ISR中调用
 */
void TaskManager_Request(uint8_t task_id);

/**
 * @brief 开启任务
 * @param  task_id 任务ID
 * @note   持久化到E2，创建关联的周期性定时器订阅（如有）
 */
void TaskManager_EnableTask(uint8_t task_id);

/**
 * @brief 关闭任务
 * @param  task_id 任务ID
 * @note   持久化到E2，退订该任务的所有订阅
 */
void TaskManager_DisableTask(uint8_t task_id);

/**
 * @brief 查询任务是否开启
 * @param  task_id 任务ID
 * @return true=开启, false=关闭
 */
bool TaskManager_IsEnabled(uint8_t task_id);

/**
 * @brief 在指定时间基准上订阅定时服务
 * @param  base     时间基准（TIMEBASE_10MS / TIMEBASE_100MS / TIMEBASE_1S）
 * @param  task_id  到期时 Request 的任务ID
 * @param  divider  分频数（每 divider 个 tick 触发一次）
 * @param  oneshot  true=到期后自动退订，false=周期性
 * @return 订阅句柄（0xFF=失败）
 * @note   中断安全：ISR中可调用以创建oneshot（内部关中断保护订阅表）
 */
uint8_t Timer_Subscribe(TimeBase_t base, uint8_t task_id,
                        uint16_t divider, bool oneshot);

/**
 * @brief 退订定时器订阅
 * @param  base     时间基准
 * @param  handle   订阅句柄
 */
void Timer_Unsubscribe(TimeBase_t base, uint8_t handle);

/**
 * @brief 修改订阅分频数
 * @param  base         时间基准
 * @param  handle       订阅句柄
 * @param  new_divider  新分频数
 * @note   用于阀门动作切换（开阀2s→关阀200ms）等场景
 */
void Timer_Resubscribe(TimeBase_t base, uint8_t handle, uint16_t new_divider);
```

### 4.7 中断安全设计

#### 4.7.1 请求位图操作

```c
// 中断中请求调度
void TaskManager_Request(uint8_t task_id)
{
    // 位操作在RL78上是原子的，无需关中断
    g_scheduler.request_bitmap |= (1UL << task_id);
    
    // 如果当前在睡眠，触发唤醒
    if (g_scheduler.sleep_level != SLEEP_LEVEL_RUN) {
        HAL_TriggerWakeup();
    }
}
```

#### 4.7.2 主循环收集请求

```c
// 主循环中收集请求到就绪队列
static void CollectRequests(void)
{
    uint32_t bitmap;
    
    // 原子读-清除：关中断保证读和清零之间不丢ISR请求
    uint8_t saved_ie = HAL_DisableInterrupts();
    bitmap = g_scheduler.request_bitmap;
    g_scheduler.request_bitmap = 0;
    HAL_RestoreInterrupts(saved_ie);
    
    while (bitmap) {
        uint8_t task_id = __builtin_ctz(bitmap);
        ReadyQueue_Push(&g_scheduler.ready_queue, task_id);
        bitmap &= bitmap - 1;  // 清除已处理位
    }
}
```

#### 4.7.3 基准定时器中断处理

三个基准定时器的 ISR 直接处理订阅表，无需轮询：

```c
// 100ms 基准定时器中断（示例，10ms/1s 逻辑相同）
void ISR_TimeBase100ms(void)
{
    TimerSub_t *p = g_scheduler.subs[TIMEBASE_100MS];
    
    for (uint8_t i = 0; i < MAX_SUB_PER_BASE; i++) {
        if (!p[i].active) continue;
        
        p[i].divider--;
        if (p[i].divider == 0) {
            TaskManager_Request(p[i].task_id);
            
            if (p[i].oneshot) {
                p[i].active = false;            // 一次性：退订
            } else {
                p[i].divider = p[i].reload;     // 周期性：重载
            }
        }
    }
}
```

此设计消除了原方案中的 `CheckTimers()` 轮询循环和 `FindFirstSet` 查找表——定时触发由硬件中断精准驱动，CPU 无需每轮主循环遍历比对。

### 4.8 三基准定时器设计

#### 4.8.1 设计原理

所有定时需求映射到三种固定时间基准（10ms / 100ms / 1s），任务通过 **订阅 + 分频** 获得定时服务。不再存在"任意周期"的软件定时器池。

```
┌──────────────────┐   ┌──────────────────┐   ┌──────────────────┐
│ TIMEBASE_10MS    │   │ TIMEBASE_100MS   │   │ TIMEBASE_1S      │
│ TAU通道0         │   │ TAU通道1         │   │ TAU通道2         │
│ 硬件中断精准驱动  │   │ 硬件中断精准驱动   │   │ 硬件中断精准驱动   │
└──────┬───────────┘   └──────┬───────────┘   └──────┬───────────┘
       │                      │                      │
       ▼                      ▼                      ▼
  订阅表[0..7]            订阅表[0..7]           订阅表[0..7]
  ┌───────────┐          ┌───────────┐          ┌───────────┐
  │Key消抖×3   │          │Metering×2 │          │PowerMgmt×1│
  │Valve×5     │          │LED×5      │          │Battery×60 │
  │50ms×5      │          │Tilt×1     │          │Key上传×10 │
  └───────────┘          └───────────┘          │LCD超时×15 │
                                                 │IR超时×20  │
                                                 └───────────┘
```

只需 3 个 TAU 通道 + 3×8 静态数组，总 RAM ~192B，优于原 16 槽池（~224B）。

#### 4.8.2 订阅与退订实现

```c
// 订阅
uint8_t Timer_Subscribe(TimeBase_t base, uint8_t task_id,
                        uint16_t divider, bool oneshot)
{
    uint8_t saved_ie = HAL_DisableInterrupts();
    
    for (uint8_t i = 0; i < MAX_SUB_PER_BASE; i++) {
        TimerSub_t *p = &g_scheduler.subs[base][i];
        if (!p->active) {
            p->active   = true;
            p->oneshot  = oneshot;
            p->task_id  = task_id;
            p->divider  = divider;
            p->reload   = divider;
            HAL_RestoreInterrupts(saved_ie);
            return i;
        }
    }
    
    HAL_RestoreInterrupts(saved_ie);
    return 0xFF;  // 该基准订阅已满
}

// 退订
void Timer_Unsubscribe(TimeBase_t base, uint8_t handle)
{
    if (handle >= MAX_SUB_PER_BASE) return;
    g_scheduler.subs[base][handle].active = false;
}

// 修改分频数（不改变订阅槽位）
void Timer_Resubscribe(TimeBase_t base, uint8_t handle, uint16_t new_divider)
{
    if (handle >= MAX_SUB_PER_BASE) return;
    g_scheduler.subs[base][handle].divider = new_divider;
    g_scheduler.subs[base][handle].reload  = new_divider;
}
```

#### 4.8.3 分频映射表

所有定时需求转换为分频值：

```c
// 任务周期性订阅配置表（编译期确定）
static const struct {
    uint8_t  task_id;
    TimeBase_t base;
    uint16_t divider;       // 每 divider 个 tick 触发一次
    // 等效周期 = base_period × divider
} task_timer_config[] = {
    // 10ms 基准
    // （无周期性订阅者；消抖/阀门监控为 oneshot）
    
    // 100ms 基准
    { TASK_METERING,   TIMEBASE_100MS, 2  },   // 200ms

    // 1s 基准
    { TASK_POWER_MGMT, TIMEBASE_1S,    1  },   // 1s
    { TASK_BATTERY,    TIMEBASE_1S,    60 },   // 60s
    { TASK_USAGE_LOG,  TIMEBASE_1S,    30 },   // 30s
    { TASK_EXT_ALARM,  TIMEBASE_1S,    1  },   // 1s（默认关闭）
    
    { 0xFF, 0, 0 }  // 结束
};
```

#### 4.8.4 任务开启/关闭时自动订阅/退订

```c
void TaskManager_EnableTask(uint8_t task_id)
{
    g_scheduler.tasks[task_id].enabled = true;
    
    for (int i = 0; task_timer_config[i].task_id != 0xFF; i++) {
        if (task_timer_config[i].task_id == task_id) {
            Timer_Subscribe(task_timer_config[i].base,
                           task_id,
                           task_timer_config[i].divider,
                           false);  // 周期性
        }
    }
    
    E2_WriteTaskConfig(task_id, true);
}

void TaskManager_DisableTask(uint8_t task_id)
{
    g_scheduler.tasks[task_id].enabled = false;
    
    // 扫描所有基准，退订该任务的所有订阅
    for (uint8_t b = 0; b < TIMEBASE_COUNT; b++) {
        for (uint8_t i = 0; i < MAX_SUB_PER_BASE; i++) {
            if (g_scheduler.subs[b][i].active &&
                g_scheduler.subs[b][i].task_id == task_id) {
                g_scheduler.subs[b][i].active = false;
            }
        }
    }
    
    E2_WriteTaskConfig(task_id, false);
}
```

#### 4.8.5 组件内部计时 vs 订阅定时器

| 类别 | 机制 | 使用场景 | 占用订阅槽 |
|------|------|----------|-----------|
| **订阅定时器** | 基准分频，到期 → Request(task_id) | 计量采样、电源巡检、延时上传、阀门动作监控 | 是 |
| **组件内部计时** | tick 差值 + FSM 状态变量 | 按键消抖、长按判断、LED 闪烁、倾斜防抖 | 否 |

**组件内部计时的典型模式**（不占订阅槽）：

```c
// 按键消抖 — FSM 内部用 tick 差值
void KeyDetect_FSM(void)
{
    uint32_t now = HAL_GetTick();
    switch (state) {
    case STATE_DEBOUNCE_PRESS:
        if ((now - state_enter_tick) >= 30) {  // 30ms 消抖
            state = STATE_PRESSED;
        }
        break;
    case STATE_PRESSED:
        uint32_t held = now - press_start_tick;
        if (held > 15000)      KeyEvent_OnLongPress15s();
        else if (held > 3000)  KeyEvent_OnLongPress3s();
        else if (held > 1000)  KeyEvent_OnLongPress1s();
        break;
    }
}

// 阀门 FSM — 单槽位复用（Resubscribe 改分频）
void ValveFSM_OnEnter(ValveState_t new_state)
{
    switch (new_state) {
    case VALVE_OPENING:
        if (valve_timer_h == 0xFF)
            valve_timer_h = Timer_Subscribe(TIMEBASE_1S, TASK_VALVE_CTRL, 2, true);
        else
            Timer_Resubscribe(TIMEBASE_1S, valve_timer_h, 2);   // 2s
        break;
    case VALVE_CLOSING:
        Timer_Resubscribe(TIMEBASE_1S, valve_timer_h, 1);       // 1s（实际200ms用10ms×20）
        break;
    }
}
```

### 4.9 睡眠管理

#### 4.9.1 睡眠级别定义

```c
typedef enum {
    SLEEP_LEVEL_RUN = 0,    // 全速运行
    SLEEP_LEVEL_SNOOZE,     // 轻度睡眠（低功耗定时器运行）
    SLEEP_LEVEL_STOP        // 深度睡眠（仅中断/RTC唤醒）
} SleepLevel_t;
```

#### 4.9.2 睡眠级别决策

决策信息源：
1. **订阅表** — 各任务在三个基准上的活跃订阅
2. **RTC 闹钟** — 绝对时间触发点（半小时日志、天日志、定时通信）

```c
static SleepLevel_t DecideSleepLevel(void)
{
    // 有被请求的任务 → RUN
    if (g_scheduler.ready_queue.count > 0) {
        return SLEEP_LEVEL_RUN;
    }
    
    bool has_short_sub  = false;   // 10ms 或 100ms 基准有活跃订阅
    bool has_long_sub   = false;   // 1s 基准有活跃订阅
    
    // 检查三个基准的订阅表
    for (uint8_t i = 0; i < MAX_SUB_PER_BASE; i++) {
        if (g_scheduler.subs[TIMEBASE_10MS][i].active)  has_short_sub = true;
        if (g_scheduler.subs[TIMEBASE_100MS][i].active) has_short_sub = true;
        if (g_scheduler.subs[TIMEBASE_1S][i].active)    has_long_sub  = true;
    }
    
    // 检查 RTC 闹钟（由 EventScheduler 管理）
    if (EventScheduler_HasPendingRtcAlarm()) {
        has_long_sub = true;
    }
    
    if (!has_short_sub && !has_long_sub) {
        return SLEEP_LEVEL_STOP;  // 无任何定时需求 → STOP
    }
    
    // 有短周期订阅（10ms/100ms）→ SNOOZE
    if (has_short_sub) {
        return SLEEP_LEVEL_SNOOZE;
    }
    
    // 仅长周期（1s + RTC）→ STOP
    return SLEEP_LEVEL_STOP;
}
```

**SNOOZE vs STOP 选择原则**：短基准（10ms/100ms）意味着 MCU 需要频繁被唤醒，SNOOZE 模式下唤醒延迟更低；纯长基准（1s + RTC）则可以深度睡眠，用低功耗定时器或 RTC 闹钟唤醒。

#### 4.9.3 进入睡眠

```c
static void EnterSleep(SleepLevel_t level)
{
    g_scheduler.sleep_level = level;
    
    switch (level) {
        case SLEEP_LEVEL_RUN:
            // 不睡眠，继续执行
            break;
            
        case SLEEP_LEVEL_SNOOZE:
            // 配置低功耗定时器唤醒
            HAL_StartLowPowerTimer(GetMinTimerInterval());
            HAL_EnterSnoozeMode();
            HAL_StopLowPowerTimer();
            break;
            
        case SLEEP_LEVEL_STOP:
            // 配置RTC闹钟（如有长周期定时器）
            uint32_t next_wakeup = GetNextRtcWakeup();
            if (next_wakeup > 0) {
                HAL_SetRtcAlarm(next_wakeup);
            }
            HAL_EnterStopMode();
            HAL_ClearRtcAlarm();
            break;
    }
    
    g_scheduler.sleep_level = SLEEP_LEVEL_RUN;
}
```

### 4.10 配置持久化

#### 4.10.1 配置存储结构

```c
// E2 配置区结构
typedef struct {
    uint8_t version;                        // 配置版本
    uint8_t task_enable_bitmap[3];          // 任务使能位图（24位，3字节）
    uint8_t crc8;                           // CRC校验
} TaskConfigE2_t;
```

#### 4.10.2 配置读写

```c
// 读取配置
void TaskManager_LoadConfig(void)
{
    TaskConfigE2_t config;
    
    if (E2_Read(TASK_CONFIG_ADDR, &config, sizeof(config)) != E2_OK) {
        // 读取失败，使用默认配置
        UseDefaultConfig();
        return;
    }
    
    if (!VerifyCrc8(&config, sizeof(config) - 1, config.crc8)) {
        // CRC错误，使用默认配置
        UseDefaultConfig();
        return;
    }
    
    // 恢复任务使能状态
    for (uint8_t i = 0; i < TASK_COUNT; i++) {
        uint8_t byte_idx = i / 8;
        uint8_t bit_idx = i % 8;
        g_scheduler.tasks[i].enabled = 
            (config.task_enable_bitmap[byte_idx] >> bit_idx) & 0x01;
    }
}

// 保存配置
void TaskManager_SaveConfig(void)
{
    TaskConfigE2_t config;
    config.version = CONFIG_VERSION;
    
    // 打包任务使能状态
    for (uint8_t i = 0; i < TASK_COUNT; i++) {
        uint8_t byte_idx = i / 8;
        uint8_t bit_idx = i % 8;
        if (g_scheduler.tasks[i].enabled) {
            config.task_enable_bitmap[byte_idx] |= (1 << bit_idx);
        } else {
            config.task_enable_bitmap[byte_idx] &= ~(1 << bit_idx);
        }
    }
    
    config.crc8 = CalcCrc8(&config, sizeof(config) - 1);
    E2_Write(TASK_CONFIG_ADDR, &config, sizeof(config));
}
```

### 4.11 任务与定时器配置表

#### 4.11.1 任务清单

| 任务ID | 名称 | 默认状态 | 周期订阅 | 说明 |
|--------|------|----------|----------|------|
| 0 | TASK_METERING | 开启 | 100ms×2=200ms | 计量采样 |
| 1 | TASK_KEY | 开启 | 无 | 消抖/长按用内部 tick 差值 |
| 2 | TASK_ALARM | 开启 | 无 | 事件驱动 |
| 3 | TASK_VALVE_CTRL | 开启 | 无 | 阀门动作通过 oneshot 订阅 |
| 4 | TASK_HMI | 开启 | 无 | LCD 超时/LED 闪烁通过 oneshot 订阅 |
| 5 | TASK_BILLING | 开启 | 无 | 事件驱动 |
| 6 | TASK_COMM | 开启 | 无 | 通信超时通过 oneshot 订阅 |
| 7 | TASK_INFRARED | 开启 | 无 | 红外超时通过 oneshot 订阅 |
| 8 | TASK_DATA_STORAGE | 开启 | 无 | 事件驱动 |
| 9 | TASK_E2_STORAGE | 开启 | 无 | 事件驱动 |
| 10 | TASK_EVENT_LOG | 开启 | 无 | 事件驱动 |
| 11 | TASK_USAGE_LOG | 开启 | 1s×30=30s | 半小时日志检查 |
| 12 | TASK_EXT_ALARM | 关闭 | 1s×1=1s | 外部报警巡检 |
| 13 | TASK_VALVE_LEAK | 关闭 | 无 | 事件驱动 |
| 14 | TASK_BIG_FLOW | 关闭 | 无 | 事件驱动 |
| 15 | TASK_NO_GAS_USAGE | 关闭 | 无 | 事件驱动 |
| 16 | TASK_CONT_FLOW | 关闭 | 无 | 事件驱动 |
| 17 | TASK_TINY_FLOW | 关闭 | 无 | 事件驱动 |
| 18 | TASK_UNCTRL_FLOW | 关闭 | 无 | 事件驱动 |
| 19 | TASK_TILT | 关闭 | 无 | 中断驱动 + oneshot 订阅 |
| 20 | TASK_UPGRADE | 开启 | 无 | 事件驱动 |
| 21 | TASK_POWER_MGMT | 开启 | 1s×1=1s | 电源状态巡检 |
| 22 | TASK_BATTERY | 开启 | 1s×60=60s | 电池电量检测 |
| 23 | TASK_CLOCK | 开启 | 无 | RTC 闹钟驱动 |

**订阅槽占用统计**：
- 10ms 基准：0 个周期（全部 oneshot）
- 100ms 基准：1 个周期（Metering ×2）
- 1s 基准：4 个周期（PowerMgmt ×1、Battery ×60、UsageLog ×30、ExtAlarm ×1）
- 动态 oneshot：按键延时上传、阀门动作、LCD 超时、LED 闪烁、通信超时、红外超时、倾斜巡检
- 每基准 8 槽，常态占用 < 4 槽/基准，余量充足

#### 4.11.2 硬件定时器分配（RUN模式）

| TAU通道 | 用途 | 周期 |
|---------|------|------|
| TAU0 | TIMEBASE_10MS 基准 | 10ms |
| TAU1 | TIMEBASE_100MS 基准 | 100ms |
| TAU2 | TIMEBASE_1S 基准 | 1s |
| TAU3 | PWM（红外调制） | — |
| TAU4 | 脉冲捕获（预留） | — |
| 低功耗间隔定时器 | SNOOZE/STOP 模式唤醒 | 动态 |
| RTC | 日历 + 长周期闹钟 | 1s / 动态 |

### 4.12 使用示例

#### 4.12.1 系统初始化

```c
int main(void)
{
    HAL_Init();
    TaskManager_Init();   // 初始化三个基准定时器 + 订阅表
    TaskManager_Run();
    return 0;
}
```

#### 4.12.2 中断中请求调度

```c
// 脉冲中断
void ISR_Pulse(void)
{
    TaskManager_Request(TASK_METERING);
}

// 按键中断 — 同时创建一个 oneshot 延时上传
void ISR_Key(void)
{
    TaskManager_Request(TASK_KEY);
    Timer_Subscribe(TIMEBASE_1S, TASK_KEY, 10, true);  // 10s 后触发延时上传
}

// RTC闹钟中断
void ISR_RtcAlarm(void)
{
    TaskManager_Request(TASK_EVENT_LOG);
}
```

#### 4.12.3 任务处理函数

```c
// 计量任务处理函数
void Metering_Process(void)
{
    ProcessPulseData();
    if (HasAlarm()) {
        TaskManager_Request(TASK_ALARM);
    }
}

// 报警任务处理函数
void Alarm_Process(void)
{
    HandleAlarm();
    if (NeedCloseValve()) {
        TaskManager_Request(TASK_VALVE_CTRL);
    }
}

// 阀门任务 — 内部管理 oneshot 订阅
static uint8_t valve_timer_h = 0xFF;

void ValveControl_Process(void)
{
    switch (valve_fsm_state) {
    case VALVE_IDLE:
        if (open_requested) {
            valve_fsm_state = VALVE_OPENING;
            valve_timer_h = Timer_Subscribe(TIMEBASE_1S, TASK_VALVE_CTRL, 2, true);
        }
        break;
    case VALVE_OPENING:
        if (timer_expired) {
            valve_fsm_state = VALVE_OPEN;
            valve_timer_h = 0xFF;
        }
        break;
    }
}
```

#### 4.12.4 配置任务开启/关闭

```c
void OnConfigCommand(uint8_t task_id, bool enable)
{
    if (enable) {
        TaskManager_EnableTask(task_id);
    } else {
        TaskManager_DisableTask(task_id);
    }
}
```

### 4.13 与原设计的差异

| 维度 | 原设计 | 新设计 |
|------|--------|--------|
| 调度模型 | 状态表遍历 + is_ready() 检查 | 请求驱动 + FIFO 队列 |
| 任务状态 | active 标志 + is_ready() 函数 | enabled 标志 + 请求位图 |
| 执行触发 | 调度器轮询检查 | 任务/中断主动请求 |
| 执行顺序 | 优先级排序 | FIFO |
| 定时管理 | 组件自治 | 三基准订阅模型（10ms/100ms/1s + 分频） |
| 定时触发 | 主循环轮询 CheckTimers() | 硬件基准定时器 ISR 精准驱动 |
| 睡眠决策 | 查询各任务 get_wakeup_time() | 扫描三基准订阅表 + RTC 闹钟 |
| 配置原子性 | 批量事务机制 | 位图天然原子 |

## 5. 定时器管理设计

本章从需求分析和硬件资源角度，说明定时器系统如何支撑第4章的 TaskManager 调度模型。

### 5.1 定时需求梳理

系统各组件的定时需求汇总如下：

| 任务/组件 | 定时需求 | 周期 | 实现机制 | 睡眠中唤醒 |
|-----------|----------|------|----------|-----------|
| Metering | 触发ADC采样 | 200ms | TaskManager 周期定时器 | ✅ 是 |
| Metering | 流速计算时间戳 | 事件触发 | HAL_GetUsTick() | ❌ 否 |
| Key | 按键消抖计时 | 30ms | 组件内部 tick 差值 | ❌ 否 |
| Key | 长按计时（1s/3s/15s） | 事件触发 | 组件内部 tick 差值 | ❌ 否 |
| Key | 延时上传计时 | 10s | TaskManager oneshot | ❌ 否 |
| ValveControl | 开阀动作计时 | 2s | TaskManager oneshot | ❌ 否 |
| ValveControl | 关阀动作计时 | 0.2s | TaskManager oneshot | ❌ 否 |
| HMI | LCD显示超时 | 可配置 | TaskManager oneshot | ❌ 否 |
| HMI | LED闪烁控制 | 可配置 | 组件内部 tick 差值 | ❌ 否 |
| Communication | 通信超时/重试 | 可配置 | TaskManager oneshot | ❌ 否 |
| Infrared | 红外通信超时 | 20s | TaskManager oneshot | ❌ 否 |
| Alarm | 声光报警时序 | 可配置 | 组件内部 tick 差值 | ❌ 否 |
| PowerMgmt | 电压检测周期 | 1s | TaskManager 周期定时器 | ❌ 否 |
| Battery | 电池电量检测 | 60s | TaskManager 周期定时器 | ❌ 否 |
| Clock | RTC日历维护 | 1s | 独立RTC外设 | ✅ 是 |
| EventScheduler | 半小时日志触发 | 30min | RTC闹钟 | ✅ 是 |
| EventScheduler | 天日志触发 | 24h | RTC闹钟 | ✅ 是 |
| EventScheduler | 定时通信触发 | 可配置 | RTC闹钟 | ✅ 是 |
| UsageLog | 半小时日志记录 | 30s | TaskManager 周期定时器 | ❌ 否 |
| TiltDetection | 倾斜检测防抖 | 100ms | 组件内部 tick 差值 | ❌ 否 |
| TiltDetection | 倾斜状态巡检 | 3s | TaskManager oneshot（可选） | ✅ 是 |
| ExtAlarm | 外部报警巡检 | 1s | TaskManager 周期定时器 | ✅ 是 |
| Key | 按键按下检测 | 100ms | get_next_wakeup() 动态 | ✅ 是 |
| ValveControl | 阀门动作监控 | 50ms | get_next_wakeup() 动态 | ✅ 是 |
| Communication | 通信活跃检查 | 1s | get_next_wakeup() 动态 | ✅ 是 |

### 5.2 定时需求分类

| 类别 | 实现机制 | 占用定时器池 | 占用硬件定时器 | 示例 |
|------|----------|-------------|---------------|------|
| **周期唤醒类** | TaskManager 周期定时器 → Request | 是 | 低功耗定时器 / RTC | 计量采样、电源巡检、外部报警巡检 |
| **一次性动作类** | TaskManager oneshot → Request → 自动销毁 | 是（临时） | 系统 tick | 阀门动作、延时上传、LCD超时、通信超时 |
| **组件内部时序类** | tick 差值 + FSM 状态变量 | 否 | 仅系统 tick | 按键消抖、长按判断、LED 闪烁、倾斜防抖 |
| **时间戳类** | HAL_GetUsTick() | 否 | TAU 捕获通道 | 流速计算时间戳 |
| **独立RTC类** | RTC 外设闹钟 | 否 | RTC | 日历维护、天日志、定时通信 |

### 5.3 硬件定时器资源（RL78示例）

| 定时器类型 | 数量 | 时钟源 | 功耗模式可用性 |
|-----------|------|--------|---------------|
| TAU（定时器阵列单元） | 8通道 | 高速系统时钟 | HALT模式可用，STOP模式停止 |
| 低功耗间隔定时器 | 1个 | 15kHz低速振荡器 | STOP模式可用 |
| 看门狗定时器 | 1个 | 独立低速时钟 | STOP模式可用 |
| RTC | 1个 | 32.768kHz晶振 | STOP模式可用 |

### 5.4 定时器资源分配方案

基于第4章的 TaskManager 统一调度模型，硬件定时器分配如下：

```
┌─────────────────────────────────────────────────────────────────┐
│ 定时器资源分配（与 TaskManager 配合）                              │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│ ┌─────────────────────────────────────────────────────────┐    │
│ │ 低功耗间隔定时器（15kHz，STOP模式可用）                     │    │
│ │ - SNOOZE 模式下的唤醒源                                   │    │
│ │ - 动态设置为最近到期定时器的间隔                            │    │
│ │ - 用途：计量采样(200ms)、短周期动态唤醒(50ms-1s)            │    │
│ └─────────────────────────────────────────────────────────┘    │
│                                                                 │
│ ┌─────────────────────────────────────────────────────────┐    │
│ │ RTC（32.768kHz，STOP模式可用）                            │    │
│ │ - 日历维护（时/分/秒/年月日）                              │    │
│ │ - STOP 模式下的长周期唤醒源                                │    │
│ │ - 闹钟功能：半小时日志、天日志、定时通信                     │    │
│ │ - 软件扩展支持多个闹钟时间点                                │    │
│ └─────────────────────────────────────────────────────────┘    │
│                                                                 │
│ ┌─────────────────────────────────────────────────────────┐    │
│ │ TAU通道0：系统节拍定时器（1ms）                            │    │
│ │ - 驱动 TaskManager 软件定时器池的时基                      │    │
│ │ - 驱动所有组件内部 tick 差值计时                           │    │
│ │ - 仅在 RUN 状态下运行                                     │    │
│ └─────────────────────────────────────────────────────────┘    │
│                                                                 │
│ ┌─────────────────────────────────────────────────────────┐    │
│ │ TAU通道1-2：硬件PWM/捕获                                   │    │
│ │ - 红外调制输出（PWM）                                      │    │
│ │ - 脉冲捕获（预留磁传感器）                                  │    │
│ │ - HAL_GetUsTick() 微秒级时间戳                             │    │
│ └─────────────────────────────────────────────────────────┘    │
│                                                                 │
│ ┌─────────────────────────────────────────────────────────┐    │
│ │ 看门狗定时器（独立时钟）                                    │    │
│ │ - 系统监控，防程序跑飞                                      │    │
│ │ - 不参与正常任务调度                                        │    │
│ └─────────────────────────────────────────────────────────┘    │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

**说明**：原方案中的 TAU通道3-7（软件定时器扩展）不再作为独立的虚拟定时器层存在。软件定时器统一由 TaskManager 的 `timer_pool[16]` 管理，所有定时器到期均通过 `TaskManager_Request()` 汇入 FIFO 队列，保证执行顺序。

### 5.5 睡眠唤醒路径

```
睡眠决策（DecideSleepLevel）综合三个信息源：

  1. timer_pool[16]        → 周期性/一次性定时器的最早到期时间
  2. get_next_wakeup()     → 各任务动态唤醒需求（按键/阀门/通信状态）
  3. RTC 闹钟列表          → 绝对时间触发点（日志/定时通信）

  取最小值 → 配置硬件唤醒源 → 进入 SLEEP_LEVEL_STOP / SLEEP_LEVEL_SNOOZE
```

唤醒后流程：

```
硬件唤醒 → ISR → （无需额外操作，MCU自动恢复运行）
                → TaskManager_Run() 主循环继续
                → CollectRequests() 收集 request_bitmap
                → CheckTimers()    检查到期定时器
                → 执行 FIFO 队列中的任务
                → DecideSleepLevel() → 再次进入睡眠
```

### 5.6 与原设计的对比

| 维度 | 原设计（Ch5） | 统一设计 |
|------|-------------|----------|
| 软件定时器管理 | 独立的 SoftwareTimer.c 层 | TaskManager timer_pool 统一管理 |
| 组件定时接口 | Timer_Create/Start/Stop 回调模式 | TaskManager_CreateTimer/CreateOneshot → Request → FIFO |
| 组件内部计时 | 全部依赖软件定时器 | tick 差值 + FSM 自给自足 |
| 执行顺序保证 | 无（回调直接分发） | FIFO 队列保证 |
| 一次性定时器 | 支持（mode参数） | 支持（oneshot字段） |
| 一个任务多个定时器 | 支持 | 支持（不限制数量） |

gas_meter/  
├── Application/ # 应用层  
│ ├── App.c # 应用入口  
│ └── TaskManager.c # 任务调度管理  
│  
├── Components/ # 业务组件层（24个组件）  
│ ├── Metering/  
│ │ ├── metering.h # 对外接口  
│ │ ├── metering.c # 核心实现  
│ │ ├── metering_alarm.c # 采样异常处理  
│ │ └── metering_hal.h # HAL抽象（内部）  
│ ├── Communication/  
│ │ ├── NBiot.c # NB-IoT 通信管理  
│ │ ├── CommScheduler.c # 定时/事件/人工触发调度  
│ │ ├── CommSession.c # 通信会话管理  
│ │ └── CommEvent.c # 多天不上传事件处理  
│ ├── Infrared/  
│ ├── Alarm/  
│ │ ├── AlarmManager.c # 报警状态管理、报警码管理  
│ │ └── AlarmSound.c # 报警声音触发  
│ ├── ValveControl/  
│ │ ├── ValveFSM.c # 阀门状态机  
│ │ ├── ValveLogic.c # 开阀条件检测、阀门锁定  
│ │ └── ValveUpload.c # 阀门动作上传  
│ ├── Billing/  
│ │ ├── BillingCalc.c # 计价算法  
│ │ ├── Balance.c # 余额管理  
│ │ └── Reserve.c # 预留量管理  
│ ├── EventLog/  
│ │ ├── EventRecord.c # 事件记录  
│ │ └── CommLog.c # 通信日志记录  
│ ├── DataStorage/  
│ │ ├── DataStore.c # 存储策略（冗余备份/CRC）  
│ │ └── DataManager.c # 数据管理（分区抽象）  
│ ├── E2Storage/  
│ │ ├── E2Driver.c # E2 读写驱动  
│ │ ├── E2Partition.c # 分区管理  
│ │ └── E2WearLevel.c # 磨损均衡  
│ ├── UsageLog/  
│ │ ├── DailyLog.c # 天用气日志  
│ │ └── HalfHourLog.c # 半小时用气日志  
│ ├── ExtAlarm/  
│ ├── ValveLeak/  
│ │ ├── ValveLeak.c # 阀关走气检测  
│ │ └── StraightThrough.c # 直通检测  
│ ├── BigFlow/  
│ ├── NoGasUsage/  
│ │ ├── NoGasLevel1.c # 一级不用气检测  
│ │ └── NoGasLevel2.c # 二级不用气检测  
│ ├── ContFlowTimeout/  
│ ├── TinyFlow/  
│ ├── UnctrlFlow/  
│ ├── TiltDetection/  
│ │ ├── TiltFSM.c # 倾斜状态机  
│ │ ├── TiltSelfTest.c # 倾斜自检  
│ │ └── TiltAlarm.c # 倾斜报警业务处理  
│ ├── Upgrade/  
│ ├── PowerMgmt/  
│ │ ├── PowerState.c # 电源状态机  
│ │ ├── MainPower.c # 主电检测  
│ │ ├── BackupPower.c # 备电检测  
│ │ └── SPC.c # SPC 管理  
│ ├── BatteryMgmt/  
│ │ ├── BatteryDetect.c # 主电类型检测  
│ │ └── BatteryLevel.c # 电量检测与低电报警  
│ ├── Clock/  
│ │ ├── RTC.c # RTC 管理  
│ │ └── ClockSync.c # 时间同步、时钟异常检测  
│ ├── Key/  
│ │ ├── KeyDetect.c # 按键检测（消抖、状态机）  
│ │ └── KeyEvent.c # 按键事件处理  
│ ├── HMI/  
│ │ ├── Display.c # LCD 显示驱动和刷新  
│ │ ├── DisplayState.c # 显示状态机  
│ │ ├── Menu.c # 菜单系统  
│ │ └── StatusBar.c # 状态栏显示  
│ └── Debug/  
│  
├── Middleware/ # 中间件层  
│ ├── DLMS/  
│ ├── BillingAlgorithm/  
│ ├── EventScheduler/  
│ └── DataManager/  
│  
├── HAL/ # 硬件抽象层  
│ ├── Include/  
│ │ ├── hal_timer.h  
│ │ ├── hal_gpio.h  
│ │ ├── hal_adc.h  
│ │ ├── hal_uart.h  
│ │ └── hal_flash.h  
│ ├── IO/  
│ │ ├── IOConfig.c  
│ │ ├── IOManager.c  
│ │ └── IOPin.h  
│ ├── Timer/  
│ │ ├── SoftwareTimer.c  
│ │ └── SoftwareTimer.h  
│ ├── RL78/  
│ │ ├── hal_timer_rl78.c  
│ │ ├── hal_gpio_rl78.c  
│ │ └── ...  
│ └── HDSC/  
│ ├── hal_timer_hdsc.c  
│ └── ...  
│  
├── Bsp/ # 板级支持包  
│ ├── BoardA_RL78/  
│ └── BoardB_HDSC/  
│  
├── Source/ # 共用源码（编译器中性）  
│  
├── Project/ # 工程文件  
│ ├── MDK/  
│ └── CSplus/  
│  
├── Build/ # 编译输出  
│  
├── Docs/ # 设计文档  
│  
└── Test/ # 单元测试

### 6.1 编译架构
- **共用源码**（Source/）：编译器中性，无 Keil/CS+ 特定语法
- **两套工程文件**：MDK（华大）+ CSplus（RL78），各自包含启动文件、链接脚本、芯片配置
- **两款机型**：碱电池版 / 锂电池版，通过编译选项或配置文件选择
- 源码一处修改，两平台同步
## 7. IO 管理设计
### 7.1 设计原则
- **集中管理** — 统一 IO 管理器，所有组件通过接口申请和使用 GPIO
- **配置表驱动** — 引脚分配通过配置表定义，不同硬件平台只需修改配置表
- **未使用 IO 妥善处理** — 所有未使用 IO 配置为安全状态（输入 + 上下拉），防浮空、防干扰
### 7.2 目录结构

HAL/IO/  
├── IOConfig.c # IO 配置表（已使用引脚 + 未使用引脚定义）  
├── IOManager.c # IO 申请/释放/读写接口实现  
└── IOPin.h # 引脚定义枚举

### 7.3 IO 配置表
```c
// 已使用 IO 配置
static const IOPinConfig_t usedPins[] = {
    {PIN_METER_PULSE, INPUT,  PULL_UP,   NO_ALT},
    {PIN_VALVE_CTRL,  OUTPUT, PUSH_PULL, NO_ALT},
    {PIN_LED_RUN,     OUTPUT, PUSH_PULL, NO_ALT},
    {PIN_KEY_MENU,    INPUT,  PULL_UP,   NO_ALT},
    // ...
};
// 未使用 IO 配置（默认安全状态）
static const IOPinConfig_t unusedPins[] = {
    {GPIO_PIN_0,  INPUT,  PULL_UP,   NO_ALT},
    {GPIO_PIN_1,  INPUT,  PULL_DOWN, NO_ALT},
    // ... 芯片所有未定义引脚
};
```

### 7.4 接口

|接口|描述|
|---|---|
|`IO_Request(pin, mode)`|申请 GPIO 口，指定输入/输出模式|
|`IO_Release(pin)`|释放 GPIO 口|
|`IO_Write(pin, level)`|写 GPIO 电平|
|`IO_Read(pin)`|读 GPIO 电平|
|`IO_Toggle(pin)`|翻转 GPIO 电平|

### 7.5 防冲突机制

- 组件申请 IO 时检查是否已被占用
    
- 已占用时返回错误，避免硬编码引脚
    
- 芯片所有可用引脚都应列入配置表（已用或未用）
    

## 8. E2 存储设计

### 8.1 分区结构

|分区|大小（参考）|用途|保护要求|
|---|---|---|---|
|计量区|2-4 KB|累计量、计量校准系数|双重存储 + CRC，防篡改|
|配置区|2-4 KB|通讯/计价/阀门参数|单备份 + CRC|
|密钥区|256-512 B|通讯密钥、授权密钥|加密存储|
|日志区|16-32 KB|事件循环记录|CRC|
|运行区|2-4 KB|阀门状态、报警标志、通信计数、电池电压记录|单备份 + CRC|
|固件区 A|64-128 KB|主固件区|签名验证|
|固件区 B|64-128 KB|OTA 升级备份区|签名验证|
|启动区|4-8 KB|启动参数、区块状态|CRC|

### 8.2 设计要点

- **计量区独立** — 满足计量认证要求，认证相关参数不可被应用层直接修改
    
- **密钥区隔离** — 独立扇区，加密存储，防止明文读取
    
- **写平衡** — 固件区使用双备份 + 写前擦除标志，保证 OTA 可靠性
    

### 8.3 磨损均衡策略

经过对各分区写入频率的详细计算，磨损均衡策略如下：

|分区|写入频率|10年磨损占比|策略|
|---|---|---|---|
|计量区|每1m³一次（约1-3次/天）|<1%|无需额外均衡|
|配置区|仅配置时写入|<0.1%|无需额外均衡|
|密钥区|几乎不写|~0%|无需额外均衡|
|天用气日志|1次/天，月循环覆盖|天然均衡|按设计实现即可|
|半小时用气日志|48次/天，双月交替|天然均衡|按设计实现即可|
|事件记录|事件触发，环形队列|天然均衡|维护队头指针即可|
|运行区|~27次/天|~10%|**无需额外均衡**|
|固件区|OTA时写入|<1%|无需额外均衡|

**结论**：各分区或因写入频率极低，或因存储结构天然具备轮转特性，均无需实现专门的磨损均衡算法。CAT24C256的100万次擦写寿命完全满足产品10-15年的设计寿命要求。

## 9. 供电设计

### 9.1 供电架构

|电源类型|描述|
|---|---|
|主电（碱电池版）|4节碱性电池供电|
|主电（锂电池版）|锂电池供电|
|备电|Supercap（超级电容）或小容量锂电池|

### 9.2 主备电切换

- 主电断电时备电瞬间接管，**计量不中断**
    
- PowerMgmt 组件管理切换逻辑
    
- 备电 Supercap 充电管理由 BatteryMgmt 组件控制
    

### 9.3 低功耗策略

|状态|描述|唤醒源|
|---|---|---|
|工作态|全速运行，通信/计量/控制|定时任务|
|轻度睡眠|CPU 暂停，外设运行|RTC、脉冲、外部中断|
|深度睡眠|仅 RTC 运行|RTC 定时、关键报警|

### 9.4 功耗控制机制

- **PowerMgmt 组件**集中管理功耗状态转换
    
- 外设时钟门控，非活跃外设断电
    
- NB-IoT / 4G 模块电源时序控制
    
- 事件驱动代替轮询，减少 CPU 活跃时间
    
- **TaskManager** 动态计算睡眠时间，最大化低功耗收益
    
- **计量采样采用200ms周期**，平均功耗约20μA，相比5ms高频采样降低10-15倍
    
### 9.5 四碱电主电电压状态

|电量状态|正常电压检测|电量百分比|
|---|---|---|
|电量正常状态|>5.75V|80%以上|
|电量低状态|5.55V-5.75V|60%-80%|
|锁定状态|4.7V-5.55V|40%-60%|
|掉电状态|3.5V-4.7V|10%-20%|
|严重掉电|<3.5V|10%以下|

### 9.6 锂电池备电电压状态

|电压等级|范围|电压检测频率|
|---|---|---|
|电量正常状态|>2.6V|1天|
|电量低状态|<2.4V|1分钟|

## 10. HMI 设计
### 10.1 硬件交互
| 设备 | 描述 |
|------|------|
| LCD | 显示详细信息（累计用量、余额、信号强度、报警状态等） |
| LED | 状态指示（运行、报警、通信、阀门状态） |
| 按键 | 菜单导航、操作确认 |
### 10.2 显示内容
- **状态栏**：信号强度、电池图标、阀门图标、报警图标
- **主显示**：累计用量、当前余额
- **菜单（用户）**：余额查询、用量查询、阀门操作
- **菜单（维护）**：参数配置、测试模式、诊断信息
### 10.3 液晶屏状态
1. 上电显示状态
2. 报警码显示状态
3. 常规显示状态
4. 锁定显示状态
5. 插入显示状态
### 10.4 报警码列表
| 错误代码 | 描述 |
|----------|------|
| E-1003 | 权限关阀 |
| E-1005 | 外部报警器报警 |
| E-1007 | 锂电池电量低 |
| E-6013 | 一级不用气报警 |
| E-6015 | 异常大流量 |
| E-6023 | 直通 |
| E-6024 | 多天不上传 |
| E-6033 | 微小流 |
| E-6034 | 采样过小 |
| E-6035 | 采样过大 |
| E-6036 | 基本关系异常 |
| E-6037 | 环境光偏差异常 |
| E-6038 | 判决门限异常 |
| E-6039 | 红外光抖动 |
| E-6040 | 更新异常 |
| E-6041 | 快速学习异常 |
| E-6042 | 自学习回差异常 |
| E-6031 | 持续流量超时 |
| E-6058 | 倾斜报警 |
| E-6048 | 不受控流量报警 |
| E-1016 | SPC过放报警 |
| E-1013 | 管道防拆装置自检未通过 |
### 10.5 权限控制
- 普通用户：查看状态、基本操作
- 维护人员：参数配置、测试模式
- 管理员：全面控制
## 11. 调试能力
| 功能 | 描述 |
|------|------|
| 串口日志 | UART 输出调试日志，可配置日志级别 |
| 红外现场调试 | 通过红外口现场调参、读取状态、触发测试 |
| 生产测试模式 | 产线校准、阀门测试、传感器测试、通信模组测试 |
| 调试信息 | LCD 显示调试信息、事件实时打印 |

## 12. 错误处理与安全设计
| 机制 | 描述 |
|------|------|
| 故障安全 | 阀门控制带机械保险，掉电时自动关闭 |
| 计量异常检测 | 脉冲计数异常（过快/过慢）触发报警 |
| 通信超时保护 | 通信失败自动重试 N 次后进入安全状态 |
| 数据校验 | 关键数据双重存储 + CRC 校验 |
| 看门狗 | 独立硬件看门狗，防止程序跑飞 |
| 存储保护 | 计量数据防篡改设计 |
| 计量算法保护 | 计量算法不可被配置参数修改（满足计量认证） |
| Bootloader 安全 | 安全启动、加密升级、防回滚 |

## 13. 测试策略
| 层级 | 测试方式 | 工具 |
|------|----------|------|
| 组件层 | 单元测试，桩函数/mock HAL | CMock / Unity |
| 中间件层 | 模块集成测试 | 自定义测试框架 |
| HAL 层 | 目标板测试，外设驱动验证 | 示波器/逻辑分析仪 |
| 系统层 | 整机功能测试、计量精度验证 | 标准流量计 |
| 生产测试 | 产线自动化测试 | 专用测试夹具 |

## 14. 功能详细设计
### 14.1 计量
#### 14.1.1 红外光传感器检测
采用红外光传感器进行计量检测，通过齿轮的黑白两块区域反射光线来判断计量。
**名称解释：**
- **环境光**：仅在环境光影响接收管下，采样得到的AD值
- **红外光**：在环境光、红外光影响接收管下，采样得到的AD值
- **环境光处理值**：环境光采样值根据转化算法计算得出
- **红外光处理值**：红外光采样值根据转化算法计算得出
- **环境光参考标准值**：正常情况下标准环境光处理值，默认为出厂环境光值（2700mV）
- **环境光参考值**：在标准值上根据自学习偏差补差后的参考值
- **自学习值p**：根据历史红外光处理值通过自学习算法计算得出，用于计算判决门限
- **偏差补偿上限Uf1、下限Uf2**：与当前自学习值配合，用于进行环境光参考补偿
- **判决门限Ustd**：用于判断齿轮面的标准值（Ustd = Uth + Uoff）
- **判决回差Ur**：判断齿轮面标准的波动范围
- **有效回差Lc**：齿轮转动过程中红外采样发生有效变化的范围
- **历史红外光最值**：在连续多个采样有效的红外光处理值中的最大值与最小值
**执行时机设计**：
计量算法采用 **"中断采集 + 主循环处理"** 的分层架构：
- **硬件层**：200ms定时器触发ADC采样，ADC完成中断中读取Le、Lc原始值存入环形缓冲区（执行时间<10us）
- **主循环层**：Metering_Process()从缓冲区取数据处理，执行有效计量检测和自学习算法
- **分段执行**：自学习算法（64组均值、一阶滤波）支持分段执行，单次执行<500us
**有效计量流程：**
1. AD采样得到Le、Lc
2. Le小于Lc，当前采样异常，采样有误次数累加，若大于等于3次，提示干扰-基本关系异常
3. Le、Ue均小于采样过小失效标准（默认20），当前采样异常，采样过小次数累加，若大于等于50次，提示采样异常-采样过小
4. Lc大于采样过大失效标准（默认1365），且Uc、Uoff差值小于采样接近标准（默认20），当前采样异常，采样过大次数累加，若大于等于50次，提示采样异常-采样过大
5. 不满足条件3、4，且存在采样，采样异常恢复次数累加，若大于等于采样异常恢复次数限制，清采样异常
6. 若当前处理发生计量异常，结束光取样操作
7. 环境光采样值Le转化为电压值，得到Ue
8. 红外光采样值Lc转化为电压值，得到Uc
9. 根据当前自学习值得出相对应的允许最小环境光，若环境光处理值小于该值，提示干扰-环境光偏差异常
10. 计算得出当前环境光参考值Uex
11. 计算得出Uoff
12. 根据自学习值计算得出Ustd
13. Ustd超过判决门限范围（默认范围为750-2100mV），提示干扰-判决门限异常
14. 若当前红外光处理值Uc与之前采样有效条件下Uc差值大于有效回差Uv，则本次采样有效
15. 若当前Uc < Ustd - Ur：当前齿轮面为白色，白色面次数累加
16. 若当前Uc > Ustd + Ur：当前齿轮面为黑色。若之前齿轮面颜色为白色，且白色面次数大于齿轮面更新标准（默认为1），则发生有效计量
**自学习流程：**
17. 根据有效计量流程中，若判定当前Uc采样数据有效，更新历史红外光最值
18. 若未发生有效计量，则退出自学习处理
19. 将历史红外光最大、最小值分别累加到历史最大值缓存和、历史最小值缓存和
20. 若更新的历史最大、最小缓存组数量小于最值缓存组数限制（默认为64），退出自学习处理
21. 分别计算得出历史最大值的均值，历史最小值的均值
22. 根据当前Usavr、Ussub，通过一阶滞后滤波算法得出最新历史均值、历史差值
23. 记录最新历史均值，更新自学习值Uth为该值
24. 记录最新历史差值，若该值在历史差值有效范围内，进行换算得出有效回差和判决回差
**快速自学习：**
25. 若本次采样无效，设置为默认查询周期
26. 修改为快速学习周期
27. 若当前红外光处理值Uc与之前快速学习值差值小于等于快速学习标准，结束光取样操作
28. 本次快速学习有效，更新到快速学习数组
29. 快速学习数组呈峰值时，若之前呈现过谷值，发生有效计量
30. 快速学习数组呈谷值时，若之前呈现过峰值，快速自学习结束，计算当前历史红外光最大、最小值的均值
**持久参数：**
| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| 环境光参考标准值 | uint16_t | 2700mV | 出厂设置 |
| 允许最小环境光 | uint16_t | 2300mV | |
| 最小历史差值 | uint16_t | 1000mV | |
| 判决回差标准值 | uint8_t | 250mV | |
| 脉冲当量 | uint32_t | 1000000 | 1imp=0.01m³ |
#### 14.1.2 计量事件处理
**采样异常：**
- 采样过小：光取样红外光、环境光采样均很小且接近，达到下限时触发
- 采样过大：光取样红外光、环境光采样均很大且接近，达到上限时触发
当非指定采样异常状态转换到指定采样异常状态时：
1. 产生报警，报警码详见附录
2. 执行普通关阀，关闭阀门不允许开启
3. 记录关阀请求事件
**提示干扰-1：**
包括：基本关系异常、环境光偏差异常、判决门限异常、红外光抖动、更新异常、快速学习异常。
当发生状态转换时：
4. 产生报警
5. 执行普通关阀
6. 记录关阀请求事件
报警解除：键按下时解除报警
**提示干扰-2：**
包括：自学习回差异常。
当发生状态转换时：产生报警
报警解除：光取样自学习过程中，下一次自学习处理若回差计算正常，自动清为正常状态
### 14.2 计量结算
#### 14.2.1 累积量
统计表累积用气量。将有效脉冲数乘以脉冲当量，得出此次消耗的气量，再累加累积气量。
- 脉冲当量：uint32_t，脉冲当量为1000000，表示1imp=0.01m³
- 累积气量：sLongLong，初始为0
#### 14.2.2 预留量
预留量用于表具厂内测试和出厂后用户开户前测试。
流程：
1. 预留量状态分为：功能开启、预留量为0状态；功能开启、预留量非0状态；功能关闭状态
2. 当预留量不为0时，每次走气，预留量扣减
3. 当预留量由不为0扣减到≤0时，为功能开启、预留量为0状态
4. 当预留量≤0，停止扣减预留量，功能关闭
**预留量用完事件：**
由状态转换触发：功能开启、预留量非0状态 → 功能开启、预留量为0状态
5. 若表具处于客户模式，如果配置了预留量用完关阀，则执行普通关阀，并锁定阀门
6. 若表具处于生产模式，如果配置了预留量用完关阀，则执行普通关阀
7. 记录关阀请求事件
#### 14.2.3 阶梯计价
**功能概述：**
根据用气量区间应用不同单价，支持多档阶梯计价。
**持久参数：**
| 参数 | 类型 | 说明 |
|------|------|------|
| 阶梯价数量 | uint8_t | 阶梯数量（1-5档） |
| 各档起始量 | uint32_t[] | 各档起始气量 |
| 各档单价 | uint32_t[] | 各档单价（分/m³） |
**计算流程：**
1. 根据累积用量确定当前所处阶梯
2. 按阶梯区间分别计算各档费用
3. 累加各档费用得出总费用
#### 14.2.4 后付费模式
**功能概述：**
后付费模式下，表端不进行余额扣减，只记录用量，后台进行计费和收费。
**业务规则：**
1. 表端记录累积用量，不进行余额判断
2. 远程通信上报用量数据给后台
3. 后台下发放开阀/关阀指令控制阀门
4. 透支标识由后台管理，表端仅做显示
**与预付费模式区别：**
| 项目 | 预付费模式 | 后付费模式 |
|------|------------|------------|
| 余额判断 | 表端判断 | 后台判断 |
| 关阀时机 | 余额不足时关阀 | 收到后台关阀指令时关阀 |
| 透支处理 | 可配置透支额度 | 由后台管理 |
### 14.3 时钟
#### 14.3.1 系统时钟（存储容错）
为燃气表提供日历时钟。
**流程设计：**
1. 时钟状态：系统端时钟状态分为异常状态和正常状态
2. 表端在掉电解除（系统上电）时，从存储设备中读取保存时钟，从硬件RTCC读取时钟
3. 若表内RTCC大于存储时钟，且差值≤1年时，使用当前的RTCC时钟
4. 若表内RTCC小于存储时钟或者表内RTCC大于存储时钟且差值＞1年，则表端使用存储时钟，并用该时间校时RTCC
5. 读取RTCC时，若RTCC时钟异常，使用存储时钟，同时将时钟状态设置为异常态，并触发时钟异常事件
6. 时钟保存：采用每天22点保存一次时钟；收到校时指令时立即校准时钟并保存
7. 校时：根据协议指令进行校时，校时前对下发的时间格式、月份和日期对应关系等进行有效性检查
**时钟异常事件处理：**
当发生状态转换时：时钟正常状态 → 时钟异常状态：
8. 产生报警，记录报警事件
9. 报警解除：校时成功后解除报警；按键不解除报警
---
### 14.4 电源管理
采用4节碱电+SPC+1节备用锂电的电源方案。
#### 14.4.1 四碱电主电
**电源状态转换规则：**
1. 系统初始复位时进入掉电状态，后随电量的变化变更电源状态
2. 除掉电状态可以转换到电量正常状态外，不允许低电量状态向高电量状态转换
3. 电量变化触发的电源事件包括：上电事件、电压低事件、锁定事件和掉电事件
**主电量检测：**
4. 当系统处于大功率状态时，持续检测电压
5. 大功率状态下，当电压>2.5V时，电量百分比与电压状态不变化
6. 其它情况下，每1秒检测一次电量
7. 非大功率状态下：共需检测3次进行确认
8. 大功率状态下：锁定确认共需检测20次进行确认；掉电确认共需检测10次进行确认
**锁定事件处理：**
当发生状态转换时（电量正常状态→锁定状态 或 电量低状态→锁定状态）：
9. 产生报警，清低电报警和电压掉电报警
10. 执行普通关阀，记录事件
11. 若液晶处于工作态，则关闭液晶
12. 关闭远程通信，关闭红外
13. 存储运行参数及关键参数
14. LED闪烁3次提示
**掉电事件处理：**
当发生状态转换时（电量正常状态→电量掉电状态 或 低电状态→掉电状态 或 锁定状态→掉电状态）：
15. 产生报警，清锁定报警和电压低报警
16. 执行普通关阀，记录事件
17. 若液晶处于工作态，则关闭液晶
18. 关闭红外
19. 存储运行参数及关键参数
20. LED闪烁3次提示
#### 14.4.2 锂电池备电
系统复位后，立即检测一次备电状态：
- 若备电＞2.6V，电量正常
- 若备电≤2.6V，电量低状态
**备电电量低事件：**
当发生状态转换时（电量正常状态→电量低状态）：
1. 产生报警，报警码详见附录
2. 立即触发液晶屏显示报警码
报警解除：键按下后，若当前备电处于电量正常状态，则解除备电报警
#### 14.4.3 SPC管理
SPC状态分为：主电充电态、备电充电态、非充电巡检态
**状态转换：**
1. 初始化时，默认假设SPC处于主电充电态
2. 当检测到SPC电压低于备电SPC充电启动阀值时，进行计次，并转为使用充电启动条件重采周期进行SPC电压采集
3. 如果连续采集到SPC电压低于备电SPC充电启动阀值次数大于SPC充电启动条件判定次数，如果当前处于非大功率下则进入备电充电态
4. 在主电充电态下，如果主电小于等于主电SPC充电启动阀值，则转为非充电巡检态
5. 在非充电巡检态、备电充电态下，如果检测到主电电压高于主电SPC充电启动阀值，则转为主电充电态
6. 大功率情况下，不巡检SPC电压，不允许启动备电充电
**SPC失效：**
首次在备电充电态下检测SPC电压低于SPC失效电压，记录SPC失效事件记录

### 14.5 阀门
#### 14.5.1 阀门动作
阀门物理状态分为：阀开状态、阀关状态、正在开阀状态、正在关阀状态、开暂停状态、关暂停状态
阀门的操作分为：普通开阀、普通关阀、强制关阀、强制开阀、二次关阀、强制停阀
**阀门操作流程：**
1. 可缓存一次阀门操作请求
2. 若阀门操作请求为强制停阀，则不管阀门当前正在执行何种操作，立即执行阀门停止，并清除当前操作
3. 若阀门操作请求不为强制停阀，且当前有操作在执行，缓存当前请求，等待当前操作执行完成
4. 若阀门操作请求不为强制停阀，且当前无操作在执行，则立即执行
**阀门状态液晶显示：**
- 阀开状态：显示"阀开"
- 阀关状态：显示"阀关"
- 开暂停状态及正在开状态：闪烁显示"阀开"
- 关暂停状态及正在关状态：闪烁显示"阀关"
**开阀前条件检测：**
存在下述条件则不允许开阀：
1. 当前电压处于锁定或掉电状态
2. 当前处于外部报警状态
3. 阀处于锁定状态
4. 光取样采样过大或采样过小异常
5. 当前处于倾斜报警未解除状态
**刹车机制：** 在阀门开或关动作结束时对阀门进行制动操作
**持久数据：**
- 开阀执行时间（单位：ms）：uint16_t，快速阀，默认2秒
- 关阀执行时间（单位：ms）：uint16_t，快速阀，默认0.2秒
#### 14.5.2 阀门管理
**事件源关阀使能配置及关阀优先级：**
1. 应用层向阀门管理业务传递阀门控制指令时，同时传递关阀类型和请求事件源
2. 电源处于锁定或掉电状态下，请求二次关阀无响应
3. 阀门处于非开状态时，微关/开阀请求无响应
4. 多请求源同时请求关阀操作时，执行高优先级事件源的关阀请求，忽略其它请求，阀关走气二次关阀大于其他事件关阀
**运行参数：**
- 阀门锁定状态【unsigned char】
- 事件源关阀使能：12个字节，每个位表示一个事件源的关阀使能配置
#### 14.5.3 阀门动作上传
1. 非指令控制关闭阀门情况下：
   - 阀门发生从关到开状态跳变时，启动延时远程通讯计时，计时达到10s，启动远程通讯
   - 阀门发生从开到关状态跳变时，立即启动远程通讯
2. 指令控制关闭阀门情况下，不启动远程通讯

### 14.6 声光提示
**功能概述：**
声光提示包括LED和蜂鸣器；亮(响)时间、灭(不响)时间、次数可调；在当前声光提示操作执行时，可以缓存一次后来请求的操作。
**流程设计：**
声光提示操作请求分为普通操作和抢占操作：
- **普通操作**：若当前无任务执行，则立即执行本次请求任务；若当前有任务正在执行，则缓存本次请求任务
- **抢占操作**：停止执行当前任务，立即执行本次请求任务

### 14.7 数据管理
#### 14.7.1 数据管理策略
**数据读写驱动层：**
- 读数据时：仅读一次，不作验证
- 写数据时：写操作完毕后回读数据，若读与写的数据不一致，则重写，最大重写次数3次
**冗余备份存储策略：**
1. 数据分主块和备块冗余保存
2. 数据写操作先写主块，若主块写成功，则再写备块；若主块写失败，则不再写备块
3. 读取存储数据时，先读主块数据，若不成功，则读备块；若读主块和备块均错误，则报E2错误
4. 读数据时：验证数据的CRC校验是否正确，若不正确，则重读，最大重读次数3次
#### 14.7.2 数据存储
**存储数据类型：**
| 数据类型 | 说明 | 存储频率 |
|----------|------|----------|
| 配置参数 | 服务器下发结束帧、各种配置指令 | 配置时存储 |
| 运行参数 | 每小时存储、服务器下发结束帧、开阀指令等 | 按条件存储 |
| 关键参数 | 累积量、预留量 | 每走1m³气量存储 |
| 记录数据 | 事件记录、日志记录 | 事件发生时存储 |
**关键参数存储：**
1. 本方案的关键参数为：累积量、预留量
2. 每走1m³气量存储一次关键参数
#### 14.7.3 数据管理策略应用
数据存于外部E2；表端数据存储策略应用如下：
| 数据类型 | 存储策略 | 说明 |
|----------|----------|------|
| 配置参数 | 冗余备份存储策略 | 服务器下发结束帧、各种配置指令时存储 |
| 运行参数 | 冗余备份存储策略 | 每小时存储、服务器下发结束帧、开阀指令等 |
| 关键参数 | 冗余备份存储策略 | 每走1m³气量存储一次（累积量、预留量） |
| 记录数据 | 直接使用驱动层 | 事件记录、日志记录直接使用驱动层接口 |
| 用户信息数据 | 写时存储，读时读取 | 收到写用户数据指令时存储，收到读指令时读取 |
#### 14.7.4 数据读写错误事件处理
基于冗余备份存储策略：
- 若主块存储失败，则记录报警事件
- 若备区存储失败，则记录报警事件
#### 14.7.5 本地通信
**功能描述：**
本地通信采用红外通讯，进行内部参数配置、数据读取等操作。
**通信流程：**
1. 红外由人工启动（按键触发）
2. 启动后，液晶显示FA-OFF或通信倒计时irdA-xx（xx为倒计时秒数）
3. 20秒内没有收到数据后，红外自动关闭
4. 收到正确的指令后重新计数，20秒内没有收到后续数据则关闭红外
5. 红外收到正确的配置指令后，声光报警一次
6. 本地通信收到关阀指令，则立即执行关阀动作
**本地通信与远程通信相互打断机制：**
| 场景 | 处理方式 |
|------|----------|
| 远程通信启动时 | 关闭本地通信（红外） |
| 本地通信启动时 | 打断远程通信（若远程通信在工作态，提前结束工作态） |
| 本地通讯结束，若表处于出厂状态且存在阀门动作上传请求 | 启动远程通讯 |
| 远程通信进行定时上传时被本地通信打断 | 不进行重试上传 |
**打断优先级：** 本地通信 > 远程通信（后来者优先）
#### 14.7.6 复位
**功能描述：**
系统复位后执行流程，远程升级之后复位不破坏EEPROM参数区的原有数据。
**复位流程：**
1. **主电电压检测**：
   - 检查主电电压，若小于系统启动门限值进行低功耗处理
   - 每秒重新检测，直到电压大于系统启动门限值，进入系统
2. **存储器检测**：
   - 检查存储器是否正常，若异常则液晶提示E-1001，不进入运行态
   - 检查方法：
     - 首次复位：清零事件记录区、用气日志区，将默认参数写入E2
     - 非首次复位：读取配置参数、运行参数、关键参数
     - 若写E2失败或读E2异常，则液晶提示E-1001，不进入运行态
3. **复位成功处理：**
   - 记录复位成功事件（运行事件）
   - 执行强制关阀
   - 记录关阀请求事件
   - LED闪烁一次（300ms）
   - 液晶进入上电显示
   - 若上电上传使能触发一次远程主动上报通信
**远程升级后复位说明：**
- 远程升级完成后，触发系统复位
- 复位过程中，EEPROM参数区数据不受影响
- 复位后系统从E2读取原有参数恢复运行状态
- 不破坏原有的计量数据、配置参数、运行数据
### 14.8 按键
表具有一个机械按键。
#### 14.8.1 按键模块架构
按键模块采用三层架构，职责分明：

┌─────────────────────────────────────────────────────────────┐  
│ 按键事件处理层 (KeyEvent) │  
│ - 4种按键事件识别与分发 │  
│ - 事件处理逻辑（开阀/通信/存储/显示） │  
│ - 按键延时上传任务管理 │  
└─────────────────────────────────────────────────────────────┘  
▲  
│ 按键事件  
│  
┌─────────────────────────────────────────────────────────────┐  
│ 按键检测层 (KeyDetect) │  
│ - 4种状态机（松开/按下消抖/按下/松开消抖） │  
│ - 消抖重触发机制 │  
│ - 按键时长计时 │  
└─────────────────────────────────────────────────────────────┘  
▲  
│ GPIO电平  
│  
┌─────────────────────────────────────────────────────────────┐  
│ 按键HAL层 │  
│ - 硬件GPIO读写接口 │  
│ - 中断/轮询模式适配 │  
└─────────────────────────────────────────────────────────────┘
#### 14.8.2 按键状态机（4种状态）

按下  
┌─────────────────────────┐  
│ ▼  
┌────┴────┐ 30ms消抖 ┌───────────┐ 30ms消抖 ┌───────┐  
│ 松开状态 │ ────────▶ │ 按下消抖状态 │ ──────────▶ │ 按下状态 │  
└─────────┘ └───────────┘ └───────┘  
▲ │ │  
│ │ │  
│ ┌───────────┘ │  
│ │ 30ms消抖 │  
│ ▼ │  
└─────────────────────────────────────────────────┘  
松开

| 状态 | 定义 | 外部输出 | 说明 |
|------|------|----------|------|
| 松开状态 | 按键未按下，处于稳定松开 | 松开 | 初始状态 |
| 按下消抖状态 | 发生按下触发时刻起到按下消抖时间到 | **仍输出松开** | 消抖期间状态不变 |
| 按下状态 | 经过按下消抖确定其按下的按键状态 | 按下 | 稳定按下状态 |
| 松开消抖状态 | 发生松开触发时刻起到松开消抖时间到 | **仍输出按下** | 消抖期间状态不变 |
**消抖重触发机制：**
- 按下消抖期间：若再次检测到按键按下，则**重启30ms消抖计时**
- 松开消抖期间：若再次检测到按键松开，则**重启30ms消抖计时**
**固定配置：**
| 参数 | 默认值 | 说明 |
|------|--------|------|
| 消抖延时时间 | 30ms | 按键消抖确认时间 |
| 最大持续按键时间 | 60s | 按下时长上限 |
#### 14.8.3 有效按键事件
表端识别的按键事件包括以下4类：
1. 键按下事件：按键按下时触发
2. 长按键事件（1<按下时长≤3秒）
3. 长按键事件（3<按下时长≤15秒）
4. 键按着事件：按键处于按着，且大于1秒以上触发
**键按下事件：**
5. 与液晶显示交互
6. 若当前主电电压非处于锁定状态或掉电状态：
   - 部分功能模块报警解除并初始化状态
   - 处于生产模式：LED灯闪烁2下
   - 触发开阀请求
**长按键事件（1-3秒）：**
1. 若当前主电电压处于锁定状态或掉电状态，则不响应该事件
2. 若当前主电电压非处于锁定状态或掉电状态，且处于生产模式时，检测备锂电电量
3. 若处于备电电量低状态，则显示报警码E-1007
4. 若处于备电电量正常状态，开启本地通信
**长按键事件（3-15秒）：**
5. 若当前主电电压处于锁定状态或掉电状态，则不响应该事件
6. 开启远程通讯
**键按着事件：**
7. 若当前主电电压处于锁定状态或掉电状态，则不响应该事件
8. 若持续按键时间>1秒：处于生产模式下，显示红外通信倒计时
9. 若持续按键时间>3秒：液晶显示-SEnd-
10. 若持续按键时间>15秒：触发关键参数存储；触发关阀请求；显示FA-OFF
#### 14.8.4 按键延时上传
**功能描述：**
按键触发开阀请求，阀门发生开阀动作时，延时启动远程通讯。
**状态机：**

开阀动作  
┌─────────────────────────┐  
│ ▼  
┌────┴────┐ ┌──────────────┐  
│ idle │ ──────────▶ │ waiting_10s │  
└─────────┘ └──────────────┘  
▲ │  
│ │  
│ 终止条件： │ 10s到达  
│ - 长按3s启动远程 └──────────────┐  
│ - 切换为未出厂 │ │  
│ - 按键按下清除计时 ▼ ▼  
│ ┌─────────┐  
└──────────────────────────────│ 启动通信 │  
└─────────┘

**延时上传流程：**
| 阶段 | 条件 | 动作 |
|------|------|------|
| 启动延时 | 开阀动作触发 | 启动延时远程通讯计时（10s） |
| 清除计时 | 延时期间发生按键按下动作 | 清除延时远程通讯计时 |
| 启动通信 | 红外不工作态 + 无按键计时达到10s | 启动远程通讯 |
| 启动通信 | 红外处于工作态 | 红外通信结束后启动远程通讯 |
| 终止任务 | 长按3s启动远程通讯 | 立即启动远程通讯，终止延时任务 |
| 终止任务 | 出厂状态切换为未出厂 | 终止延时启动通讯任务 |
**配置参数：**
| 参数 | 默认值 | 说明 |
|------|--------|------|
| 延时启动通讯时间 | 10s | 开阀后延时启动远程通信的时间 |
| 长按键启动通讯阈值 | 3s | 超过此时间触发长按键事件 |
---
### 14.9 液晶
采用段码液晶屏实现人机交互，机械按键的键按下事件用于屏幕切换。
#### 14.9.1 液晶显示
**常规显示内容：**
1. 剩余金额（单价不为0显示，该屏为首屏）
2. 单价（单价不为0显示）
3. 累计量（不显示剩余金额时该屏为首屏）
4. 年月日
5. 时分秒
6. 表号前六位
7. 表号后六位
8. 软件版本号
**插入显示：**
- 按键时间达到1秒（生产模式）：显示irdA-20
- 按键时间达到3秒：-SEnd-
- 键按着事件时间达到15秒：显示FA-OFF
- NB-IoT开始连接：显示-SEnd-
- NB-IoT连接成功：Sxxxxyyy
- NB-IoT通讯成功：-SUCC-
**锁定状态：**
表端正常运行中，每次刷新显示内容前均会检查电池状态，若电池状态为锁定状态，立即切换到锁定状态。
#### 14.9.2 菜单结构
**菜单层级：**

主菜单  
├── 用户模式  
│ ├── 1. 余额查询  
│ ├── 2. 用量查询  
│ ├── 3. 阀门操作  
│ └── 4. 信号查询  
│  
├── 维护模式（长按3秒进入）  
│ ├── 1. 参数配置  
│ ├── 2. 测试模式  
│ ├── 3. 诊断信息  
│ └── 4. 通讯测试  
│  
└── 管理模式（长按15秒进入）  
├── 1. 系统配置  
├── 2. 出厂设置  
└── 3. 清除数据

**菜单导航规则：**
- 短按按键：切换下一项/确认选择
- 按下时长1-3秒：进入子菜单/返回上级
- 按下时长>15秒：进入管理模式（需验证权限）

### 14.10 本地通信
1. 本地通信采用红外通讯，进行内部参数配置，数据读取等操作
2. 红外由人工启动；启动后，液晶显示FA-OFF或通信倒计时irdA-xx
3. 20秒内没有收到数据后，红外自动关闭
4. 收到正确的指令后重新计数，20秒内没有收到后续数据则关闭红外
5. 红外收到正确的配置指令后，声光报警一次
6. 本地通信，收到关阀指令，则立即执行关阀动作
**本地通信与远程通信相互打断：**
7. 远程通信和本地通信可以相互打断，后来者优先
8. 本地通讯结束，若表处于出厂状态且存在阀门动作上传请求，则启动远程通讯
### 14.11 远程通信
远程通信采用NB-IoT方式，工作模式设计兼容间歇性断电模式和PSM模式。
#### 14.11.1 工作模式
- **间歇性断电模式**：通信请求时，给NB-IoT模块供电，通信结束后给模块断电
- **PSM模式**：通信请求时，模块已经供电或重新供电，通信结束后不进行断电操作
**通讯模块状态：**
- 关闭态：通信模块处于断电状态
- 工作态：通信请求数据交互的一个过程态
- 待机态：专指PSM模式下的状态
#### 14.11.2 通信启动
通信可通过下列三类事件启动：
1. **人工（按键）启动**：如果通信模块已处于工作状态，显示E-4003错误；否则立即启动通信
2. **事件启动**：若通信模块已处于工作状态，则立即打断原来的通信；否则立即启动通信
3. **定时启动**：若通信模块已处于工作状态，则立即打断原来的通信；否则立即启动通信
#### 14.11.3 定时通信管理策略
1. 可指定是否启用定时通信
2. 定时通信周期可配置为按月管理或按天管理
3. 按月管理时，可指定上传日期
4. 按天管理时，可指定多少天通信一次；可指定每天上传的次数（最大为3）
5. 错峰机制：错峰时间（单位s）计算方式：表号后三位*(错峰系数*60/8)+60s
6. 每月最后一天上传一次
#### 14.11.4 电量限制与启动频率限制
1. 在电量锁定状态下禁止启动远程通信
2. 在电压锁定状态或掉电状态下禁止启动远程通信
3. 若工作态或休眠态检测到电量锁定或电压锁定或电压掉电，则立即进入关闭态
4. 启动频率限制：20分钟内启动次数不得超过启动频率限制次数（默认6次）
5. 每0分、20分、40分清除启动次数
#### 14.11.5 多天不上传事件
1. 此功能默认关闭
2. 当通信结束后，若多天不上传天数大于等于不上传天数阀值，触发多天不上传事件
3. 不管是何种启动方式，若通信成功，则清除多天不上传状态
**多天不上传事件处理：**
当发生状态转换时（正常状态→多天不上传状态）：
4. 产生报警
5. 执行普通关阀，并锁定阀门
6. 记录关阀请求事件
报警解除：仅能通过远程/本地通信解除报警
### 14.12 通信协议
金卡通用协议：采用物联网远程数据协议V1.17
**配置指令：**
| 指令 | 配置参数 | 客户模式可配置 |
|------|----------|----------------|
| 结束帧 | 系统时钟、单价、剩余金额、服务器提示信息 | 是 |
| 校对时钟 | 系统时钟 | 否 |
| 强制关阀 | 无 | 是 |
| 开阀 | 无 | 是 |
| 设置几天无数据上传关阀 | 不上传天数 | 是 |
| 设置表具回传周期 | 上传周期的模式，回传天数，回传时间 | 是 |
| 配置出厂标志 | 出厂标志 | 是 |
| 修改备份服务器参数 | 备份服务器IP、端口、上传服务器天数 | 是 |
| 修改累积量 | 修改累积量 | 否 |
| 设置几天不用气关阀 | 一级不用气天数 | 是 |
| 修改GPRS通讯地址及端口 | 客户服务器IP、端口、网络模式、表号 | 不可配置表号 |
| 配置表基本参数 | 具体见协议 | 是 |
| 普通关阀 | 无 | 是 |
| 配置预留量 | 预留量 | 否 |
| 配置NB工作模式 | 工作模式 | 是 |
| 配置NB通信参数 | 入网方式、NBAND、NEARFCN等 | 是 |
| 倾斜报警 | 倾斜报警后阀门控制选项 | 是 |
| 配置持续用气超时功能参数 | 持续用气超时功能使能、阀门控制选项、标准、I档流速、II档流速 | 是 |
| 配置几天一上传功能参数 | 几天一上传、每月固定上传日、每天上传时分 | 是 |
| 配置微小流功能参数 | 微小流功能使能、报警上限、报警下限、检测时间阈值 | 是 |
| 配置不受控流量功能 | 不受控流量功能控制、检测时间、检测值 | 是 |
| 生产出厂功能参数配置 | 多天不上传关阀使能、多天不用气关阀使能等 | 是 |
| 报警解除 | 无 | 是 |
**读取指令：**
读取客户资料、读取阀门动作存储信息、读用户资料数据、读取SIM卡信息、读取指定日的一小时用气量信息、读取指定日的半小时用气量信息、读取光取样参数信息、读取电量信息、读取复位事件信息、读取通信日志信息、读取通信时长信息、读取NB通信参数信息、获取模组版本信息

### 14.13 事件记录
#### 14.13.1 事件记录
1. **关阀请求事件记录**：存储10条关阀请求事件，数据满后进行循环覆盖
2. **运行事件记录**：存储10条事件（复位事件记录），数据满后进行循环覆盖
3. **通信网络情况事件记录**：存储20条事件，数据满后进行循环覆盖
**事件记录格式：**
AAAA+YYMMDDHHMMSS+XX
其中AAAA:事件类型，2字节；YYMMDDHHMMSS：事件时间6字节,BCD；XX预留1字节
#### 14.13.2 通信日志记录
1. **定时远程通信日志记录**：存储5条通信结果事件，数据满后进行循环覆盖
2. **人工远程通信日志记录**：存储5条通信结果事件，数据满后进行循环覆盖
3. **定时远程通信未启动日志记录**：存储5条通信未启动事件，数据满后进行循环覆盖
**日志记录格式：**
AAAA+YYMMDDHHMMSS+XX+RSRP+SNR+PCI

### 14.14 用气日志
#### 14.14.1 天用气日志存储
1. 每天零点保存累积量。存储1年的用气记录，每月固定首地址
2. 每月1号0点先清除该月的32天的存储数据，再保存累积量
3. 当出现掉电（非复位）后，间隔多天上电，将掉电期间的数值一律修补为当前的用气量值
4. 采用内存缓存当月数据的策略
**用气日志格式：**
单条用气日志格式：累积量（4字节整数+1字节小数+1字节CRC8校验）
#### 14.14.2 半小时用气日志存储
1. 单条用户记录数据格式：累积量（4字节整数+1字节小数+1字节CRC8校验）
2. 一天半小时日志组织：每半点和整点进行日志记录，共49个点
3. 两个月用气记录存储排布（奇数月/偶数月）
4. 总共消耗存储空间：31 × 2 × (49 × 6 + 2) = 18352 Byte

### 14.15 用气安全监测
#### 14.15.1 ExtAlarm 外部报警
**功能概述：** 检测外部报警器的报警信息，当检测到报警信号后输出事件
**持久参数：**
- 外部报警功能使能位：默认禁能；可通过本地和远程通信配置
**流程设计：**
1. 以1秒的时间间隔查询外部报警信号
2. 若查询到外部报警异常信号，则累加外部报警计数，当外部报警计数≥外部报警确认次数阈值（默认3次）时，置为外部报警状态
3. 一旦查询到外部报警正常，则外部报警解除
**事件处理：**
当发生状态转换（正常状态→外部报警状态）：
4. 产生报警
5. 执行普通关阀，记录关阀请求事件
6. 触发一次远程通信
报警解除：当前不处于外部报警状态时，键按下时解除报警
#### 14.15.2 ValveLeak 阀关走气检测
**功能概述：** 检测阀门关不严情形，在阀关条件下仍检测到流量时输出事件
**持久参数：**
- 阀关走气确认次数阈值：默认2次
**流程设计：**
1. 发生有效计量时，若阀门为关状态，则累加阀关走气计数
2. 当阀关走气≥阀关走气确认次数阈值时，置为阀关走气状态
3. 发生有效计量时，若阀门为开状态，清除阀关走气计数
**事件处理：**
当发生状态转换（正常状态→阀关走气状态）：
4. 若当前电源不处于锁定状态或掉电状态，则执行二次关阀，记录关阀请求事件
5. 第一次阀关走气事件清除阀关走气状态
#### 14.15.3 直通检测
**功能概述：** 连续两次阀关走气事件触发直通报警
**事件处理：**
当直通事件被触发时：
1. 产生报警
2. 执行普通关阀，并锁定阀门
3. 记录关阀请求事件
报警解除：通过远程或本地通信解除报警
#### 14.15.4 BigFlow 异常大流量
**功能概述：** 检测燃气流速超过1.5倍最大用户负荷流量的情形
**持久参数：**
- 异常大流量系数（单位：秒）：uint8_t
- 异常大流量功能使能：默认禁能
**流程设计：**
1. 通过计算相邻0.1 m³气量间的时间，并与异常大流量系数进行比较
2. 当相邻0.1 m³气量间的时间＜异常大流量系数，且连续次数≥异常大流量次数阈值（默认3次），置为异常大流量状态
**事件处理：**
当发生状态转换（正常状态→异常大流量状态）：
3. 产生报警
4. 执行普通关阀
报警解除：键按下解除报警
#### 14.15.5 NoGasUsage 多天不用气
**功能概述：** 检测燃气表阀开状态下用户长期不用气的情形
**持久参数：**
- 一级不用气天数阈值（单位：天）：uint16_t
- 一级不用气使能：默认禁能
- 二级不用气天数阈值（单位：天）：uint16_t
- 二级不用气使能：默认禁能
**状态：**
1. 初始化状态
2. 不用气正常状态
3. 一级不用气状态
4. 二级不用气状态
**流程设计：**
5. 阀开状态时，且没有检测到有效计量，发生时钟的天更新，则统计不用气计时
6. 发生有效计量，不管处于何种状态，均清除不用气计时，回到正常状态
**事件处理：**
一级不用气事件：
7. 当状态从正常状态转换到一级不用气状态时：产生报警，执行普通关阀
8. 报警解除：键按下时清除状态
二级不用气事件：
9. 当发生状态转换到二级不用气状态时：产生报警，执行普通关阀，并锁定阀门
10. 报警解除：只能通过远程、本地通信进行报警解除
#### 14.15.6 ContFlowTimeout 持续流量超时
**功能概述：** 检测持续用气达到一定时间标准的情形
**持久参数：**
- 持续流量超时检测使能：默认不启用
- 持续流量超时计时标准：uint16_t，单位分（默认值600）
- 持续流量超时I档流速：uint16_t，单位L/h（默认值：0.01m³/h）
- 持续流量超时II档流速：uint16_t，单位L/h（默认值：0.2m³/h）
**状态：**
1. 初始状态
2. 正常状态
3. 用气状态
4. 用气超限状态
**流程设计：**
5. 发生有效计量时，通过计算消耗0.01m³气量的时间，检测当前用气状态
6. 当一档计时≥用气超时计时标准，置为用气超限状态
**事件处理：**
7. 触发持续流量超时报警
8. 关阀使能条件下，触发关阀请求
9. 触发一次远程通信
**报警解除：**
10. 持续流量超时管理阀门选项非锁定关，按键解除报警
11. 持续流量超时管理阀门选项为锁定关，开阀指令解除报警
#### 14.15.7 TinyFlow 微小流
**功能概述：** 检测气体流量持续小于2倍始动流量的情形
**持久参数：**
- 微小流量系数上限（单位：分）：uint16_t
- 微小流量系数下限（单位：分）：uint16_t
- 微小流量持续时间阈值（单位：小时）：uint16_t
- 微小流量功能使能禁能标志：默认禁能
**状态：**
1. 初始化状态
2. 正常状态
3. 异常微小流量状态
**流程设计：**
4. 当相邻0.01 m³气量间的时间≤微小流量系数下限，则本次流速正常
5. 当相邻0.01 m³气量间的时间＞微小流量系数下限，且＜微小流量系数上限，则认为当前流量为微小流量范围
6. 若微小流量统计次数累加达到统计阈值（默认12次），则认定为异常微小流量状态
**事件处理：**
7. 触发微小流报警
8. 关阀使能条件下，触发关阀请求
9. 触发一次远程通信
**报警解除：**
10. 微小流管理阀门选项非锁定关，按键解除报警
11. 微小流管理阀门选项为锁定关，开阀指令解除报警
#### 14.15.8 UnctrlFlow 不受控流量
**功能概述：** 当阀门从关闭状态跳转到开启状态时，检测到设定的检测时间内，累计脉冲计量达到设定的检测值
**持久参数：**
- 不受控流量检测使能：uint8_t，默认不启用
- 不受控流量检测时间：uint16_t，单位秒
- 不受控流量检测值：uint16_t，单位0.0001m³
**状态：**
1. 初始状态
2. 正常状态
3. 异常状态
**流程设计：**
4. 发生阀门状态从关闭状态到开启状态跳转时，开始计时并统计计量值
5. 当计时大于等于不受控流量检测时间，则结束检测
6. 当统计的计量值大于等于不受控流量检测值，则输出不受控流量事件
**事件处理：**
7. 触发不受控流量报警
8. 关阀使能条件下，触发关阀请求
9. 触发一次远程通信
**报警解除：** 有效按键解除不受控流量报警

### 14.16 倾斜检测
#### 14.16.1 倾斜检测模块
**功能概述：** 检测燃气表是否发生倾斜行为，当检测到发生倾斜现象时输出事件
**持久参数：**
- 配置参数：倾斜检测使能，默认禁止
- 运行参数：自检标志，默认通过
- 默认参数：状态确认次数为10次，放正判定状态或倾斜判定状态下的检测周期为100ms
**流程设计：**
1. 以中断方式检测倾斜拆除信号，以查询方式检测放正信号
2. 产生倾斜中断时，立即进入倾斜判定状态，每100ms检测一次报警IO口电平状态
3. 若连续10次均检测到倾斜电平状态，则置倾斜状态
4. 处于倾斜状态下，以3秒为周期检测报警口电平信号
5. 检测到放正电平信号时，每100ms检测一次倾斜检测IO口电平信号，若连续10次均检测到放正电平状态，则置正常状态
#### 14.16.2 倾斜功能报警业务处理
**持久参数：**
配置参数：可通过指令配置倾斜报警是否关阀，本项目中倾斜报警方式默认触发权限关阀
**状态变化：**
正常状态到倾斜报警状态：
1. 当主电处于掉电状态且备电大于2.7V：
   - 记录倾斜报警事件记录
   - 若业务允许锁定阀门权限，则锁定阀门权限
   - 不触发远程告警
   - 不触发关阀请求
2. 当主电处于未掉电状态：执行报警事件处理
**报警解除：**
表具处于放正状态时，收到报警解除指令后才能解除倾斜报警。报警解除后，收到开阀指令，才允许开阀
#### 14.16.3 倾斜功能自检业务处理
**功能描述：** 出厂前进行自检，检查倾斜报警元器件工作是否正常
**检测流程：**
1. 启动条件：表端完全复位且处于未出厂模式，无视功能使能与否，无视之前自检结果
2. 自检通过条件：若分别检测到过倾斜状态和放正状态，则自检通过
**事件处理：**
当表端未出厂且自检未通过，不允许开阀，提示错误代码。出厂后不再提示

### 14.17 复位
系统复位后执行流程：
1. 检查主电电压，若小于系统启动门限值进行低功耗处理
2. 检查存储器是否正常，若异常则液晶提示E-1001，不进入运行态
3. 复位成功，触发复位成功事件：
   - 记录复位成功事件
   - 执行强制关阀
   - LED闪烁一次（300ms）
   - 液晶进入上电显示
   - 若上电上传使能触发一次远程主动上报通信
### 14.18 Alarm 统一报警管理
**功能概述：**
Alarm组件作为统一报警管理者，负责：
- 维护全局报警状态
- 管理报警码优先级
- 触发报警声音
- 协调各安全检测组件的报警逻辑
**接口设计：**
| 接口 | 描述 |
|------|------|
| `Alarm_SetStatus(alarm_id, status)` | 设置报警状态 |
| `Alarm_GetStatus(alarm_id)` | 获取报警状态 |
| `Alarm_TriggerSound(alarm_id)` | 触发报警声音 |
| `Alarm_Clear(alarm_id)` | 清除报警 |
| `Alarm_GetActiveAlarms()` | 获取所有活动报警 |
**报警优先级：**
1. 计量异常（最高优先级）
2. 安全报警（外部报警、直通、异常大流量等）
3. 电源报警
4. 通信报警（最低优先级）
**状态机：**
- 无报警状态
- 单一报警状态
- 多报警状态（显示最高优先级报警）
## 15. 硬件设计需求
### 15.1 硬件结构框图
产品电子硬件设计分为7个部分：主控板、远程通信模块、阀门、天线、报警接口（硬件预留）、采样齿轮、电源、安全模块、以及倾斜报警模块。
**主要模块：**
- **主控板**：产品的主控电路，包括主MCU，液晶，按键，存储（eeprom和flash），光取样计量电路，声光电路，电源检测电路，磁取样电路（预留），本地红外通信模块
- **采样齿轮**：为实现光取样计量的特制齿轮
- **阀门**：安置于基表上，通过连接线的形式和主控板进行对接
- **电源**：为整套系统提供电源
- **远程通信模块**：基于NB-IoT的通信模块
- **天线**：通信模块的射频匹配天线
- **报警接口**：硬件预留（报警器常开接口方式）
- **倾斜检测模块**：安置于基板上，通过滚珠模块检测是否发生倾斜
### 15.2 MCU
主平台MCU选择瑞萨的R7F0C004M2芯片：
- ROM：128K Byte
- RAM：8K Byte
- 引脚：80P
备用MCU：R7F0C020M2（256K ROM，24K RAM）
### 15.3 电源方案
**电池选型：** 4节碱电+1节备用锂电（14250）+1个SPC(1520)
**电压转换：**
- 主电碱电+SPC系统电压转换：1入分3出模式
  - 6.4V通过LDO转换为3.8V输出供SPC充电
  - SPC出来一路供阀门电路工作使用
  - 再通过LDO转换为3.0V输出，供单片机及相关电路工作使用
- 备电锂电系统电压转换：1入1出模式
### 15.4 供电检测
采用3路AD检测方式模块化电路，与MCU连接占用6个IO口，AD检测口使用至少10位AD精度
### 15.5 存储
- 外部EEPROM存储方式
- 通用存储使用资源要求在8K字节
- 半小时存储（2个月）：约18K字节
- 设计采用EEPROM：CAT24C256（32K Byte）
### 15.6 其他硬件需求
- **晶振**：外部32.768kHz时钟源
- **液晶**：金卡通用24脚液晶
- **按键**：机械按键，使用1个中断IO
- **LED/蜂鸣器**：1个普通IO引脚
- **计量采样**：光取样技术，使用3个IO口
- **阀门控制电路**：使用DRV8837芯片，2个普通IO口
- **红外通信**：UART接口和一个PWM调制引脚
- **倾斜检测电路**：2个普通IO引脚
- **外部报警器**：1个IO（预留带中断功能）
- **远程通信电路**：NB-IoT方式，使用移远电信模组
## 16. 结构设计需求
### 16.1 结构架构
### 16.2 主要组件
- **基表**：金卡铝壳、丹东钢壳
- **阀门**：沿用金卡快速阀
- **底座**：满足IP65防护需求
- **计数器**：各基表表型采用现有符合光取样技术的计数器
- **采样齿轮**：各基表计数器配套采用现有对应的符合光取样技术的采样齿轮
- **上盖**：满足IP65防护需求
- **挡光片**：对光取样区域以及采样齿轮区域进行深度的挡光防护作用
- **控制器**：安装于上盖上
- **通信板**：安装于上盖的电池仓边上
- **电池仓**：碱电池版满足主电加备电的方式
### 16.3 射频天线
采用FPC天线，优先考虑钢壳铝壳统一设计
---
## 17. 工艺设计需求
### 17.1 基表制造工艺
沿用各基表的制造工艺
### 17.2 整表组装工艺
1. 上盖铭牌信息与警示信息的打印
2. 底座控制器、通信版和天线的安装
3. 各插线的连接
4. 上盖的固定
### 17.3 基板制造工艺
1. 先烧写基板程序
2. 再焊接SPC电容引脚短接点
3. 其它制造工艺沿用JD1702
### 17.4 整表检测工艺
1. 工艺流程沿用G2碱电，使用G2对应的测试软件
2. 管道防拆功能在整表时进行检测
3. 外部报警器检测
## 18. 系统软件设计需求
### 18.1 测试调试软件
主要用于本地红外测试调试：
- 对新增指令进行添加，完善测试软件
### 18.2 售后维护软件
使用APP工具作为售后维护工具
### 18.3 主控板测试工装软件
用于完成主控板的自检任务
### 18.4 通信板测试工装软件
用于完成通信板的检定以及注册等任务
### 18.5 生产设置软件
主要用于生产设置写表号参数等
### 18.6 生产跑功能软件
主要用于生产跑功能确认，通过远程通信完成生产确认
## 19. 接口需求
### 19.1 结构与硬件接口需求
1. 主控板和通信板对应的尺寸与布局配合
2. 液晶、按键、LED的接口确认其相对位置
3. 碱电池和备用锂电池的硬件接口位置的布局
4. 光取样技术的接口配合
5. 挡光片防止光干扰的相对保护位置
6. 阀门、射频天线对应的连接线与PCB的连接
### 19.2 软件与工艺接口需求
1. 主控板工装自检接口
2. 通信板工装检测接口
3. 通过本地红外通信接口完成生产设置和品质检定
4. 通过远程通信接口完成生产确认验证
## 20. DFX
### 20.1 可测试性
1. 满足研发测试要求，本地通信软件支持参数的配置和读取
2. 满足主控板和通信板的工装测试，硬件预留测试焊盘
### 20.2 可制造性
1. PCB的布局便于提高贴片化的效率，使用单面贴面
2. PCB的拼版的工艺边设计满足生产分板要求
3. 硬件满足工装测试，预留测试串口
### 20.3 可维护性
1. 本地通信维护方式，采用APP来完成功能操作
2. 人性化的终端业务设计展现
3. 碱性电池可以由用户自行更换
## 21. 风险预估
1. 硬件上增加滚珠防拆，做钢壳铝壳兼容需要适配
2. 终端需要与采集系统、业务系统对接，联调进度可能不一致
3. 计量算法在200ms采样周期下的精度需通过型评认证验证
## 22. 标准要求
| 标准编号 | 名称 |
|----------|------|
| GB 3836.1-2010 | 爆炸性环境 第1部分：设备 通用要求 |
| GB 3836.4-2010 | 爆炸性环境 第4部分：由本质安全型"i"保护的设备 |
| GB/T 4208－2017 | 外壳防护等级(IP代码) |
| GB/T 6968-2019 | 膜式燃气表 |
| GB/T 17626.2-2006 | 电磁兼容 试验和测量技术 静电放电抗扰度试验 |
| JJF 1354-2012 | 膜式燃气表型式评价大纲 |
| CJT 449-2014 | 切断型膜式燃气表 |
| GB/T 17626.3-2006 | 电磁兼容 试验和测量技术 射频电磁场辐射抗挠度试验 |

## 23. 认证要求
- **IP65的防护认证**
- **防爆认证**：借用G1碱电防爆认证
- **型评认证**：借用G1碱电型评认证
- **计量认证**：满足 OIML R31 / GB/T 6968 标准
- 计量算法和关键参数存储需通过认证测试，不可随意修改
## 24. 后续步骤
本设计文档批准后，将进入 **writing-plans** 阶段，制定详细实现计划。
重点实现任务：
1. TaskManager调度器框架搭建
2. 软件定时器抽象层实现
3. Metering计量模块独立实现与验证
4. HAL层双平台（RL78/华大）适配
5. 各业务组件状态机实现
6. 系统集成与计量精度验证
## 25. 修订历史
| 版本 | 日期 | 描述 |
|------|------|------|
| v1.0 | 2026-04-13 | 初始版本 |
| v1.1 | 2026-04-13 | 增加 E2Storage 组件及 E2 分区设计（计量区/配置区/密钥区/日志区/运行区/固件区/启动区） |
| v1.2 | 2026-04-13 | 增加 IO 管理设计（集中管理、配置表驱动、未使用 IO 安全处理） |
| v1.3 | 2026-04-15 | 补充总体技术方法中的详细功能规格（计量、结算、时钟、电源、阀门、声光、数据管理、按键、液晶、通信、用气安全监测、倾斜检测等）及硬件/结构/工艺设计需求 |
| v1.4 | 2026-04-15 | 框架调整：组件从19个拆分为24个，拆分GasSafety为独立检测组件（ExtAlarm/ValveLeak/BigFlow/NoGasUsage/ContFlowTimeout/TinyFlow/UnctrlFlow），增加Alarm统一报警管理、Billing阶梯计价、后付费模式、HMI菜单结构设计 |
| v1.5 | 2026-04-15 | 补充按键检测详细设计：增加4状态机图、状态定义表、外部输出规则、消抖重触发机制、详细检测流程 |
| v1.6 | 2026-04-15 | 补充按键延时上传设计：状态机、延时上传流程、时序示例、配置参数 |
| v1.7 | 2026-04-15 | 重构按键模块：新增Key组件（KeyDetect+KeyEvent），HMI仅保留显示相关，定义按键事件类型和分发接口，按键延时上传由KeyEvent管理 |
| v1.8 | 2026-04-15 | 补充缺失章节：数据管理策略应用（存储策略分类说明）、本地通信（通信打断机制）、复位（远程升级后不破坏EEPROM参数） |
| v1.9 | 2026-04-15 | 架构调整：EventScheduler从组件层迁移至中间件层，新增中间件组件清单（DLMS/BillingAlgorithm/EventScheduler/DataManager），组件数量从25调整为24 |
| v2.0 | 2026-04-16 | 新增第4章"任务调度器设计"，更新系统架构图，补充TaskManager与各组件的交互关系 |
| v2.1 | 2026-04-17 | 新增可配置任务管理设计（4.4节）；组件分类细化（核心/可配置/模式相关）；补充状态表方案对比分析；更新调度器接口定义 |
| v2.2 | 2026-04-24 | 新增批量配置事务机制（4.4.4节）：增加 TaskManager_BeginConfig/CommitConfig/CancelConfig/PendTaskChange 接口，解决多配置并发下发的原子性问题；更新调度器接口列表（4.4.7节） |
| v3.0 | 2026-04-28 | 重构任务调度器设计（第4章）：采用请求驱动 + FIFO队列模型替代状态表遍历；新增软件定时器池统一管理定时任务；新增分级睡眠管理（STOP/SNOOZE/RUN）；简化任务状态为开启/关闭二元模型；更新系统架构图 |
| v3.1 | 2026-04-29 | 定时器设计统一：取消1任务=1定时器限制，增加oneshot模式，区分组件内部计时与TaskManager定时器；修复FindFirstSet查表范围bug、请求位图竞态条件；整合第5章与第4章为统一架构，删除独立SoftwareTimer.c层；get_next_wakeup()接入睡眠决策 |
