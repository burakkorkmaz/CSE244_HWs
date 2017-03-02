all: clean hw1

hw1:
	gcc -c hw1.c
	gcc -ansi -pedantic -Wall hw1.o -o list

clean:
	rm -f *.o list

valgrind:
	valgrind --leak-check=full ./list ece test.txt
