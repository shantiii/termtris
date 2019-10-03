termtris: main.c tetrimino.c display.c grid.c bag.c event_queue.c
	gcc $^ -lncursesw -o termtris
