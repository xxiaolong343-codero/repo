/**
 * @file BoardConfig.h
 * @brief HDSC 板级配置 — 引脚分配、时钟设置
 *
 * 本文件定义华大芯片平台的板级配置。
 * 时钟和任务 ID 与 RL78 平台一致（复用 BoardA_RL78/BoardConfig.h）。
 * 引脚枚举同名但映射到不同的物理引脚。
 */
#ifndef BOARD_CONFIG_HDSC_H
#define BOARD_CONFIG_HDSC_H

#include "hal_common.h"

/* ---- 时钟配置（与 RL78 一致） ---- */
#define MAIN_CLOCK_HZ       32000000UL   /**< 主时钟频率（Hz） */
#define SUB_CLOCK_HZ        32768UL      /**< 子时钟频率（Hz） */

/* ---- 任务数量（与 RL78 一致） ---- */
#define TASK_COUNT           24   /**< 任务总数 */

/* 复用 RL78 的 TaskId_t 枚举（跨平台一致） */
#include "../BoardA_RL78/BoardConfig.h"

/* ---- GPIO 引脚分配枚举（HDSC 平台） ---- */
/**
 * @brief GPIO 引脚枚举 — HDSC 平台的引脚分配
 *
 * 枚举名称与 RL78 平台一致，但映射到 HDSC 芯片的不同物理引脚。
 * 组件代码通过枚举值引用引脚，实现平台无关性。
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

#endif /* BOARD_CONFIG_HDSC_H */
