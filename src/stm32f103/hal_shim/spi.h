/**
 * @file spi.h
 * @brief STM32 HAL SPI compatibility shim for libopencm3.
 *
 * Provides a minimal implementation of STM32 HAL SPI APIs
 * to integrate HAL-based drivers without bringing in the full HAL.
 */

#pragma once

#include "hal_shim_base.h"
#include <libopencm3/stm32/spi.h>
#include <stdint.h>

// Minimal dummy handle type for HAL compatibility
typedef struct {
    uint32_t instance;
} SPI_HandleTypeDef;

/**
 * @brief Transmit data over SPI
 * @param spi Pointer to SPI handle
 * @param data Pointer to data buffer to transmit
 * @param size Number of bytes to transmit
 * @param timeout Timeout duration (not used in this implementation)
 * @return HAL status code (HAL_OK or HAL_ERROR)
 */
extern HAL_StatusTypeDef HAL_SPI_Transmit(SPI_HandleTypeDef* spi, const uint8_t* data, uint16_t size, uint32_t timeout);
