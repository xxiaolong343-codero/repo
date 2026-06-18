/**
 * @file BoardConfig.h
 * @brief RL78 board configuration — pin assignments, clock settings
 */
#ifndef BOARD_CONFIG_RL78_H
#define BOARD_CONFIG_RL78_H

#include "hal_common.h"

/* ---- Clock configuration ---- */
#define MAIN_CLOCK_HZ       32000000UL
#define SUB_CLOCK_HZ        32768UL

/* ---- Task count ---- */
#define TASK_COUNT           24

/* ---- Task IDs ---- */
typedef enum {
    TASK_METERING      = 0,
    TASK_KEY           = 1,
    TASK_ALARM         = 2,
    TASK_VALVE_CTRL    = 3,
    TASK_HMI           = 4,
    TASK_BILLING       = 5,
    TASK_COMM          = 6,
    TASK_INFRARED      = 7,
    TASK_DATA_STORAGE  = 8,
    TASK_E2_STORAGE    = 9,
    TASK_EVENT_LOG     = 10,
    TASK_USAGE_LOG     = 11,
    TASK_EXT_ALARM     = 12,
    TASK_VALVE_LEAK    = 13,
    TASK_BIG_FLOW      = 14,
    TASK_NO_GAS_USAGE  = 15,
    TASK_CONT_FLOW     = 16,
    TASK_TINY_FLOW     = 17,
    TASK_UNCTRL_FLOW   = 18,
    TASK_TILT          = 19,
    TASK_UPGRADE       = 20,
    TASK_POWER_MGMT    = 21,
    TASK_BATTERY       = 22,
    TASK_CLOCK         = 23
} TaskId_t;

/* ---- GPIO pin assignments (RL78) ---- */
typedef enum {
    PIN_METER_PULSE = 0,
    PIN_METER_LED_TX,
    PIN_METER_LED_RX,
    PIN_VALVE_CTRL_A,
    PIN_VALVE_CTRL_B,
    PIN_VALVE_FB,
    PIN_LED_RUN,
    PIN_LED_ALARM,
    PIN_BUZZER,
    PIN_KEY_MENU,
    PIN_EXT_ALARM,
    PIN_TILT_DETECT,
    PIN_NB_POWER,
    PIN_NB_RESET,
    PIN_NB_RI,
    PIN_IR_TX,
    PIN_IR_RX,
    PIN_BAT_MAIN_SENSE,
    PIN_BAT_BACKUP_SENSE,
    PIN_SPC_SENSE,
    PIN_LCD_CS,
    PIN_LCD_RST,
    PIN_LCD_SDA,
    PIN_LCD_SCL,
    PIN_UART_TX,
    PIN_UART_RX,
    PIN_COUNT
} IOPin_t;

#endif /* BOARD_CONFIG_RL78_H */
