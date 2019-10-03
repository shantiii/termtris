#include <locale.h>
#include <stdlib.h>
#include <error.h>
#include <string.h>
#include <curses.h>
#include <time.h>

#include "tetrimino.h"
#include "bag.h"
#include "grid.h"
#include "event_queue.h"
#include "state.h"
#include "display.h"

// TODO: standardize on one calling convention (out params, or return values or something)

struct game_state {
	/* the randomizer */
	struct tetris_bag *bag;
	/* the 10x40 play field grid */
	struct tetris_grid grid;
	/* the "current" piece */
	struct tetrimino piece;
	/* true if the current piece is valid */
	bool piece_active;
	/* current phase the tetris engine is in */
	enum engine_phase phase;
	/* true when the game is paused, false otherwise */
	bool paused;
	/* true when it's time for game to exit */
	bool exiting;
	/* the falling speed of blocks */
	int64_t level;
	/* state for marking which lines are to be deleted in the pattern phase */
	unsigned long long int lines_marked : 40;
	/* keeping things aligned */
	int _padding:14;
	/* the number of lines successfully cleared */
	int64_t lines_cleared;
	/* event queue - TODO: should this be part of the state? */
	struct event_queue *events;
	/* the nanotime since at which the game started */
	int64_t start_time;
	/* the nanotime since the most recent event processed */
	int64_t now;
};

void phase_transition(struct game_state *state, enum engine_phase phase);

void generate_piece(struct game_state *state);

bool game_paused(const struct game_state *state) {
	return state->paused;
}

const struct tetris_grid * game_grid(const struct game_state *state) {
	return &state->grid;
}

bool valid_placement(const struct tetris_grid *grid, const struct tetrimino piece) {
	bool result = TRUE;
	for (size_t i=0; i<4; ++i) {
		int x = (piece.minos[i].x + piece.pos_x);
		int y = (piece.minos[i].y + piece.pos_y);
		result = result &&
			x >= 0 && x < GRID_WIDTH &&
			y >=0 && y < GRID_HEIGHT &&
			(tg_getcell(grid, x, y) == GC_EMPTY);
	}
	return result;
}

/* locks down a tetrimino, making it part of the grid */
void lockdown(struct tetris_grid *grid, const struct tetrimino piece) {
	for (size_t i=0; i<4; ++i) {
		int x = (piece.minos[i].x + piece.pos_x);
		int y = (piece.minos[i].y + piece.pos_y);
		tg_setcell(grid, x, y, GC_FILL1);
	}
}

void step_generation(struct game_state *state, const struct game_event *event);
void step_falling(struct game_state *state, const struct game_event *event);
void step_lock(struct game_state *state, const struct game_event *event);
void step_pattern(struct game_state *state, const struct game_event *event);
void step_iterate(struct game_state *state, const struct game_event *event);
void step_animate(struct game_state *state, const struct game_event *event);
void step_eliminate(struct game_state *state, const struct game_event *event);
void step_completion(struct game_state *state, const struct game_event *event);

void (*phase_handlers[NUM_ENGINE_PHASES])(struct game_state *state, const struct game_event *event) = {
	step_generation,
	step_falling,
	step_lock,
	step_pattern,
	step_iterate,
	step_animate,
	step_eliminate,
	step_completion,

	0,0,0
};

void game_step(struct game_state *state, const struct game_event *event) {
	if (phase_handlers[state->phase] != NULL) {
		phase_handlers[state->phase](state, event);
	}

	/* TODO: don't special case this, move logic into phase handler for newgame */
	if (event->type == GE_NEWGAME) {
		eq_clear(state->events); 
		phase_transition(state, EP_GENERATION);
	}

	/* respond to player input */
	if (state->piece_active) {
		switch (event->type) {
			case GE_LSHIFT:
				state->piece.pos_x--;
				if (!valid_placement(&state->grid, state->piece)) {
					state->piece.pos_x++;
				}
				break;
			case GE_RSHIFT:
				state->piece.pos_x++;
				if (!valid_placement(&state->grid, state->piece)) {
					state->piece.pos_x--;
				}
				break;
			case GE_CWROTATE:
				{
					/* TODO: generate kick translations and sequentially test */
					struct tetrimino potential = tet_rotate_cw(state->piece);
					if (valid_placement(&state->grid, potential)) {
						state->piece = potential;
					}
				}
				break;
			case GE_CCWROTATE:
				{
					/* TODO: generate kick translations and sequentially test */
					struct tetrimino potential = tet_rotate_ccw(state->piece);
					if (valid_placement(&state->grid, potential)) {
						state->piece = potential;
					}
				}
				break;
			case GE_HARDDROP:
				while (valid_placement(&state->grid, state->piece)) {
					state->piece.pos_y--;
				}
				state->piece.pos_y++;
				/* TODO: replace with a transition to the lockdown state */
				phase_transition(state, EP_PATTERN);
				break;
			case GE_SOFTDROP:
				state->piece.pos_y--;
				if (!valid_placement(&state->grid, state->piece)) {
					state->piece.pos_y++;
					/* TODO: transition to lockdown state instead */
					phase_transition(state, EP_PATTERN);
				}
				break;
			case GE_PAUSE:
				if (state->paused == TRUE) {
					state->paused = FALSE;
				} else {
					state->paused = TRUE;
				}
				break;
			default:
				break;
		}
	}

	/* update the event */
	if (event->time > state->now) {
		state->now = event->time;
	}
}

