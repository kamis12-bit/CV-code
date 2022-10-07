#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "fifo_queue.h"


// Simple realloc(queue->array, 2*queue->size) doesn't work, because if the head is earlier then the tail,
//  the memory which is added falls inside the queue,
// The solution is to make a new queue of twice the size, move all elements from old to new queue, and 
//  free the old one.
Queue_t reallocate_queue(Queue_t queue, bool *is_error) {
    Queue_t new_queue = initialise_queue(2 * queue->size_of_queue, is_error);
    if (*is_error) {
        free_queue(queue);
        return new_queue;
    }
    while (!queue_is_empty(queue)) {
        insert_into_queue(&new_queue, take_from_queue(queue));
    }
    free_queue(queue);
    return new_queue;


}

// is_error is set to true if allocation fails
Queue_t initialise_queue(size_t size, bool *is_error) {
    assert(size > 0);
    Queue_t queue = (Queue_t)malloc(sizeof(struct Fifo_queue));
    if (queue == NULL) {
        *is_error = true;
        return queue;
    }
    queue->size_of_queue = size;
    queue->head = queue->tail = 0;
    queue->array = (uintmax_t*)malloc(size * sizeof(uintmax_t));
    if (queue->array == NULL) *is_error = true;
    return queue;
}


uintmax_t take_from_queue(Queue_t queue) {
    assert(!queue_is_empty(queue));
    size_t handle = queue->tail;
    // Makes sure index doesn't go beyond the size of array
    queue->tail = (queue->tail + 1) % queue->size_of_queue;
    return queue->array[handle];
}

// Uses *pointer_to_queue in case the queue gets reallocated - because the adress changes, it would
//  otherwise have been lost to whoever uses the function and memory which they would hold would have 
//  been freed. With the pointer, it's simply set to new memory, while the old one was freed.
// Function returns true, if everything goes well; false if there was a problem with memory allocation.
bool insert_into_queue(Queue_t *pointer_to_queue, uintmax_t element) {
    bool error = false;
    Queue_t queue = *pointer_to_queue;
    if (queue_is_full(queue)) queue = reallocate_queue(queue, &error);
    *pointer_to_queue = queue;
    if (error) return false;
    queue->array[queue->head] = element;
    // Makes sure index doesn't go beyond the size of array
    queue->head = (queue->head + 1) % queue->size_of_queue;
    return true;
}

