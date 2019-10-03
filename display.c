#include <stdlib.h> /* malloc() and free() */
#include <curses.h> /* WINDOW definition and most function calls */
#include <error.h> /* error() */

#include "display.h"
#include "tetrimino.h"
#include "state.h" /* render_state definition */
#include "grid.h" /* render_grid definition */

/* display struct definition */

struct display {
	/* the screen to which the overall game will be rendered */
	WINDOW *output;
	/* cache of the width and height of the output screen */
	int width, height;
	WINDOW *grid_win;
	int grid_width;
	int grid_height;
	int grid_startx, grid_starty;
	WINDOW *hold_stats_win;
	// TODO: implement stats window WINDOW *hold_stats_win;
	// TODO: implement hold queue WINDOW *next_queue_win;
};

struct display * create_display(WINDOW *out, int grid_width, int grid_height) {
	struct display *ret = (struct display *)malloc(sizeof(struct display));
	if (ret == NULL) {
		return NULL;
	}
	ret->output = out;
	getmaxyx(out, ret->height, ret->width);
	ret->grid_height = grid_height;
	ret->grid_width = grid_width;
	ret->grid_starty = (ret->height - grid_height) / 2;
	ret->grid_startx = (ret->width - grid_width) / 2;
	ret->grid_win = subwin(ret->output, ret->grid_height, ret->grid_width, ret->grid_starty, ret->grid_startx);

	if (ret->grid_win == NULL) {
		return NULL;
	}

	wbkgdset(ret->grid_win, COLOR_PAIR(2));
	leaveok(ret->output, TRUE);
	leaveok(ret->grid_win, TRUE);

	return ret;
}

void destroy_display(struct display *disp) {
	if (disp == NULL) {
		return;
	}
	if (disp->grid_win != NULL) {
		delwin(disp->grid_win);
	}
	free(disp);
}

/* rendering forward declarations */
void render_pause(WINDOW *win);
void render_grid(WINDOW* window, const struct tetris_grid *grid);
void render_borders(struct display *disp);
void render_piece(struct display *disp, const struct tetrimino *piece);

void render_state(struct display *disp, const struct game_state *state) {
	render_borders(disp);
	if (game_paused(state)) {
		render_pause(disp->grid_win);
	} else {
		render_grid(disp->grid_win, game_grid(state));
		render_piece(disp, game_piece(state));
	}
	wnoutrefresh(disp->grid_win);
	//clearok(disp->grid_win, FALSE);
	doupdate();
}

/* render the "pause" icon and obscure the game grid */
void render_pause(WINDOW *win) {
	int height, width;
	getmaxyx(win, height, width);		/* get the number of rows and columns */
	/* repaint cleanly */
	werase(win);
	mvwprintw(win, height/2, 0, " -PAUSED- ");
}

void render_grid(WINDOW* window, const struct tetris_grid *grid) {
	/* repaint cleanly */
	//clearok(window, TRUE);
	wmove(window, 0, 0);
	for (int row = GRID_VISIBLE_HEIGHT-1; row >= 0; --row) {
		for (int col = 0; col < 10; ++col) {
			const char *disp_str;
			switch (tg_getcell(grid, col, row)) {
				case GC_EMPTY:
					disp_str = " ";
					break;
				case GC_FILL1:
					disp_str = "░";
					break;
				case GC_FILL2:
					disp_str = "█";
					break;
				default:
					error(1, 0, "invalid cell state during render");
					break;
			}
			wprintw(window, disp_str); 
		}
	}
}

void render_piece(struct display *disp, const struct tetrimino *piece) {
	if (piece == NULL) {
		return;
	}
	for (size_t i = 0; i < 4; ++i) {
		int x = piece->minos[i].x + piece->pos_x;
		int y = piece->minos[i].y + piece->pos_y;
		// TODO: test visibility before we attempt outputting it
		mvwprintw(disp->grid_win, 20-y, x, "█");
	}
}

/* draw borders around the grid and other ui elements */
void render_borders(struct display *disp) {
	mvwvline(disp->output, disp->grid_starty, disp->grid_startx-1, '|', disp->grid_height);
	mvwvline(disp->output, disp->grid_starty, disp->grid_startx + disp->grid_width, '|', disp->grid_height);
	mvwhline(disp->output, disp->grid_starty-1, disp->grid_startx, '_', disp->grid_width);
	mvwhline(disp->output, disp->grid_starty+disp->grid_height, disp->grid_startx, '^', disp->grid_width);
}

/* render the clear stats on the side of the menu */
void render_stats(WINDOW *win, const struct game_state *state) {
	werase(win);
	wmove(win, 0, 0);
	wprintw(win, "LEVEL   \n%8ld\n", game_level(state));
	wprintw(win, "LINES   \n%8ld", game_lines_cleared(state));
}
