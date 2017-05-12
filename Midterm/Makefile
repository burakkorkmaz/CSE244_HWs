all: clean server client

server:
	gcc -c timeServer.c
	gcc timeServer.o -o timeServer -lm
client:
	gcc -c seeWhat.c
	gcc seeWhat.o -o seeWhat -lm

result:
	gcc -c showResults.c
	gcc showResults.o -o showResults -lm
run:
	./timeServer 5 4 fifo

clean:
	rm -f *.o timeServer seeWhat showResults fifo*
