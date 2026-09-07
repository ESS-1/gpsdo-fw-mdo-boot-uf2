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
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/st_usbfs.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/desig.h>
#include <libopencm3/cm3/scb.h>

#include "target.h"
#include "config.h"
#include "si5351.h"
#include "st7735.h"
#include "systick_hal.h"
#include "compressed_font.h"
#include "bootlog.h"
#include "flashmap.h"
#include "colors.h"

_Static_assert((TOTAL_FLASH_SIZE >= BOOTLOADER_SIZE), "Incompatible flash size");


static void target_error_handler(void)
{
    __asm__("cpsid i"); // Disable IRQ
    while (1) {
        __asm__("nop");
    }
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

static void target_lcd_io_init(void)
{
    // Init GPIO
    gpio_set(LCD_RES_GPIO_PORT, LCD_RES_PIN);
    gpio_set(LCD_CS_GPIO_PORT, LCD_CS_PIN);
    gpio_set(LCD_DC_GPIO_PORT, LCD_DC_PIN);

    gpio_set_mode(LCD_RES_GPIO_PORT, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, LCD_RES_PIN);
    gpio_set_mode(LCD_CS_GPIO_PORT, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, LCD_CS_PIN);
    gpio_set_mode(LCD_DC_GPIO_PORT, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, LCD_DC_PIN);
}

static void target_lcd_spi_init(void)
{
    // Enable SPI clock
    rcc_periph_clock_enable(LCD_SPI_RCC);
    spi_disable(LCD_SPI);

    // Configure SPI pins (SCK and MOSI) as Alternate Function Push-Pull (50MHz)
    gpio_set_mode(LCD_SPI_GPIO_PORT, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, LCD_SPI_PINS);

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

static void target_pll_i2c_init(void)
{
    // Enable peripheral clocks for AFIO and I2C1
    rcc_periph_clock_enable(RCC_AFIO);
    rcc_periph_clock_enable(PLL_I2C_RCC);

    // Hardware reset I2C peripheral state machine
    rcc_periph_reset_pulse(PLL_I2C_RST);       // e.g. RST_I2C1

    // Apply pin remapping if defined
#ifdef PLL_I2C_AFIO_MAPR
    AFIO_MAPR |= PLL_I2C_AFIO_MAPR;
#endif

    // Configure I2C GPIO as Alternate Function Open-Drain (2 MHz)
    gpio_set_mode(PLL_I2C_GPIO_PORT, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_ALTFN_OPENDRAIN, PLL_I2C_PINS);

    // Reset and configure I2C for 100 kHz standard mode (36 MHz APB1 clock)
    i2c_set_speed(PLL_I2C, i2c_speed_sm_100k, 36);
    i2c_peripheral_enable(PLL_I2C);
}

static void target_lcd_init(void)
{
    // Initialize the compressed font
    compressed_font_init();

    // Initialize the LCD GPIO and SPI
    target_lcd_io_init();
    target_lcd_spi_init();

    // Initialize and clear the LCD
    ST7735_Init();
    ST7735_FillScreen(COLOR_BG);
}

static bool target_pll_primary_out_init()
{
    si5351_Init(0, SI5351_CRYSTAL_LOAD_6PF, false);

    // Init PLL A (VCO = 10M * 88 = 880M)
    si5351PLLConfig_t pll_config = { 0 };
    pll_config.mult  = 88;
    pll_config.num   = 0;
    pll_config.denom = 1;
    si5351_SetupPLL(SI5351_PLL_A, &pll_config);

    // CLK0: 880M / 110 = 8M
    si5351OutputConfig_t out_config = { 0 };
    out_config.allowIntegerMode = 1;
    out_config.div = 110;
    out_config.num = 0;
    out_config.denom = 1;
    out_config.rdiv  = SI5351_R_DIV_1;
    if (si5351_SetupChannel(0, SI5351_PLL_A, SI5351_DRIVE_STRENGTH_8MA, &out_config, 0) != 0) {
        return false;
    }

    si5351_ResetPLL(SI5351_PLL_A);
    return true;
}

static void target_pll_init()
{
    // Wait for contact bounce to settle before turning the OCXO on
    HAL_Delay(750);

    // Enable OCXO
    gpio_set(OCXO_EN_GPIO_PORT, OCXO_EN_PIN);
    gpio_set_mode(OCXO_EN_GPIO_PORT, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, OCXO_EN_PIN);

    // Wait for OCXO startup
    HAL_Delay(500);

    // Initialize I2C
    target_pll_i2c_init();

    // Init SI5351 PLL
    bootlog_add("Init PLL...", BOOTLOG_MSG_TYPE_INFO);
    if (!target_pll_primary_out_init()) {
        bootlog_add("PLL init failure!", BOOTLOG_MSG_TYPE_ERROR);

        // Turn off OCXO
        gpio_clear(OCXO_EN_GPIO_PORT, OCXO_EN_PIN);
        target_error_handler();
    }

    // Wait for PLL lock
    if (si5351_WaitPLLReady(SI5351_PLL_A, 2000)) {
        // Enable output
        si5351_EnableOutputs(1);
    }
    else {
        bootlog_add("PLL lock failure!", BOOTLOG_MSG_TYPE_ERROR);

        // Turn off OCXO
        gpio_clear(OCXO_EN_GPIO_PORT, OCXO_EN_PIN);
        target_error_handler();
    }

    // Switch to the normal operation mode
    bootlog_add("Done", BOOTLOG_MSG_TYPE_INFO);
    HAL_Delay(750);
}

static void target_clock_setup(void)
{
    // Enable HSE Bypass for external oscillator
    rcc_osc_bypass_enable(RCC_HSE);

    // Setup 72 MHz PLL clock from 8 MHz HSE input
    rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]);
}

