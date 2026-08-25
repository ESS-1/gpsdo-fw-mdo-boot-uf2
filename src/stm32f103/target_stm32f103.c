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

/* Common STM32F103 target functions */

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/st_usbfs.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/desig.h>
#include <libopencm3/cm3/scb.h>

#include "target.h"
#include "config.h"

#ifndef USES_GPIOA
#define USES_GPIOA 0
#endif

#ifndef USES_GPIOB
#define USES_GPIOB 0
#endif

#ifndef USES_GPIOC
#define USES_GPIOC 0
#endif

#ifdef FLASH_SIZE_OVERRIDE
_Static_assert((FLASH_BASE + FLASH_SIZE_OVERRIDE >= APP_BASE_ADDRESS),
               "Incompatible flash size");
#endif

void target_clock_setup(void) {
    // Set system clock to 72 MHz from an external crystal
    #ifdef CRYSTAL_16MHZ
        rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE16_72MHZ]);
    #else
        rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]);
    #endif
}

void target_gpio_enable(void) {
    if (USES_GPIOA) {
        rcc_periph_clock_enable(RCC_GPIOA);
    }
    if (USES_GPIOB) {
        rcc_periph_clock_enable(RCC_GPIOB);
    }
    if (USES_GPIOC) {
        rcc_periph_clock_enable(RCC_GPIOC);
    }
}

void target_gpio_disable(void) {
    if (USES_GPIOA) {
        rcc_periph_clock_disable(RCC_GPIOA);
    }
    if (USES_GPIOB) {
        rcc_periph_clock_disable(RCC_GPIOB);
    }
    if (USES_GPIOC) {
        rcc_periph_clock_disable(RCC_GPIOC);
    }
}

static void target_lcd_io_init(void)
{
    // Init GPIO
    gpio_set(LCD_RES_GPIO_Port, LCD_RES_Pin);
    gpio_set(LCD_CS_GPIO_Port, LCD_CS_Pin);
    gpio_set(LCD_DC_GPIO_Port, LCD_DC_Pin);

    gpio_set_mode(LCD_RES_GPIO_Port, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, LCD_RES_Pin);
    gpio_set_mode(LCD_CS_GPIO_Port, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, LCD_CS_Pin);
    gpio_set_mode(LCD_DC_GPIO_Port, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, LCD_DC_Pin);
}

static void target_lcd_spi_init(void)
{
    // Enable SPI clock
    rcc_periph_clock_enable(LCD_SPI_RCC);
    spi_disable(LCD_SPI);

    // Configure SPI pins (SCK and MOSI) as Alternate Function Push-Pull (50MHz)
    gpio_set_mode(LCD_SCL_GPIO_Port, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, LCD_SCL_Pin);
    gpio_set_mode(LCD_SDA_GPIO_Port, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, LCD_SDA_Pin);

    // Reset and init SPI
    spi_init_master(
        LCD_SPI,
        SPI_CR1_BAUDRATE_FPCLK_DIV_8,
        SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
        SPI_CR1_CPHA_CLK_TRANSITION_1,
        SPI_CR1_DFF_8BIT,
        SPI_CR1_MSBFIRST);

    spi_enable_software_slave_management(LCD_SPI);
    spi_set_nss_high(LCD_SPI);

    // Enable SPI
    spi_enable(LCD_SPI);
}

void target_lcd_init(void)
{
    target_lcd_io_init();
    target_lcd_spi_init();
}

bool target_is_button_pressed(void) {
    // Setup the internal pull-up/pull-down for the button
    const uint8_t mode = GPIO_MODE_INPUT;
    const uint8_t conf = GPIO_CNF_INPUT_PULL_UPDOWN;
    gpio_set_mode(BUTTON_GPIO_PORT, mode, conf, BUTTON_GPIO_PIN);

    if (BUTTON_ACTIVE_HIGH) {
        gpio_clear(BUTTON_GPIO_PORT, BUTTON_GPIO_PIN);
    } else {
        gpio_set(BUTTON_GPIO_PORT, BUTTON_GPIO_PIN);
    }

    // Check if the user button is held down
    bool btn = gpio_get(BUTTON_GPIO_PORT, BUTTON_GPIO_PIN);
    return BUTTON_ACTIVE_HIGH ? btn : !btn;
}

const usbd_driver* target_usb_init(void) {
    rcc_periph_reset_pulse(RST_USB);

    // Enable USB pullup to connect
    if (USB_PULLUP_ACTIVE_HIGH) {
        gpio_set(USB_PULLUP_GPIO_PORT, USB_PULLUP_GPIO_PIN);
    } else {
        gpio_clear(USB_PULLUP_GPIO_PORT, USB_PULLUP_GPIO_PIN);
    }

    const uint8_t mode = GPIO_MODE_OUTPUT_10_MHZ;
    const uint8_t conf = (USB_PULLUP_OPEN_DRAIN ? GPIO_CNF_OUTPUT_OPENDRAIN
                                                : GPIO_CNF_OUTPUT_PUSHPULL);
    gpio_set_mode(USB_PULLUP_GPIO_PORT, mode, conf, USB_PULLUP_GPIO_PIN);

    return &st_usbfs_v1_usb_driver;
}

void target_get_serial_number(char* dest, size_t max_chars) {
    desig_get_unique_id_as_string(dest, max_chars+1);
}

static uint16_t* get_flash_end(void) {
#ifdef FLASH_SIZE_OVERRIDE
    /* Allow access to the unofficial full 128KiB flash size */
    return (uint16_t*)(FLASH_BASE + FLASH_SIZE_OVERRIDE);
#else
    /* Only allow access to the chip's self-reported flash size */
    return (uint16_t*)(FLASH_BASE + (size_t)DESIG_FLASH_SIZE*FLASH_PAGE_SIZE);
#endif
}

void target_relocate_vector_table(void) {
    SCB_VTOR = APP_BASE_ADDRESS & 0xFFFF;
}

void target_flash_unlock(void) {
    flash_unlock();
}

void target_flash_lock(void) {
    flash_lock();
}

static inline uint16_t* get_flash_page_address(uint16_t* dest) {
    return (uint16_t*)(((uint32_t)dest / FLASH_PAGE_SIZE) * FLASH_PAGE_SIZE);
}

bool target_flash_program_array(uint16_t* dest, const uint16_t* data, size_t half_word_count) {
    bool verified = true;

    /* Remember the bounds of erased data in the current page */
    static uint16_t* erase_start;
    static uint16_t* erase_end;

    const uint16_t* flash_end = get_flash_end();
    while (half_word_count > 0) {
        /* Avoid writing past the end of flash */
        if (dest >= flash_end) {
            verified = false;
            break;
        }

        if (dest >= erase_end || dest < erase_start) {
            erase_start = get_flash_page_address(dest);
            erase_end = erase_start + (FLASH_PAGE_SIZE)/sizeof(uint16_t);
            flash_erase_page((uint32_t)erase_start);
        }
        flash_program_half_word((uint32_t)dest, *data);
        erase_start = dest + 1;
        if (*dest != *data) {
            verified = false;
            break;
        }
        dest++;
        data++;
        half_word_count--;
    }

    return verified;
}
