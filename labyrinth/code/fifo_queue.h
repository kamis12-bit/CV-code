#ifndef FIFO_QUEUE_H
#define FIFO_QUEUE_H

#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>

// Cyclical implementation which uses an array and two distinguishable indices
// which
//  identify which portion(s) of array form the queue
typedef struct Fifo_queue {
    size_t head;
    size_t tail;
    size_t size_of_queue;
    uintmax_t *array;
} *Queue_t;

static inline bool queue_is_empty(Queue_t queue) {
    return queue->head == queue->tail;
}
static inline bool queue_is_full(Queue_t queue) {
    return (queue->head + 1) % queue->size_of_queue == queue->tail;
}

extern Queue_t initialise_queue(size_t size, bool *is_error);

extern uintmax_t take_from_queue(Queue_t queue);
extern bool insert_into_queue(Queue_t *queue, uintmax_t element);

static inline void free_queue(Queue_t queue) {
    free(queue->array);
    free(queue);
}

#endif
