#include "unity.h"
#include "log_helpers.h"

#include <string.h>

#include "memory/mbc/mbc.h"

#define BANKS 64
#define ROM_SIZE (BANKS * 0x4000) // 1 MiB — also the MBC1 multicart size
#define RAM_SIZE 0x8000           // 4 banks of 0x2000

// The header logo MBC1 multicart detection looks for, at bank 0x10 + 0x0104.
#define MULTICART_LOGO_OFFSET (0x10 * 0x4000 + 0x0104)

static const uint8_t nintendo_logo[48] = {
    0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B,
    0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
    0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
    0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
    0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC,
    0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E,
};

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
    cartridge.mbc = &mbc1_ops;

    mbc1_ops.init(&cartridge); // no logo present -> not a multicart
}

void tearDown(void) {
    restore_logs();
}

// Place the multicart logo and re-run init so detection triggers.
static void enable_multicart(void) {
    memcpy(rom + MULTICART_LOGO_OFFSET, nintendo_logo, sizeof(nintendo_logo));
    mbc1_ops.init(&cartridge);
}

// ---- init ----

void test_mbc1_init_sets_defaults(void) {
    TEST_ASSERT_EQUAL_UINT8(0x01, cartridge.state.mbc1.bank1);
    TEST_ASSERT_EQUAL_UINT8(0x00, cartridge.state.mbc1.bank2);
    TEST_ASSERT_EQUAL_UINT8(0, cartridge.state.mbc1.banking_mode);
    TEST_ASSERT_FALSE(cartridge.state.mbc1.ram_enabled);
    TEST_ASSERT_FALSE(cartridge.state.mbc1.is_multicart);
}

// ---- ROM banking ----

void test_mbc1_rom_read_fixed_bank0_in_mode0(void) {
    TEST_ASSERT_EQUAL_UINT8(0, mbc1_ops.rom_read(&cartridge, 0x0000));
}

void test_mbc1_rom_read_switchable_bank(void) {
    mbc1_ops.rom_write(&cartridge, 0x2000, 0x05); // bank1 = 5

    TEST_ASSERT_EQUAL_UINT8(5, mbc1_ops.rom_read(&cartridge, 0x4000));
}

void test_mbc1_bank1_zero_becomes_one(void) {
    mbc1_ops.rom_write(&cartridge, 0x2000, 0x00);

    TEST_ASSERT_EQUAL_UINT8(0x01, cartridge.state.mbc1.bank1);
}

void test_mbc1_bank1_masks_5_bits(void) {
    mbc1_ops.rom_write(&cartridge, 0x2000, 0xE5);

    TEST_ASSERT_EQUAL_UINT8(0x05, cartridge.state.mbc1.bank1);
}

void test_mbc1_bank2_masks_2_bits(void) {
    mbc1_ops.rom_write(&cartridge, 0x4000, 0x07);

    TEST_ASSERT_EQUAL_UINT8(0x03, cartridge.state.mbc1.bank2);
}

void test_mbc1_rom_read_combines_bank2_upper_bits(void) {
    mbc1_ops.rom_write(&cartridge, 0x2000, 0x01); // bank1 = 1
    mbc1_ops.rom_write(&cartridge, 0x4000, 0x01); // bank2 = 1 -> +32

    TEST_ASSERT_EQUAL_UINT8(33, mbc1_ops.rom_read(&cartridge, 0x4000));
}

void test_mbc1_mode1_maps_bank2_into_low_region(void) {
    mbc1_ops.rom_write(&cartridge, 0x4000, 0x01); // bank2 = 1
    mbc1_ops.rom_write(&cartridge, 0x6000, 0x01); // banking mode 1

    // 0x0000-0x3FFF now reflects bank2 << 5 = 32.
    TEST_ASSERT_EQUAL_UINT8(32, mbc1_ops.rom_read(&cartridge, 0x0000));
}

// ---- RAM ----

void test_mbc1_toggles_ram_enable(void) {
    mbc1_ops.rom_write(&cartridge, 0x0000, 0x0A);
    TEST_ASSERT_TRUE(cartridge.state.mbc1.ram_enabled);

    mbc1_ops.rom_write(&cartridge, 0x0000, 0x00);
    TEST_ASSERT_FALSE(cartridge.state.mbc1.ram_enabled);
}

void test_mbc1_ram_read_disabled_returns_0xFF(void) {
    TEST_ASSERT_EQUAL_UINT8(0xFF, mbc1_ops.ram_read(&cartridge, 0x0000));
}

