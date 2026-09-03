/**
 * @file i2c.h
 * @brief STM32 HAL I2C compatibility shim for libopencm3.
 *
 * Provides a minimal implementation of STM32 HAL I2C APIs
 * to integrate HAL-based drivers without bringing in the full HAL.
 */

#pragma once

#include <stdint.h>
#include <libopencm3/stm32/i2c.h>
#include "hal_shim_base.h"

// Minimal handle type for HAL compatibility
typedef struct {
    uint32_t instance; // e.g. I2C1, I2C2
} I2C_HandleTypeDef;

#define I2C_MEMADD_SIZE_8BIT 0x00000001U

/**
 * @brief Checks if target I2C device responds with ACK (ping/ready check).
 * @param hi2c Pointer to I2C handle
 * @param DevAddress Target device address (HAL format: 7-bit shifted left by 1)
 * @param Trials Number of retry attempts (default 1 if 0 passed)
 * @param Timeout Unused
 * @return HAL_OK if device ACKed, HAL_ERROR if NACK/not responding
 */
extern HAL_StatusTypeDef HAL_I2C_IsDeviceReady(I2C_HandleTypeDef* hi2c, uint16_t DevAddress, uint32_t Trials, uint32_t Timeout);

/**
 * @brief Write data to an 8-bit memory/register address of target I2C device.
 * @param hi2c Pointer to I2C handle
 * @param DevAddress Target device address (HAL format: 7-bit shifted left by 1)
 * @param MemAddress 8-bit register address to write to
 * @param MemAddSize Size of register address (should be I2C_MEMADD_SIZE_8BIT)
 * @param pData Pointer to payload data buffer
 * @param Size Number of data bytes to write
 * @param Timeout Unused
 * @return HAL_OK or HAL_ERROR
 */
extern HAL_StatusTypeDef HAL_I2C_Mem_Write(I2C_HandleTypeDef* hi2c, uint16_t DevAddress, uint16_t MemAddress,
    uint16_t MemAddSize, const uint8_t* pData, uint16_t Size, uint32_t Timeout);

/**
 * @brief Read data from an 8-bit memory/register address of target I2C device.
 * @param hi2c Pointer to I2C handle
 * @param DevAddress Target device address (HAL format: 7-bit shifted left by 1)
 * @param MemAddress 8-bit register address to read from
 * @param MemAddSize Size of register address (should be I2C_MEMADD_SIZE_8BIT)
 * @param pData Pointer to receive buffer
 * @param Size Number of data bytes to read
 * @param Timeout Unused
 * @return HAL_OK or HAL_ERROR
 */
extern HAL_StatusTypeDef HAL_I2C_Mem_Read(I2C_HandleTypeDef* hi2c, uint16_t DevAddress, uint16_t MemAddress,
    uint16_t MemAddSize, uint8_t* pData, uint16_t Size, uint32_t Timeout);
