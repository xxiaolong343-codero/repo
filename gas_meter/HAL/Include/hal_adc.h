/**
 * @file hal_adc.h
 * @brief ADC hardware abstraction interface
 */
#ifndef HAL_ADC_H
#define HAL_ADC_H

#include "hal_common.h"

typedef void (*HAL_ADC_Callback_t)(uint8_t channel, uint16_t value);

void HAL_ADC_Init(void);
void HAL_ADC_Start(uint8_t channel);
uint16_t HAL_ADC_Read(uint8_t channel);
void HAL_ADC_RegisterCallback(uint8_t channel, HAL_ADC_Callback_t cb);

#endif /* HAL_ADC_H */
