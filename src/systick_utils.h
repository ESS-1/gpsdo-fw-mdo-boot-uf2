#pragma once

#include <stdint.h>

extern volatile uint32_t systick_ticks;

/**
 * @brief Initialize or reconfigure SysTick timer for 1 ms tick intervals.
 * @param cpu_freq_hz Current CPU core frequency in Hz (e.g. 8000000 for HSI or 72000000 for 72 MHz PLL).
 */
extern void systick_init(uint32_t cpu_freq_hz);

/**
 * @brief Disable SysTick timer.
 */
extern void systick_deinit(void);

/**
 * @return Number of milliseconds since system start.
 */
static inline uint32_t systick_get_tick(void)
{
    return systick_ticks;
}

/**
 * @brief STM32 HAL Delay compatibility shim (blocking millisecond delay).
 * @param delay_ms Milliseconds to wait.
 */
extern void systick_delay(uint32_t delay_ms);
