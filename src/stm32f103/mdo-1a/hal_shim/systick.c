/**
 * @file systick.c
 * @brief STM32 HAL_SysTick and HAL_GetTick implementation for libopencm3.
 */

#include "systick.h"
#include <libopencm3/cm3/systick.h>
#include <libopencm3/cm3/nvic.h>

volatile uint32_t system_ticks = 0;

/**
 * @brief SysTick ISR handler (called every 1 ms).
 */
void sys_tick_handler(void)
{
    ++system_ticks;
}

void systick_init(uint32_t cpu_freq_hz)
{
    systick_set_reload(cpu_freq_hz / 1000 - 1);
    systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);
    systick_clear();
    systick_counter_enable();
    systick_interrupt_enable();
}

void systick_deinit(void)
{
    systick_interrupt_disable();
    systick_counter_disable();
    systick_clear();
}

void HAL_Delay(uint32_t delay_ms)
{
    uint32_t start = HAL_GetTick();

    // Add +1 to guarantee waiting AT LEAST the requested amount of ms
    // (guards against calling HAL_Delay just microseconds before the next tick)
    uint32_t wait = delay_ms;
    if (wait < 0xFFFFFFFFU) {
        ++wait;
    }

    while ((HAL_GetTick() - start) < wait) {}
}
