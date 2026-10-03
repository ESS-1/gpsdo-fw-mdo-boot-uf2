#pragma once

#include "boot_data_defs.h"

__attribute__((section(".boot_data")))        extern const uint8_t boot_logo_41x60[];
__attribute__((section(".boot_data")))        extern const uint8_t lcd_init_commands[];
__attribute__((section(".boot_data.header"))) extern const boot_data_t g_boot_data;
