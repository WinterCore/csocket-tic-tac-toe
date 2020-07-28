CC=gcc
CFLAGS=-Wall -ggdb

# SERVER

bin/server: $(addprefix bin/, main.o socket.o game.o)
	$(CC) -o bin/server $(addprefix bin/, main.o socket.o game.o helpers.o)

bin/helpers.o: $(addprefix src/server/, helpers.h helpers.c)
	$(CC) -c src/server/helpers.c -o bin/helpers.o

bin/main.o: $(addprefix src/server/, main.c) bin/socket.o
	$(CC) -c src/server/main.c -o bin/main.o

bin/socket.o: $(addprefix src/server/, socket.h socket.c macros.h) bin/game.o
	$(CC) -c src/server/socket.c -o bin/socket.o

bin/game.o: $(addprefix src/server/, game.c game.h macros.h) bin/helpers.o
	$(CC) -c src/server/game.c -o bin/game.o


clean:
	rm bin/*
