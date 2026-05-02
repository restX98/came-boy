#include "unity.h"
#include "test_helpers.h"

#include <assert.h>
#include <string.h>

#include "bus.h"

#define FAKE_ROM_SIZE 0x8000 // 32 KiB, covers ROM bank 0 and 1

static bus_t bus;
static cartridge_t cartridge;
static uint8_t fake_rom[FAKE_ROM_SIZE];

// ---- Mock functions ----

// Mock mem_init
typedef struct {
    mem_t *memory;
    size_t size;
    const char *name;
    int return_value;
} mem_init_call_t;

typedef struct {
    size_t call_count;
    mem_init_call_t calls[10];
} mem_init_stats_t;

static mem_init_stats_t mem_init_stats;

int mem_init(mem_t *memory, size_t size, const char *name) {
    if (mem_init_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for mem_init_stats");
    }

    mem_init_call_t *call = &mem_init_stats.calls[mem_init_stats.call_count];
    call->memory = memory;
    call->size = size;
    call->name = name;

    mem_init_stats.call_count++;

    return mem_init_stats.calls[mem_init_stats.call_count - 1].return_value;
}

// Mock mem_free
typedef struct {
    mem_t *memory;
} mem_free_call_t;

typedef struct {
    size_t call_count;
    mem_free_call_t calls[10];
} mem_free_stats_t;

static mem_free_stats_t mem_free_stats = {
    .call_count = 0
};

void mem_free(mem_t *memory) {
    if (mem_free_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for mem_free_stats");
    }

    mem_free_stats.calls[mem_free_stats.call_count].memory = memory;
    mem_free_stats.call_count++;
}

void setUp(void) {
    suppress_logs();

    memset(fake_rom, 0, sizeof(fake_rom));
    bus = (bus_t){ 0 };
    cartridge = (cartridge_t){ .rom = fake_rom, .size = FAKE_ROM_SIZE, .bank = 1 };

    mem_init_stats = (mem_init_stats_t){ 0 };
    mem_free_stats = (mem_free_stats_t){ 0 };
}

void tearDown(void) {
    restore_logs();
}

// ---- bus_init ----

void test_bus_init_returns_0_on_success(void) {
    int result = bus_init(&bus, &cartridge);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_INT(3, mem_init_stats.call_count);
}

void test_bus_init_sets_cartridge_pointer(void) {
    bus_init(&bus, &cartridge);
    TEST_ASSERT_EQUAL_PTR(&cartridge, bus.cartridge);
}

void test_bus_init_calls_mem_init_to_initialize_wram(void) {
    bus_init(&bus, &cartridge);

    TEST_ASSERT_EQUAL_PTR(&bus.wram, mem_init_stats.calls[0].memory);
    TEST_ASSERT_EQUAL_size_t(WRAM_SIZE, mem_init_stats.calls[0].size);
    TEST_ASSERT_EQUAL_STRING("WRAM", mem_init_stats.calls[0].name);
}

void test_bus_init_calls_mem_init_to_initialize_vram(void) {
    bus_init(&bus, &cartridge);
    TEST_ASSERT_EQUAL_PTR(&bus.vram, mem_init_stats.calls[1].memory);
    TEST_ASSERT_EQUAL_size_t(VRAM_SIZE, mem_init_stats.calls[1].size);
    TEST_ASSERT_EQUAL_STRING("VRAM", mem_init_stats.calls[1].name);
}

void test_bus_init_calls_mem_init_to_initialize_hram(void) {
    bus_init(&bus, &cartridge);
    TEST_ASSERT_EQUAL_PTR(&bus.hram, mem_init_stats.calls[2].memory);
    TEST_ASSERT_EQUAL_size_t(HRAM_SIZE, mem_init_stats.calls[2].size);
    TEST_ASSERT_EQUAL_STRING("HRAM", mem_init_stats.calls[2].name);
}

