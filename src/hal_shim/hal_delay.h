 /**
  * @file hal_delay.h
  * @brief STM32 HAL Delay compatibility shim for libopencm3.
  */

#pragma once

#include <stdint.h>
#include <libopencm3/stm32/rcc.h>

/**
 * @brief Frequency-independent busy-wait delay in milliseconds
 * @param ms Number of milliseconds to delay
 */
static inline void HAL_Delay(uint32_t ms)
{
    uint32_t freq = rcc_ahb_frequency ? rcc_ahb_frequency : 8000000UL;
    uint32_t loops_per_ms = freq / 4000UL;

    while (ms--) {
        uint32_t cycles = loops_per_ms;
        __asm__ volatile(
            "1: subs %[cycles], %[cycles], #1 \n"
            "   bne 1b \n"
            : [cycles] "+r" (cycles)
            :
            : "cc"
            );
    }
}
