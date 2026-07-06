#include "unity.h"
#include "log_helpers.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "memory/cartridge.h"

#define DUMMY_ROM_PATH "/tmp/test_rom.gb"
#define DUMMY_ROM_SIZE 512

static cartridge_t cartridge;

// ---- Mock mem_init ----

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

    return call->return_value;
}

// ---- Mock mem_free ----

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

// ---- Mock MBC ops ----

typedef struct {
    cartridge_t *cartridge;
} mbc_init_call_t;

typedef struct {
    size_t call_count;
    mbc_init_call_t calls[10];
} mbc_init_stats_t;

static mbc_init_stats_t mbc_init_stats;

static void mock_mbc_init(cartridge_t *cartridge) {
    if (mbc_init_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for mbc_init_stats");
    }

    mbc_init_stats.calls[mbc_init_stats.call_count].cartridge = cartridge;
    mbc_init_stats.call_count++;
}

typedef struct {
    cartridge_t *cartridge;
    uint16_t addr;
} mbc_read_call_t;

typedef struct {
    size_t call_count;
    uint8_t return_value;
    mbc_read_call_t calls[10];
} mbc_read_stats_t;

typedef struct {
    cartridge_t *cartridge;
    uint16_t addr;
    uint8_t value;
} mbc_write_call_t;

typedef struct {
    size_t call_count;
    mbc_write_call_t calls[10];
} mbc_write_stats_t;

static mbc_read_stats_t mbc_rom_read_stats;
static mbc_write_stats_t mbc_rom_write_stats;
static mbc_read_stats_t mbc_ram_read_stats;
static mbc_write_stats_t mbc_ram_write_stats;

static uint8_t record_read(mbc_read_stats_t *stats, cartridge_t *c, uint16_t a) {
    if (stats->call_count == 10) {
        assert(0 && "Exceeded maximum call count for mbc read stats");
    }

    mbc_read_call_t *call = &stats->calls[stats->call_count];
    call->cartridge = c;
    call->addr = a;

    stats->call_count++;

    return stats->return_value;
}

static void record_write(mbc_write_stats_t *stats, cartridge_t *c, uint16_t a, uint8_t v) {
    if (stats->call_count == 10) {
        assert(0 && "Exceeded maximum call count for mbc write stats");
    }

    mbc_write_call_t *call = &stats->calls[stats->call_count];
    call->cartridge = c;
    call->addr = a;
    call->value = v;

    stats->call_count++;
}

static uint8_t mock_mbc_rom_read(cartridge_t *c, uint16_t a) {
    return record_read(&mbc_rom_read_stats, c, a);
}

static void mock_mbc_rom_write(cartridge_t *c, uint16_t a, uint8_t v) {
    record_write(&mbc_rom_write_stats, c, a, v);
}

static uint8_t mock_mbc_ram_read(cartridge_t *c, uint16_t a) {
    return record_read(&mbc_ram_read_stats, c, a);
}

static void mock_mbc_ram_write(cartridge_t *c, uint16_t a, uint8_t v) {
    record_write(&mbc_ram_write_stats, c, a, v);
}

#define MOCK_OPS { \
    .init = mock_mbc_init, \
    .rom_read = mock_mbc_rom_read, \
    .rom_write = mock_mbc_rom_write, \
    .ram_read = mock_mbc_ram_read, \
    .ram_write = mock_mbc_ram_write, \
}

const mbc_interface_t no_mbc_ops = MOCK_OPS;
const mbc_interface_t mbc1_ops = MOCK_OPS;
const mbc_interface_t mbc2_ops = MOCK_OPS;
const mbc_interface_t mbc5_ops = MOCK_OPS;

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
    mbc_init_stats = (mbc_init_stats_t){ 0 };
    mbc_rom_read_stats = (mbc_read_stats_t){ 0 };
    mbc_rom_write_stats = (mbc_write_stats_t){ 0 };
    mbc_ram_read_stats = (mbc_read_stats_t){ 0 };
    mbc_ram_write_stats = (mbc_write_stats_t){ 0 };

    uint8_t rom[DUMMY_ROM_SIZE];
    make_valid_rom(rom, 0x00, 0x00, 0x00);
    write_rom_file(rom, DUMMY_ROM_SIZE);
}

void tearDown(void) {
    cartridge_unload(&cartridge);
    remove(DUMMY_ROM_PATH);

    restore_logs();
}

// ---- cartridge_load: file handling ----

void test_cartridge_load_returns_0_on_valid_rom(void) {
    int result = cartridge_load(&cartridge, DUMMY_ROM_PATH);
    TEST_ASSERT_EQUAL_INT(0, result);
}

