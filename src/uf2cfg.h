// Address range where UF2 files are allowed to write data (prevents overwriting the bootloader)
#define USER_FLASH_START ((uint32_t)(FLASH_BASE + BOOTLOADER_SIZE))
#define USER_FLASH_END (0x08000000+FLASH_SIZE_OVERRIDE)
