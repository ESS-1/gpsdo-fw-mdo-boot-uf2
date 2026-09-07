#pragma once

extern void flashmap_init(void);
extern void flashmap_draw_cell_erased(uint32_t flashOffset);
extern void flashmap_draw_cell_written(uint32_t flashOffset, bool verified);