static void target_get_flash_info_str(char *buf, size_t maxChars)
{
    if (buf == NULL || maxChars == 0) {
        return;
    }

    uint16_t flashKB = desig_get_flash_size();
    uint32_t pageKB  = target_get_flash_page_size() / 1024;

    char *p = buf;
    char *end = buf + maxChars - 1;

    // Write flash size in KB
    if (flashKB >= 1000 && p < end) { *p++ = '0' + (flashKB / 1000) % 10; }
    if (flashKB >= 100 && p < end)  { *p++ = '0' + (flashKB / 100) % 10; }
    if (flashKB >= 10 && p < end)   { *p++ = '0' + (flashKB / 10) % 10; }
    if (p < end) { *p++ = '0' + (flashKB % 10); }

    // Write "KB FLASH, "
    const char mid[] = "KB FLASH, ";
    for (uint8_t i = 0; mid[i] && p < end; i++) {
        *p++ = mid[i];
    }

    // Write page size in KB
    if (p < end) { *p++ = '0' + (pageKB % 10); }

    // Write "KB PAGE"
    const char tail[] = "KB PAGE";
    for (uint8_t i = 0; tail[i] && p < end; i++) {
        *p++ = tail[i];
    }

    *p = '\0';
}

void target_init(AppStatus appStatus)
{
    // Init systick for 8MHz HSI and 8MHz SYSCLK
    systick_init(8000000);

    // Enable GPIO
    target_gpio_enable();

    // Init LCD
    target_lcd_init();

    // Log the app status
    switch (appStatus) {
        case APP_STATUS_NO_APP:
            bootlog_add("APP is missing!", BOOTLOG_MSG_TYPE_ERROR);
            break;
        case APP_STATUS_CRC_ERROR:
            bootlog_add("APP CRC error!", BOOTLOG_MSG_TYPE_ERROR);
            break;
    }

    // Init bootlog
    bootlog_add("BOOT MODE", BOOTLOG_MSG_TYPE_HIGHLIGHTED);
    bootlog_add("Ver. " UF2_INFO_VERSION "-" UF2_VERSION, BOOTLOG_MSG_TYPE_INFO);
    char flashInfo[23] = { '\0' };
    target_get_flash_info_str(flashInfo, sizeof(flashInfo) / sizeof(flashInfo[0]));
    bootlog_add(flashInfo, BOOTLOG_MSG_TYPE_INFO);

    // Setup OCXO clock
    target_pll_init();
    target_clock_setup();

    // Reconfigure systick for 72MHz SYSCLK
    systick_init(72000000);

    // Draw flash map
    flashmap_init();
}

