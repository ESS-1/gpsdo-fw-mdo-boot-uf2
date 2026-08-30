#include "bootlog.h"
#include "st7735.h"
#include "compressed_font.h"
#include <stdint.h>
#include <string.h>

#define MAX_ENTRIES           6
#define MAX_MSG_LENGTH        23

#define BG_COLOR              ST7735_BLACK

#define MSG_COLOR_INFO        ST7735_COLOR565(175, 175, 175)
#define MSG_COLOR_HIGHLIGHTED ST7735_GREEN
#define MSG_COLOR_ERROR       ST7735_RED

#define LINE_HEIGHT_PX        11


typedef struct {
    char msg[MAX_MSG_LENGTH];
    bootlog_msg_type_t type;
} bootlog_entry_t;

static bootlog_entry_t bootlog[MAX_ENTRIES] = { 0 };
static int32_t bootlog_next_line = 0;
static int32_t bootlog_total_lines = 0;

static uint16_t bootlog_get_color(bootlog_msg_type_t type)
{
    switch (type) {
        case BOOTLOG_MSG_TYPE_HIGHLIGHTED:
            return MSG_COLOR_HIGHLIGHTED;
        case BOOTLOG_MSG_TYPE_ERROR:
            return MSG_COLOR_ERROR;
        default:
        case BOOTLOG_MSG_TYPE_INFO:
            return MSG_COLOR_INFO;
    }
}

void bootlog_init(void)
{
    compressed_font_init();
    ST7735_Init();
    ST7735_FillScreen(BG_COLOR);
}

static void bootlog_draw(void)
{
    int32_t start_line = bootlog_total_lines > MAX_ENTRIES ? bootlog_next_line : 0;
    int32_t lines_to_draw = bootlog_total_lines > MAX_ENTRIES ? MAX_ENTRIES : bootlog_total_lines;

    for (int32_t i = 0; i < lines_to_draw; ++i) {
        int32_t line_index = (start_line + i) % MAX_ENTRIES;
        const bootlog_entry_t* entry = &bootlog[line_index];

        ST7735_WriteString(0, i * LINE_HEIGHT_PX, entry->msg, CompressedFont_7x10, bootlog_get_color(entry->type), BG_COLOR);
    }
}

void bootlog_add(const char* msg, bootlog_msg_type_t type)
{
    bootlog[bootlog_next_line].type = type;
    char* entry_msg = bootlog[bootlog_next_line].msg;

    size_t len = 0;
    while (msg[len] != '\0' && len < (MAX_MSG_LENGTH - 1)) {
        entry_msg[len] = msg[len];
        len++;
    }

    while (len < (MAX_MSG_LENGTH - 1)) {
        entry_msg[len++] = ' ';
    }
    entry_msg[MAX_MSG_LENGTH - 1] = '\0';

    ++bootlog_total_lines;
    if (++bootlog_next_line >= MAX_ENTRIES) {
        bootlog_next_line = 0;
    }

    bootlog_draw();
}
