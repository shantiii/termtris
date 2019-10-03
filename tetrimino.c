#include "tetrimino.h"

/* the 7 tetrimino pieces, indexable by the tetrimino_type enum */
const struct tetrimino TETRIMINOS[7] = {

	/*TET_I*/{
		.minos = { {-1, 0}, {0, 0}, {1, 0}, {2, 0} },
		.rs = RS_NORTH,
		.type = TT_I,
		.pos_x = 4,
		.pos_y = 20
	},

	/*TET_J*/{
		.minos = { {-1, 1}, {-1, 0}, {0, 0}, {1, 0} },
		.rs = RS_NORTH,
		.type = TT_J,
		.pos_x = 4,
		.pos_y = 20
	},

	/*TET_L*/{
		.minos = { {1, 1}, {-1, 0}, {0, 0}, {1, 0} },
		.rs = RS_NORTH,
		.type = TT_L,
		.pos_x = 4,
		.pos_y = 20
	},

	/*TET_O*/{
		.minos = { {0, 0}, {1, 0}, {0, 1}, {1, 1} },
		.rs = RS_NORTH,
		.type = TT_O,
		.pos_x = 4,
		.pos_y = 20
	},

	/*TET_S*/{
		.minos = { {-1, 0}, {0, 0}, {0, 1}, {1, 1} },
		.rs = RS_NORTH,
		.type = TT_S,
		.pos_x = 4,
		.pos_y = 20
	},

	/*TET_Z*/{
		.minos = { {-1, 1}, {0, 1}, {0, 0}, {1, 0} },
		.rs = RS_NORTH,
		.type = TT_Z,
		.pos_x = 4,
		.pos_y = 20
	},

	/*TET_T*/{
		.minos = { {0, 1}, {-1, 0}, {0, 0}, {1, 0} },
		.rs = RS_NORTH,
		.type = TT_T,
		.pos_x = 4,
		.pos_y = 20
	}

};

enum rotation_state rs_cw(enum rotation_state rs) {
	return (rs + 1) % 4;
}

enum rotation_state rs_ccw(enum rotation_state rs) {
	return (rs + 3) % 4;
}

struct tetrimino tet_rotate_cw(struct tetrimino orig) {
	struct tetrimino rotated;
	rotated.type = orig.type;
	rotated.pos_x = orig.pos_x;
	rotated.pos_y = orig.pos_y;
	rotated.rs = rs_cw(orig.rs);
	for (int i = 0; i < 4; ++i) {
		rotated.minos[i].y = -1 * orig.minos[i].x;
		rotated.minos[i].x = orig.minos[i].y;
	}
	return rotated;
}

struct tetrimino tet_rotate_ccw(struct tetrimino orig) {
	struct tetrimino rotated;
	rotated.type = orig.type;
	rotated.pos_x = orig.pos_x;
	rotated.pos_y = orig.pos_y;
	rotated.rs = rs_ccw(orig.rs);
	for (int i = 0; i < 4; ++i) {
		rotated.minos[i].y = orig.minos[i].x;
		rotated.minos[i].x = -1 * orig.minos[i].y;
	}
	return rotated;
}
