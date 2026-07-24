#include "unity.h"
#include "log_helpers.h"

#include <string.h>
#include <time.h>

#include "memory/mbc/mbc.h"

#define BANKS 8
#define ROM_SIZE (BANKS * 0x4000)
#define RAM_SIZE 0x8000 // 4 banks of 0x2000

// RTC register indices, mirroring the enum in mbc3.c.
#define RTC_SECONDS  0
#define RTC_MINUTES  1
#define RTC_HOURS    2
#define RTC_DAY_LOW  3
#define RTC_DAY_HIGH 4

#define RTC_DH_DAY_MSB 0x01
#define RTC_DH_HALT    0x40
#define RTC_DH_CARRY   0x80

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

    // MBC3+RAM+BATTERY: RAM present, no real-time clock by default.
    rom[0x0147] = 0x13;

    cartridge.rom = rom;
    cartridge.size = ROM_SIZE;
    cartridge.banks_number = BANKS;
    cartridge.ram.mem = ram;
    cartridge.ram.size = RAM_SIZE;
    cartridge.has_ram = true;
    cartridge.mbc = &mbc3_ops;

    mbc3_ops.init(&cartridge);
}

void tearDown(void) {
    restore_logs();
}

// Re-initialise the cartridge as an MBC3+TIMER type so the RTC is present.
static void enable_rtc(void) {
    rom[0x0147] = 0x10; // MBC3+TIMER+RAM+BATTERY
    mbc3_ops.init(&cartridge);
}

// ---- init ----

void test_mbc3_init_sets_defaults(void) {
    TEST_ASSERT_EQUAL_UINT8(0x01, cartridge.state.mbc3.rom_bank);
    TEST_ASSERT_EQUAL_UINT8(0x00, cartridge.state.mbc3.ram_bank);
    TEST_ASSERT_FALSE(cartridge.state.mbc3.ram_enabled);
    TEST_ASSERT_EQUAL_UINT8(0xFF, cartridge.state.mbc3.latch);
    TEST_ASSERT_FALSE(cartridge.state.mbc3.has_rtc);
}

void test_mbc3_init_detects_rtc_from_header(void) {
    enable_rtc();
    TEST_ASSERT_TRUE(cartridge.state.mbc3.has_rtc);
}

// ---- ROM banking ----

void test_mbc3_rom_read_fixed_bank(void) {
    TEST_ASSERT_EQUAL_UINT8(0, mbc3_ops.rom_read(&cartridge, 0x0000));
}

void test_mbc3_rom_read_switchable_bank(void) {
    mbc3_ops.rom_write(&cartridge, 0x2000, 0x03);

    TEST_ASSERT_EQUAL_UINT8(3, mbc3_ops.rom_read(&cartridge, 0x4000));
}

void test_mbc3_rom_bank_zero_remaps_to_one(void) {
    mbc3_ops.rom_write(&cartridge, 0x2000, 0x00);

    TEST_ASSERT_EQUAL_UINT8(0x01, cartridge.state.mbc3.rom_bank);
    TEST_ASSERT_EQUAL_UINT8(1, mbc3_ops.rom_read(&cartridge, 0x4000));
}

void test_mbc3_rom_bank_masks_to_7_bits(void) {
    // 0x80 has only its 8th bit set; MBC3 keeps 7 bits, leaving 0 -> remapped to 1.
    mbc3_ops.rom_write(&cartridge, 0x2000, 0x80);

    TEST_ASSERT_EQUAL_UINT8(0x01, cartridge.state.mbc3.rom_bank);
}

void test_mbc3_rom_bank_high_value(void) {
    // 0xFF & 0x7F = 0x7F; the read then wraps by banks_number (8) to bank 7.
    mbc3_ops.rom_write(&cartridge, 0x2000, 0xFF);

    TEST_ASSERT_EQUAL_UINT8(0x7F, cartridge.state.mbc3.rom_bank);
    TEST_ASSERT_EQUAL_UINT8(7, mbc3_ops.rom_read(&cartridge, 0x4000));
}

// ---- RAM enable ----

void test_mbc3_toggles_ram_enable(void) {
    mbc3_ops.rom_write(&cartridge, 0x0000, 0x0A);
    TEST_ASSERT_TRUE(cartridge.state.mbc3.ram_enabled);

    mbc3_ops.rom_write(&cartridge, 0x0000, 0x00);
    TEST_ASSERT_FALSE(cartridge.state.mbc3.ram_enabled);
}

void test_mbc3_ram_enable_only_low_nibble_0xA(void) {
    // Any value whose low nibble is 0xA enables; others do not.
    mbc3_ops.rom_write(&cartridge, 0x0000, 0xFA);
    TEST_ASSERT_TRUE(cartridge.state.mbc3.ram_enabled);

    mbc3_ops.rom_write(&cartridge, 0x0000, 0x0B);
    TEST_ASSERT_FALSE(cartridge.state.mbc3.ram_enabled);
}

// ---- RAM access ----

