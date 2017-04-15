all: 
	gcc -c timeServer.c
	gcc timeServer.o -o timeServer
client:
	gcc -c client.c
	gcc client.o -o seeWhat
run:
	./timeServer 5 4 fifo

clean:
	rm -f *.o timeServer seeWhat
