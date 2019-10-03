#include <string.h> /* memmove and memset */
#include "grid.h"

void tg_setcell(struct tetris_grid *grid, unsigned int col, unsigned int row, enum grid_cell cell) {
	grid->cells[row * 10 + col] = cell;
}

enum grid_cell tg_getcell(const struct tetris_grid *grid, unsigned int col, unsigned int row) {
	return grid->cells[row * 10 + col];
}

void tg_rmline(struct tetris_grid *grid, unsigned int line) {
	memmove(grid->cells + (10*line), grid->cells + (10*(line+1)), sizeof(enum grid_cell) * 10 * (39-line));
	memset(grid->cells + 390, (int) GC_EMPTY, sizeof(enum grid_cell) * 10);
}

void tg_clear(struct tetris_grid *grid) {
	memset(grid->cells, (int) GC_EMPTY, sizeof(grid->cells));
}
