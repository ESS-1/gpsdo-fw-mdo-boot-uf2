/**
 * @file spi.c
 * @brief STM32 HAL SPI compatibility shim for libopencm3.
 *
 * Provides a minimal implementation of STM32 HAL SPI APIs
 * to integrate HAL-based drivers without bringing in the full HAL.
 */

#include "spi.h"

HAL_StatusTypeDef HAL_SPI_Transmit(SPI_HandleTypeDef* spi, const uint8_t* data, uint16_t size, uint32_t timeout)
{
    (void)timeout;

    if (!data || size == 0) {
        return HAL_ERROR;
    }

    for (uint16_t i = 0; i < size; ++i) {
        spi_send(spi->instance, data[i]);
    }

    // Wait until the last byte is completely pushed out on the wire
    while (SPI_SR(spi->instance) & SPI_SR_BSY) {}

    return HAL_OK;
}
