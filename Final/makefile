all: final

final:
	gcc -c server.c clients.c
	gcc server.o -o server 	-lpthread -lm
	gcc clients.o -o clients -lpthread -lm

clean:
	rm -f *.o clients server
