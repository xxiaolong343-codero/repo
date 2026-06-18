/**
 * @file hal_wdg.h
 * @brief Watchdog hardware abstraction interface
 */
#ifndef HAL_WDG_H
#define HAL_WDG_H

#include "hal_common.h"

void HAL_WDG_Init(uint32_t timeout_ms);
void HAL_WDG_Feed(void);

#endif /* HAL_WDG_H */
