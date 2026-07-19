#include "unity.h"
#include "log_helpers.h"

#include <string.h>

#include "bus.h"
#include "memory/mbc/mbc.h"
#include "memory/mem.h"

static bus_t       bus;
static cartridge_t cartridge;
static uint8_t     fake_rom[0x8000];

void setUp(void) {
    suppress_logs();

    memset(fake_rom, 0, sizeof(fake_rom));
    cartridge = (cartridge_t){ .rom = fake_rom, .size = sizeof(fake_rom), .mbc = &no_mbc_ops };
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
    bus_write(&bus, 0xC000, 0x42);
    TEST_ASSERT_EQUAL_UINT8(0x42, bus_read(&bus, 0xC000));
}

void test_bus_read_returns_value_written_to_vram(void) {
    bus_write(&bus, 0x8000, 0xAB);
    TEST_ASSERT_EQUAL_UINT8(0xAB, bus_read(&bus, 0x8000));
}

void test_bus_read_returns_value_written_to_hram(void) {
    bus_write(&bus, 0xFF80, 0x77);
    TEST_ASSERT_EQUAL_UINT8(0x77, bus_read(&bus, 0xFF80));
}

void test_bus_read_wram_last_byte(void) {
    bus_write(&bus, 0xDFFF, 0xFF);
    TEST_ASSERT_EQUAL_UINT8(0xFF, bus_read(&bus, 0xDFFF));
}

void test_bus_read_hram_last_byte(void) {
    bus_write(&bus, 0xFFFE, 0xBB);
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
    bus_write(&bus, 0xC001, 0x11);
    bus_write(&bus, 0x8001, 0x22);

    TEST_ASSERT_EQUAL_UINT8(0x11, bus_read(&bus, 0xC001));
    TEST_ASSERT_EQUAL_UINT8(0x22, bus_read(&bus, 0x8001));
}

// ---- Echo RAM mirrors WRAM ($E000-$FDFF -> $C000-$DDFF) ----

void test_bus_echo_ram_reflects_wram_write(void) {
    bus_write(&bus, 0xC000, 0x5A);
    TEST_ASSERT_EQUAL_UINT8(0x5A, bus_read(&bus, 0xE000));
}

void test_bus_wram_reflects_echo_ram_write(void) {
    bus_write(&bus, 0xE000, 0xA5);
    TEST_ASSERT_EQUAL_UINT8(0xA5, bus_read(&bus, 0xC000));
}

void test_bus_echo_ram_last_byte_mirrors_wram(void) {
    // $FDFF is the last Echo RAM byte; it mirrors WRAM $DDFF.
    bus_write(&bus, 0xFDFF, 0x3C);
    TEST_ASSERT_EQUAL_UINT8(0x3C, bus_read(&bus, 0xDDFF));
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
    RUN_TEST(test_bus_echo_ram_reflects_wram_write);
    RUN_TEST(test_bus_wram_reflects_echo_ram_write);
    RUN_TEST(test_bus_echo_ram_last_byte_mirrors_wram);
    RUN_TEST(test_bus_free_sets_wram_mem_to_null);
    RUN_TEST(test_bus_free_sets_vram_mem_to_null);
    RUN_TEST(test_bus_free_sets_hram_mem_to_null);

    return UNITY_END();
}
