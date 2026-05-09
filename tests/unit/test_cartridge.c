#include "unity.h"
#include "log_helpers.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "cartridge.h"

#define DUMMY_ROM_PATH "/tmp/test_rom.gb"
#define DUMMY_ROM_SIZE 512

static cartridge_t cartridge;

// ---- Mock functions ----

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

typedef struct {
    mem_t *memory;
} mem_free_call_t;

typedef struct {
    size_t call_count;
    mem_free_call_t calls[10];
} mem_free_stats_t;

static mem_free_stats_t mem_free_stats;

void mem_free(mem_t *memory) {
    if (mem_free_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for mem_free_stats");
    }

    mem_free_stats.calls[mem_free_stats.call_count].memory = memory;
    mem_free_stats.call_count++;
}

// ---- ROM helpers ----

static void set_checksum(uint8_t *rom) {
    uint8_t checksum = 0;
    for (uint16_t addr = 0x0134; addr <= 0x014C; addr++) {
        checksum = checksum - rom[addr] - 1;
    }
    rom[0x014D] = checksum;
}

static void make_valid_rom(uint8_t *rom, uint8_t mbc_code, uint8_t rom_size_code, uint8_t ram_size_code) {
    memset(rom, 0, DUMMY_ROM_SIZE);
    rom[0x0147] = mbc_code;
    rom[0x0148] = rom_size_code;
    rom[0x0149] = ram_size_code;
    set_checksum(rom);
}

static void write_rom_file(const uint8_t *data, size_t size) {
    FILE *ptr = fopen(DUMMY_ROM_PATH, "wb");
    fwrite(data, 1, size, ptr);
    fclose(ptr);
}

void setUp(void) {
    suppress_logs();
    memset(&cartridge, 0, sizeof(cartridge));

    mem_init_stats = (mem_init_stats_t){ 0 };
    mem_free_stats = (mem_free_stats_t){ 0 };

    uint8_t rom[DUMMY_ROM_SIZE];
    make_valid_rom(rom, 0x00, 0x00, 0x00);
    write_rom_file(rom, DUMMY_ROM_SIZE);
}

void tearDown(void) {
    cartridge_unload(&cartridge);
    remove(DUMMY_ROM_PATH);

    restore_logs();
}

// ---- cartridge_load ----

void test_cartridge_load_returns_0_on_valid_rom(void) {
    int result = cartridge_load(&cartridge, DUMMY_ROM_PATH);
    TEST_ASSERT_EQUAL_INT(0, result);
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
    uint8_t rom[DUMMY_ROM_SIZE];
    make_valid_rom(rom, 0x00, 0x00, 0x00);
    rom[0x0150] = 0xAB;
    rom[0x0151] = 0xCD;
    write_rom_file(rom, DUMMY_ROM_SIZE);

    cartridge_load(&cartridge, DUMMY_ROM_PATH);
    TEST_ASSERT_EQUAL_MEMORY(rom, cartridge.rom, DUMMY_ROM_SIZE);
}

void test_cartridge_load_calls_mem_init_for_ext_ram(void) {
    uint8_t rom[DUMMY_ROM_SIZE];
    make_valid_rom(rom, 0x00, 0x00, 0x02);
    write_rom_file(rom, DUMMY_ROM_SIZE);

    cartridge_load(&cartridge, DUMMY_ROM_PATH);

    TEST_ASSERT_EQUAL_size_t(1, mem_init_stats.call_count);
    TEST_ASSERT_EQUAL_PTR(&cartridge.ext_ram, mem_init_stats.calls[0].memory);
    TEST_ASSERT_EQUAL_size_t(8 * 1024, mem_init_stats.calls[0].size);
    TEST_ASSERT_EQUAL_STRING("External RAM", mem_init_stats.calls[0].name);
}

void test_cartridge_load_does_not_call_mem_init_when_no_ext_ram(void) {
    cartridge_load(&cartridge, DUMMY_ROM_PATH);
    TEST_ASSERT_EQUAL_size_t(0, mem_init_stats.call_count);
}

void test_cartridge_load_returns_minus1_on_ext_ram_init_failure(void) {
    uint8_t rom[DUMMY_ROM_SIZE];
    make_valid_rom(rom, 0x00, 0x00, 0x02);
    write_rom_file(rom, DUMMY_ROM_SIZE);

    mem_init_stats.calls[0].return_value = -1;

    int result = cartridge_load(&cartridge, DUMMY_ROM_PATH);

    TEST_ASSERT_EQUAL_INT(-1, result);
    TEST_ASSERT_EQUAL_size_t(1, mem_init_stats.call_count);
}

