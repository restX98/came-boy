#include "unity.h"
#include "test_helpers.h"
#include "cartridge.h"
#include <stdio.h>
#include <string.h>

#define DUMMY_ROM_PATH "/tmp/test_rom.gb"
#define DUMMY_ROM_SIZE 32

static cartridge_t cartridge;

void setUp(void) {
    suppress_logs();
    memset(&cartridge, 0, sizeof(cartridge));

    // Create a dummy ROM file for testing
    FILE *ptr = fopen(DUMMY_ROM_PATH, "wb");
    uint8_t dummy_data[DUMMY_ROM_SIZE] = { 0 };
    fwrite(dummy_data, sizeof(dummy_data), 1, ptr);
    fclose(ptr);
}

void tearDown(void) {
    cartridge_unload(&cartridge);
    remove(DUMMY_ROM_PATH);

    restore_logs();
}

// ---- cartridge_load ----

void test_cartridge_load_returns_0_on_valid_rom(void) {
    int result = cartridge_load(&cartridge, DUMMY_ROM_PATH);
    TEST_ASSERT_EQUAL(0, result);
}

void test_cartridge_load_returns_minus1_on_invalid_file(void) {
    int result = cartridge_load(&cartridge, "/nonexistent.gb");
    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_cartridge_load_sets_correct_size(void) {
    cartridge_load(&cartridge, DUMMY_ROM_PATH);
    TEST_ASSERT_EQUAL_size_t(DUMMY_ROM_SIZE, cartridge.size);
}

void test_cartridge_load_sets_bank_to_1(void) {
    cartridge_load(&cartridge, DUMMY_ROM_PATH);
    TEST_ASSERT_EQUAL_UINT8(1, cartridge.bank);
}

void test_cartridge_load_rom_pointer_not_null(void) {
    cartridge_load(&cartridge, DUMMY_ROM_PATH);
    TEST_ASSERT_NOT_NULL(cartridge.rom);
}

void test_cartridge_load_rom_content_is_correct(void) {
    // Write known data to the fake ROM
    FILE *f = fopen(DUMMY_ROM_PATH, "wb");
    uint8_t expected[DUMMY_ROM_SIZE];
    for (int i = 0; i < DUMMY_ROM_SIZE; i++) {
        expected[i] = (uint8_t)i;
    }
    fwrite(expected, 1, DUMMY_ROM_SIZE, f);
    fclose(f);

    cartridge_load(&cartridge, DUMMY_ROM_PATH);
    TEST_ASSERT_EQUAL_MEMORY(expected, cartridge.rom, DUMMY_ROM_SIZE);
}

// ---- cartridge_unload ----

void test_cartridge_unload_frees_rom_memory(void) {
    cartridge_load(&cartridge, DUMMY_ROM_PATH);
    cartridge_unload(&cartridge);
    TEST_ASSERT_NULL(cartridge.rom);
    TEST_ASSERT_EQUAL_size_t(0, cartridge.size);
    TEST_ASSERT_EQUAL_UINT8(0, cartridge.bank);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_cartridge_load_returns_0_on_valid_rom);
    RUN_TEST(test_cartridge_load_returns_minus1_on_invalid_file);
    RUN_TEST(test_cartridge_load_sets_correct_size);
    RUN_TEST(test_cartridge_load_sets_bank_to_1);
    RUN_TEST(test_cartridge_load_rom_pointer_not_null);
    RUN_TEST(test_cartridge_load_rom_content_is_correct);
    RUN_TEST(test_cartridge_unload_frees_rom_memory);

    return UNITY_END();
}
