/**
 * @file BoardConfig.h
 * @brief RL78 板级配置 — 引脚分配、时钟设置、任务 ID 枚举
 *
 * 本文件定义 RL78 平台的板级配置：
 * - 主时钟和子时钟频率
 * - 24 个任务 ID 枚举（TaskId_t）
 * - 27 个 GPIO 引脚分配枚举（IOPin_t）
 *
 * 组件代码通过 TaskId_t 和 IOPin_t 引用任务和引脚，
 * 不直接使用数字常量，实现平台无关性。
 */
#ifndef BOARD_CONFIG_RL78_H
#define BOARD_CONFIG_RL78_H

#include "hal_common.h"

/* ---- 时钟配置 ---- */
#define MAIN_CLOCK_HZ       32000000UL   /**< 主时钟频率（Hz） */
#define SUB_CLOCK_HZ        32768UL      /**< 子时钟频率（Hz，32.768kHz 晶振） */

/* ---- 任务数量 ---- */
#define TASK_COUNT           24   /**< 任务总数（24 个业务组件） */

/* ---- 任务 ID 枚举 ---- */
/**
 * @brief 任务 ID 枚举 — 24 个业务组件的唯一标识
 *
 * 核心组件（0-7, 20-23）：系统运行基础，不可配置关闭
 * 可配置组件（8-19）：可通过远程/本地通信指令使能/禁能
 */
typedef enum {
    TASK_METERING      = 0,   /**< 计量组件 — 红外光采样、脉冲采集、自学习 */
    TASK_KEY           = 1,   /**< 按键组件 — 消抖检测、长按事件 */
    TASK_ALARM         = 2,   /**< 报警组件 — 统一报警管理 */
    TASK_VALVE_CTRL    = 3,   /**< 阀门控制 — 状态机、开阀条件检测 */
    TASK_HMI           = 4,   /**< 人机交互 — LCD 显示、LED 指示 */
    TASK_BILLING       = 5,   /**< 计费组件 — 阶梯计价、余额管理 */
    TASK_COMM          = 6,   /**< 远程通信 — NB-IoT 通信管理 */
    TASK_INFRARED      = 7,   /**< 红外通信 — 本地配置/调试 */
    TASK_DATA_STORAGE  = 8,   /**< 数据存储 — 参数持久化策略 */
    TASK_E2_STORAGE    = 9,   /**< E2 存储 — 分区管理、磨损均衡 */
    TASK_EVENT_LOG     = 10,  /**< 事件记录 — 关阀/运行/通信事件 */
    TASK_USAGE_LOG     = 11,  /**< 用气日志 — 天用气/半小时用气 */
    TASK_EXT_ALARM     = 12,  /**< 外部报警 — 外部报警器信号检测 */
    TASK_VALVE_LEAK    = 13,  /**< 阀关走气 — 阀门泄漏检测 */
    TASK_BIG_FLOW      = 14,  /**< 异常大流量 — 流速超限检测 */
    TASK_NO_GAS_USAGE  = 15,  /**< 多天不用气 — 一级/二级检测 */
    TASK_CONT_FLOW     = 16,  /**< 持续流量超时 — 长时间用气检测 */
    TASK_TINY_FLOW     = 17,  /**< 微小流 — 微小流量检测 */
    TASK_UNCTRL_FLOW   = 18,  /**< 不受控流量 — 开阀后异常流量检测 */
    TASK_TILT          = 19,  /**< 倾斜检测 — 倾斜状态检测与自检 */
    TASK_UPGRADE       = 20,  /**< OTA 升级 — 远程固件升级 */
    TASK_POWER_MGMT    = 21,  /**< 电源管理 — 主备电切换、SPC 管理 */
    TASK_BATTERY       = 22,  /**< 电池检测 — 主电类型/电量检测 */
    TASK_CLOCK         = 23   /**< 时钟 — RTC 管理、时间同步 */
} TaskId_t;

/* ---- GPIO 引脚分配枚举 ---- */
/**
 * @brief GPIO 引脚枚举 — RL78 平台的引脚分配
 *
 * 组件代码通过枚举值引用引脚，不直接使用端口号。
 * HDSC 平台使用同名枚举但映射到不同的物理引脚。
 */
typedef enum {
    PIN_METER_PULSE = 0,   /**< 计量脉冲输入 */
    PIN_METER_LED_TX,      /**< 计量红外发射管 */
    PIN_METER_LED_RX,      /**< 计量红外接收管 */
    PIN_VALVE_CTRL_A,      /**< 阀门控制 A 相 */
    PIN_VALVE_CTRL_B,      /**< 阀门控制 B 相 */
    PIN_VALVE_FB,          /**< 阀门反馈信号 */
    PIN_LED_RUN,           /**< 运行指示灯 */
    PIN_LED_ALARM,         /**< 报警指示灯 */
    PIN_BUZZER,            /**< 蜂鸣器 */
    PIN_KEY_MENU,          /**< 菜单按键 */
    PIN_EXT_ALARM,         /**< 外部报警器输入 */
    PIN_TILT_DETECT,       /**< 倾斜检测输入 */
    PIN_NB_POWER,          /**< NB-IoT 模组电源控制 */
    PIN_NB_RESET,          /**< NB-IoT 模组复位 */
    PIN_NB_RI,             /**< NB-IoT 模组振铃指示 */
    PIN_IR_TX,             /**< 红外发射 */
    PIN_IR_RX,             /**< 红外接收 */
    PIN_BAT_MAIN_SENSE,    /**< 主电电压检测 */
    PIN_BAT_BACKUP_SENSE,  /**< 备电电压检测 */
    PIN_SPC_SENSE,         /**< SPC 电压检测 */
    PIN_LCD_CS,            /**< LCD 片选 */
    PIN_LCD_RST,           /**< LCD 复位 */
    PIN_LCD_SDA,           /**< LCD 数据 */
    PIN_LCD_SCL,           /**< LCD 时钟 */
    PIN_UART_TX,           /**< 调试串口 TX */
    PIN_UART_RX,           /**< 调试串口 RX */
    PIN_COUNT              /**< 引脚总数（仅用于数组大小） */
} IOPin_t;

#endif /* BOARD_CONFIG_RL78_H */