void test_cartridge_load_returns_minus1_on_missing_file(void) {
    int result = cartridge_load(&cartridge, "/nonexistent/path/rom.gb");
    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_cartridge_load_sets_size(void) {
    cartridge_load(&cartridge, DUMMY_ROM_PATH);
    TEST_ASSERT_EQUAL_size_t(DUMMY_ROM_SIZE, cartridge.size);
}

void test_cartridge_load_sets_rom_not_null(void) {
    cartridge_load(&cartridge, DUMMY_ROM_PATH);
    TEST_ASSERT_NOT_NULL(cartridge.rom);
}

void test_cartridge_load_copies_rom_content(void) {
    uint8_t rom[DUMMY_ROM_SIZE];
    make_valid_rom(rom, 0x00, 0x00, 0x00);
    rom[0x0200] = 0xAB;
    rom[0x0201] = 0xCD;
    write_rom_file(rom, DUMMY_ROM_SIZE);

    cartridge_load(&cartridge, DUMMY_ROM_PATH);

    TEST_ASSERT_EQUAL_MEMORY(rom, cartridge.rom, DUMMY_ROM_SIZE);
}

// ---- cartridge_load: cartridge type ----

void test_cartridge_load_returns_minus1_on_unsupported_type(void) {
    uint8_t rom[DUMMY_ROM_SIZE];
    make_valid_rom(rom, 0x08, 0x00, 0x00); // ROM+RAM — no ops
    write_rom_file(rom, DUMMY_ROM_SIZE);

    int result = cartridge_load(&cartridge, DUMMY_ROM_PATH);
    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_cartridge_load_returns_minus1_on_unknown_type(void) {
    uint8_t rom[DUMMY_ROM_SIZE];
    make_valid_rom(rom, 0x04, 0x00, 0x00); // not in table
    write_rom_file(rom, DUMMY_ROM_SIZE);

    int result = cartridge_load(&cartridge, DUMMY_ROM_PATH);
    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_cartridge_load_sets_mbc(void) {
    cartridge_load(&cartridge, DUMMY_ROM_PATH);
    TEST_ASSERT_EQUAL_PTR(&no_mbc_ops, cartridge.mbc);
}

void test_cartridge_load_sets_type_flags_and_name(void) {
    uint8_t rom[DUMMY_ROM_SIZE];
    make_valid_rom(rom, 0x03, 0x00, 0x00); // MBC1+RAM+BATTERY
    write_rom_file(rom, DUMMY_ROM_SIZE);

    cartridge_load(&cartridge, DUMMY_ROM_PATH);

    TEST_ASSERT_EQUAL_PTR(&mbc1_ops, cartridge.mbc);
    TEST_ASSERT_TRUE(cartridge.has_ram);
    TEST_ASSERT_TRUE(cartridge.has_battery);
    TEST_ASSERT_EQUAL_STRING("MBC1+RAM+BATTERY", cartridge.cartridge_type);
}

void test_cartridge_load_calls_mbc_init(void) {
    cartridge_load(&cartridge, DUMMY_ROM_PATH);

    TEST_ASSERT_EQUAL_size_t(1, mbc_init_stats.call_count);
    TEST_ASSERT_EQUAL_PTR(&cartridge, mbc_init_stats.calls[0].cartridge);
}

// ---- cartridge_load: header fields ----

void test_cartridge_load_sets_title(void) {
    uint8_t rom[DUMMY_ROM_SIZE];
    make_valid_rom(rom, 0x00, 0x00, 0x00);
    memcpy(rom + 0x0134, "TESTROM         ", 16);
    set_checksum(rom);
    write_rom_file(rom, DUMMY_ROM_SIZE);

    cartridge_load(&cartridge, DUMMY_ROM_PATH);

    TEST_ASSERT_EQUAL_MEMORY("TESTROM         ", cartridge.title, 16);
}

void test_cartridge_load_returns_minus1_on_unknown_rom_size(void) {
    uint8_t rom[DUMMY_ROM_SIZE];
    make_valid_rom(rom, 0x00, 9, 0x00);
    write_rom_file(rom, DUMMY_ROM_SIZE);

    int result = cartridge_load(&cartridge, DUMMY_ROM_PATH);
    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_cartridge_load_sets_banks_number(void) {
    uint8_t rom[DUMMY_ROM_SIZE];
    make_valid_rom(rom, 0x00, 0x01, 0x00);
    write_rom_file(rom, DUMMY_ROM_SIZE);

    cartridge_load(&cartridge, DUMMY_ROM_PATH);

    TEST_ASSERT_EQUAL_UINT16(4, cartridge.banks_number);
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

void test_cartridge_load_returns_minus1_on_wrong_checksum(void) {
    uint8_t rom[DUMMY_ROM_SIZE];
    make_valid_rom(rom, 0x00, 0x00, 0x00);
    rom[0x014D] ^= 0xFF;
    write_rom_file(rom, DUMMY_ROM_SIZE);

    int result = cartridge_load(&cartridge, DUMMY_ROM_PATH);
    TEST_ASSERT_EQUAL_INT(-1, result);
}

// ---- cartridge_unload ----

void test_cartridge_unload_frees_rom_and_ram(void) {
    cartridge_load(&cartridge, DUMMY_ROM_PATH);

    cartridge_unload(&cartridge);

    TEST_ASSERT_NULL(cartridge.rom);
    TEST_ASSERT_EQUAL_size_t(0, cartridge.size);
    TEST_ASSERT_EQUAL_size_t(1, mem_free_stats.call_count);
    TEST_ASSERT_EQUAL_PTR(&cartridge.ram, mem_free_stats.calls[0].memory);
}

void test_cartridge_unload_handles_null_cartridge(void) {
    cartridge_unload(NULL);

    TEST_ASSERT_EQUAL_size_t(0, mem_free_stats.call_count);
}

// ---- cartridge_load: external RAM ----

void test_cartridge_load_calls_mem_init_for_ext_ram(void) {
    uint8_t rom[DUMMY_ROM_SIZE];
    make_valid_rom(rom, 0x03, 0x00, 0x02); // MBC1+RAM+BATTERY, 8KB RAM
    write_rom_file(rom, DUMMY_ROM_SIZE);

    cartridge_load(&cartridge, DUMMY_ROM_PATH);

    TEST_ASSERT_EQUAL_size_t(1, mem_init_stats.call_count);
    TEST_ASSERT_EQUAL_PTR(&cartridge.ram, mem_init_stats.calls[0].memory);
    TEST_ASSERT_EQUAL_size_t(8 * 1024, mem_init_stats.calls[0].size);
    TEST_ASSERT_EQUAL_STRING("External RAM", mem_init_stats.calls[0].name);
}

void test_cartridge_load_no_mem_init_when_no_ext_ram(void) {
    cartridge_load(&cartridge, DUMMY_ROM_PATH);
    TEST_ASSERT_EQUAL_size_t(0, mem_init_stats.call_count);
}

void test_cartridge_load_mbc2_forces_512_byte_ram(void) {
    uint8_t rom[DUMMY_ROM_SIZE];
    make_valid_rom(rom, 0x05, 0x00, 0x00); // MBC2, RAM size code 0
    write_rom_file(rom, DUMMY_ROM_SIZE);

    cartridge_load(&cartridge, DUMMY_ROM_PATH);

    TEST_ASSERT_EQUAL_size_t(1, mem_init_stats.call_count);
    TEST_ASSERT_EQUAL_size_t(512, mem_init_stats.calls[0].size);
}

void test_cartridge_load_returns_minus1_on_ext_ram_init_failure(void) {
    uint8_t rom[DUMMY_ROM_SIZE];
    make_valid_rom(rom, 0x03, 0x00, 0x02);
    write_rom_file(rom, DUMMY_ROM_SIZE);

    mem_init_stats.calls[0].return_value = -1;

    int result = cartridge_load(&cartridge, DUMMY_ROM_PATH);

    TEST_ASSERT_EQUAL_INT(-1, result);
    TEST_ASSERT_EQUAL_size_t(1, mem_init_stats.call_count);
}

// ---- cartridge_rom_read ----

void test_cartridge_rom_read_dispatches_to_mbc(void) {
    cartridge.mbc = &no_mbc_ops;

    cartridge_rom_read(&cartridge, 0x1234);

    TEST_ASSERT_EQUAL_size_t(1, mbc_rom_read_stats.call_count);
    TEST_ASSERT_EQUAL_PTR(&cartridge, mbc_rom_read_stats.calls[0].cartridge);
    TEST_ASSERT_EQUAL_HEX16(0x1234, mbc_rom_read_stats.calls[0].addr);
}

void test_cartridge_rom_read_returns_mbc_value(void) {
    cartridge.mbc = &no_mbc_ops;
    mbc_rom_read_stats.return_value = 0xA5;

    uint8_t value = cartridge_rom_read(&cartridge, 0x0000);

    TEST_ASSERT_EQUAL_HEX8(0xA5, value);
}

// ---- cartridge_rom_write ----

void test_cartridge_rom_write_dispatches_to_mbc(void) {
    cartridge.mbc = &no_mbc_ops;

    cartridge_rom_write(&cartridge, 0x2000, 0x0A);

    TEST_ASSERT_EQUAL_size_t(1, mbc_rom_write_stats.call_count);
    TEST_ASSERT_EQUAL_PTR(&cartridge, mbc_rom_write_stats.calls[0].cartridge);
    TEST_ASSERT_EQUAL_HEX16(0x2000, mbc_rom_write_stats.calls[0].addr);
    TEST_ASSERT_EQUAL_HEX8(0x0A, mbc_rom_write_stats.calls[0].value);
}

// ---- cartridge_ext_ram_read ----

void test_cartridge_ext_ram_read_dispatches_to_mbc(void) {
    cartridge.mbc = &no_mbc_ops;

    cartridge_ext_ram_read(&cartridge, 0xA100);

    TEST_ASSERT_EQUAL_size_t(1, mbc_ram_read_stats.call_count);
    TEST_ASSERT_EQUAL_PTR(&cartridge, mbc_ram_read_stats.calls[0].cartridge);
    TEST_ASSERT_EQUAL_HEX16(0xA100, mbc_ram_read_stats.calls[0].addr);
}

void test_cartridge_ext_ram_read_returns_mbc_value(void) {
    cartridge.mbc = &no_mbc_ops;
    mbc_ram_read_stats.return_value = 0x5A;

    uint8_t value = cartridge_ext_ram_read(&cartridge, 0xA000);

    TEST_ASSERT_EQUAL_HEX8(0x5A, value);
}

// ---- cartridge_ext_ram_write ----

void test_cartridge_ext_ram_write_dispatches_to_mbc(void) {
    cartridge.mbc = &no_mbc_ops;

    cartridge_ext_ram_write(&cartridge, 0xA200, 0x3C);

    TEST_ASSERT_EQUAL_size_t(1, mbc_ram_write_stats.call_count);
    TEST_ASSERT_EQUAL_PTR(&cartridge, mbc_ram_write_stats.calls[0].cartridge);
    TEST_ASSERT_EQUAL_HEX16(0xA200, mbc_ram_write_stats.calls[0].addr);
    TEST_ASSERT_EQUAL_HEX8(0x3C, mbc_ram_write_stats.calls[0].value);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_cartridge_load_returns_0_on_valid_rom);
    RUN_TEST(test_cartridge_load_returns_minus1_on_missing_file);
    RUN_TEST(test_cartridge_load_sets_size);
    RUN_TEST(test_cartridge_load_sets_rom_not_null);
    RUN_TEST(test_cartridge_load_copies_rom_content);

    RUN_TEST(test_cartridge_load_returns_minus1_on_unsupported_type);
    RUN_TEST(test_cartridge_load_returns_minus1_on_unknown_type);
    RUN_TEST(test_cartridge_load_sets_mbc);
    RUN_TEST(test_cartridge_load_sets_type_flags_and_name);
    RUN_TEST(test_cartridge_load_calls_mbc_init);

    RUN_TEST(test_cartridge_load_sets_title);
    RUN_TEST(test_cartridge_load_returns_minus1_on_unknown_rom_size);
    RUN_TEST(test_cartridge_load_sets_banks_number);
    RUN_TEST(test_cartridge_load_sets_version);
    RUN_TEST(test_cartridge_load_returns_minus1_on_wrong_checksum);

    RUN_TEST(test_cartridge_load_calls_mem_init_for_ext_ram);
    RUN_TEST(test_cartridge_load_no_mem_init_when_no_ext_ram);
    RUN_TEST(test_cartridge_load_mbc2_forces_512_byte_ram);
    RUN_TEST(test_cartridge_load_returns_minus1_on_ext_ram_init_failure);

    RUN_TEST(test_cartridge_unload_frees_rom_and_ram);
    RUN_TEST(test_cartridge_unload_handles_null_cartridge);

    RUN_TEST(test_cartridge_rom_read_dispatches_to_mbc);
    RUN_TEST(test_cartridge_rom_read_returns_mbc_value);
    RUN_TEST(test_cartridge_rom_write_dispatches_to_mbc);
    RUN_TEST(test_cartridge_ext_ram_read_dispatches_to_mbc);
    RUN_TEST(test_cartridge_ext_ram_read_returns_mbc_value);
    RUN_TEST(test_cartridge_ext_ram_write_dispatches_to_mbc);

    return UNITY_END();
}
