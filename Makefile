CC      = gcc
CFLAGS  = -Wall -Wextra -g -Isrc
LDFLAGS =

SRC     = $(wildcard src/*.c)
OBJ     = $(SRC:src/%.c=build/%.o)

# --- Test config ---
UNITY_DIR    = tests/unity
TEST_SRC     = $(wildcard tests/test_*.c)
TEST_BIN_DIR = tests/bin
TEST_BINS 	 = $(TEST_SRC:tests/%.c=$(TEST_BIN_DIR)/%)
TEST_COMMON_OBJ = build/logger.o

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
		printf "Running $$t: \n"; \
		./$$t; \
		printf "\n=========================\n\n"; \
	done

test_%: $(TEST_BIN_DIR)/test_%
	./$(TEST_BIN_DIR)/$@

# Extract the module name from test_foo -> foo, then link only build/foo.o
define TEST_RULE
$(TEST_BIN_DIR)/test_$(1): tests/test_$(1).c $(UNITY_DIR)/unity.c build/$(1).o $(TEST_COMMON_OBJ)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) -I$(UNITY_DIR) $$< $(UNITY_DIR)/unity.c build/$(1).o $(TEST_COMMON_OBJ) -o $$@ $(LDFLAGS)
endef

# Generate a rule for each test, extracting module name by stripping "test_" prefix
TEST_MODULES = $(patsubst tests/test_%.c,%,$(TEST_SRC))
$(foreach mod,$(TEST_MODULES),$(eval $(call TEST_RULE,$(mod))))

# TODO: add coverage target using gcov/lcov

clean:
	rm -rf build tests/bin
