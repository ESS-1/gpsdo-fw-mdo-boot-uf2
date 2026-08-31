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
static inline HAL_StatusTypeDef HAL_I2C_IsDeviceReady(I2C_HandleTypeDef* hi2c, uint16_t DevAddress, uint32_t Trials, uint32_t Timeout)
{
    (void)Timeout;
    uint32_t i2c = hi2c->instance;
    uint8_t addr_7bit = (uint8_t)(DevAddress >> 1);
    uint32_t max_trials = Trials ? Trials : 1;

    for (uint32_t i = 0; i < max_trials; ++i) {
        while (I2C_SR2(i2c) & I2C_SR2_BUSY) {}

        i2c_send_start(i2c);
        i2c_enable_ack(i2c);
        while (!(I2C_SR1(i2c) & I2C_SR1_SB)) {}

        i2c_send_7bit_address(i2c, addr_7bit, I2C_WRITE);

        // Wait for ACK (ADDR set) or NACK (AF flag set)
        while (!(I2C_SR1(i2c) & (I2C_SR1_ADDR | I2C_SR1_AF))) {}

        if (I2C_SR1(i2c) & I2C_SR1_ADDR) {
            i2c_send_stop(i2c);
            (void)I2C_SR2(i2c); // Clear ADDR flag by reading SR2
            return HAL_OK;
        }

        // NACK received: clear AF flag and release bus with STOP
        I2C_SR1(i2c) = (uint32_t)~I2C_SR1_AF;
        i2c_send_stop(i2c);
    }

    return HAL_ERROR;
}

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
static inline HAL_StatusTypeDef HAL_I2C_Mem_Write(I2C_HandleTypeDef* hi2c, uint16_t DevAddress, uint16_t MemAddress,
    uint16_t MemAddSize, const uint8_t* pData, uint16_t Size, uint32_t Timeout)
{
    (void)MemAddSize;
    (void)Timeout;

    uint32_t i2c = hi2c->instance;

    while (I2C_SR2(i2c) & I2C_SR2_BUSY) {}

    i2c_send_start(i2c);
    while (!(I2C_SR1(i2c) & I2C_SR1_SB)) {}

    i2c_send_7bit_address(i2c, (uint8_t)(DevAddress >> 1), I2C_WRITE);
    while (!(I2C_SR1(i2c) & I2C_SR1_ADDR)) {}
    (void)I2C_SR2(i2c); // Clear ADDR

    // Send 8-bit Register Address
    i2c_send_data(i2c, (uint8_t)MemAddress);

    // Stream data payload
    for (uint16_t i = 0; i < Size; ++i) {
        while (!(I2C_SR1(i2c) & I2C_SR1_TxE)) {}
        i2c_send_data(i2c, pData[i]);
    }

    while (!(I2C_SR1(i2c) & (I2C_SR1_BTF | I2C_SR1_TxE))) {}
    i2c_send_stop(i2c);

    return HAL_OK;
}

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
static inline HAL_StatusTypeDef HAL_I2C_Mem_Read(I2C_HandleTypeDef* hi2c, uint16_t DevAddress, uint16_t MemAddress,
    uint16_t MemAddSize, uint8_t* pData, uint16_t Size, uint32_t Timeout)
{
    (void)MemAddSize;
    (void)Timeout;

    uint8_t reg = (uint8_t)MemAddress;
    i2c_transfer7(hi2c->instance, (uint8_t)(DevAddress >> 1), &reg, 1, pData, (size_t)Size);

    return HAL_OK;
}
