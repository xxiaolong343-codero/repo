/**
 * @file BoardConfig.h
 * @brief HDSC board configuration — pin assignments, clock settings
 */
#ifndef BOARD_CONFIG_HDSC_H
#define BOARD_CONFIG_HDSC_H

#include "hal_common.h"

/* ---- Clock configuration ---- */
#define MAIN_CLOCK_HZ       32000000UL
#define SUB_CLOCK_HZ        32768UL

/* ---- Task count and IDs — identical across platforms ---- */
#define TASK_COUNT           24

/* Reuse the same TaskId_t enum for both platforms */
#include "../BoardA_RL78/BoardConfig.h"  /* shares TaskId_t */

/* ---- GPIO pin assignments (HDSC) ---- */
/* Pin numbering differs from RL78, but enum names are the same
   so component code is platform-independent */
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

#endif /* BOARD_CONFIG_HDSC_H */
