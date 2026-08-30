// Configuration for MDO-1A

#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

#define UF2_FAMILY             0xCA8A701A // MDO-1A family ID

#define FLASH_SIZE_OVERRIDE    0x20000 // 128K
#define FLASH_PAGE_SIZE        1024    // 1024 for genuine chips, 2048 for clones

#define UF2_NUM_BLOCKS         8000 // UF2 drive size in blocks (512 bytes each)

#define BUTTON_GPIO_PORT       GPIOB
#define BUTTON_GPIO_PIN        GPIO5
#define BUTTON_ACTIVE_HIGH     0

#define USB_PULLUP_GPIO_PORT   GPIOB
#define USB_PULLUP_GPIO_PIN    GPIO14
#define USB_PULLUP_OPEN_DRAIN  0
#define USB_PULLUP_ACTIVE_HIGH 1

#define USES_GPIOA             1
#define USES_GPIOB             1
#define USES_GPIOC             0

#define VENDOR_ID              "Generic"
#define VOLUME_LABEL           "MDO-1A"
#define PRODUCT_NAME           "MDO-1A GPSDO"
#define BOARD_ID               "MDO-1A-v1"
#define INDEX_URL              "https://github.com/ESS-1/gpsdo-fw-mdo-1a"

// PLL/OCXO pins
#define OCXO_EN_PIN             GPIO6
#define OCXO_EN_GPIO_PORT       GPIOA
#define PLL_I2C                 I2C1
#define PLL_I2C_RCC             RCC_I2C1
#define PLL_I2C_RST             RST_I2C1
#define PLL_I2C_PINS            (GPIO8 | GPIO9)
#define PLL_I2C_GPIO_PORT       GPIOB
#define PLL_I2C_AFIO_MAPR       AFIO_MAPR_I2C1_REMAP // Remap I2C1 pins to PB8 (SCL) and PB9 (SDA)

// LCD GPIO
#define LCD_RES_PIN            GPIO4
#define LCD_RES_GPIO_PORT      GPIOA
#define LCD_CS_PIN             GPIO2
#define LCD_CS_GPIO_PORT       GPIOA
#define LCD_DC_PIN             GPIO3
#define LCD_DC_GPIO_PORT       GPIOA

// LCD SPI
#define LCD_SPI_PINS            (GPIO5 | GPIO7)
#define LCD_SPI_GPIO_PORT       GPIOA
#define LCD_SPI                 SPI1
#define LCD_SPI_RCC             RCC_SPI1

// USB
#define USB_BUS_POWERED         1
#define USB_MAX_CURRENT         0xFA // 500 mA

#endif
