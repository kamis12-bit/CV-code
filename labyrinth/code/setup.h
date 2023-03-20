#ifndef SETUP_H
#define SETUP_H

#include "bitmap.h"
#include "vector.h"

// A helpful structure that contains the information gotten from input
// Positions are described by single numbers. To decode, write it in varied
// radix: i-th digit is written in radix which is the i-th dimension of the
// labyrinth. and that digit is the position along that dimension.
//
// Labyrinth bitmap encodes positions where the proper cell is filled
// (a wall)

typedef struct struct_setup {
    Vector_t dimensions;
    uintmax_t entrance;
    uintmax_t exit;
    Bitmap_t labyrinth;
    uintmax_t size_of_labyrinth; // # of bits or cells in the labyrinth
} *Setup;

// extern void print_setup(Setup setup);
static inline void free_setup(Setup setup) {
    free_vector(setup->dimensions);
    free_bitmap(setup->labyrinth);
    free(setup);
}

#endif
