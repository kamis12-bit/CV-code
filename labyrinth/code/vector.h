#ifndef VECTOR_H
#define VECTOR_H
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

// A structure which takes care of allocating it's own memory
typedef struct Dynamic_Array {
  size_t capacity_of_array; // allocated memory
  size_t size_of_array;     // number of elements in array
  uintmax_t *array;         // void*
} * Vector_t;

extern Vector_t initialise_vector(size_t size, bool *is_error);

extern bool give_element_to_vector(uintmax_t element, Vector_t vector);
extern void remove_element_from_vector(size_t index, Vector_t vector);

static inline uintmax_t element_from_vector(Vector_t vector, size_t i) {
  return vector->array[i];
}
static inline size_t the_size_of_array(Vector_t vector) {
  return vector->size_of_array;
}

static inline void free_vector(Vector_t vector) {
  free(vector->array);
  free(vector);
}

#endif