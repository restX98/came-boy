#include "unity.h"
#include "log_helpers.h"

#include <assert.h>
#include <string.h>

#include "bus.h"

static bus_t bus;
static cartridge_t cartridge;

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

// Mock io_reg_init
typedef struct {
    io_reg_t *io_reg;
} io_reg_init_call_t;

typedef struct {
    size_t call_count;
    io_reg_init_call_t calls[10];
} io_reg_init_stats_t;

static io_reg_init_stats_t io_reg_init_stats;

void io_reg_init(io_reg_t *io_reg) {
    if (io_reg_init_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for io_reg_init_stats");
    }

    io_reg_init_call_t *call = &io_reg_init_stats.calls[io_reg_init_stats.call_count];
    call->io_reg = io_reg;

    io_reg_init_stats.call_count++;
}

// Mock io_reg_read
typedef struct {
    io_reg_t *io_reg;
    uint16_t addr;
    uint8_t return_value;
} io_reg_read_call_t;

typedef struct {
    size_t call_count;
    io_reg_read_call_t calls[10];
} io_reg_read_stats_t;

static io_reg_read_stats_t io_reg_read_stats;

uint8_t io_reg_read(io_reg_t *io_reg, uint16_t addr) {
    if (io_reg_read_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for io_reg_read_stats");
    }

    io_reg_read_call_t *call = &io_reg_read_stats.calls[io_reg_read_stats.call_count];
    call->io_reg = io_reg;
    call->addr = addr;

    io_reg_read_stats.call_count++;

    return io_reg_read_stats.calls[io_reg_read_stats.call_count - 1].return_value;
}

// Mock io_reg_write
typedef struct {
    io_reg_t *io_reg;
    uint16_t addr;
    uint8_t value;
} io_reg_write_call_t;

typedef struct {
    size_t call_count;
    io_reg_write_call_t calls[10];
} io_reg_write_stats_t;

static io_reg_write_stats_t io_reg_write_stats;

void io_reg_write(io_reg_t *io_reg, uint16_t addr, uint8_t value) {
    if (io_reg_write_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for io_reg_write_stats");
    }

    io_reg_write_call_t *call = &io_reg_write_stats.calls[io_reg_write_stats.call_count];
    call->io_reg = io_reg;
    call->addr = addr;
    call->value = value;

    io_reg_write_stats.call_count++;
}

// Mock cartridge_rom_read
typedef struct {
    cartridge_t *cartridge;
    uint16_t addr;
    uint8_t return_value;
} cartridge_rom_read_call_t;

typedef struct {
    size_t call_count;
    cartridge_rom_read_call_t calls[10];
} cartridge_rom_read_stats_t;

static cartridge_rom_read_stats_t cartridge_rom_read_stats;

uint8_t cartridge_rom_read(cartridge_t *cartridge, uint16_t addr) {
    if (cartridge_rom_read_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for cartridge_rom_read_stats");
    }

    cartridge_rom_read_call_t *call = &cartridge_rom_read_stats.calls[cartridge_rom_read_stats.call_count];
    call->cartridge = cartridge;
    call->addr = addr;

    cartridge_rom_read_stats.call_count++;

    return cartridge_rom_read_stats.calls[cartridge_rom_read_stats.call_count - 1].return_value;
}

// Mock cartridge_rom_write
typedef struct {
    cartridge_t *cartridge;
    uint16_t addr;
    uint8_t value;
} cartridge_rom_write_call_t;

typedef struct {
    size_t call_count;
    cartridge_rom_write_call_t calls[10];
} cartridge_rom_write_stats_t;

static cartridge_rom_write_stats_t cartridge_rom_write_stats;

void cartridge_rom_write(cartridge_t *cartridge, uint16_t addr, uint8_t value) {
    if (cartridge_rom_write_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for cartridge_rom_write_stats");
    }

    cartridge_rom_write_call_t *call = &cartridge_rom_write_stats.calls[cartridge_rom_write_stats.call_count];
    call->cartridge = cartridge;
    call->addr = addr;
    call->value = value;

    cartridge_rom_write_stats.call_count++;
}

