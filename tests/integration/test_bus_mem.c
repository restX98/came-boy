#include "unity.h"
#include "log_helpers.h"

#include <string.h>

#include "bus.h"
#include "memory/mem.h"

static bus_t       bus;
static cartridge_t cartridge;
static uint8_t     fake_rom[0x8000];

void setUp(void) {
    suppress_logs();

    memset(fake_rom, 0, sizeof(fake_rom));
    cartridge = (cartridge_t){ .rom = fake_rom, .size = sizeof(fake_rom), .bank = 1 };
    bus_init(&bus, &cartridge);
}

void tearDown(void) {
    bus_free(&bus);
    restore_logs();
}

// ---- bus_init + mem ----

void test_bus_init_allocates_wram(void) {
    TEST_ASSERT_NOT_NULL(bus.wram.mem);
    TEST_ASSERT_EQUAL_size_t(WRAM_SIZE, bus.wram.size);
}

void test_bus_init_allocates_vram(void) {
    TEST_ASSERT_NOT_NULL(bus.vram.mem);
    TEST_ASSERT_EQUAL_size_t(VRAM_SIZE, bus.vram.size);
}

void test_bus_init_allocates_hram(void) {
    TEST_ASSERT_NOT_NULL(bus.hram.mem);
    TEST_ASSERT_EQUAL_size_t(HRAM_SIZE, bus.hram.size);
}

// ---- bus_read reads from real allocated memory ----

void test_bus_read_returns_value_written_to_wram(void) {
    // TODO: replace with bus_write once implemented
    bus.wram.mem[0x0000] = 0x42;
    TEST_ASSERT_EQUAL_UINT8(0x42, bus_read(&bus, 0xC000));
}

void test_bus_read_returns_value_written_to_vram(void) {
    // TODO: replace with bus_write once implemented
    bus.vram.mem[0x0000] = 0xAB;
    TEST_ASSERT_EQUAL_UINT8(0xAB, bus_read(&bus, 0x8000));
}

void test_bus_read_returns_value_written_to_hram(void) {
    // TODO: replace with bus_write once implemented
    bus.hram.mem[0x0000] = 0x77;
    TEST_ASSERT_EQUAL_UINT8(0x77, bus_read(&bus, 0xFF80));
}

void test_bus_read_wram_last_byte(void) {
    // TODO: replace with bus_write once implemented
    bus.wram.mem[WRAM_SIZE - 1] = 0xFF;
    TEST_ASSERT_EQUAL_UINT8(0xFF, bus_read(&bus, 0xDFFF));
}

void test_bus_read_hram_last_byte(void) {
    // TODO: replace with bus_write once implemented
    bus.hram.mem[HRAM_SIZE - 1] = 0xBB;
    TEST_ASSERT_EQUAL_UINT8(0xBB, bus_read(&bus, 0xFFFE));
}

void test_bus_read_rom_bank0(void) {
    fake_rom[0x0000] = 0xAA;
    TEST_ASSERT_EQUAL_UINT8(0xAA, bus_read(&bus, 0x0000));
}

void test_bus_read_rom_bank1(void) {
    fake_rom[0x4000] = 0xCC;
    TEST_ASSERT_EQUAL_UINT8(0xCC, bus_read(&bus, 0x4000));
}

void test_bus_read_correct_offset_mapping(void) {
    // TODO: replace with bus_write once implemented
    bus.wram.mem[1] = 0x11;
    bus.vram.mem[1] = 0x22;

    TEST_ASSERT_EQUAL_UINT8(0x11, bus_read(&bus, 0xC001));
    TEST_ASSERT_EQUAL_UINT8(0x22, bus_read(&bus, 0x8001));
}

// ---- bus_free releases memory ----

void test_bus_free_sets_wram_mem_to_null(void) {
    bus_free(&bus);
    TEST_ASSERT_NULL(bus.wram.mem);
}

void test_bus_free_sets_vram_mem_to_null(void) {
    bus_free(&bus);
    TEST_ASSERT_NULL(bus.vram.mem);
}

void test_bus_free_sets_hram_mem_to_null(void) {
    bus_free(&bus);
    TEST_ASSERT_NULL(bus.hram.mem);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_bus_init_allocates_wram);
    RUN_TEST(test_bus_init_allocates_vram);
    RUN_TEST(test_bus_init_allocates_hram);
    RUN_TEST(test_bus_read_returns_value_written_to_wram);
    RUN_TEST(test_bus_read_returns_value_written_to_vram);
    RUN_TEST(test_bus_read_returns_value_written_to_hram);
    RUN_TEST(test_bus_read_wram_last_byte);
    RUN_TEST(test_bus_read_hram_last_byte);
    RUN_TEST(test_bus_read_rom_bank0);
    RUN_TEST(test_bus_read_rom_bank1);
    RUN_TEST(test_bus_read_correct_offset_mapping);
    RUN_TEST(test_bus_free_sets_wram_mem_to_null);
    RUN_TEST(test_bus_free_sets_vram_mem_to_null);
    RUN_TEST(test_bus_free_sets_hram_mem_to_null);

    return UNITY_END();
}
