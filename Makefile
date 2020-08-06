CC=gcc
CFLAGS=-Wall

all: bin/server

debug: CFLAGS += -g
debug: bin/server

# SERVER

bin/server: $(addprefix bin/, main.o socket.o game.o game_logic.o)
	$(CC) $(CFLAGS) -o bin/server $(addprefix bin/, main.o socket.o game.o helpers.o game_logic.o)

bin/helpers.o: $(addprefix src/server/, helpers.h helpers.c)
	$(CC) $(CFLAGS) -c src/server/helpers.c -o bin/helpers.o

bin/main.o: $(addprefix src/server/, main.c macros.h) bin/socket.o bin/helpers.o
	$(CC) $(CFLAGS) -c src/server/main.c -o bin/main.o

bin/socket.o: $(addprefix src/server/, socket.h socket.c macros.h) bin/game.o
	$(CC) $(CFLAGS) -c src/server/socket.c -o bin/socket.o

bin/game.o: $(addprefix src/server/, game.c game.h macros.h) bin/helpers.o
	$(CC) $(CFLAGS) -c src/server/game.c -o bin/game.o

bin/game_logic.o: $(addprefix src/server/, game_logic.c game_logic.h game.h)
	$(CC) $(CFLAGS) -c src/server/game_logic.c -o bin/game_logic.o

clean:
	rm -f bin/*
