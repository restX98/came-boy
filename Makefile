CC      = gcc
CFLAGS  = -Wall -Wextra -g -Isrc
LDFLAGS =

SRC     = $(shell find src -name '*.c')
OBJ     = $(SRC:src/%.c=build/%.o)

# --- Test config ---
UNITY_DIR        = tests/unity
HELPERS_DIR      = tests/helpers

# Tests mirror the src/ layout: tests/unit/io/test_lcd.c covers src/io/lcd.c.
UNIT_SRC         = $(shell find tests/unit -name 'test_*.c')
UNIT_BIN_DIR     = tests/bin/unit
UNIT_BINS        = $(UNIT_SRC:tests/unit/%.c=$(UNIT_BIN_DIR)/%)
UNIT_STEMS       = $(UNIT_SRC:tests/unit/%.c=%)

INTEG_SRC        = $(shell find tests/integration -name 'test_*.c')
INTEG_BIN_DIR    = tests/bin/integration
INTEG_BINS       = $(INTEG_SRC:tests/integration/%.c=$(INTEG_BIN_DIR)/%)
INTEG_STEMS      = $(INTEG_SRC:tests/integration/%.c=%)

TEST_COMMON_OBJ  = build/logger.o

# Map a unit-test stem (e.g. io/test_lcd or test_bus) to the module object it
# links against (build/io/lcd.o or build/bus.o): keep the sub-directory, drop the
# leading ./ of root-level stems, and strip the test_ prefix from the basename.
unit_mod_obj = build/$(subst ./,,$(dir $(1)))$(patsubst test_%,%,$(notdir $(1))).o
unit_cov_obj = $(COV_DIR)/$(subst ./,,$(dir $(1)))$(patsubst test_%,%,$(notdir $(1))).o

# --- Coverage config ---
COV_DIR          = build/coverage
COV_BIN_UNIT     = tests/bin/coverage/unit
COV_BIN_INTEG    = tests/bin/coverage/integration
COV_REPORT_DIR   = tests/coverage_report
COV_FLAGS        = -fprofile-arcs -ftest-coverage
COV_CFLAGS       = $(CFLAGS) $(COV_FLAGS)

# Derive coverage objects directly from SRC, excluding main.c
COV_SRC          = $(filter-out src/main.c, $(SRC))
COV_SRC_OBJS     = $(COV_SRC:src/%.c=$(COV_DIR)/%.o)
COV_COMMON_OBJ   = $(COV_DIR)/logger.o

.PHONY: all clean test test_unit test_integration coverage coverage_unit

all: build/came-boy

build/came-boy: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

build/%.o: src/%.c
	@mkdir -p $(@D)
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

unit_%:
	@target="$(filter %/test_$*,$(UNIT_BINS))"; \
	if [ -z "$$target" ]; then echo "No unit test matching '$*'"; exit 1; fi; \
	$(MAKE) $$target && ./$$target

define UNIT_TEST_RULE
$(UNIT_BIN_DIR)/$(1): tests/unit/$(1).c $(UNITY_DIR)/unity.c $(call unit_mod_obj,$(1)) $(TEST_COMMON_OBJ)
	@mkdir -p $$(@D)
	$(CC) $(CFLAGS) -I$(UNITY_DIR) -I$(HELPERS_DIR) $$< $(UNITY_DIR)/unity.c $(call unit_mod_obj,$(1)) $(TEST_COMMON_OBJ) -o $$@ $(LDFLAGS)
endef

$(foreach stem,$(UNIT_STEMS),$(eval $(call UNIT_TEST_RULE,$(stem))))

# --- Integration tests ---
test_integration: $(INTEG_BINS)
	@printf "\n=== Integration Tests ===\n\n"
	@for t in $(INTEG_BINS); do \
		printf "Running $$t:\n"; \
		./$$t; \
		printf "\n=========================\n\n"; \
	done

integ_%:
	@target="$(filter %/test_$*,$(INTEG_BINS))"; \
	if [ -z "$$target" ]; then echo "No integration test matching '$*'"; exit 1; fi; \
	$(MAKE) $$target && ./$$target

INTEG_SRC_OBJS = $(filter-out build/main.o, $(OBJ))

