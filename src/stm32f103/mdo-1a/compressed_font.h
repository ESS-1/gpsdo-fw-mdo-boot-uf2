#pragma once

#include "fonts.h"

extern FontDef CompressedFont_7x10;

/**
 * @brief Initialize the compressed font by decompressing it into RAM.
 */
void compressed_font_init(void);
