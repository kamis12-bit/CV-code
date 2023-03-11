#include "input.h"
#include "setup.h"
#include "bfs.h"
 

int main() {
    Setup labyrinth = load_input();
    breadth_first_search(labyrinth);
    free_setup(labyrinth);
    return 0;
}