void test_bus_init_returns_minus1_on_wram_allocation_failure(void) {
    mem_init_stats.calls[0].return_value = -1;

    int result = bus_init(&bus, &cartridge);

    TEST_ASSERT_EQUAL_size_t(1, mem_init_stats.call_count);
    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_bus_init_returns_minus1_on_vram_allocation_failure(void) {
    mem_init_stats.calls[1].return_value = -1;

    int result = bus_init(&bus, &cartridge);

    TEST_ASSERT_EQUAL_size_t(2, mem_init_stats.call_count);
    TEST_ASSERT_EQUAL_INT(-1, result);

    TEST_ASSERT_EQUAL_size_t(1, mem_free_stats.call_count);
    TEST_ASSERT_EQUAL_PTR(&bus.wram, mem_free_stats.calls[0].memory);
}

void test_bus_init_returns_minus1_on_hram_allocation_failure(void) {
    mem_init_stats.calls[2].return_value = -1;

    int result = bus_init(&bus, &cartridge);

    TEST_ASSERT_EQUAL_size_t(3, mem_init_stats.call_count);
    TEST_ASSERT_EQUAL_INT(-1, result);

    TEST_ASSERT_EQUAL_size_t(2, mem_free_stats.call_count);
    TEST_ASSERT_EQUAL_PTR(&bus.wram, mem_free_stats.calls[0].memory);
    TEST_ASSERT_EQUAL_PTR(&bus.vram, mem_free_stats.calls[1].memory);
}

// ---- bus_free ----

void test_bus_free_calls_mem_free_for_wram(void) {
    bus_free(&bus);
    TEST_ASSERT_EQUAL_PTR(&bus.wram, mem_free_stats.calls[0].memory);
}

void test_bus_free_calls_mem_free_for_vram(void) {
    bus_free(&bus);
    TEST_ASSERT_EQUAL_PTR(&bus.vram, mem_free_stats.calls[1].memory);
}

void test_bus_free_calls_mem_free_for_hram(void) {
    bus_free(&bus);
    TEST_ASSERT_EQUAL_PTR(&bus.hram, mem_free_stats.calls[2].memory);
}

// ---- bus_read ----

void test_bus_read_rom_bank0(void) {
    bus.cartridge = &cartridge;
    cartridge.rom[0x0000] = 0xAB;
    TEST_ASSERT_EQUAL_UINT8(0xAB, bus_read(&bus, 0x0000));
}

void test_bus_read_rom_bank1(void) {
    bus.cartridge = &cartridge;
    cartridge.rom[0x4000] = 0xEF;
    TEST_ASSERT_EQUAL_UINT8(0xEF, bus_read(&bus, 0x4000));
}

void test_bus_read_vram(void) {
    uint8_t memory[1] = { 0x55 };
    bus.vram.mem = memory;
    TEST_ASSERT_EQUAL_UINT8(0x55, bus_read(&bus, 0x8000));
}

void test_bus_read_external_ram(void) {
    // Since external RAM is not implemented, it should return 0xFF
    // TODO: complete later when MBC support is added
    TEST_ASSERT_EQUAL_UINT8(0xFF, bus_read(&bus, 0xA000));
}

void test_bus_read_wram(void) {
    uint8_t memory[1] = { 0x42 };
    bus.wram.mem = memory;
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
    uint8_t memory[1] = { 0x77 };
    bus.hram.mem = memory;
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
    RUN_TEST(test_bus_init_calls_mem_init_to_initialize_wram);
    RUN_TEST(test_bus_init_calls_mem_init_to_initialize_vram);
    RUN_TEST(test_bus_init_calls_mem_init_to_initialize_hram);
    RUN_TEST(test_bus_init_returns_minus1_on_wram_allocation_failure);
    RUN_TEST(test_bus_init_returns_minus1_on_vram_allocation_failure);
    RUN_TEST(test_bus_init_returns_minus1_on_hram_allocation_failure);
    RUN_TEST(test_bus_free_calls_mem_free_for_wram);
    RUN_TEST(test_bus_free_calls_mem_free_for_vram);
    RUN_TEST(test_bus_free_calls_mem_free_for_hram);
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
