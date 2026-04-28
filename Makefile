CC      = gcc
CFLAGS  = -Wall -Wextra -g -Isrc
LDFLAGS =

SRC     = $(wildcard src/*.c)
OBJ     = $(SRC:src/%.c=build/%.o)

# --- Test config ---
UNITY_DIR  = tests/unity
TEST_SRC   = $(wildcard tests/test_*.c)
TEST_OBJ   = $(filter-out build/main.o, $(OBJ))  # exclude main.o to avoid duplicate main()
TEST_BIN_DIR = tests/bin
TEST_BINS = $(TEST_SRC:tests/%.c=$(TEST_BIN_DIR)/%)

.PHONY: all clean test

all: build/game-boy-emulator

build/game-boy-emulator: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

build/%.o: src/%.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TEST_BINS)
	@printf "\n=========================\n\n"
	@for t in $(TEST_BINS); do \
		printf "Running $$t:"; \
		./$$t; \
		printf "\n=========================\n\n"; \
	done

$(TEST_BIN_DIR)/%: tests/%.c $(UNITY_DIR)/unity.c $(TEST_OBJ)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) -I$(UNITY_DIR) $< $(UNITY_DIR)/unity.c $(TEST_OBJ) -o $@ $(LDFLAGS)

# TODO: add coverage target using gcov/lcov

clean:
	rm -rf build tests/bin
