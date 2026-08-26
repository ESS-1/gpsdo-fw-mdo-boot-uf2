/*
 * Copyright (c) 2016, Devan Lai
 *
 * Permission to use, copy, modify, and/or distribute this software
 * for any purpose with or without fee is hereby granted, provided
 * that the above copyright notice and this permission notice
 * appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <string.h>
#include <libopencm3/cm3/vector.h>
#include <libopencm3/stm32/flash.h>

#include "uf2.h"
#include "target.h"
#include "bootlog.h"
#include "usb_conf.h"
#include "config.h"

#include <libopencm3/usb/msc.h>

static inline void __set_MSP(uint32_t topOfMainStack) {
    asm("msr msp, %0" : : "r" (topOfMainStack));
}

static bool is_application_valid(void) {
    if ((*(volatile uint32_t *)(FLASH_BASE + BOOTLOADER_SIZE) & 0x2FFE0000) == 0x20000000) {
        return true;
    }
    return false;
}

static void jump_to_application(void) __attribute__ ((noreturn));

static void jump_to_application(void) {
    vector_table_t* app_vector_table = (vector_table_t*)(FLASH_BASE + BOOTLOADER_SIZE);

    /* Use the application's vector table */
    target_relocate_vector_table();

    /* Initialize the application's stack pointer */
    __set_MSP((uint32_t)(app_vector_table->initial_sp_value));

    /* Jump to the application entry point */
    app_vector_table->reset();

    while (1);
}

int main(void) {
    // Read the key
    target_gpio_enable();
    bool isButtonPressed = target_is_button_pressed();
    target_gpio_disable();

    // Jump to the app if the button is not pressed and the app is valid
    if (!isButtonPressed && is_application_valid()) {
         jump_to_application();
         return 0;
    }

    // Bootloader mode
    {
        // Setup clocks
        target_clock_setup();

        // Enable and init peripherals
        target_gpio_enable();
        target_lcd_init();
        bootlog_init();
        bootlog_add("BOOT MODE", BOOTLOG_MSG_TYPE_HIGHLIGHTED);

        // Setup USB
        {
            char serial[USB_SERIAL_NUM_LENGTH+1];
            serial[0] = '\0';
            target_get_serial_number(serial, USB_SERIAL_NUM_LENGTH);
            usb_set_serial_number(serial);
        }

        usbd_device* usbd_dev = usb_setup();
        usb_msc_init(usbd_dev, 0x82, 64, 0x01, 64, VENDOR_ID, "UF2 Bootloader",
            "1.00", UF2_NUM_BLOCKS, read_block, write_block);

        int cycleCount = 0;
        while (1) {
            cycleCount++;

            if (cycleCount >= 700) {
                cycleCount = 0;
                ghostfat_1ms();
            }

            usbd_poll(usbd_dev);
        }
    }

    return 0;
}
