#.PHONY all inverse_permutation_example clean	

all: inverse_permutation_example
	
inverse_permutation_example: inverse_permutation_example.o inverse_permutation.o
	gcc -z noexecstack -o inverse_permutation_example inverse_permutation_example.o inverse_permutation.o

inverse_permutation.o: inverse_permutation.asm
	nasm -f elf64 -w+all -w+error -o inverse_permutation.o inverse_permutation.asm
#-g -F dwarf is for debug

inverse_permutation_example.o: inverse_permutation_example.c
	gcc -c -Wall -Wextra -std=c17 -O2 -o inverse_permutation_example.o inverse_permutation_example.c
#-g is for debug +O2

clean:
	rm *.o
