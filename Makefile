CC = gcc
CFLAGS = -Wall -Wextra -pedantic -O2 -std=c99


all: bfic.out

debug: debug_bfic.out

bfic.out: bfic.c build/interpreter.o
	$(CC) $(CFLAGS) -DNDEBUG -o  $@ $^

debug_bfic.out: bfic.c build/debug_interpreter.o
	$(CC) $(CFLAGS) -g -o $@ $^

build/interpreter.o: src/interpreter.c
	$(CC) $(CFLAGS) -DNDEBUG -c $< -o $@

build/debug_interpreter.o: src/interpreter.c
	$(CC) $(CFLAGS) -g -c $< -o $@