void test_mbc3_ram_read_disabled_returns_0xFF(void) {
    TEST_ASSERT_EQUAL_UINT8(0xFF, mbc3_ops.ram_read(&cartridge, 0x0000));
}

void test_mbc3_ram_read_without_ram_returns_0xFF(void) {
    cartridge.has_ram = false;
    mbc3_ops.rom_write(&cartridge, 0x0000, 0x0A); // enabled, but no RAM present

    TEST_ASSERT_EQUAL_UINT8(0xFF, mbc3_ops.ram_read(&cartridge, 0x0000));
}

void test_mbc3_ram_write_disabled_is_ignored(void) {
    mbc3_ops.ram_write(&cartridge, 0x0000, 0xAB);

    TEST_ASSERT_EQUAL_UINT8(0x00, ram[0x0000]);
}

void test_mbc3_ram_round_trip_when_enabled(void) {
    mbc3_ops.rom_write(&cartridge, 0x0000, 0x0A); // enable RAM

    mbc3_ops.ram_write(&cartridge, 0x0010, 0xAB);

    TEST_ASSERT_EQUAL_UINT8(0xAB, ram[0x0010]);
    TEST_ASSERT_EQUAL_UINT8(0xAB, mbc3_ops.ram_read(&cartridge, 0x0010));
}

void test_mbc3_ram_bank_selects_offset(void) {
    mbc3_ops.rom_write(&cartridge, 0x0000, 0x0A); // enable RAM
    mbc3_ops.rom_write(&cartridge, 0x4000, 0x01); // select RAM bank 1

    mbc3_ops.ram_write(&cartridge, 0x0000, 0x5A);

    // Bank 1 maps to offset 0x2000 in the RAM array.
    TEST_ASSERT_EQUAL_UINT8(0x5A, ram[0x2000]);
    TEST_ASSERT_EQUAL_UINT8(0x00, ram[0x0000]);
}

void test_mbc3_ram_bank_in_gap_returns_0xFF(void) {
    // Bank selectors 0x04-0x07 are neither RAM banks nor RTC registers.
    mbc3_ops.rom_write(&cartridge, 0x0000, 0x0A); // enable RAM
    mbc3_ops.rom_write(&cartridge, 0x4000, 0x05); // gap selector

    TEST_ASSERT_EQUAL_UINT8(0xFF, mbc3_ops.ram_read(&cartridge, 0x0000));
}

// ---- RTC ----

void test_mbc3_rtc_write_and_read_register(void) {
    enable_rtc();
    mbc3_ops.rom_write(&cartridge, 0x0000, 0x0A); // enable RAM/RTC access
    mbc3_ops.rom_write(&cartridge, 0x4000, 0x08); // select RTC seconds register

    mbc3_ops.ram_write(&cartridge, 0x0000, 30);

    TEST_ASSERT_EQUAL_UINT8(30, mbc3_ops.ram_read(&cartridge, 0x0000));
}

void test_mbc3_rtc_read_without_rtc_returns_0xFF(void) {
    // Default cartridge (type 0x13) has no RTC.
    mbc3_ops.rom_write(&cartridge, 0x0000, 0x0A);
    mbc3_ops.rom_write(&cartridge, 0x4000, 0x08); // RTC seconds selector

    TEST_ASSERT_EQUAL_UINT8(0xFF, mbc3_ops.ram_read(&cartridge, 0x0000));
}

void test_mbc3_rtc_read_disabled_returns_0xFF(void) {
    enable_rtc();
    mbc3_ops.rom_write(&cartridge, 0x4000, 0x08); // RTC selector, access still disabled

    TEST_ASSERT_EQUAL_UINT8(0xFF, mbc3_ops.ram_read(&cartridge, 0x0000));
}

void test_mbc3_rtc_latch_snapshots_live_counters(void) {
    enable_rtc();
    mbc3_ops.rom_write(&cartridge, 0x0000, 0x0A);
    mbc3_ops.rom_write(&cartridge, 0x4000, 0x08); // select RTC seconds

    // Live counter differs from the (zeroed) latched snapshot.
    cartridge.state.mbc3.rtc[RTC_SECONDS] = 45;
    TEST_ASSERT_EQUAL_UINT8(0, mbc3_ops.ram_read(&cartridge, 0x0000));

    // A 0x00 -> 0x01 latch sequence copies the live counters into the snapshot.
    mbc3_ops.rom_write(&cartridge, 0x6000, 0x00);
    mbc3_ops.rom_write(&cartridge, 0x6000, 0x01);

    TEST_ASSERT_EQUAL_UINT8(45, mbc3_ops.ram_read(&cartridge, 0x0000));
}

void test_mbc3_rtc_latch_requires_zero_then_one(void) {
    enable_rtc();
    mbc3_ops.rom_write(&cartridge, 0x0000, 0x0A);
    mbc3_ops.rom_write(&cartridge, 0x4000, 0x08);

    cartridge.state.mbc3.rtc[RTC_SECONDS] = 45;

    // A lone 0x01 (without a preceding 0x00) must not latch.
    mbc3_ops.rom_write(&cartridge, 0x6000, 0x01);

    TEST_ASSERT_EQUAL_UINT8(0, mbc3_ops.ram_read(&cartridge, 0x0000));
}

