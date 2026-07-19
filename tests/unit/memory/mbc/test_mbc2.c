#include "unity.h"
#include "log_helpers.h"

#include <string.h>

#include "memory/mbc/mbc.h"

#define BANKS 16
#define ROM_SIZE (BANKS * 0x4000)
#define RAM_SIZE 0x200 // MBC2 has 512 x 4 bits of built-in RAM

static cartridge_t cartridge;
static uint8_t rom[ROM_SIZE];
static uint8_t ram[RAM_SIZE];

void setUp(void) {
    suppress_logs();

    cartridge = (cartridge_t){ 0 };
    memset(rom, 0, sizeof(rom));
    memset(ram, 0, sizeof(ram));

    // Mark the first byte of each ROM bank with its index for easy assertions.
    for (int b = 0; b < BANKS; b++) {
        rom[b * 0x4000] = (uint8_t)b;
    }

    cartridge.rom = rom;
    cartridge.size = ROM_SIZE;
    cartridge.banks_number = BANKS;
    cartridge.ram.mem = ram;
    cartridge.ram.size = RAM_SIZE;
    cartridge.has_ram = true;
    cartridge.mbc = &mbc2_ops;

    mbc2_ops.init(&cartridge);
}

void tearDown(void) {
    restore_logs();
}

void test_mbc2_init_sets_defaults(void) {
    TEST_ASSERT_EQUAL_UINT8(0x01, cartridge.state.mbc2.bank);
    TEST_ASSERT_FALSE(cartridge.state.mbc2.ram_enabled);
}

void test_mbc2_rom_read_fixed_bank(void) {
    // 0x0000-0x3FFF is always bank 0.
    TEST_ASSERT_EQUAL_UINT8(0, mbc2_ops.rom_read(&cartridge, 0x0000));
}

void test_mbc2_rom_read_switchable_bank(void) {
    // Select bank via 0x2000-0x3FFF with the address bit 8 set.
    mbc2_ops.rom_write(&cartridge, 0x0100, 0x05);

    TEST_ASSERT_EQUAL_UINT8(5, mbc2_ops.rom_read(&cartridge, 0x4000));
}

void test_mbc2_rom_write_bank_zero_becomes_one(void) {
    mbc2_ops.rom_write(&cartridge, 0x0100, 0x00);

    TEST_ASSERT_EQUAL_UINT8(0x01, cartridge.state.mbc2.bank);
}

void test_mbc2_rom_write_bank_masks_low_nibble(void) {
    mbc2_ops.rom_write(&cartridge, 0x0100, 0xF3);

    TEST_ASSERT_EQUAL_UINT8(0x03, cartridge.state.mbc2.bank);
}

void test_mbc2_rom_write_toggles_ram_enable(void) {
    // Address bit 8 clear -> RAM enable register.
    mbc2_ops.rom_write(&cartridge, 0x0000, 0x0A);
    TEST_ASSERT_TRUE(cartridge.state.mbc2.ram_enabled);

    mbc2_ops.rom_write(&cartridge, 0x0000, 0x00);
    TEST_ASSERT_FALSE(cartridge.state.mbc2.ram_enabled);
}

void test_mbc2_ram_read_disabled_returns_0xFF(void) {
    TEST_ASSERT_EQUAL_UINT8(0xFF, mbc2_ops.ram_read(&cartridge, 0x0000));
}

void test_mbc2_ram_write_disabled_is_ignored(void) {
    mbc2_ops.rom_write(&cartridge, 0x0000, 0x00); // ensure disabled
    mbc2_ops.ram_write(&cartridge, 0x0000, 0x0A);

    TEST_ASSERT_EQUAL_UINT8(0x00, ram[0]);
}

void test_mbc2_ram_stores_low_nibble_and_reads_high_bits_set(void) {
    mbc2_ops.rom_write(&cartridge, 0x0000, 0x0A); // enable RAM

    mbc2_ops.ram_write(&cartridge, 0x0000, 0x35);

    TEST_ASSERT_EQUAL_UINT8(0x05, ram[0]);                             // only low nibble stored
    TEST_ASSERT_EQUAL_UINT8(0xF5, mbc2_ops.ram_read(&cartridge, 0x0000)); // upper nibble reads as 1s
}

void test_mbc2_ram_address_wraps_at_512_bytes(void) {
    mbc2_ops.rom_write(&cartridge, 0x0000, 0x0A); // enable RAM

    mbc2_ops.ram_write(&cartridge, 0x0000, 0x03);

    // 0x0200 aliases 0x0000 (addr & 0x01FF).
    TEST_ASSERT_EQUAL_UINT8(0xF3, mbc2_ops.ram_read(&cartridge, 0x0200));
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_mbc2_init_sets_defaults);
    RUN_TEST(test_mbc2_rom_read_fixed_bank);
    RUN_TEST(test_mbc2_rom_read_switchable_bank);
    RUN_TEST(test_mbc2_rom_write_bank_zero_becomes_one);
    RUN_TEST(test_mbc2_rom_write_bank_masks_low_nibble);
    RUN_TEST(test_mbc2_rom_write_toggles_ram_enable);
    RUN_TEST(test_mbc2_ram_read_disabled_returns_0xFF);
    RUN_TEST(test_mbc2_ram_write_disabled_is_ignored);
    RUN_TEST(test_mbc2_ram_stores_low_nibble_and_reads_high_bits_set);
    RUN_TEST(test_mbc2_ram_address_wraps_at_512_bytes);

    return UNITY_END();
}