int64_t now64() {
	struct timespec spec;
	clock_gettime(CLOCK_MONOTONIC, &spec);
	return spec.tv_sec * 1000000000L + spec.tv_nsec;
}

/* initializes a game state structure. returns 0 on success */
int game_init(struct game_state *state) {
	if (state == NULL) {
		return -1;
	}
	tg_clear(&state->grid); /* clear grid */
	state->bag = create_bag(1); /*initialize bag */
	state->events = create_eq(); /* initialize event queue */

	state->now = state->start_time = now64();

	state->phase = EP_NEWGAME;
	state->piece_active = FALSE;
	state->paused = FALSE;
	state->exiting = FALSE; /* TODO: do I need this? */
	state->level = 0;
	state->lines_cleared = 0;

	return 0;
}

/* STEP HANDLERS */
/* This is where the meat of the state transition flow goes */

void step_generation(struct game_state *state, const struct game_event *event) {
	if (event->type == GE_ENTER) {
		state->piece = TETRIMINOS[bag_pull(state->bag)];
		/* if there is a piece in the way of generation, the game is over */
		if (!valid_placement(&state->grid, state->piece)) {
			phase_transition(state, EP_GAMEOVER);
		} else {
			phase_transition(state, EP_FALLING);
		}
	} else {
		// TODO: handle other types of events here
		// user input events should probably be buffered
	}
}

void step_falling(struct game_state *state, const struct game_event *event) {
	/* attempt to fall once */
	/* if successful */
	if (event->type == GE_ENTER) {
		state->piece_active = TRUE;
		state->piece.pos_y--;
		if (!valid_placement(&state->grid, state->piece)) {
			state->piece.pos_y++;
			phase_transition(state, EP_LOCK);
			return;
		}
		struct game_event next_fall = {
			.type = GE_ENTER,
			.time = event->time + 1000000000L // TODO: use the level speed
		};
		eq_push(state->events, next_fall);
	}
}

void step_lock(struct game_state *state, const struct game_event *event) {
	if (event->type == GE_ENTER) {
		struct game_event lockdown = {
			.type = GE_LOCKDOWN,
			.time = event->time + 500000000L // TODO: use the lock delay
		};
		eq_push(state->events, lockdown);
	} else if (event->type == GE_LOCKDOWN) {
		phase_transition(state, EP_PATTERN);
	}
}

void step_pattern(struct game_state *state, const struct game_event *event) {
	state->piece_active = FALSE;
	lockdown(&state->grid, state->piece);
	state->lines_marked = 0;
	for (int row = 0; row < GRID_HEIGHT; ++row) {
		bool good = true;
		for (int col = 0; col < GRID_WIDTH; ++col) {
			good = good && tg_getcell(&state->grid, col, row) != GC_EMPTY;
		}
		state->lines_marked |= ((unsigned int)good) << row;
	}
	phase_transition(state, EP_ITERATE);
}
void step_iterate(struct game_state *state, const struct game_event *event) {
	phase_transition(state, EP_ANIMATE);
}
void step_animate(struct game_state *state, const struct game_event *event) {
	phase_transition(state, EP_ELIMINATE);
}
void step_eliminate(struct game_state *state, const struct game_event *event) {
	for (int row = 0; row < GRID_HEIGHT; ++row) {
		if (state->lines_marked & (1 << row)) {
			tg_rmline(&state->grid, row);
		}
	}
	phase_transition(state, EP_COMPLETION);
}
void step_completion(struct game_state *state, const struct game_event *event) {
	phase_transition(state, EP_GENERATION);
}

void phase_transition(struct game_state *state, enum engine_phase phase) {
	struct game_event entrance = {
		.type = GE_ENTER,
		.time = state->now
	};
	state->phase = phase;
	eq_clear(state->events);
	eq_push(state->events, entrance);
}

void generate_piece(struct game_state *state) {
	state->piece = TETRIMINOS[bag_pull(state->bag)];
}

