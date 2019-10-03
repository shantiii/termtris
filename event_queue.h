#pragma once

/* all of the types of event which can be handled by the game engine */
enum game_event_type {
	/* no event (pseudo-event) */
	GE_NOOP = 0,
	/* input-based events */
	GE_PAUSE,
	GE_LSHIFT,
	GE_RSHIFT,
	GE_HARDDROP,
	GE_SOFTDROP,
	GE_CWROTATE,
	GE_CCWROTATE,
	GE_QUIT,
	/* game-state based events */
	GE_NEWGAME,
	GE_CONTACT,
	GE_LOCKDOWN,
	GE_GAMEOVER,
	/* entrance */
	GE_ENTER, /* called during phase transitions */
};

struct game_event {
	/* the type of event this is */
	enum game_event_type type;
	/* the timestamp at which this event occurred or will occur */
	int64_t time;
};

/* a priority queue of events, from earliest (lowest) time to latest. */
struct event_queue;

/* allocate and create an event queue */
struct event_queue * create_eq();
/* destroy and deallocate an event queue */
void destroy_eq(struct event_queue *);

/**
 * eq_peek
 * looks for the first event in the queue and copies it to evt
 * returns FALSE if queue is NULL or empty
 * returns TRUE and populates evt otherwise
 */
bool eq_peek(const struct event_queue *queue, struct game_event *evt);

/**
 * eq_pop
 * looks for the first event in the queue and copies it to evt
 * returns FALSE if queue is NULL or empty
 * returns TRUE and populates evt otherwise
 */
bool eq_pop(struct event_queue *queue, struct game_event *event);

void eq_push(struct event_queue *queue, struct game_event event);

/**
 * eq_len
 * returns the length of the queue
 * returns -1 if queue is NULL
 */
int eq_len(const struct event_queue *queue);

/**
 * eq_clear
 * removes all items from the event queue
 */
void eq_clear(struct event_queue *queue);
