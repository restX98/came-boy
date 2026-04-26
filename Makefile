CC      = gcc
CFLAGS  = -Wall -Wextra -g -Isrc
LDFLAGS =

SRC     = $(wildcard src/*.c)
OBJ     = $(SRC:src/%.c=build/%.o)

.PHONY: all clean

all: build/game-boy-emulator

build/game-boy-emulator: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

build/%.o: src/%.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build