void test_cartridge_load_returns_minus1_on_unknown_mbc(void) {
    uint8_t rom[DUMMY_ROM_SIZE];
    make_valid_rom(rom, 0x04, 0x00, 0x00);
    write_rom_file(rom, DUMMY_ROM_SIZE);

    int result = cartridge_load(&cartridge, DUMMY_ROM_PATH);
    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_cartridge_load_returns_minus1_on_unknown_rom_size(void) {
    uint8_t rom[DUMMY_ROM_SIZE];
    make_valid_rom(rom, 0x00, 9, 0x00);
    write_rom_file(rom, DUMMY_ROM_SIZE);

    int result = cartridge_load(&cartridge, DUMMY_ROM_PATH);
    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_cartridge_load_returns_minus1_on_wrong_checksum(void) {
    uint8_t rom[DUMMY_ROM_SIZE];
    make_valid_rom(rom, 0x00, 0x00, 0x00);
    rom[0x014D] ^= 0xFF;
    write_rom_file(rom, DUMMY_ROM_SIZE);

    int result = cartridge_load(&cartridge, DUMMY_ROM_PATH);
    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_cartridge_load_sets_mbc(void) {
    uint8_t rom[DUMMY_ROM_SIZE];
    make_valid_rom(rom, 0x01, 0x00, 0x00);
    write_rom_file(rom, DUMMY_ROM_SIZE);

    cartridge_load(&cartridge, DUMMY_ROM_PATH);

    TEST_ASSERT_EQUAL_UINT8(0x01, cartridge.mbc.code);
    TEST_ASSERT_EQUAL_STRING("MBC1", cartridge.mbc.name);
}

void test_cartridge_load_sets_banks_number(void) {
    uint8_t rom[DUMMY_ROM_SIZE];
    make_valid_rom(rom, 0x00, 0x01, 0x00);
    write_rom_file(rom, DUMMY_ROM_SIZE);

    cartridge_load(&cartridge, DUMMY_ROM_PATH);

    TEST_ASSERT_EQUAL_UINT8(4, cartridge.banks_number);
}

void test_cartridge_load_sets_title(void) {
    uint8_t rom[DUMMY_ROM_SIZE];
    make_valid_rom(rom, 0x00, 0x00, 0x00);
    memcpy(rom + 0x0134, "TESTROM         ", 16);
    set_checksum(rom);
    write_rom_file(rom, DUMMY_ROM_SIZE);

    cartridge_load(&cartridge, DUMMY_ROM_PATH);

    TEST_ASSERT_EQUAL_MEMORY("TESTROM         ", cartridge.title, 16);
}

void test_cartridge_load_sets_version(void) {
    uint8_t rom[DUMMY_ROM_SIZE];
    make_valid_rom(rom, 0x00, 0x00, 0x00);
    rom[0x014C] = 0x03;
    set_checksum(rom);
    write_rom_file(rom, DUMMY_ROM_SIZE);

    cartridge_load(&cartridge, DUMMY_ROM_PATH);

    TEST_ASSERT_EQUAL_UINT8(0x03, cartridge.version);
}

// ---- cartridge_unload ----

void test_cartridge_unload_frees_rom_memory(void) {
    cartridge_load(&cartridge, DUMMY_ROM_PATH);
    cartridge_unload(&cartridge);
    TEST_ASSERT_NULL(cartridge.rom);
    TEST_ASSERT_EQUAL_size_t(0, cartridge.size);
    TEST_ASSERT_EQUAL_UINT8(0, cartridge.bank);
}

void test_cartridge_unload_calls_mem_free_for_ext_ram(void) {
    cartridge_load(&cartridge, DUMMY_ROM_PATH);
    cartridge_unload(&cartridge);

    TEST_ASSERT_EQUAL_size_t(1, mem_free_stats.call_count);
    TEST_ASSERT_EQUAL_PTR(&cartridge.ext_ram, mem_free_stats.calls[0].memory);
}

void test_cartridge_unload_does_not_crash_on_null_pointer(void) {
    cartridge_unload(NULL);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_cartridge_load_returns_0_on_valid_rom);
    RUN_TEST(test_cartridge_load_returns_minus1_on_invalid_file);
    RUN_TEST(test_cartridge_load_sets_correct_size);
    RUN_TEST(test_cartridge_load_sets_bank_to_1);
    RUN_TEST(test_cartridge_load_rom_pointer_not_null);
    RUN_TEST(test_cartridge_load_rom_content_is_correct);
    RUN_TEST(test_cartridge_load_calls_mem_init_for_ext_ram);
    RUN_TEST(test_cartridge_load_does_not_call_mem_init_when_no_ext_ram);
    RUN_TEST(test_cartridge_load_returns_minus1_on_ext_ram_init_failure);
    RUN_TEST(test_cartridge_load_returns_minus1_on_unknown_mbc);
    RUN_TEST(test_cartridge_load_returns_minus1_on_unknown_rom_size);
    RUN_TEST(test_cartridge_load_returns_minus1_on_wrong_checksum);
    RUN_TEST(test_cartridge_load_sets_mbc);
    RUN_TEST(test_cartridge_load_sets_banks_number);
    RUN_TEST(test_cartridge_load_sets_title);
    RUN_TEST(test_cartridge_load_sets_version);
    RUN_TEST(test_cartridge_unload_frees_rom_memory);
    RUN_TEST(test_cartridge_unload_calls_mem_free_for_ext_ram);
    RUN_TEST(test_cartridge_unload_does_not_crash_on_null_pointer);

    return UNITY_END();
}
