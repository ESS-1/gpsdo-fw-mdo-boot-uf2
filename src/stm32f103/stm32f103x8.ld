/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2015 Karl Palsson <karlp@tweak.net.au>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

/* Linker script for STM32F103x8, 64k flash, 20k RAM. */

/* Define memory regions. */
MEMORY
{
    rom (rx)       : ORIGIN = 0x08000000, LENGTH = __BOOTLOADER_SIZE__ - __BOOTLOADER_DATA_SIZE__
    boot_data (rx) : ORIGIN = 0x08000000 + (__BOOTLOADER_SIZE__ - __BOOTLOADER_DATA_SIZE__), LENGTH = __BOOTLOADER_DATA_SIZE__
    ram (rwx)      : ORIGIN = 0x20000000, LENGTH = 20K
}

SECTIONS
{
    .boot_data : ALIGN(4)
    {
        KEEP(*(.boot_data.header))
        KEEP(*(.boot_data))
        . = ALIGN(4);
    } > boot_data
}

/* Include the common ld script. */
INCLUDE cortex-m-generic.ld