int64_t game_next_event_time(const struct game_state *state) {
	struct game_event peek;
	if (eq_peek(state->events, &peek)) {
		return peek.time;
	}
	return -1;
}

int event_for_key(struct game_event *event, int key);

void game_loop(struct display *disp, struct game_state *state) {
	struct game_event new_game_event = { .type = GE_NEWGAME, .time = now64() };
	eq_push(state->events, new_game_event);
	int64_t now = now64();
	int64_t frame_count = 0;;
	while(!state->exiting) {
		++frame_count;
		/* fast forward game state through event queue */
		struct game_event peek;
		while (eq_peek(state->events, &peek) && now >= peek.time) {
			struct game_event pop;
			eq_pop(state->events, &peek);
			game_step(state, &peek);
		}
		render_state(disp, state);

		/* timestamp of the last refresh */
		int64_t last = now;
		now = now64();

		int64_t next_event = game_next_event_time(state);
		if (next_event == -1) {
			timeout(16);
		} else {
			/* 60 fps */
			int64_t next_thing = (last + 16666667L) > next_event ? (last + 16666667L) : next_event;
			int64_t timeout_ns = next_thing - now;
			if (timeout_ns < 0) { timeout_ns = 0; }
			timeout(timeout_ns / 1000000L);
		}

		int key = getch();
		// TODO: recreate display on terminal resizing events (KEY_RESIZE)
		// TODO: handle timeouts nicely
		struct game_event event = { .type = GE_NOOP, .time = now};
		if (event_for_key(&event, key)) {
			eq_push(state->events, event);
		}
		mvprintw(0,0,"key: %d\n", key);
		printw("paused: %d\n", state->paused);
		printw("frm: %ld\n", frame_count);
		printw("phs: %ld\n", state->phase);
		printw("qlen: %d\n", eq_len(state->events));
		{
			struct game_event peek;
			if (eq_peek(state->events, &peek)) {
				printw("event: %d\n", (int) peek.type);
			}
		}
	}
}

int64_t game_level(const struct game_state *state) {
	return state->level;
}
int64_t game_lines_cleared(const struct game_state *state) {
	return state->lines_cleared;
}
const struct event_queue * game_queue(const struct game_state *state) {
	return state->events;
}

int event_for_key(struct game_event *event, int key) {
	switch (key) {
		case ERR:
			return FALSE;
		case KEY_UP:
			event->type = GE_HARDDROP;
			break;
		case KEY_DOWN:
			event->type = GE_SOFTDROP;
			break;
		case 10:
			mvprintw(1,0,"boop: %x\n", key);
			event->type = GE_PAUSE;
			break;
		case 'z':
			event->type = GE_CCWROTATE;
			break;
		case 'x':
			event->type = GE_CWROTATE;
			break;
		case KEY_LEFT: /* intentional fall-through */
		case 'j':
			event->type = GE_LSHIFT;
			break;
		case KEY_RIGHT: /* intentional fall-through */
		case 'k':
			event->type = GE_RSHIFT;
			break;

			/* timeouts case a fall */
			/* TODO: fix this so it actually does nothing? */
		default:
			// TODO: output the missed key as debug
			break;
	}
	return TRUE;
}

/* initialize all global ncurses-related setup */
void init_ncurses() {
	/* initialize locale for ncurses */
	setlocale(LC_CTYPE, "");

	initscr();						/* initialize ncurses library */
	start_color();				/* enable colors */
	cbreak();							/* disable line-buffering */
	noecho();							/* disable echo of input to terminal */
	curs_set(0);					/* disable cursor */
	keypad(stdscr, TRUE); /* allow big 'ol keys */

	/* initialize colors */

	init_pair(1, COLOR_RED, COLOR_BLUE);
	init_pair(2, COLOR_WHITE, COLOR_BLACK);
}

/* terminate all global ncurses-related state */
void term_ncurses() {
	endwin();
}

/* all main should do is initialize the display, input, and game state, and
 * then hand control to the game until it's time for the end */
int main() {
	/* store the intermediate game state */
	struct game_state state;
	/* store the ncurses display information */
	struct display *disp;

	/* initialize ncurses */
	init_ncurses();

	/* initialize screen */
	if ((disp = create_display(stdscr, GRID_WIDTH, GRID_VISIBLE_HEIGHT)) == NULL) {
		// TODO report error here
		return 1;
	}

	/* initialize the game state */
	if (game_init(&state) != 0) {
		return 1;
	}

	/* enter the main game event loop */
	game_loop(disp, &state);

	destroy_display(disp); /* deinitialize screen */
	term_ncurses(); /* peace out */
	return 0;
}

const struct tetrimino * game_piece(const struct game_state *state) {
	if (!state->piece_active) {
		return NULL;
	}
	return &state->piece;
}
