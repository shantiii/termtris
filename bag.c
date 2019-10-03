#include <stdlib.h> /* random functions */
#include <error.h>

#include "bag.h"
#include "tetrimino.h"

// BAG IMPL //
struct tetris_bag {
	/* stores the random state as used by initstate_r */
	char random_state[256];
	/* stores the random data as used by random_r functions */
  struct random_data random_data_buf;
	/* the 14 next items in the bag, realized */
	enum tetrimino_type realized[14];
};

struct tetris_bag* create_bag(unsigned int seed) {
	struct tetris_bag *bag = (struct tetris_bag *) malloc(sizeof(struct tetris_bag));
	bag->random_data_buf.state = NULL;
	if (initstate_r(seed, bag->random_state, 256, &bag->random_data_buf) != 0) {
		error(1, 0, "could not initialize bag random state");
	}
	return bag;
}

void destroy_bag(struct tetris_bag *bag) {
	if (bag != NULL) {
		free(bag);
	}
}

enum tetrimino_type bag_peek(const struct tetris_bag *bag, unsigned int n) {
	// TODO: actually randomize what we peek
	return TT_I;
}

enum tetrimino_type bag_pull(struct tetris_bag *bag) {
	int32_t rand_val;
	random_r(&bag->random_data_buf, &rand_val);
	return (enum tetrimino_type) (rand_val % 7);
}