// Mock cartridge_ext_ram_read
typedef struct {
    cartridge_t *cartridge;
    uint16_t addr;
    uint8_t return_value;
} cartridge_ext_ram_read_call_t;

typedef struct {
    size_t call_count;
    cartridge_ext_ram_read_call_t calls[10];
} cartridge_ext_ram_read_stats_t;

static cartridge_ext_ram_read_stats_t cartridge_ext_ram_read_stats;

uint8_t cartridge_ext_ram_read(cartridge_t *cartridge, uint16_t addr) {
    if (cartridge_ext_ram_read_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for cartridge_ext_ram_read_stats");
    }

    cartridge_ext_ram_read_call_t *call = &cartridge_ext_ram_read_stats.calls[cartridge_ext_ram_read_stats.call_count];
    call->cartridge = cartridge;
    call->addr = addr;

    cartridge_ext_ram_read_stats.call_count++;

    return cartridge_ext_ram_read_stats.calls[cartridge_ext_ram_read_stats.call_count - 1].return_value;
}

// Mock cartridge_ext_ram_write
typedef struct {
    cartridge_t *cartridge;
    uint16_t addr;
    uint8_t value;
} cartridge_ext_ram_write_call_t;

typedef struct {
    size_t call_count;
    cartridge_ext_ram_write_call_t calls[10];
} cartridge_ext_ram_write_stats_t;

static cartridge_ext_ram_write_stats_t cartridge_ext_ram_write_stats;

void cartridge_ext_ram_write(cartridge_t *cartridge, uint16_t addr, uint8_t value) {
    if (cartridge_ext_ram_write_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for cartridge_ext_ram_write_stats");
    }

    cartridge_ext_ram_write_call_t *call = &cartridge_ext_ram_write_stats.calls[cartridge_ext_ram_write_stats.call_count];
    call->cartridge = cartridge;
    call->addr = addr;
    call->value = value;

    cartridge_ext_ram_write_stats.call_count++;
}

void setUp(void) {
    suppress_logs();

    bus = (bus_t){ 0 };
    bus.vram_accessible = true;
    bus.oam_accessible = true;
    cartridge = (cartridge_t){ 0 };

    mem_init_stats = (mem_init_stats_t){ 0 };
    mem_free_stats = (mem_free_stats_t){ 0 };
    io_reg_init_stats = (io_reg_init_stats_t){ 0 };
    io_reg_read_stats = (io_reg_read_stats_t){ 0 };
    io_reg_write_stats = (io_reg_write_stats_t){ 0 };
    cartridge_rom_read_stats = (cartridge_rom_read_stats_t){ 0 };
    cartridge_rom_write_stats = (cartridge_rom_write_stats_t){ 0 };
    cartridge_ext_ram_read_stats = (cartridge_ext_ram_read_stats_t){ 0 };
    cartridge_ext_ram_write_stats = (cartridge_ext_ram_write_stats_t){ 0 };
}

void tearDown(void) {
    restore_logs();
}

// ---- bus_init ----

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