void test_mbc3_rtc_day_high_write_masks_reserved_bits(void) {
    enable_rtc();
    mbc3_ops.rom_write(&cartridge, 0x0000, 0x0A);
    mbc3_ops.rom_write(&cartridge, 0x4000, 0x0C); // select day-high register

    mbc3_ops.ram_write(&cartridge, 0x0000, 0xFF);

    // Only the day-MSB, halt and carry bits are writable.
    uint8_t expected = RTC_DH_DAY_MSB | RTC_DH_HALT | RTC_DH_CARRY; // 0xC1
    TEST_ASSERT_EQUAL_UINT8(expected, mbc3_ops.ram_read(&cartridge, 0x0000));
}

void test_mbc3_rtc_advances_over_elapsed_time(void) {
    enable_rtc();
    mbc3_ops.rom_write(&cartridge, 0x0000, 0x0A);
    mbc3_ops.rom_write(&cartridge, 0x4000, 0x08); // select RTC seconds

    // Pretend the clock was last updated 5 seconds ago.
    cartridge.state.mbc3.rtc[RTC_SECONDS] = 0;
    cartridge.state.mbc3.rtc_last = time(NULL) - 5;

    // Latching ticks the live counters before snapshotting them.
    mbc3_ops.rom_write(&cartridge, 0x6000, 0x00);
    mbc3_ops.rom_write(&cartridge, 0x6000, 0x01);

    // Allow +/-1s slack in case a second boundary is crossed mid-test.
    TEST_ASSERT_INT_WITHIN(1, 5, mbc3_ops.ram_read(&cartridge, 0x0000));
}

void test_mbc3_rtc_does_not_advance_while_halted(void) {
    enable_rtc();
    mbc3_ops.rom_write(&cartridge, 0x0000, 0x0A);
    mbc3_ops.rom_write(&cartridge, 0x4000, 0x08); // select RTC seconds

    cartridge.state.mbc3.rtc[RTC_SECONDS] = 0;
    cartridge.state.mbc3.rtc[RTC_DAY_HIGH] = RTC_DH_HALT;
    cartridge.state.mbc3.rtc_last = time(NULL) - 100;

    mbc3_ops.rom_write(&cartridge, 0x6000, 0x00);
    mbc3_ops.rom_write(&cartridge, 0x6000, 0x01);

    TEST_ASSERT_EQUAL_UINT8(0, mbc3_ops.ram_read(&cartridge, 0x0000));
}

void test_mbc3_rtc_write_ignored_without_rtc(void) {
    // Default cartridge has no RTC: writing an RTC selector must not corrupt state.
    mbc3_ops.rom_write(&cartridge, 0x0000, 0x0A);
    mbc3_ops.rom_write(&cartridge, 0x4000, 0x08);

    mbc3_ops.ram_write(&cartridge, 0x0000, 0x30); // no-op

    TEST_ASSERT_EQUAL_UINT8(0, cartridge.state.mbc3.rtc[RTC_SECONDS]);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_mbc3_init_sets_defaults);
    RUN_TEST(test_mbc3_init_detects_rtc_from_header);
    RUN_TEST(test_mbc3_rom_read_fixed_bank);
    RUN_TEST(test_mbc3_rom_read_switchable_bank);
    RUN_TEST(test_mbc3_rom_bank_zero_remaps_to_one);
    RUN_TEST(test_mbc3_rom_bank_masks_to_7_bits);
    RUN_TEST(test_mbc3_rom_bank_high_value);
    RUN_TEST(test_mbc3_toggles_ram_enable);
    RUN_TEST(test_mbc3_ram_enable_only_low_nibble_0xA);
    RUN_TEST(test_mbc3_ram_read_disabled_returns_0xFF);
    RUN_TEST(test_mbc3_ram_read_without_ram_returns_0xFF);
    RUN_TEST(test_mbc3_ram_write_disabled_is_ignored);
    RUN_TEST(test_mbc3_ram_round_trip_when_enabled);
    RUN_TEST(test_mbc3_ram_bank_selects_offset);
    RUN_TEST(test_mbc3_ram_bank_in_gap_returns_0xFF);
    RUN_TEST(test_mbc3_rtc_write_and_read_register);
    RUN_TEST(test_mbc3_rtc_read_without_rtc_returns_0xFF);
    RUN_TEST(test_mbc3_rtc_read_disabled_returns_0xFF);
    RUN_TEST(test_mbc3_rtc_latch_snapshots_live_counters);
    RUN_TEST(test_mbc3_rtc_latch_requires_zero_then_one);
    RUN_TEST(test_mbc3_rtc_day_high_write_masks_reserved_bits);
    RUN_TEST(test_mbc3_rtc_advances_over_elapsed_time);
    RUN_TEST(test_mbc3_rtc_does_not_advance_while_halted);
    RUN_TEST(test_mbc3_rtc_write_ignored_without_rtc);

    return UNITY_END();
}
