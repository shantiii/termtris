#pragma once

enum tetrimino_type;

/* a bag is a randomizer for tetris pieces */
struct tetris_bag;

/* initialize a new bag */
struct tetris_bag * create_bag(unsigned int seed);
void destroy_bag(struct tetris_bag *);

enum tetrimino_type bag_peek(const struct tetris_bag *, unsigned int n);
enum tetrimino_type bag_pull(struct tetris_bag *);
