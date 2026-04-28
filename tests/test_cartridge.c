#include "unity.h"
#include "test_helpers.h"
#include "cartridge.h"
#include <stdio.h>
#include <string.h>

#define FAKE_ROM_PATH "/tmp/test_rom.gb"
#define DUMMY_ROM_SIZE 32

static cartridge_t cartridge;

void setUp(void) {
    suppress_logs();
    memset(&cartridge, 0, sizeof(cartridge));

    // Create a dummy ROM file for testing
    FILE *ptr = fopen(FAKE_ROM_PATH, "wb");
    uint8_t dummy_data[DUMMY_ROM_SIZE] = { 0 };
    fwrite(dummy_data, sizeof(dummy_data), 1, ptr);
    fclose(ptr);
}

void tearDown(void) {
    cartridge_unload(&cartridge);
    remove(FAKE_ROM_PATH);

    restore_logs();
}

// ---- cartridge_load ----

void test_cartridge_load_returns_0_on_valid_rom(void) {
    int result = cartridge_load(&cartridge, FAKE_ROM_PATH);
    TEST_ASSERT_EQUAL(0, result);
}


void test_cartridge_load_returns_minus_1_on_nonexistent_file(void) {
    int result = cartridge_load(&cartridge, "/nonexistent/path.gb");
    TEST_ASSERT_EQUAL(-1, result);
}

void test_cartridge_load_returns_minus_1_on_unseekable_file(void) {
    // Use /dev/stdin as an unseekable file
    int result = cartridge_load(&cartridge, "/dev/stdin");
    TEST_ASSERT_EQUAL(-1, result);
}

// TODO: Add more tests for cartridge_load, e.g. invalid ROM formats, read errors, etc.

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_cartridge_load_returns_0_on_valid_rom);
    RUN_TEST(test_cartridge_load_returns_minus_1_on_nonexistent_file);
    RUN_TEST(test_cartridge_load_returns_minus_1_on_unseekable_file);

    return UNITY_END();
}
