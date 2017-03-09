all: clean hw2

hw2:
	gcc -c hw2.c
	gcc -ansi -pedantic -Wall hw2.o -o listdir

clean:
	rm -f *.o *.log listdir

#valgrind:
#	valgrind --leak-check=full ./listdir ece this
