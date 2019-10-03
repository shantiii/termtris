#include <inttypes.h>

struct mino {
	int8_t x, y;
};

enum tetrimino_type {
	TT_I = 0,
	TT_O,
	TT_J,
	TT_L,
	TT_S,
	TT_Z,
	TT_T
};

enum rotation_state {
	RS_NORTH,
	RS_EAST,
	RS_SOUTH,
	RS_WEST
};

/* representative of the NORTH rotation state */
struct tetrimino {
	struct mino minos[4];
	enum rotation_state rs;
	enum tetrimino_type type;
	int8_t pos_x, pos_y;
};

/* the 7 tetrimino pieces, indexable by the tetrimino_type enum */
extern const struct tetrimino TETRIMINOS[7];

enum rotation_state rs_cw(enum rotation_state rs);

enum rotation_state rs_ccw(enum rotation_state rs);

struct tetrimino tet_rotate_cw(struct tetrimino orig);

struct tetrimino tet_rotate_ccw(struct tetrimino orig);
