CC=gcc
CFLAGS=-Wall

all: server client

debug: CFLAGS += -g
debug: bin/server

server: bin/ttt-server
client: bin/ttt-client


bin/helpers.o: $(addprefix src/, helpers.h helpers.c)
	$(CC) $(CFLAGS) -c src/helpers.c -o bin/helpers.o

# SERVER

bin/ttt-server: $(addprefix bin/server/, main.o socket.o game.o game_logic.o)
	$(CC) $(CFLAGS) -o bin/ttt-server $(addprefix bin/server/, main.o socket.o game.o game_logic.o) bin/helpers.o

bin/server/main.o: $(addprefix src/server/, main.c macros.h) bin/server/socket.o bin/helpers.o
	$(CC) $(CFLAGS) -c src/server/main.c -o bin/server/main.o

bin/server/socket.o: $(addprefix src/server/, socket.h socket.c macros.h) bin/server/game.o src/helpers.h
	$(CC) $(CFLAGS) -c src/server/socket.c -o bin/server/socket.o

bin/server/game.o: $(addprefix src/server/, game.c game.h macros.h) bin/helpers.o
	$(CC) $(CFLAGS) -c src/server/game.c -o bin/server/game.o

bin/server/game_logic.o: $(addprefix src/server/, game_logic.c game_logic.h game.h)
	$(CC) $(CFLAGS) -c src/server/game_logic.c -o bin/server/game_logic.o

# CLIENT

bin/ttt-client: $(addprefix bin/client/, main.o)
	$(CC) $(CFLAGS) -lcurses -o bin/ttt-client $(addprefix bin/client/, main.o socket.o game.o) bin/helpers.o

bin/client/main.o: $(addprefix src/client/, main.c) bin/helpers.o bin/client/game.o bin/client/socket.o
	$(CC) $(CFLAGS) -c src/client/main.c -o $(addprefix bin/client/, main.o)

bin/client/socket.o: $(addprefix src/client/, socket.h socket.c)
	$(CC) $(CFLAGS) -c src/client/socket.c -o bin/client/socket.o

bin/client/game.o: $(addprefix src/client/, game.h game.c macros.h) bin/helpers.o
	$(CC) $(CFLAGS) -c src/client/game.c -o bin/client/game.o

clean:
	rm -f bin/server/*
	rm -f bin/client/*
	rm -f helpers.o ttt-server ttt-client
