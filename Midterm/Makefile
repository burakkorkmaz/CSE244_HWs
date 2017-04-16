all: clean server
server:
	gcc -c timeServer.c
	gcc timeServer.o -o timeServer
client:
	gcc -c seeWhat.c
	gcc seeWhat.o -o seeWhat
run:
	./timeServer 5 4 fifo

clean:
	rm -f *.o timeServer seeWhat fifo request
