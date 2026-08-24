// Configuration for MDO-1A

#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

#define UF2_FAMILY             0xCA8A701A // MDO-1A family ID

#define APP_BASE_ADDRESS       0x08004000
#define FLASH_SIZE_OVERRIDE    0x20000
#define FLASH_PAGE_SIZE        1024

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

#endif
