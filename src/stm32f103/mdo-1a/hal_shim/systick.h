/**
 * @file systick.h
 * @brief STM32 HAL_SysTick and HAL_GetTick compatibility shim for libopencm3.
 */

#pragma once

#include <stdint.h>

extern volatile uint32_t system_ticks;

/**
 * @brief Initialize or reconfigure SysTick timer for 1 ms tick intervals.
 * @param cpu_freq_hz Current CPU core frequency in Hz (e.g. 8000000 for HSI or 72000000 for 72 MHz PLL).
 */
void systick_init(uint32_t cpu_freq_hz);

/**
 * @brief Disable SysTick timer.
 */
void systick_deinit(void);

/**
 * @brief STM32 HAL_GetTick compatibility shim.
 * @return Number of milliseconds since system start.
 */
static inline uint32_t HAL_GetTick(void)
{
    return system_ticks;
}

/**
 * @brief STM32 HAL Delay compatibility shim (blocking millisecond delay).
 * @param delay_ms Milliseconds to wait.
 */
void HAL_Delay(uint32_t delay_ms);
