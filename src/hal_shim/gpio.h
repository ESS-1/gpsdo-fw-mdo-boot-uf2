/**
 * @file gpio.h
 * @brief STM32 HAL GPIO compatibility shim for libopencm3.
 *
 * Provides a minimal implementation of STM32 HAL GPIO APIs
 * to integrate HAL-based drivers without bringing in the full HAL.
 */

#pragma once

#include <stdint.h>
#include <libopencm3/stm32/gpio.h>

typedef enum {
    GPIO_PIN_RESET = 0,
    GPIO_PIN_SET = 1
} GPIO_PinState;

static inline void HAL_GPIO_WritePin(uint32_t port, uint16_t pin, GPIO_PinState state)
{
    if (state != GPIO_PIN_RESET) {
        gpio_set(port, pin);
    }
    else {
        gpio_clear(port, pin);
    }
}
