/**
 * @file i2c.c
 * @brief STM32 HAL I2C compatibility shim for libopencm3.
 *
 * Provides a minimal implementation of STM32 HAL I2C APIs
 * to integrate HAL-based drivers without bringing in the full HAL.
 *
 * Only I2C_MEMADD_SIZE_8BIT is supported.
 */

#include "i2c.h"

HAL_StatusTypeDef HAL_I2C_IsDeviceReady(I2C_HandleTypeDef* hi2c, uint16_t DevAddress, uint32_t Trials, uint32_t Timeout)
{
    (void)Timeout;
    uint32_t i2c = hi2c->instance;
    uint8_t addr_7bit = (uint8_t)(DevAddress >> 1);
    uint32_t max_trials = Trials ? Trials : 1;

    // Wait until bus is free
    while (I2C_SR2(i2c) & I2C_SR2_BUSY) {}

    for (uint32_t trial = 0; trial < max_trials; ++trial) {
        // Generate START condition
        i2c_send_start(i2c);

        // Wait until SB flag is set (START condition generated)
        while (!(I2C_SR1(i2c) & I2C_SR1_SB)) {}

        // Send 7-bit slave address with Write bit
        i2c_send_7bit_address(i2c, addr_7bit, I2C_WRITE);

        // Wait until ADDR (ACK received) or AF (NACK received) flag is set
        while (!(I2C_SR1(i2c) & (I2C_SR1_ADDR | I2C_SR1_AF))) {}

        if (I2C_SR1(i2c) & I2C_SR1_ADDR) {
            // Device acknowledged (ACK):
            // Generate STOP condition before clearing ADDR (per HAL implementation)
            i2c_send_stop(i2c);

            // Clear ADDR flag by reading SR1 followed by SR2
            (void)I2C_SR1(i2c);
            (void)I2C_SR2(i2c);

            // Wait until STOP condition completes and bus is free
            while (I2C_SR2(i2c) & I2C_SR2_BUSY) {}

            return HAL_OK;
        }
        else {
            // Device did not acknowledge (NACK): AF flag is set
            // Generate STOP condition
            i2c_send_stop(i2c);

            // Clear AF flag
            I2C_SR1(i2c) = (uint16_t)~I2C_SR1_AF;

            // Wait until bus is free before next trial
            while (I2C_SR2(i2c) & I2C_SR2_BUSY) {}
        }
    }

    return HAL_ERROR;
}

HAL_StatusTypeDef HAL_I2C_Mem_Write(I2C_HandleTypeDef* hi2c, uint16_t DevAddress, uint16_t MemAddress,
    uint16_t MemAddSize, const uint8_t* pData, uint16_t Size, uint32_t Timeout)
{
    (void)MemAddSize;
    (void)Timeout;

    uint32_t i2c = hi2c->instance;
    uint8_t addr_7bit = (uint8_t)(DevAddress >> 1);

    // Wait until bus is free
    while (I2C_SR2(i2c) & I2C_SR2_BUSY) {}

    // Generate START condition
    i2c_send_start(i2c);
    while (!(I2C_SR1(i2c) & I2C_SR1_SB)) {}

    // Send 7-bit slave address with Write bit
    i2c_send_7bit_address(i2c, addr_7bit, I2C_WRITE);
    while (!(I2C_SR1(i2c) & (I2C_SR1_ADDR | I2C_SR1_AF))) {}

    // Check for NACK on device address
    if (I2C_SR1(i2c) & I2C_SR1_AF) {
        i2c_send_stop(i2c);
        I2C_SR1(i2c) = (uint16_t)~I2C_SR1_AF;
        while (I2C_SR2(i2c) & I2C_SR2_BUSY) {}
        return HAL_ERROR;
    }

    // Clear ADDR flag by reading SR1 followed by SR2
    (void)I2C_SR1(i2c);
    (void)I2C_SR2(i2c);

    // Send 8-bit memory/register address
    while (!(I2C_SR1(i2c) & I2C_SR1_TxE)) {}
    i2c_send_data(i2c, (uint8_t)MemAddress);

    // Stream data payload
    for (uint16_t i = 0; i < Size; ++i) {
        while (!(I2C_SR1(i2c) & (I2C_SR1_TxE | I2C_SR1_AF))) {}
        if (I2C_SR1(i2c) & I2C_SR1_AF) {
            i2c_send_stop(i2c);
            I2C_SR1(i2c) = (uint16_t)~I2C_SR1_AF;
            while (I2C_SR2(i2c) & I2C_SR2_BUSY) {}
            return HAL_ERROR;
        }
        i2c_send_data(i2c, pData[i]);
    }

    // Wait for BTF (Byte Transfer Finished) before STOP (HAL requirement)
    while (!(I2C_SR1(i2c) & I2C_SR1_BTF)) {}
    i2c_send_stop(i2c);

    // Wait until STOP condition completes and bus is free
    while (I2C_SR2(i2c) & I2C_SR2_BUSY) {}

    return HAL_OK;
}

