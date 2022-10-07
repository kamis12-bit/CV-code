CFLAGS=-Wall -Wextra -Wno-implicit-fallthrough -std=c17 -O2
CC=gcc
LDFLAGS=

.PHONY: all clean

labyrinth: labyrinth.o fifo_queue.o vector.o bfs.o input.o bitmap.o

labyrinth.o: labyrinth.c
bfs.o: bfs.c
fifo_queue.o: fifo_queue.c
vector.o: vector.c
input.o: input.c 
bitmap.o: bitmap.c

clean:
	@echo "Deleting compilation files"
	rm *.o
	rm labyrinth