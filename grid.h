#pragma once

/* the width of a tetris grid */
#define GRID_WIDTH 10
/* the total height of a tetris grid */
#define GRID_HEIGHT 40
/* the visible height of a tetris grid */
#define GRID_VISIBLE_HEIGHT 21

/* the possible contents of a tetris grid cell
 * TODO: add support for colors / tetrimino-specific */
enum grid_cell {
	GC_EMPTY = 0,
	GC_FILL1 = 1,
	GC_FILL2 = 2
};

struct tetris_grid {
	/* the grid is indexed bottom up, left-to-right
	 * the first 20 rows of 10 are the Buffer Zone
	 * the next 20 rows are the Matrix, which is the main visible play area
	 */
	enum grid_cell cells[GRID_WIDTH * GRID_HEIGHT];
};

/* set the value of a cell */
void tg_setcell(struct tetris_grid *, unsigned int col, unsigned int row, enum grid_cell cell);

/* get the value of a cell */
enum grid_cell tg_getcell(const struct tetris_grid *, unsigned int col, unsigned int row);

/* clear one line and shift the rest down 1*/
void tg_rmline(struct tetris_grid *, unsigned int line);

/* clear the board */
void tg_clear(struct tetris_grid *);
