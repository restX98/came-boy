#include "unity.h"
#include "log_helpers.h"

#include <string.h>

#include "memory/mbc/mbc.h"

#define BANKS 8
#define ROM_SIZE (BANKS * 0x4000)
#define RAM_SIZE 0x8000 // 4 banks of 0x2000

static cartridge_t cartridge;
static uint8_t rom[ROM_SIZE];
static uint8_t ram[RAM_SIZE];

void setUp(void) {
    suppress_logs();

    cartridge = (cartridge_t){ 0 };
    memset(rom, 0, sizeof(rom));
    memset(ram, 0, sizeof(ram));

    for (int b = 0; b < BANKS; b++) {
        rom[b * 0x4000] = (uint8_t)b;
    }

    cartridge.rom = rom;
    cartridge.size = ROM_SIZE;
    cartridge.banks_number = BANKS;
    cartridge.ram.mem = ram;
    cartridge.ram.size = RAM_SIZE;
    cartridge.has_ram = true;
    cartridge.mbc = &mbc5_ops;

    mbc5_ops.init(&cartridge);
}

void tearDown(void) {
    restore_logs();
}

void test_mbc5_init_sets_defaults(void) {
    TEST_ASSERT_EQUAL_UINT8(0x01, cartridge.state.mbc5.rom_bank);
    TEST_ASSERT_EQUAL_UINT8(0x00, cartridge.state.mbc5.ram_bank);
    TEST_ASSERT_FALSE(cartridge.state.mbc5.ram_enabled);
}

void test_mbc5_rom_read_fixed_bank(void) {
    TEST_ASSERT_EQUAL_UINT8(0, mbc5_ops.rom_read(&cartridge, 0x0000));
}

void test_mbc5_rom_read_switchable_bank(void) {
    // 0x2000-0x2FFF sets the low 8 bits of the ROM bank.
    mbc5_ops.rom_write(&cartridge, 0x2000, 0x03);

    TEST_ASSERT_EQUAL_UINT8(3, mbc5_ops.rom_read(&cartridge, 0x4000));
}

void test_mbc5_rom_bank_zero_is_selectable(void) {
    // Unlike MBC1/2, MBC5 can map bank 0 into the switchable region.
    mbc5_ops.rom_write(&cartridge, 0x2000, 0x00);

    TEST_ASSERT_EQUAL_UINT8(0, mbc5_ops.rom_read(&cartridge, 0x4000));
}

void test_mbc5_rom_write_high_bit_register(void) {
    mbc5_ops.rom_write(&cartridge, 0x2000, 0x03); // low byte = 3

    // 0x3000-0x3FFF is the 9th ROM-bank bit. rom_bank is 8-bit here, so the bit
    // is truncated away and the low byte selection is preserved.
    mbc5_ops.rom_write(&cartridge, 0x3000, 0x01);

    TEST_ASSERT_EQUAL_UINT8(3, mbc5_ops.rom_read(&cartridge, 0x4000));
}

void test_mbc5_toggles_ram_enable(void) {
    mbc5_ops.rom_write(&cartridge, 0x0000, 0x0A);
    TEST_ASSERT_TRUE(cartridge.state.mbc5.ram_enabled);

    mbc5_ops.rom_write(&cartridge, 0x0000, 0x00);
    TEST_ASSERT_FALSE(cartridge.state.mbc5.ram_enabled);
}

void test_mbc5_ram_read_disabled_returns_0xFF(void) {
    TEST_ASSERT_EQUAL_UINT8(0xFF, mbc5_ops.ram_read(&cartridge, 0x0000));
}

void test_mbc5_ram_read_without_ram_returns_0xFF(void) {
    cartridge.has_ram = false;
    mbc5_ops.rom_write(&cartridge, 0x0000, 0x0A); // enabled, but no RAM present

    TEST_ASSERT_EQUAL_UINT8(0xFF, mbc5_ops.ram_read(&cartridge, 0x0000));
}

void test_mbc5_ram_write_disabled_is_ignored(void) {
    // RAM disabled by default after init.
    mbc5_ops.ram_write(&cartridge, 0x0000, 0xAB);

    TEST_ASSERT_EQUAL_UINT8(0x00, ram[0x0000]);
}

void test_mbc5_ram_round_trip_when_enabled(void) {
    mbc5_ops.rom_write(&cartridge, 0x0000, 0x0A); // enable RAM

    mbc5_ops.ram_write(&cartridge, 0x0010, 0xAB);

    TEST_ASSERT_EQUAL_UINT8(0xAB, ram[0x0010]);
    TEST_ASSERT_EQUAL_UINT8(0xAB, mbc5_ops.ram_read(&cartridge, 0x0010));
}

void test_mbc5_ram_bank_selects_offset(void) {
    mbc5_ops.rom_write(&cartridge, 0x0000, 0x0A); // enable RAM
    mbc5_ops.rom_write(&cartridge, 0x4000, 0x01); // select RAM bank 1

    mbc5_ops.ram_write(&cartridge, 0x0000, 0x5A);

    // Bank 1 maps to offset 0x2000 in the RAM array.
    TEST_ASSERT_EQUAL_UINT8(0x5A, ram[0x2000]);
    TEST_ASSERT_EQUAL_UINT8(0x00, ram[0x0000]);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_mbc5_init_sets_defaults);
    RUN_TEST(test_mbc5_rom_read_fixed_bank);
    RUN_TEST(test_mbc5_rom_read_switchable_bank);
    RUN_TEST(test_mbc5_rom_bank_zero_is_selectable);
    RUN_TEST(test_mbc5_rom_write_high_bit_register);
    RUN_TEST(test_mbc5_toggles_ram_enable);
    RUN_TEST(test_mbc5_ram_read_disabled_returns_0xFF);
    RUN_TEST(test_mbc5_ram_read_without_ram_returns_0xFF);
    RUN_TEST(test_mbc5_ram_write_disabled_is_ignored);
    RUN_TEST(test_mbc5_ram_round_trip_when_enabled);
    RUN_TEST(test_mbc5_ram_bank_selects_offset);

    return UNITY_END();
}
