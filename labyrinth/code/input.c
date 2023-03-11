#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <assert.h>
#include <inttypes.h> 
#include "vector.h"
#include "bitmap.h"
#include "setup.h"


#define TWO32 4294967296

uintmax_t add_digit_to_number(int digit, uintmax_t number, bool *error) {
    uintmax_t new_number;
    // check if there occurred an overflow in the process - if so, sets the *error flag
    *error |= __builtin_mul_overflow(number, 10, &new_number);
    *error |= __builtin_add_overflow(new_number, digit, &new_number);
    return new_number;
}

// Assuming 4-bit digit
int calculate_leading_zeroes(int digit) {
    if (digit == '0') return 4;
    else if (digit == '1') return 3;
    else if (digit < '4') return 2;
    else if (digit < '8') return 1;
    else return 0;
}

bool decimal(int digit) {
    return '0' <= digit && digit <= '9';
}
bool big_hexadecimal(int digit) {
    return 'A' <= digit && digit <= 'F';
}
bool small_hexadecimal(int digit) {
    return 'a' <= digit && digit <= 'f';
}

bool significant_hex(int digit) {
    return decimal(digit) || big_hexadecimal(digit) || small_hexadecimal(digit);
}

bool end_of_file() {
    int digit = getchar();
    if (digit == EOF) return true;
    else return false;
}

int hex_into_number(int digit) {
    assert(significant_hex(digit));
    if (decimal(digit)) return digit - '0';
    else if (big_hexadecimal(digit)) return digit - 'A' + 10;
    else if (small_hexadecimal(digit)) return digit - 'a' + 10;
    else return 0; //never called
}

void report_error(int line) {
    // The 'reported' static flag makes sure only one error is reported
    static bool reported = false;
    if (!reported) fprintf(stderr, "ERROR %d\n", line);
    reported = true;
}

// Returns true if limits exist && (sizes don't match up || at some index 'i' vector[i] is outside interval [1, limits[i]])
bool vector_is_outside_limits (Vector_t vector, Vector_t limits) {
    size_t limit_dimension = the_size_of_array(limits);
    if (limit_dimension == 0) return false; // 
    if (the_size_of_array(vector) != limit_dimension) return true;
    for (size_t i = 0; i < limit_dimension; i++)
        if (element_from_vector(vector, i) < 1 || 
            element_from_vector(limits, i) < element_from_vector(vector, i))
                return true;
    return false;
}


void ignore_leading_zeroes(int *digit) {
    do *digit = getchar(); while (*digit == '0');
}


// For loading first three lines
void load_numbers_line(Vector_t vector, Vector_t limits, bool *is_error) {
    // Keeps track of which line it's loading
    static int line = 0;
    line++;

    int digit = 0;
    uintmax_t number = 0;

    // Until the end of the line checks every individual digit and takes appriopriate actions
    do {
        digit = getchar();
        if (decimal(digit)) {  // actual digit
            number = add_digit_to_number(digit - '0', number, is_error);
        }
        else if (isspace(digit)) { // white space
            if (number > 0) {
                if (!give_element_to_vector(number, vector)) {
                    report_error(0);
                    return;
                };
                number = 0;
            }            
        }
        else { // anything else
            *is_error = true;
        }
    } while (!(digit == '\n'));

    if (vector_is_outside_limits(vector, limits)) *is_error = true;
    if (!the_size_of_array(vector)) *is_error = true;   
    if (*is_error) report_error(line);
}



// For loading the fourth line in the R version
void load_R(uint32_t *setup_table, bool *is_error) {
    int digit = 0;
    uint32_t number = 0;
    size_t i = 0;
    bool active = false;
    do {
        digit = getchar();
        if (decimal(digit)) {  // actual digit
            number = (uint32_t)add_digit_to_number(digit - '0', (uintmax_t) number, is_error);
            active = true;
        } 
        else if (isspace(digit) || digit == EOF) { // white-space
            if (active) {
                setup_table[i] = number;
                i++;
                number = 0;
                active = false;
            }            
        }
        else { // anything else
            *is_error = true;
        }
    } while (!(digit == EOF || digit == '\n'));
    if (i != 5) *is_error = true;
    if (*is_error) {
        report_error(4);
    }
    // If there was a non-empty fifth line we want ERROR 5 not ERROR 4,
    else if (digit == '\n' && !end_of_file()) {
        report_error(5);
        *is_error = true;
    }
}

// Function which loads the R version of the fourth line and generates the labyrinth based on it
// Returns true if everything goes well, false if not
bool load_and_generate_modulo(Bitmap_t labyrinth, uintmax_t size_of_labyrinth) {
    uint32_t *r_line = (unsigned int*)malloc(5 * sizeof(uint32_t));
    bool error = false;

    load_R(r_line, &error);

    if (error) {
        free(r_line);
        return false;
    }

    uint32_t a = r_line[0];
    uint32_t b = r_line[1];
    uint32_t m = r_line[2];
    uint32_t r = r_line[3];
    uintmax_t s = r_line[4];
    uintmax_t w;

    free(r_line);

    for (size_t i = 0; i < r; i++) {
        // generates s_{i+1} and w_{i+1}
        s = (a * s + b) % m;
        w = s % size_of_labyrinth;
        // if w_{i+1} >= TWO32 it doesn't play any role in the labyrinth
        if (w >= TWO32) continue;
        while (w < size_of_labyrinth) {
            set_bit_in_bitmap(w, labyrinth);
            w += TWO32;
        }
    }
    return true;
}