void test_mbc1_ram_read_without_ram_returns_0xFF(void) {
    cartridge.has_ram = false;
    mbc1_ops.rom_write(&cartridge, 0x0000, 0x0A);

    TEST_ASSERT_EQUAL_UINT8(0xFF, mbc1_ops.ram_read(&cartridge, 0x0000));
}

void test_mbc1_ram_write_disabled_is_ignored(void) {
    // RAM disabled by default after init.
    mbc1_ops.ram_write(&cartridge, 0x0000, 0xAB);

    TEST_ASSERT_EQUAL_UINT8(0x00, ram[0x0000]);
}

void test_mbc1_ram_round_trip_when_enabled(void) {
    mbc1_ops.rom_write(&cartridge, 0x0000, 0x0A); // enable RAM

    mbc1_ops.ram_write(&cartridge, 0x0010, 0xAB);

    TEST_ASSERT_EQUAL_UINT8(0xAB, ram[0x0010]);
    TEST_ASSERT_EQUAL_UINT8(0xAB, mbc1_ops.ram_read(&cartridge, 0x0010));
}

void test_mbc1_ram_bank_offset_in_mode1(void) {
    mbc1_ops.rom_write(&cartridge, 0x0000, 0x0A); // enable RAM
    mbc1_ops.rom_write(&cartridge, 0x4000, 0x01); // bank2 = 1
    mbc1_ops.rom_write(&cartridge, 0x6000, 0x01); // banking mode 1

    mbc1_ops.ram_write(&cartridge, 0x0000, 0x5A);

    // Bank 1 maps to offset 0x2000 in the RAM array.
    TEST_ASSERT_EQUAL_UINT8(0x5A, ram[0x2000]);
    TEST_ASSERT_EQUAL_UINT8(0x00, ram[0x0000]);
}

// ---- multicart ----

void test_mbc1_not_multicart_when_size_differs(void) {
    cartridge.size = 0x8000; // not the 1 MiB multicart size
    mbc1_ops.init(&cartridge);

    TEST_ASSERT_FALSE(cartridge.state.mbc1.is_multicart);
}

void test_mbc1_multicart_detected_from_logo(void) {
    enable_multicart();

    TEST_ASSERT_TRUE(cartridge.state.mbc1.is_multicart);
}

void test_mbc1_multicart_rom_read_splits_bank_nibbles(void) {
    enable_multicart();
    mbc1_ops.rom_write(&cartridge, 0x2000, 0x03); // bank1 low nibble = 3
    mbc1_ops.rom_write(&cartridge, 0x4000, 0x01); // bank2 -> << 4

    // Multicart bank = (bank1 & 0x0F) | (bank2 << 4) = 3 | 16 = 19.
    TEST_ASSERT_EQUAL_UINT8(19, mbc1_ops.rom_read(&cartridge, 0x4000));
}

void test_mbc1_multicart_mode1_low_region(void) {
    enable_multicart();
    mbc1_ops.rom_write(&cartridge, 0x4000, 0x01); // bank2 = 1
    mbc1_ops.rom_write(&cartridge, 0x6000, 0x01); // banking mode 1

    // Multicart shift is 4: bank2 << 4 = 16.
    TEST_ASSERT_EQUAL_UINT8(16, mbc1_ops.rom_read(&cartridge, 0x0000));
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_mbc1_init_sets_defaults);

    RUN_TEST(test_mbc1_rom_read_fixed_bank0_in_mode0);
    RUN_TEST(test_mbc1_rom_read_switchable_bank);
    RUN_TEST(test_mbc1_bank1_zero_becomes_one);
    RUN_TEST(test_mbc1_bank1_masks_5_bits);
    RUN_TEST(test_mbc1_bank2_masks_2_bits);
    RUN_TEST(test_mbc1_rom_read_combines_bank2_upper_bits);
    RUN_TEST(test_mbc1_mode1_maps_bank2_into_low_region);

    RUN_TEST(test_mbc1_toggles_ram_enable);
    RUN_TEST(test_mbc1_ram_read_disabled_returns_0xFF);
    RUN_TEST(test_mbc1_ram_read_without_ram_returns_0xFF);
    RUN_TEST(test_mbc1_ram_write_disabled_is_ignored);
    RUN_TEST(test_mbc1_ram_round_trip_when_enabled);
    RUN_TEST(test_mbc1_ram_bank_offset_in_mode1);

    RUN_TEST(test_mbc1_not_multicart_when_size_differs);
    RUN_TEST(test_mbc1_multicart_detected_from_logo);
    RUN_TEST(test_mbc1_multicart_rom_read_splits_bank_nibbles);
    RUN_TEST(test_mbc1_multicart_mode1_low_region);

    return UNITY_END();
}
