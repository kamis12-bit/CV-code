#ifndef BITMAP_H
#define BITMAP_H

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// Implementation of a bitmap
typedef struct Bit_Map_Array {
    size_t size_of_array; // number of 8-bit cells in bitmap array
    uint8_t *array;       // bits are encoded in uint8_t numbers //void*
} *Bitmap_t;

extern Bitmap_t initialise_bitmap(uintmax_t size, bool *is_error);
extern void set_bit_in_bitmap(uintmax_t bit_number, Bitmap_t bitmap);
extern void shift_bitmap_down(Bitmap_t bitmap, uintmax_t shift);
extern bool read_bit_in_bitmap(uintmax_t bit_number, Bitmap_t bitmap);
extern void free_bitmap(Bitmap_t bitmap);
static inline uintmax_t size_of_bitmap(Bitmap_t bitmap) {
    return bitmap->size_of_array;
};
#endif
