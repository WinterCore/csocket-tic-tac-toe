CC=gcc
CFLAGS=-Wall -ggdb

# SERVER

bin/server: $(addprefix bin/, main.o socket.o)
	$(CC) -o bin/server $(addprefix bin/, main.o socket.o)


bin/main.o: $(addprefix src/server/, main.c socket.o)
	$(CC) -c src/server/main.c -o bin/main.o

bin/socket.o: $(addprefix src/server/, socket.h socket.c macros.h)
	$(CC) -c src/server/socket.c -o bin/socket.o


clean:
	rm bin/*
