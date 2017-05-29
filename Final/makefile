all: final

final:
	gcc -c -Wall server.c clients.c
	gcc server.o -o server 	-lpthread
	gcc clients.o -o clients -lpthread

clean:
	rm -f *.o clients server
