#include "unity.h"
#include "test_helpers.h"
#include "bus.h"
#include <string.h>

#define FAKE_ROM_SIZE 0x8000 // 32 KiB, covers ROM bank 0 and 1

static bus_t bus;
static cartridge_t cartridge;
static uint8_t fake_rom[FAKE_ROM_SIZE];

void setUp(void) {
    suppress_logs();

    memset(fake_rom, 0, sizeof(fake_rom));
    cartridge = (cartridge_t){ .rom = fake_rom, .size = FAKE_ROM_SIZE, .bank = 1 };
    bus_init(&bus, &cartridge);
}

void tearDown(void) {
    bus_free(&bus);
    restore_logs();
}

// ---- bus_init ----

void test_bus_init_returns_0_on_success(void) {
    bus_t b = { 0 };
    int result = bus_init(&b, &cartridge);
    TEST_ASSERT_EQUAL_INT(0, result);
    bus_free(&b);
}

void test_bus_init_sets_cartridge_pointer(void) {
    TEST_ASSERT_EQUAL_PTR(&cartridge, bus.cartridge);
}

void test_bus_init_allocates_wram(void) {
    TEST_ASSERT_NOT_NULL(bus.wram.mem);
}

void test_bus_init_allocates_vram(void) {
    TEST_ASSERT_NOT_NULL(bus.vram.mem);
}

void test_bus_init_allocates_hram(void) {
    TEST_ASSERT_NOT_NULL(bus.hram.mem);
}

// ---- bus_free ----

void test_bus_free_sets_wram_to_null(void) {
    bus_free(&bus);
    TEST_ASSERT_NULL(bus.wram.mem);
}

void test_bus_free_sets_vram_to_null(void) {
    bus_free(&bus);
    TEST_ASSERT_NULL(bus.vram.mem);
}

void test_bus_free_sets_hram_to_null(void) {
    bus_free(&bus);
    TEST_ASSERT_NULL(bus.hram.mem);
}

// ---- bus_read ----

void test_bus_read_rom_bank0(void) {
    fake_rom[0x0000] = 0xAB;
    TEST_ASSERT_EQUAL_UINT8(0xAB, bus_read(&bus, 0x0000));
}

void test_bus_read_rom_bank1(void) {
    fake_rom[0x4000] = 0xEF;
    TEST_ASSERT_EQUAL_UINT8(0xEF, bus_read(&bus, 0x4000));
}

void test_bus_read_vram(void) {
    bus.vram.mem[0x0000] = 0x55;
    TEST_ASSERT_EQUAL_UINT8(0x55, bus_read(&bus, 0x8000));
}

void test_bus_read_external_ram(void) {
    // Since external RAM is not implemented, it should return 0xFF
    // TODO: complete later when MBC support is added
    TEST_ASSERT_EQUAL_UINT8(0xFF, bus_read(&bus, 0xA000));
}

void test_bus_read_wram(void) {
    bus.wram.mem[0x0000] = 0x42;
    TEST_ASSERT_EQUAL_UINT8(0x42, bus_read(&bus, 0xC000));
}

void test_bus_read_echo_ram(void) {
    // Since echo RAM is not implemented, it should return 0xFF
    // TODO: complete later when echo RAM support is added
    TEST_ASSERT_EQUAL_UINT8(0xFF, bus_read(&bus, 0xE000));
}

void test_bus_read_oam(void) {
    // Since OAM is not implemented, it should return 0xFF
    // TODO: complete later when OAM support is added
    TEST_ASSERT_EQUAL_UINT8(0xFF, bus_read(&bus, 0xFE00));
}

void test_bus_read_not_usable(void) {
    // Since not usable area is not implemented, it should return 0xFF
    // TODO: complete later when not usable area support is added
    TEST_ASSERT_EQUAL_UINT8(0xFF, bus_read(&bus, 0xFEA0));
}

void test_bus_read_io_reg(void) {
    // Since I/O registers are not implemented, it should return 0xFF
    // TODO: complete later when I/O register support is added
    TEST_ASSERT_EQUAL_UINT8(0xFF, bus_read(&bus, 0xFF00));
}

void test_bus_read_hram(void) {
    bus.hram.mem[0x0000] = 0x77;
    TEST_ASSERT_EQUAL_UINT8(0x77, bus_read(&bus, 0xFF80));
}

void test_bus_read_interrupt_reg(void) {
    // Since interrupt register is not implemented, it should return 0xFF
    // TODO: complete later when interrupt register support is added
    TEST_ASSERT_EQUAL_UINT8(0xFF, bus_read(&bus, 0xFFFF));
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_bus_init_returns_0_on_success);
    RUN_TEST(test_bus_init_sets_cartridge_pointer);
    RUN_TEST(test_bus_init_allocates_wram);
    RUN_TEST(test_bus_init_allocates_vram);
    RUN_TEST(test_bus_init_allocates_hram);
    RUN_TEST(test_bus_free_sets_wram_to_null);
    RUN_TEST(test_bus_free_sets_vram_to_null);
    RUN_TEST(test_bus_free_sets_hram_to_null);
    RUN_TEST(test_bus_read_rom_bank0);
    RUN_TEST(test_bus_read_rom_bank1);
    RUN_TEST(test_bus_read_vram);
    RUN_TEST(test_bus_read_external_ram);
    RUN_TEST(test_bus_read_wram);
    RUN_TEST(test_bus_read_echo_ram);
    RUN_TEST(test_bus_read_oam);
    RUN_TEST(test_bus_read_not_usable);
    RUN_TEST(test_bus_read_io_reg);
    RUN_TEST(test_bus_read_hram);
    RUN_TEST(test_bus_read_interrupt_reg);

    return UNITY_END();
}
