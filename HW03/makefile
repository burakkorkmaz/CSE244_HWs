all: clean hw3

hw3:
	gcc -c hw3.c
	gcc -ansi -pedantic -Wall hw3.o -o withPipeandFIFO

clean:
	clear
	rm -f *.o *.log withPipeandFIFO

valgrind:
	valgrind --leak-check=full ./withPipeandFIFO oto this
