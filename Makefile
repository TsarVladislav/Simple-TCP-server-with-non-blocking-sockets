CC = gcc
CFLAGS = -Wall -pedantic -std=c89

all: server.o
	${CC} server.o -o server
server.o: server.c
	${CC} ${CFLAGS} -c server.c
clean:
	rm -f *.o server