HAL_StatusTypeDef HAL_I2C_Mem_Read(I2C_HandleTypeDef* hi2c, uint16_t DevAddress, uint16_t MemAddress,
    uint16_t MemAddSize, uint8_t* pData, uint16_t Size, uint32_t Timeout)
{
    (void)MemAddSize;
    (void)Timeout;

    if (Size == 0) {
        return HAL_OK;
    }

    uint32_t i2c = hi2c->instance;
    uint8_t addr_7bit = (uint8_t)(DevAddress >> 1);

    // Wait until bus is free
    while (I2C_SR2(i2c) & I2C_SR2_BUSY) {}

    // Generate START condition
    i2c_send_start(i2c);
    while (!(I2C_SR1(i2c) & I2C_SR1_SB)) {}

    // Send 7-bit slave address with Write bit to select target register
    i2c_send_7bit_address(i2c, addr_7bit, I2C_WRITE);
    while (!(I2C_SR1(i2c) & (I2C_SR1_ADDR | I2C_SR1_AF))) {}

    // Check for NACK on device address
    if (I2C_SR1(i2c) & I2C_SR1_AF) {
        i2c_send_stop(i2c);
        I2C_SR1(i2c) = (uint16_t)~I2C_SR1_AF;
        while (I2C_SR2(i2c) & I2C_SR2_BUSY) {}
        return HAL_ERROR;
    }

    (void)I2C_SR1(i2c);
    (void)I2C_SR2(i2c);

    // Send 8-bit memory/register address
    while (!(I2C_SR1(i2c) & I2C_SR1_TxE)) {}
    i2c_send_data(i2c, (uint8_t)MemAddress);
    while (!(I2C_SR1(i2c) & I2C_SR1_BTF)) {}

    // Generate Repeated START condition
    i2c_send_start(i2c);
    while (!(I2C_SR1(i2c) & I2C_SR1_SB)) {}

    // Send 7-bit slave address with Read bit
    i2c_send_7bit_address(i2c, addr_7bit, I2C_READ);
    while (!(I2C_SR1(i2c) & (I2C_SR1_ADDR | I2C_SR1_AF))) {}

    // Check for NACK on device read address
    if (I2C_SR1(i2c) & I2C_SR1_AF) {
        i2c_send_stop(i2c);
        I2C_SR1(i2c) = (uint16_t)~I2C_SR1_AF;
        i2c_enable_ack(i2c);
        while (I2C_SR2(i2c) & I2C_SR2_BUSY) {}
        return HAL_ERROR;
    }

    // Receive data strictly adhering to RM0008 Master Receiver hardware sequence
    if (Size == 1) {
        // RM0008 Method for N = 1:
        // Clear ACK BEFORE clearing ADDR
        i2c_disable_ack(i2c);

        // Clear ADDR flag
        (void)I2C_SR1(i2c);
        (void)I2C_SR2(i2c);

        // Generate STOP condition
        i2c_send_stop(i2c);

        // Wait for byte reception (RxNE)
        while (!(I2C_SR1(i2c) & I2C_SR1_RxNE)) {}
        *pData = (uint8_t)i2c_get_data(i2c);

    }
    else if (Size == 2) {
        // RM0008 Method for N = 2:
        // Set POS, clear ACK BEFORE clearing ADDR
        i2c_nack_next(i2c);
        i2c_disable_ack(i2c);

        // Clear ADDR flag
        (void)I2C_SR1(i2c);
        (void)I2C_SR2(i2c);

        // Wait until both bytes are received into DR and shift register (BTF flag)
        while (!(I2C_SR1(i2c) & I2C_SR1_BTF)) {}

        // Generate STOP condition
        i2c_send_stop(i2c);

        // Read byte 1, then byte 2
        pData[0] = (uint8_t)i2c_get_data(i2c);
        pData[1] = (uint8_t)i2c_get_data(i2c);

        // Clear POS bit
        i2c_nack_current(i2c);
    }
    else {
        // RM0008 Method for N > 2:
        i2c_enable_ack(i2c);

        // Clear ADDR flag
        (void)I2C_SR1(i2c);
        (void)I2C_SR2(i2c);

        uint16_t remaining = Size;
        uint16_t idx = 0;

        while (remaining > 3) {
            while (!(I2C_SR1(i2c) & I2C_SR1_RxNE)) {}
            pData[idx++] = (uint8_t)i2c_get_data(i2c);
            remaining--;
        }

        // Remaining 3 bytes sequence per RM0008:
        // Wait for BTF (Data N-2 in DR, Data N-1 in shift register)
        while (!(I2C_SR1(i2c) & I2C_SR1_BTF)) {}

        // Clear ACK to NACK byte N
        i2c_disable_ack(i2c);

        // Read Data N-2
        pData[idx++] = (uint8_t)i2c_get_data(i2c);

        // Wait for next BTF (Data N-1 in DR, Data N in shift register)
        while (!(I2C_SR1(i2c) & I2C_SR1_BTF)) {}

        // Generate STOP condition
        i2c_send_stop(i2c);

        // Read Data N-1
        pData[idx++] = (uint8_t)i2c_get_data(i2c);

        // Wait for RxNE to read Data N
        while (!(I2C_SR1(i2c) & I2C_SR1_RxNE)) {}
        pData[idx++] = (uint8_t)i2c_get_data(i2c);
    }

    // Always restore peripheral state for subsequent transactions
    i2c_enable_ack(i2c);
    i2c_nack_current(i2c); // Ensure POS bit is cleared

    // Wait until STOP condition completes and bus is free
    while (I2C_SR2(i2c) & I2C_SR2_BUSY) {}

    return HAL_OK;
}
