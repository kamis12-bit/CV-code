#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>
#include <assert.h>
#include "bitmap.h"


// Initialises bitmap with error (memory allocation) control
Bitmap_t initialise_bitmap(uintmax_t size, bool *is_error) {
    Bitmap_t bitmap = (Bitmap_t)malloc(sizeof(struct Bit_Map_Array));
    if (bitmap == NULL) *is_error = true;

    // The number of bitmap cells is set to ceil(size / 8)
    bitmap->size_of_array = size / 8 + ((size % 8) ? 1 : 0);
    bitmap->array = calloc(bitmap->size_of_array, sizeof(uint8_t));
    if (bitmap->array == NULL) *is_error = true;

    return bitmap;
}

// Sets the pointed-to bit to 1
//  bit_number is assumed to be indexed from 1
// The appropriate cell is ORed with a 1 in the proper bit place 
void set_bit_in_bitmap(uintmax_t bit_number, Bitmap_t bitmap) {
    assert(bit_number / 8 <= bitmap->size_of_array);
    bitmap->array[bit_number / 8] |= 1 << (bit_number % 8);
} 

void shift_bitmap_down(Bitmap_t bitmap, uintmax_t shift) {
    assert(shift < bitmap->size_of_array * 8);
    uint64_t cell_shift = shift / 8;
    uint64_t bit_shift = shift % 8;

    // The loop is indexed over the cells from which the bits are to be carried over
    //  'new_element' is the cell that is going to be copied over
    // Since 'shift' does not need to be divisible by 8, 'new_element' needs to made from two neighbouring cells,
    //  so it takes the last 'bit_shift' bits from current cell and then the rest from the next (if it exists)
    // Since bitmap has uint8_t cells, 'bitmap->array[i+1] << (8 - bit_shift)' shifts up the bits we want
    //  to where we want them, and the rest disappear (because they are then divisable by 2^8)
    // Therefore, after the shift, the order of the bits that remain is conserved 
    for (size_t i = cell_shift; i < bitmap->size_of_array; i++) {
        uint8_t new_element = bitmap->array[i] >> bit_shift;
        new_element += (i == bitmap->size_of_array - 1 ? 0 : (bitmap->array[i+1] << (8 - bit_shift)));
        bitmap->array[i - cell_shift] = new_element;
    }
    // Cleans the memory, where it is not overriten by the first loop (from the end of bitmap)
    for (size_t i = 0; i < cell_shift; i++) {
        bitmap->array[bitmap->size_of_array - i - 1] = 0;
    }
}

bool read_bit_in_bitmap(uintmax_t bit_number, Bitmap_t bitmap) {
    assert(bit_number / 8 <= bitmap->size_of_array);
    return (bitmap->array[bit_number / 8] >> (bit_number % 8)) & 1;
}

void free_bitmap(Bitmap_t bitmap) {
    free(bitmap->array);
    free(bitmap);
}