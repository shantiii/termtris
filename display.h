#pragma once

#include <curses.h> /* for WINDOW type definition */

/* display interface header */

struct display;
struct game_state;

/* returns a new display based off of ncurses' window */
struct display * create_display(WINDOW *out, int grid_width, int grid_height);
void destroy_display(struct display *disp);

void render_state(struct display *disp, const struct game_state *state);