uintmax_t load_hex(Bitmap_t labyrinth, bool *is_error) {
    int digit = 0;
    bool active = true; // Flag that says "number not ended"
    uintmax_t bit_count = 0;
    // Since we don't know how many bits we're going to receive, we don't know where to put the ones we're receiving. 
    // Therefore we'll add them from the end of available memory, and shift them down in the 'load_hexadecimal_number' function
    uintmax_t const maxbit = 8 * size_of_bitmap(labyrinth) - 1; // The index of the last bit
    
    ignore_leading_zeroes(&digit);
    // First digit needs to be considered separatly since there might be bit leading zeroes
    if (significant_hex(digit)) {
        bit_count = 4 - calculate_leading_zeroes(digit);

        int digit2 = hex_into_number(digit);
        digit2 <<= 4 - bit_count;

        for (size_t i = 0; i < bit_count; i++) {
            // (x << i) & 1 tells us if x has its i'th bit set 
            if (digit2 & 8) set_bit_in_bitmap(maxbit - i, labyrinth);
            digit2 <<= 1;
        }
    }
    else {
        active = false;
    }

    while (digit != '\n' && digit != EOF) {
        digit = getchar();
        if (active && significant_hex(digit)) { // hexadecimal digit
            digit = hex_into_number(digit);
            for (size_t i = 0; i < 4; i++) {
                if (digit & 8) set_bit_in_bitmap(maxbit - bit_count, labyrinth);
                bit_count++;
                digit <<= 1;
            }
        }
        else if (isspace(digit)) { // white space
            active = false;
        }
        else if (digit == EOF) { // no action necessary, but we don't want to trigger *is_error flag
        }
        else {
            *is_error = true;
            report_error(4);
        }
    }
    if (digit == '\n' && !end_of_file()) {
        *is_error = true;
        report_error(5);
    }
    return bit_count;
}


// Returns false if something went wrong (e.g. there were too many bits)
bool load_hexadecimal_number(Bitmap_t labyrinth, uintmax_t size_of_labyrinth) {
    bool error = false;
    uintmax_t bit_count = load_hex(labyrinth, &error);

    if (error || bit_count > size_of_labyrinth) return false;
    if (bit_count == 0) return true;
    // Shifts the labyrinth bitmap down, so that the indices are correct
    shift_bitmap_down(labyrinth, size_of_bitmap(labyrinth) * 8 - bit_count);
    return true;
}

// Returns the labyrinth
Bitmap_t load_walls_line(bool * is_error, uintmax_t size_of_labyrinth) {
    Bitmap_t labyrinth = initialise_bitmap(size_of_labyrinth, is_error);
    if (*is_error) return labyrinth;
    int digit = ' ';

    do digit = getchar();
    while (isspace(digit) && digit != '\n');

    if (digit == '0') {
        digit = getchar();
        if (digit == 'x') *is_error |= !load_hexadecimal_number(labyrinth, size_of_labyrinth);
        else *is_error = true;
    }
    else if (digit == 'R') {
        *is_error |= !load_and_generate_modulo(labyrinth, size_of_labyrinth);
    }
    else {
        *is_error = true;
    }
    if (*is_error) report_error(4);
    
    return labyrinth;
}

 


// Based on coordinates, calculates the position as a number
uintmax_t calculate_position(Vector_t dimensions, Vector_t position) {
    uintmax_t accumulator = 0;
    uintmax_t multiplier = 1;
    for (size_t i = 0; i < the_size_of_array(dimensions); i++) {
        accumulator += (element_from_vector(position, i) - 1) * multiplier;
        multiplier *= element_from_vector(dimensions, i);
    }
    return accumulator; 
} 

static inline void emergency_exit(Vector_t v1, Vector_t v2, Vector_t v3, Setup s) {
    free_vector(v1);
    free_vector(v2);
    free_vector(v3);
    free(s);
    exit(1);
}

Setup load_input() {
    Setup input = (Setup)malloc(sizeof(struct struct_setup));
    bool error = false;

    input->dimensions = initialise_vector(1, &error);
    Vector_t second_line = initialise_vector(1, &error);
    Vector_t third_line = initialise_vector(1, &error);

    if (error) {
        report_error(0);
        emergency_exit(input->dimensions, second_line, third_line, input);
    }
    
    load_numbers_line(input->dimensions, input->dimensions, &error);
    load_numbers_line(second_line, input->dimensions, &error);
    load_numbers_line(third_line, input->dimensions, &error);

    // Here, if error occures, it has already been reported
    if (error) emergency_exit(input->dimensions, second_line, third_line, input);


    input->size_of_labyrinth = 1;

    // Calculates the size of the labyrinth, taking into account possible overflow
    for (size_t i = 0; i < the_size_of_array(input->dimensions); i++) {
        if (__builtin_mul_overflow(input->size_of_labyrinth, 
                                   element_from_vector(input->dimensions, i), 
                                   &(input->size_of_labyrinth))) {
            input->size_of_labyrinth = SIZE_MAX;
            break;
        }
    }

    input->labyrinth = load_walls_line(&error, input->size_of_labyrinth);
    if (error) {
        report_error(0);
        free_bitmap(input->labyrinth);
        emergency_exit(input->dimensions, second_line, third_line, input);
    }

    input->entrance = calculate_position(input->dimensions, second_line);
    input->exit = calculate_position(input->dimensions, third_line);

    // Check if the entrance & exit positions aren't inside any walls
    if (read_bit_in_bitmap(input->entrance, input->labyrinth)) {
        report_error(2);
        free_bitmap(input->labyrinth);
        emergency_exit(input->dimensions, second_line, third_line, input);
    }
    if (read_bit_in_bitmap(input->exit, input->labyrinth)) {
        report_error(3);
        free_bitmap(input->labyrinth);
        emergency_exit(input->dimensions, second_line, third_line, input);
    }
    free_vector(second_line);
    free_vector(third_line);

    return input;
}

