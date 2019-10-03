#pragma once

struct game_state;

struct event_queue;
struct tetris_grid;
struct tetrimino;

bool game_paused(const struct game_state *);
const struct tetrimino * game_piece(const struct game_state *);
const struct tetris_grid * game_grid(const struct game_state *);
int64_t game_level(const struct game_state *);
int64_t game_lines_cleared(const struct game_state *);
const struct event_queue * game_queue(const struct game_state *);

enum engine_phase {
	/* official tetris engine phases */
	EP_GENERATION = 0,
	EP_FALLING,
	EP_LOCK,
	EP_PATTERN,
	EP_ITERATE,
	EP_ANIMATE,
	EP_ELIMINATE,
	EP_COMPLETION,

	/* special pseudo-phases */
	EP_NEWGAME,
	EP_GAMEOVER,
	EP_QUITTING,

	/* number of phases */
	NUM_ENGINE_PHASES
};