define INTEG_TEST_RULE
$(INTEG_BIN_DIR)/$(1): tests/integration/$(1).c $(UNITY_DIR)/unity.c $(INTEG_SRC_OBJS)
	@mkdir -p $$(@D)
	$(CC) $(CFLAGS) -I$(UNITY_DIR) -I$(HELPERS_DIR) $$< $(UNITY_DIR)/unity.c $(INTEG_SRC_OBJS) -o $$@ $(LDFLAGS)
endef

$(foreach stem,$(INTEG_STEMS),$(eval $(call INTEG_TEST_RULE,$(stem))))

# --- Coverage ---

# Pattern rule for instrumented object files — must be a standalone rule, not inside a define
$(COV_DIR)/%.o: src/%.c
	@mkdir -p $(@D)
	$(CC) $(COV_CFLAGS) -c $< -o $@

define UNIT_COV_RULE
$(COV_BIN_UNIT)/$(1): tests/unit/$(1).c $(UNITY_DIR)/unity.c $(call unit_cov_obj,$(1)) $(COV_COMMON_OBJ)
	@mkdir -p $$(@D)
	$(CC) $(COV_CFLAGS) -I$(UNITY_DIR) -I$(HELPERS_DIR) $$< $(UNITY_DIR)/unity.c $(call unit_cov_obj,$(1)) $(COV_COMMON_OBJ) -o $$@ $(LDFLAGS) $(COV_FLAGS)
endef

define INTEG_COV_RULE
$(COV_BIN_INTEG)/$(1): tests/integration/$(1).c $(UNITY_DIR)/unity.c $(COV_SRC_OBJS)
	@mkdir -p $$(@D)
	$(CC) $(COV_CFLAGS) -I$(UNITY_DIR) -I$(HELPERS_DIR) $$< $(UNITY_DIR)/unity.c $(COV_SRC_OBJS) -o $$@ $(LDFLAGS) $(COV_FLAGS)
endef

$(foreach stem,$(UNIT_STEMS),$(eval $(call UNIT_COV_RULE,$(stem))))
$(foreach stem,$(INTEG_STEMS),$(eval $(call INTEG_COV_RULE,$(stem))))

COV_UNIT_BINS  = $(UNIT_STEMS:%=$(COV_BIN_UNIT)/%)
COV_INTEG_BINS = $(INTEG_STEMS:%=$(COV_BIN_INTEG)/%)

coverage_unit: $(COV_UNIT_BINS)
	@printf "\n=== Running unit tests for coverage ===\n\n"
	lcov --zerocounters --directory build/coverage --directory tests/bin/coverage
	@for t in $(COV_UNIT_BINS); do ./$$t; done
	lcov --capture \
		--directory build/coverage \
		--directory tests/bin/coverage/unit \
		--output-file $(COV_DIR)/coverage_unit.info \
		--rc branch_coverage=1
	lcov --remove $(COV_DIR)/coverage_unit.info \
		'*/tests/*' \
		--output-file $(COV_DIR)/coverage_unit_filtered.info \
		--rc branch_coverage=1 \
		--ignore-errors unused
	genhtml $(COV_DIR)/coverage_unit_filtered.info \
		--output-directory $(COV_REPORT_DIR)/unit \
		--branch-coverage \
		--title "Unit Test Coverage"
	@printf "\nReport ready: $(COV_REPORT_DIR)/unit/index.html\n"

coverage: $(COV_UNIT_BINS) $(COV_INTEG_BINS)
	@printf "\n=== Running all tests for coverage ===\n\n"
	lcov --zerocounters --directory build/coverage --directory tests/bin/coverage
	@for t in $(COV_UNIT_BINS) $(COV_INTEG_BINS); do ./$$t; done
	lcov --capture \
		--directory build/coverage \
		--directory tests/bin/coverage \
		--output-file $(COV_DIR)/coverage.info \
		--rc branch_coverage=1
	lcov --remove $(COV_DIR)/coverage.info \
		'*/tests/*' \
		--output-file $(COV_DIR)/coverage_filtered.info \
		--rc branch_coverage=1 \
		--ignore-errors unused
	genhtml $(COV_DIR)/coverage_filtered.info \
		--output-directory $(COV_REPORT_DIR)/global \
		--branch-coverage \
		--title "Game Boy Emulator Coverage"
	@printf "\nReport ready: $(COV_REPORT_DIR)/global/index.html\n"

clean:
	rm -rf build tests/bin $(COV_REPORT_DIR)
