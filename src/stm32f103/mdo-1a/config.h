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

// LCD GPIO
#define LCD_RES_Pin            GPIO4
#define LCD_RES_GPIO_Port      GPIOA
#define LCD_CS_Pin             GPIO2
#define LCD_CS_GPIO_Port       GPIOA
#define LCD_DC_Pin             GPIO3
#define LCD_DC_GPIO_Port       GPIOA

// LCD SPI
#define LCD_SCL_Pin             GPIO5
#define LCD_SCL_GPIO_Port       GPIOA
#define LCD_SDA_Pin             GPIO7
#define LCD_SDA_GPIO_Port       GPIOA
#define LCD_SPI                 SPI1
#define LCD_SPI_RCC             RCC_SPI1

// USB
#define USB_BUS_POWERED         1
#define USB_MAX_CURRENT         0xFA // 500 mA

#endif
