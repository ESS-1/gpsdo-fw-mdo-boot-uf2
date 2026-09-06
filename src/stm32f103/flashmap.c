#include "flashmap.h"
#include "st7735.h"
#include "compressed_font.h"
#include "colors.h"

#include "config.h"
#include "uf2cfg.h"

#define GRID_ROWS         4
#define GRID_COLUMNS      32
#define GRID_CELL_SIZE    3
#define GRID_WIDTH        (GRID_COLUMNS*GRID_CELL_SIZE + (GRID_COLUMNS+1))
#define GRID_HEIGHT       (GRID_ROWS   *GRID_CELL_SIZE + (GRID_ROWS   +1))
#define GRID_X            (160-GRID_WIDTH)
#define GRID_Y            (80 -GRID_HEIGHT)

#define BYTES_PER_CELL    (TOTAL_FLASH_SIZE / (GRID_ROWS * GRID_COLUMNS))

static void flashmap_draw_cell(int cell, uint16_t color)
{
    uint16_t row = cell / GRID_COLUMNS;
    uint16_t col = cell % GRID_COLUMNS;
    uint16_t x = GRID_X + 1 + col * (GRID_CELL_SIZE + 1);
    uint16_t y = GRID_Y + 1 + row * (GRID_CELL_SIZE + 1);
    ST7735_FillRectangle(x, y, GRID_CELL_SIZE, GRID_CELL_SIZE, color);
}

void flashmap_init(void)
{
    // Draw legend
    ST7735_WriteString(0,       GRID_Y,     "BL",  CompressedFont_7x10, COLOR_FLASHMAP_BLOCK_BL,  COLOR_BG);
    ST7735_WriteString(2*7 + 3, GRID_Y,     "EE",  CompressedFont_7x10, COLOR_FLASHMAP_BLOCK_EE,  COLOR_BG);
    ST7735_WriteString(5,       GRID_Y + 9, "APP", CompressedFont_7x10, COLOR_FLASHMAP_BLOCK_APP, COLOR_BG);

    // Draw horizontal gridlines
    int y = GRID_Y;
    for (int r = 0; r <= GRID_ROWS; ++r) {
        ST7735_FillRectangle(GRID_X, y, GRID_WIDTH, 1, COLOR_FLASHMAP_FRAME);
        y += (GRID_CELL_SIZE + 1);
    }

    // Draw vertical gridlines
    int x = GRID_X;
    for (int c = 0; c <= GRID_COLUMNS; ++c) {
        ST7735_FillRectangle(x, GRID_Y, 1, GRID_HEIGHT, COLOR_FLASHMAP_FRAME);
        x += (GRID_CELL_SIZE + 1);
    }

    // Draw cells
    for (uint32_t i = 0; i < GRID_ROWS * GRID_COLUMNS; ++i) {
        uint32_t flash_offset = i * BYTES_PER_CELL;

        uint16_t color;
        if (flash_offset < BOOTLOADER_SIZE) {
            color = COLOR_FLASHMAP_BLOCK_BL;
        } else if (flash_offset < (TOTAL_FLASH_SIZE - FLASH_EEPROM_SIZE)) {
            color = COLOR_FLASHMAP_BLOCK_APP;
        } else {
            color = COLOR_FLASHMAP_BLOCK_EE;
        }

        flashmap_draw_cell(i, color);
    }
}