void test_bus_init_calls_mem_init_to_initialize_oam(void) {
    bus_init(&bus, &cartridge);
    TEST_ASSERT_EQUAL_PTR(&bus.oam, mem_init_stats.calls[3].memory);
    TEST_ASSERT_EQUAL_size_t(OAM_SIZE, mem_init_stats.calls[3].size);
    TEST_ASSERT_EQUAL_STRING("OAM", mem_init_stats.calls[3].name);
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

void test_bus_init_returns_minus1_on_oam_allocation_failure(void) {
    mem_init_stats.calls[3].return_value = -1;

    int result = bus_init(&bus, &cartridge);

    TEST_ASSERT_EQUAL_size_t(4, mem_init_stats.call_count);
    TEST_ASSERT_EQUAL_INT(-1, result);

    TEST_ASSERT_EQUAL_size_t(3, mem_free_stats.call_count);
    TEST_ASSERT_EQUAL_PTR(&bus.wram, mem_free_stats.calls[0].memory);
    TEST_ASSERT_EQUAL_PTR(&bus.vram, mem_free_stats.calls[1].memory);
    TEST_ASSERT_EQUAL_PTR(&bus.hram, mem_free_stats.calls[2].memory);
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

void test_bus_read_rom(void) {
    bus.cartridge = &cartridge;
    cartridge_rom_read_stats.calls[0].return_value = 0xAB;

    TEST_ASSERT_EQUAL_UINT8(0xAB, bus_read(&bus, 0x4000));

    TEST_ASSERT_EQUAL_size_t(1, cartridge_rom_read_stats.call_count);
    TEST_ASSERT_EQUAL_PTR(&cartridge, cartridge_rom_read_stats.calls[0].cartridge);
    TEST_ASSERT_EQUAL_UINT16(0x4000, cartridge_rom_read_stats.calls[0].addr);
}

void test_bus_write_rom(void) {
    bus.cartridge = &cartridge;

    bus_write(&bus, 0x2000, 0x0A);

    TEST_ASSERT_EQUAL_size_t(1, cartridge_rom_write_stats.call_count);
    TEST_ASSERT_EQUAL_PTR(&cartridge, cartridge_rom_write_stats.calls[0].cartridge);
    TEST_ASSERT_EQUAL_UINT16(0x2000, cartridge_rom_write_stats.calls[0].addr);
    TEST_ASSERT_EQUAL_UINT8(0x0A, cartridge_rom_write_stats.calls[0].value);
}

void test_bus_read_vram(void) {
    uint8_t memory[1] = { 0x55 };
    bus.vram.mem = memory;
    TEST_ASSERT_EQUAL_UINT8(0x55, bus_read(&bus, 0x8000));
}

void test_bus_write_vram(void) {
    uint8_t memory[1] = { 0x00 };
    bus.vram.mem = memory;

    bus_write(&bus, 0x8000, 0x55);

    TEST_ASSERT_EQUAL_UINT8(0x55, memory[0]);
}

void test_bus_read_vram_not_accessible_returns_0xFF(void) {
    uint8_t memory[1] = { 0x55 };
    bus.vram.mem = memory;
    bus.vram_accessible = false;

    TEST_ASSERT_EQUAL_UINT8(0xFF, bus_read(&bus, 0x8000));
}

void test_bus_write_vram_not_accessible_is_ignored(void) {
    uint8_t memory[1] = { 0x00 };
    bus.vram.mem = memory;
    bus.vram_accessible = false;

    bus_write(&bus, 0x8000, 0x55);

    TEST_ASSERT_EQUAL_UINT8(0x00, memory[0]);
}

void test_bus_read_ext_ram(void) {
    bus.cartridge = &cartridge;
    cartridge_ext_ram_read_stats.calls[0].return_value = 0xCD;

    // The bus passes the cartridge-relative offset (addr - 0xA000).
    TEST_ASSERT_EQUAL_UINT8(0xCD, bus_read(&bus, 0xA100));

    TEST_ASSERT_EQUAL_size_t(1, cartridge_ext_ram_read_stats.call_count);
    TEST_ASSERT_EQUAL_PTR(&cartridge, cartridge_ext_ram_read_stats.calls[0].cartridge);
    TEST_ASSERT_EQUAL_UINT16(0x0100, cartridge_ext_ram_read_stats.calls[0].addr);
}

void test_bus_write_ext_ram(void) {
    bus.cartridge = &cartridge;

    bus_write(&bus, 0xA100, 0x3C);

    TEST_ASSERT_EQUAL_size_t(1, cartridge_ext_ram_write_stats.call_count);
    TEST_ASSERT_EQUAL_PTR(&cartridge, cartridge_ext_ram_write_stats.calls[0].cartridge);
    TEST_ASSERT_EQUAL_UINT16(0x0100, cartridge_ext_ram_write_stats.calls[0].addr);
    TEST_ASSERT_EQUAL_UINT8(0x3C, cartridge_ext_ram_write_stats.calls[0].value);
}

void test_bus_read_wram(void) {
    uint8_t memory[1] = { 0x42 };
    bus.wram.mem = memory;
    TEST_ASSERT_EQUAL_UINT8(0x42, bus_read(&bus, 0xC000));
}

void test_bus_read_echo_ram(void) {
    // Echo RAM mirrors WRAM: 0xE000 -> 0xC000
    uint8_t memory[1] = { 0x42 };
    bus.wram.mem = memory;
    TEST_ASSERT_EQUAL_UINT8(0x42, bus_read(&bus, 0xE000));
}

void test_bus_write_echo_ram(void) {
    // Echo RAM mirrors WRAM: 0xE000 -> 0xC000
    uint8_t memory[1] = { 0x00 };
    bus.wram.mem = memory;

    bus_write(&bus, 0xE000, 0x42);

    TEST_ASSERT_EQUAL_UINT8(0x42, memory[0]);
}

void test_bus_read_oam(void) {
    uint8_t memory[1] = { 0x99 };
    bus.oam.mem = memory;
    TEST_ASSERT_EQUAL_UINT8(0x99, bus_read(&bus, 0xFE00));
}

void test_bus_write_oam(void) {
    uint8_t memory[1] = { 0x00 };
    bus.oam.mem = memory;

    bus_write(&bus, 0xFE00, 0x99);

    TEST_ASSERT_EQUAL_UINT8(0x99, memory[0]);
}

void test_bus_read_oam_not_accessible_returns_0xFF(void) {
    uint8_t memory[1] = { 0x99 };
    bus.oam.mem = memory;
    bus.oam_accessible = false;

    TEST_ASSERT_EQUAL_UINT8(0xFF, bus_read(&bus, 0xFE00));
}

void test_bus_write_oam_not_accessible_is_ignored(void) {
    uint8_t memory[1] = { 0x00 };
    bus.oam.mem = memory;
    bus.oam_accessible = false;

    bus_write(&bus, 0xFE00, 0x99);

    TEST_ASSERT_EQUAL_UINT8(0x00, memory[0]);
}

void test_bus_read_not_usable_returns_00_when_oam_accessible(void) {
    bus.oam_accessible = true;
    TEST_ASSERT_EQUAL_UINT8(0x00, bus_read(&bus, 0xFEA0));
}

void test_bus_read_not_usable_returns_ff_when_oam_blocked(void) {
    bus.oam_accessible = false;
    TEST_ASSERT_EQUAL_UINT8(0xFF, bus_read(&bus, 0xFEA0));
}

void test_bus_write_not_usable_is_ignored(void) {
    uint8_t memory[OAM_SIZE]; // 0xA0 bytes
    memset(memory, 0x99, sizeof(memory));
    bus.oam.mem = memory;

    // Writes to the not usable region must be silently ignored and must not
    // be misrouted into OAM (e.g. via an off-by-one in the memory map).
    bus_write(&bus, 0xFEA0, 0x55); // first byte of the region
    bus_write(&bus, 0xFEFF, 0x55); // last byte of the region

    for (size_t i = 0; i < sizeof(memory); i++) {
        TEST_ASSERT_EQUAL_UINT8(0x99, memory[i]);
    }
}


void test_bus_read_io_reg(void) {
    io_reg_read_stats.calls[0].return_value = 0x3C;

    TEST_ASSERT_EQUAL_UINT8(0x3C, bus_read(&bus, 0xFF00));

    TEST_ASSERT_EQUAL_size_t(1, io_reg_read_stats.call_count);
    TEST_ASSERT_EQUAL_PTR(&bus.io_reg, io_reg_read_stats.calls[0].io_reg);
    TEST_ASSERT_EQUAL_UINT16(0xFF00, io_reg_read_stats.calls[0].addr);
}

void test_bus_write_io_reg(void) {
    bus_write(&bus, 0xFF00, 0x3C);

    TEST_ASSERT_EQUAL_size_t(1, io_reg_write_stats.call_count);
    TEST_ASSERT_EQUAL_PTR(&bus.io_reg, io_reg_write_stats.calls[0].io_reg);
    TEST_ASSERT_EQUAL_UINT16(0xFF00, io_reg_write_stats.calls[0].addr);
    TEST_ASSERT_EQUAL_UINT8(0x3C, io_reg_write_stats.calls[0].value);
}

void test_bus_read_hram(void) {
    uint8_t memory[1] = { 0x77 };
    bus.hram.mem = memory;
    TEST_ASSERT_EQUAL_UINT8(0x77, bus_read(&bus, 0xFF80));
}

void test_bus_write_wram(void) {
    uint8_t memory[1] = { 0x00 };
    bus.wram.mem = memory;

    bus_write(&bus, 0xC000, 0x42);

    TEST_ASSERT_EQUAL_UINT8(0x42, memory[0]);
}

void test_bus_write_hram(void) {
    uint8_t memory[1] = { 0x00 };
    bus.hram.mem = memory;

    bus_write(&bus, 0xFF80, 0x77);

    TEST_ASSERT_EQUAL_UINT8(0x77, memory[0]);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_bus_init_sets_cartridge_pointer);
    RUN_TEST(test_bus_init_calls_mem_init_to_initialize_wram);
    RUN_TEST(test_bus_init_calls_mem_init_to_initialize_vram);
    RUN_TEST(test_bus_init_calls_mem_init_to_initialize_hram);
    RUN_TEST(test_bus_init_calls_mem_init_to_initialize_oam);
    RUN_TEST(test_bus_init_returns_minus1_on_wram_allocation_failure);
    RUN_TEST(test_bus_init_returns_minus1_on_vram_allocation_failure);
    RUN_TEST(test_bus_init_returns_minus1_on_hram_allocation_failure);
    RUN_TEST(test_bus_init_returns_minus1_on_oam_allocation_failure);

    RUN_TEST(test_bus_free_calls_mem_free_for_wram);
    RUN_TEST(test_bus_free_calls_mem_free_for_vram);
    RUN_TEST(test_bus_free_calls_mem_free_for_hram);

    RUN_TEST(test_bus_read_rom);
    RUN_TEST(test_bus_write_rom);

    RUN_TEST(test_bus_read_vram);
    RUN_TEST(test_bus_write_vram);
    RUN_TEST(test_bus_read_vram_not_accessible_returns_0xFF);
    RUN_TEST(test_bus_write_vram_not_accessible_is_ignored);

    RUN_TEST(test_bus_read_ext_ram);
    RUN_TEST(test_bus_write_ext_ram);

    RUN_TEST(test_bus_read_wram);
    RUN_TEST(test_bus_write_wram);

    RUN_TEST(test_bus_read_echo_ram);
    RUN_TEST(test_bus_write_echo_ram);

    RUN_TEST(test_bus_read_oam);
    RUN_TEST(test_bus_write_oam);
    RUN_TEST(test_bus_read_oam_not_accessible_returns_0xFF);
    RUN_TEST(test_bus_write_oam_not_accessible_is_ignored);

    RUN_TEST(test_bus_read_not_usable_returns_00_when_oam_accessible);
    RUN_TEST(test_bus_read_not_usable_returns_ff_when_oam_blocked);
    RUN_TEST(test_bus_write_not_usable_is_ignored);

    RUN_TEST(test_bus_read_io_reg);
    RUN_TEST(test_bus_write_io_reg);

    RUN_TEST(test_bus_read_hram);
    RUN_TEST(test_bus_write_hram);

    return UNITY_END();
}
