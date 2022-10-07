#ifndef SETUP_H
#define SETUP_H

#include "vector.h"
#include "bitmap.h"


// A helpful structure that contains the information gotten from input

typedef struct struct_setup{
    Vector_t dimensions;
    uintmax_t entrance;
    uintmax_t exit;
    Bitmap_t labyrinth;
    uintmax_t size_of_labyrinth; // # of bits
}* Setup;

//extern void print_setup(Setup setup);
static inline void free_setup(Setup setup) {
    free_vector(setup->dimensions);
    free_bitmap(setup->labyrinth);
    free(setup);
}

#endif