#include <stdio.h>
#include <stdlib.h>
#include "vector.h"
#include <inttypes.h>
#include <assert.h>




// is_error turns on if there was some problem with allocation
Vector_t initialise_vector(size_t size, bool *is_error) {
    Vector_t array = (Vector_t)malloc(sizeof(struct Dynamic_Array));
    if (array == NULL) *is_error = true;

    array->size_of_array = 0;
    array->capacity_of_array = (size > 0 ? size : 1);

    array->array = malloc(size * sizeof(uint64_t));
    if (array->array == NULL) *is_error = true;
    return array;
}


// Gives the vector an element, first checking if there is enough place
// - if not, doubles available memory, returns false if reallocation failed, true otherwise
bool give_element_to_vector(uintmax_t element, Vector_t vector) {
    if (vector->size_of_array == vector->capacity_of_array) {
        void * new_memory = realloc((vector->array), 2 * vector->capacity_of_array * sizeof(uintmax_t));
        if (new_memory == NULL) {
            return false;
        } 
        else {
            vector->array = (uintmax_t*)new_memory;
        }
        vector->capacity_of_array = 2 * vector->capacity_of_array;
    }

    vector->array[vector->size_of_array] = element;
    vector->size_of_array++;
    return true;
}

// Helps with removal of elements - instead of adding another element, replaces one at 'i'th place
void insert_element_into_vector(uintmax_t element, Vector_t vector, size_t i) {
    assert(i <= vector->size_of_array);
    vector->array[i] = element;
}

// Removes element by copying all numbers back a step
void remove_element_from_vector(size_t index, Vector_t vector) {
    assert(index < the_size_of_array(vector));
    for (size_t i = index + 1; i < the_size_of_array(vector); i++) 
        insert_element_into_vector(element_from_vector(vector, i), vector, i - 1);
    vector->size_of_array--;
}

