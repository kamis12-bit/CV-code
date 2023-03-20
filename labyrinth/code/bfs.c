#include "bitmap.h"
#include "fifo_queue.h"
#include "setup.h"
#include "vector.h"
#include <stdio.h>
#include <stdlib.h>

#define INITIAL_QUEUE_SIZE 2

// Generates the vector of positions next to the given one, taking into account
// the walls of the labyrinth The positions are generated in the format of a
// number, according to the instructions, as it's easier to store.
Vector_t generate_neighbours(Vector_t dimensions, uintmax_t position,
                             bool *error) {
    uintmax_t move = 1;
    uintmax_t cut_down_position =
        position;                  // position divided product of dimensions
    uintmax_t length_of_dimension; // the current considered dimension

    Vector_t neighbours = initialise_vector(1, error);
    if (*error) {
        printf("ERROR 0\n");
        return neighbours;
    }

    for (size_t i = 0; i < the_size_of_array(dimensions); i++) {
        length_of_dimension = element_from_vector(dimensions, i);

        // checks if the move doesn't go beyond the boundaries of the labyrinth
        if (cut_down_position % length_of_dimension > 0)
            *error |= !give_element_to_vector(position - move, neighbours);
        if (cut_down_position % length_of_dimension < length_of_dimension - 1)
            *error |= !give_element_to_vector(position + move, neighbours);

        cut_down_position /= length_of_dimension;
        move *= length_of_dimension;
    }
    return neighbours;
}

// Filters the given moves based on whether they are possible in the given the
// layout of the labyrinth
void filter_neighbours(Bitmap_t labyrinth, Vector_t neighbours) {
    for (size_t i = 0; i < the_size_of_array(neighbours); i++) {
        uintmax_t element = element_from_vector(neighbours, i);
        bool bit = read_bit_in_bitmap(element, labyrinth);
        if (bit) {
            remove_element_from_vector(i, neighbours);
            i--; // The elements are removed online, so we need to adjust which
                 // element we're considering
        }
    }
}

void breadth_first_search(Setup input) {
    if (input->entrance == input->exit) {
        printf("0\n");
        return;
    }
    bool error_flag = false;
    bool found_flag = false;
    uintmax_t length_of_path = 1; // Contains are answer if a path exists
    Queue_t a_queue = initialise_queue(INITIAL_QUEUE_SIZE, &error_flag);
    Queue_t b_queue = initialise_queue(INITIAL_QUEUE_SIZE, &error_flag);
    if (error_flag) {
        free_queue(a_queue);
        free_queue(b_queue);
        printf("ERROR 0\n");
    }

    insert_into_queue(&a_queue, input->entrance);
    // There's no need to return to the beginning
    set_bit_in_bitmap(input->entrance, input->labyrinth);

    // This loop is the overall search
    while (!error_flag && !found_flag) {
        // Parity of the step determines which queue contains the old states vs
        // the new ones
        Queue_t active_queue = (length_of_path % 2 ? a_queue : b_queue);
        Queue_t receiving_queue = (length_of_path % 2 ? b_queue : a_queue);

        // This loop is one step in a search: expands all current states into
        // new ones
        while (!queue_is_empty(active_queue) && !error_flag && !found_flag) {
            uintmax_t position = take_from_queue(active_queue);
            Vector_t neighbours =
                generate_neighbours(input->dimensions, position, &error_flag);
            if (error_flag) { // in case memory allocation goes wrong
                free_vector(neighbours);
                fprintf(stderr, "ERROR 0\n");
                break;
            }

            filter_neighbours(input->labyrinth, neighbours);
            // This loop adds the good moves to the receiving array
            for (size_t i = 0; i < the_size_of_array(neighbours); i++) {
                uintmax_t candidate = element_from_vector(neighbours, i);
                // If we found are destination, there's no point in continuing
                // the search
                if (candidate == input->exit) {
                    printf("%" PRIuMAX "\n", length_of_path);
                    found_flag = true;
                    break;
                }
                // Since we don't want to check the places we've already been
                // to, we might as well
                //  just set their bits in the labyrinth
                set_bit_in_bitmap(candidate, input->labyrinth);
                error_flag |= !insert_into_queue(&receiving_queue, candidate);
                if (error_flag) {
                    fprintf(stderr, "ERROR 0\n");
                    break;
                }
            }
            free_vector(neighbours);
        }
        // If the queues were reallocated, their address changed, so the
        // pointers must be reset properly
        a_queue = (length_of_path % 2 ? active_queue : receiving_queue);
        b_queue = (length_of_path % 2 ? receiving_queue : active_queue);

        if (error_flag || found_flag)
            break;

        if (queue_is_empty(receiving_queue)) {
            printf("NO WAY\n");
            found_flag = true;
            break;
        }
        length_of_path++;
    }
    free_queue(a_queue);
    free_queue(b_queue);
}
