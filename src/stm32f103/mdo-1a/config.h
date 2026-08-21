// Configuration for MDO-1A

#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

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

#define USES_GPIOA             0
#define USES_GPIOB             1
#define USES_GPIOC             0

#define UF2_FAMILY             0xCA8A701A // MDO-1A

#undef VOLUME_LABEL
#define VOLUME_LABEL           "MDO-1A"
#undef PRODUCT_NAME
#define PRODUCT_NAME           "MDO-1A GPSDO"
#undef BOARD_ID
#define BOARD_ID               "MDO-1A-v1"
#undef INDEX_URL
#define INDEX_URL              "https://github.com/ESS-1/gpsdo-fw-mdo-1a"

#endif
