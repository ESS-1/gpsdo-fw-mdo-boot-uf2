/**
 * @file systick_hal.h
 * @brief STM32 HAL_SysTick and HAL_GetTick compatibility shim for libopencm3.
 */

#pragma once

#include "hal_shim_base.h"
#include "systick_utils.h"

/**
 * @brief STM32 HAL_GetTick compatibility shim.
 * @return Number of milliseconds since system start.
 */
static inline uint32_t HAL_GetTick(void)
{
    return systick_get_tick();
}

/**
 * @brief STM32 HAL Delay compatibility shim (blocking millisecond delay).
 * @param delay_ms Milliseconds to wait.
 */
static inline void HAL_Delay(uint32_t delay_ms)
{
    systick_delay(delay_ms);
}
