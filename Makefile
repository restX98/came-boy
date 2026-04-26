CC      = gcc
CFLAGS  = -Wall -Wextra -g

.PHONY: all clean

all: build/game-boy-emulator

build/game-boy-emulator: src/main.c
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -rf build
