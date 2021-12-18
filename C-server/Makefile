CFLAGS = -c -Wall
CC = gcc
LIBS =  -lm 

all: pjC

pjC: server.o
	${CC} -pthread server.o -o pjC -I/usr/include/postgresql -lpq -std=c99

# pjC: sql.o server.o
# 	${CC} server.o sql.o -o pjC

server.o: server.c
	${CC} -pthread ${CFLAGS} server.c -I/usr/include/postgresql -lpq -std=c99

# sql.o: sql.c
# 	${CC} ${CFLAGS} sql.c -I/usr/include/postgresql -lpq -std=c99


clean:
	rm -f *.o *~