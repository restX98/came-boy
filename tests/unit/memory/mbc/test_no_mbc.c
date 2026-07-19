#include "unity.h"
#include "log_helpers.h"

#include <string.h>

#include "memory/mbc/mbc.h"

#define ROM_SIZE 0x8000

static cartridge_t cartridge;
static uint8_t rom[ROM_SIZE];

void setUp(void) {
    suppress_logs();

    cartridge = (cartridge_t){ 0 };
    memset(rom, 0, sizeof(rom));
    cartridge.rom = rom;
    cartridge.size = ROM_SIZE;
    cartridge.mbc = &no_mbc_ops;
}

void tearDown(void) {
    restore_logs();
}

void test_no_mbc_init_is_noop(void) {
    // No MBC keeps no state; init must simply not crash.
    no_mbc_ops.init(&cartridge);
}

void test_no_mbc_rom_read_returns_rom_byte(void) {
    rom[0x0000] = 0xAA;
    rom[0x3FFF] = 0xBB;
    rom[0x7FFF] = 0xCC;

    TEST_ASSERT_EQUAL_UINT8(0xAA, no_mbc_ops.rom_read(&cartridge, 0x0000));
    TEST_ASSERT_EQUAL_UINT8(0xBB, no_mbc_ops.rom_read(&cartridge, 0x3FFF));
    TEST_ASSERT_EQUAL_UINT8(0xCC, no_mbc_ops.rom_read(&cartridge, 0x7FFF));
}

void test_no_mbc_rom_read_out_of_range_returns_0xFF(void) {
    TEST_ASSERT_EQUAL_UINT8(0xFF, no_mbc_ops.rom_read(&cartridge, 0x8000));
}

void test_no_mbc_rom_write_is_ignored(void) {
    rom[0x2000] = 0x11;

    no_mbc_ops.rom_write(&cartridge, 0x2000, 0x55);

    TEST_ASSERT_EQUAL_UINT8(0x11, rom[0x2000]); // ROM is read-only
}

void test_no_mbc_ram_read_returns_0xFF(void) {
    TEST_ASSERT_EQUAL_UINT8(0xFF, no_mbc_ops.ram_read(&cartridge, 0x0000));
}

void test_no_mbc_ram_write_is_ignored(void) {
    // No RAM backing store to touch; call must be a safe no-op.
    no_mbc_ops.ram_write(&cartridge, 0x0000, 0x42);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_no_mbc_init_is_noop);
    RUN_TEST(test_no_mbc_rom_read_returns_rom_byte);
    RUN_TEST(test_no_mbc_rom_read_out_of_range_returns_0xFF);
    RUN_TEST(test_no_mbc_rom_write_is_ignored);
    RUN_TEST(test_no_mbc_ram_read_returns_0xFF);
    RUN_TEST(test_no_mbc_ram_write_is_ignored);

    return UNITY_END();
}
