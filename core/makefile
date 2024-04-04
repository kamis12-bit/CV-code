.PHONY: core_example clean
example_file := example.c

core_example: core_example.o core.o
	@gcc -z noexecstack -lpthread -o core_example core_example.o core.o

core.o: core.asm
	@nasm -DN=2 -f elf64 -g -F dwarf -w+all -w+error -o core.o core.asm
#-g -F dwarf is for debug

core_example.o: $(example_file)
	@gcc -c -Wall -Wextra -std=c17 -g -o core_example.o $(example_file)
#-g is for debug +O2

clean:
	@rm *.o
