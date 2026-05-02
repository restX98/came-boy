CC      = gcc
CFLAGS  = -Wall -Wextra -g -Isrc
LDFLAGS =

SRC     = $(wildcard src/*.c)
OBJ     = $(SRC:src/%.c=build/%.o)

# --- Test config ---
UNITY_DIR    = tests/unity
HELPERS_DIR      = tests/helpers

UNIT_SRC         = $(wildcard tests/unit/test_*.c)
UNIT_BIN_DIR     = tests/bin/unit
UNIT_BINS        = $(UNIT_SRC:tests/unit/%.c=$(UNIT_BIN_DIR)/%)

INTEG_SRC        = $(wildcard tests/integration/test_*.c)
INTEG_BIN_DIR    = tests/bin/integration
INTEG_BINS       = $(INTEG_SRC:tests/integration/%.c=$(INTEG_BIN_DIR)/%)

TEST_COMMON_OBJ = build/logger.o

# old
TEST_SRC     = $(wildcard tests/test_*.c)
TEST_BIN_DIR = tests/bin
TEST_BINS 	 = $(TEST_SRC:tests/%.c=$(TEST_BIN_DIR)/%)

.PHONY: all clean test test_unit test_integration

all: build/came-boy

build/came-boy: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

build/%.o: src/%.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

# --- Run all tests ---
test: test_unit test_integration

# --- Unit tests ---
test_unit: $(UNIT_BINS)
	@printf "\n=== Unit Tests ===\n\n"
	@for t in $(UNIT_BINS); do \
		printf "Running $$t:\n"; \
		./$$t; \
		printf "\n=========================\n\n"; \
	done

# Run a single unit test
unit_%: $(UNIT_BIN_DIR)/test_%
	./$(UNIT_BIN_DIR)/test_$*

# Extract the module name from test_foo -> foo, then link only build/foo.o
define UNIT_TEST_RULE
$(UNIT_BIN_DIR)/test_$(1): tests/unit/test_$(1).c $(UNITY_DIR)/unity.c build/$(1).o $(TEST_COMMON_OBJ)
	@mkdir -p $(UNIT_BIN_DIR)
	$(CC) $(CFLAGS) -I$(UNITY_DIR) -I$(HELPERS_DIR) $$< $(UNITY_DIR)/unity.c build/$(1).o $(TEST_COMMON_OBJ) -o $$@ $(LDFLAGS)
endef

# Generate a rule for each test, extracting module name by stripping "test_" prefix
UNIT_MODULES = $(patsubst tests/unit/test_%.c,%,$(UNIT_SRC))
$(foreach mod,$(UNIT_MODULES),$(eval $(call UNIT_TEST_RULE,$(mod))))


# --- Integration tests ---
test_integration: $(INTEG_BINS)
	@printf "\n=== Integration Tests ===\n\n"
	@for t in $(INTEG_BINS); do \
		printf "Running $$t:\n"; \
		./$$t; \
		printf "\n=========================\n\n"; \
	done

# Run a single integration test
integ_%: $(INTEG_BIN_DIR)/test_%
	./$(INTEG_BIN_DIR)/test_$*

# Integration tests link ALL real src objects (minus main.o if you have one)
INTEG_SRC_OBJS = $(filter-out build/main.o, $(OBJ))

define INTEG_TEST_RULE
$(INTEG_BIN_DIR)/test_$(1): tests/integration/test_$(1).c $(UNITY_DIR)/unity.c $(INTEG_SRC_OBJS)
	@mkdir -p $(INTEG_BIN_DIR)
	$(CC) $(CFLAGS) -I$(UNITY_DIR) -I$(HELPERS_DIR) $$< $(UNITY_DIR)/unity.c $(INTEG_SRC_OBJS) -o $$@ $(LDFLAGS)
endef

INTEG_MODULES = $(patsubst tests/integration/test_%.c,%,$(INTEG_SRC))
$(foreach mod,$(INTEG_MODULES),$(eval $(call INTEG_TEST_RULE,$(mod))))


# TODO: add coverage target using gcov/lcov

clean:
	rm -rf build tests/bin
