/**
 * @file i2c.c
 * @brief STM32 HAL I2C compatibility shim for libopencm3.
 *
 * Provides a minimal implementation of STM32 HAL I2C APIs
 * to integrate HAL-based drivers without bringing in the full HAL.
 */

#include "i2c.h"

HAL_StatusTypeDef HAL_I2C_IsDeviceReady(I2C_HandleTypeDef* hi2c, uint16_t DevAddress, uint32_t Trials, uint32_t Timeout)
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

HAL_StatusTypeDef HAL_I2C_Mem_Write(I2C_HandleTypeDef* hi2c, uint16_t DevAddress, uint16_t MemAddress,
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

HAL_StatusTypeDef HAL_I2C_Mem_Read(I2C_HandleTypeDef* hi2c, uint16_t DevAddress, uint16_t MemAddress,
    uint16_t MemAddSize, uint8_t* pData, uint16_t Size, uint32_t Timeout)
{
    (void)MemAddSize;
    (void)Timeout;

    uint8_t reg = (uint8_t)MemAddress;
    i2c_transfer7(hi2c->instance, (uint8_t)(DevAddress >> 1), &reg, 1, pData, (size_t)Size);

    return HAL_OK;
}
