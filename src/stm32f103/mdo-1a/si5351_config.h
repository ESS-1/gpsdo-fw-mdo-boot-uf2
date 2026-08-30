#ifndef _SI5351_CONFIG_53839ADE62B7_H_
#define _SI5351_CONFIG_53839ADE62B7_H_

#include "config.h"
#include "systick.h"

#define SI5351_ADDRESS 0x60

__attribute__((weak)) I2C_HandleTypeDef hi2c1 = { .instance = PLL_I2C };
#define I2C_HANDLE                      hi2c1

#define SI5351_XTAL_FREQ 10000000

#endif
