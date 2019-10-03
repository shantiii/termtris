#include <stdlib.h>
#include <stdbool.h>

#include "event_queue.h"

struct event_queue {
	struct event_queue_node *head;
};

struct event_queue_node {
	struct event_queue_node *next;
	struct game_event event;
};

/* allocate and create an event queue */
struct event_queue * create_eq() {
	struct event_queue *queue = (struct event_queue *) malloc(sizeof(struct event_queue));
	queue->head = NULL;
	return queue;
}

/* destroy and deallocate an event queue */
void destroy_eq(struct event_queue *queue) {
	if (queue == NULL) { return; }
	struct event_queue_node *node = queue->head;
	while(node != NULL) {
		node = node->next;
		free(queue->head);
		queue->head = node;
	}
	free(queue);
}

/**
 * eq_peek
 * looks for the first event in the queue and copies it to evt
 * returns false if queue is NULL or empty
 * returns true and populates evt otherwise
 */
bool eq_peek(const struct event_queue *queue, struct game_event *evt) {
	if (queue == NULL || queue->head == NULL) {
		return false;
	}
	*evt = queue->head->event;
	return true;
}

/**
 * eq_pop
 * looks for the first event in the queue and copies it to evt
 * returns false if queue is NULL or empty
 * returns true and populates evt otherwise
 */
bool eq_pop(struct event_queue *queue, struct game_event *event) {
	if (queue == NULL || queue->head == NULL) {
		return false;
	}
	struct event_queue_node *new_head = queue->head->next;
	*event = queue->head->event;
	free(queue->head);
	queue->head = new_head;
	return true;
}

void eq_push(struct event_queue *queue, struct game_event event) {
	struct event_queue_node *node = malloc(sizeof(struct event_queue_node));
	node->event = event;
	if (queue->head == NULL) {
		queue->head = node;
		node->next = NULL;
	} else if (event.time < queue->head->event.time) {
		node->next = queue->head;
		queue->head = node;
	} else {
		struct event_queue_node *curr = queue->head->next;
		struct event_queue_node *prev = queue->head;
		while (curr != NULL && node->event.time >= curr->event.time) {
			prev = curr;
			curr = curr->next;
		};
		prev->next = node;
		node->next = curr;
	}
}

/* return the length of the event queue, or -1 if the queue is invalid */
int eq_len(const struct event_queue *queue) {
	if (queue == NULL) {
		return -1;
	}
	int size = 0;
	struct event_queue_node *ptr;
	ptr = queue->head;
	while(ptr != NULL) {
		ptr = ptr->next;
		++size;
	}
	return size;
}

/* return the length of the event queue, or -1 if the queue is invalid */
void eq_clear(struct event_queue *queue) {
	if (queue == NULL) { return; }
	struct event_queue_node *node = queue->head;
	while ((node = queue->head) != NULL) {
		node = node->next;
		free(queue->head);
		queue->head = node;
	}
	queue->head = NULL;
}