static inline void target_usb_pullup_enable(bool enable) {
    if ((USB_PULLUP_ACTIVE_HIGH != 0) == enable) {
        gpio_set(USB_PULLUP_GPIO_PORT, USB_PULLUP_GPIO_PIN);
    }
    else {
        gpio_clear(USB_PULLUP_GPIO_PORT, USB_PULLUP_GPIO_PIN);
    }
}

const usbd_driver* target_usb_init(void) {
    rcc_periph_reset_pulse(RST_USB);

    // Enable USB pullup to connect
    target_usb_pullup_enable(true);

    const uint8_t mode = GPIO_MODE_OUTPUT_2_MHZ;
    const uint8_t conf = (USB_PULLUP_OPEN_DRAIN ? GPIO_CNF_OUTPUT_OPENDRAIN
                                                : GPIO_CNF_OUTPUT_PUSHPULL);
    gpio_set_mode(USB_PULLUP_GPIO_PORT, mode, conf, USB_PULLUP_GPIO_PIN);

    return &st_usbfs_v1_usb_driver;
}

void target_get_serial_number(char* dest, size_t max_chars) {
    desig_get_unique_id_as_string(dest, max_chars+1);
}

void target_relocate_vector_table(void) {
    SCB_VTOR = (FLASH_BASE + BOOTLOADER_SIZE) & 0xFFFF;
}

void target_flash_unlock(void) {
    flash_unlock();
}

void target_flash_lock(void) {
    flash_lock();
}

static uint16_t* get_flash_end(void) {
    return (uint16_t*)(FLASH_BASE + TOTAL_FLASH_SIZE);
}

static inline uint16_t* get_flash_page_address(uint16_t* dest) {
    uint32_t pageSize = target_get_flash_page_size();
    return (uint16_t*)(((uint32_t)dest / pageSize) * pageSize);
}

uint32_t target_get_flash_page_size(void) {
    return (desig_get_flash_size() > 128) ? TARGET_FLASH_PAGE_SIZE_2K : TARGET_FLASH_PAGE_SIZE_1K;
}

bool target_flash_program_array(uint16_t* dest, const uint16_t* data, size_t half_word_count) {
    bool verified = true;

    uint32_t write_start = (uint32_t)dest;

    /* Remember the bounds of erased data in the current page */
    static uint16_t* erase_start = NULL;
    static uint16_t* erase_end   = NULL;

    const uint16_t* flash_end = get_flash_end();
    while (half_word_count > 0) {
        /* Avoid writing past the end of flash */
        if (dest >= flash_end) {
            verified = false;
            break;
        }

        if (dest >= erase_end || dest < erase_start) {
            erase_start = get_flash_page_address(dest);
            erase_end = erase_start + (target_get_flash_page_size())/sizeof(uint16_t);
            flashmap_draw_cell_erased((uint32_t)erase_start - FLASH_BASE);
            flash_erase_page((uint32_t)erase_start);
        }
        flash_program_half_word((uint32_t)dest, *data);
        erase_start = dest + 1;
        if (*(volatile uint16_t*)dest != *data) {
            verified = false;
            break;
        }
        dest++;
        data++;
        half_word_count--;
    }

    flashmap_draw_cell_written(write_start - FLASH_BASE, verified);

    return verified;
}

void target_on_fw_update_completed(void)
{
    bootlog_add("Firmware updated", BOOTLOG_MSG_TYPE_HIGHLIGHTED);
    bootlog_add("Restarting...", BOOTLOG_MSG_TYPE_INFO);

    // Disable USB pullup to disconnect
    target_usb_pullup_enable(false);

    // Reset the device
    HAL_Delay(3500);
    scb_reset_system();
    while (1) {}
}
