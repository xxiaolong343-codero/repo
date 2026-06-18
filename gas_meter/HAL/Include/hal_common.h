/**
 * @file hal_common.h
 * @brief Common HAL types and platform detection
 */
#ifndef HAL_COMMON_H
#define HAL_COMMON_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* ---- Platform detection ---- */
#if defined(__CCRL__) || defined(__RL78__)
    #define PLATFORM_RL78  1
    #define PLATFORM_HDSC  0
#elif defined(__CC_ARM) || defined(__ARMCC_VERSION) || defined(__KEIL__)
    #define PLATFORM_RL78  0
    #define PLATFORM_HDSC  1
#else
    #define PLATFORM_RL78  0
    #define PLATFORM_HDSC  0
    #define PLATFORM_HOST  1   /* Unit test / simulation */
#endif

/* ---- Common result codes ---- */
typedef enum {
    HAL_OK       = 0,
    HAL_ERROR    = -1,
    HAL_BUSY     = -2,
    HAL_TIMEOUT  = -3
} HAL_Status_t;

/* ---- Interrupt helpers (platform-specific, implemented in HAL driver) ---- */
#if PLATFORM_HOST
    /* Host stubs for unit testing */
    static inline uint8_t  HAL_DisableInterrupts(void)  { return 0; }
    static inline void     HAL_RestoreInterrupts(uint8_t s) { (void)s; }
#else
    /* Declared in platform HAL, not inline for real targets */
    uint8_t  HAL_DisableInterrupts(void);
    void     HAL_RestoreInterrupts(uint8_t saved_ie);
#endif

/* ---- Bit manipulation helpers ---- */
#define BIT_SET(reg, bit)     ((reg) |=  (1UL << (bit)))
#define BIT_CLR(reg, bit)     ((reg) &= ~(1UL << (bit)))
#define BIT_GET(reg, bit)     (((reg) >> (bit)) & 1UL)
#define BIT_TOGGLE(reg, bit)  ((reg) ^=  (1UL << (bit)))

#endif /* HAL_COMMON_H */
