#include "unity.h"
#include "log_helpers.h"

#include "gameboy.h"

static gameboy_t gb;

// ---- Mocks: every collaborator gameboy.c calls is stubbed here so the module
// can be tested in isolation. Each mock records what it was handed and returns
// a value the test can preset. ----

// cartridge_load
static struct {
    size_t call_count;
    cartridge_t *cartridge;
    const char *filename;
    int return_value;
} cartridge_load_mock;

int cartridge_load(cartridge_t *cartridge, const char *filename) {
    cartridge_load_mock.call_count++;
    cartridge_load_mock.cartridge = cartridge;
    cartridge_load_mock.filename = filename;
    return cartridge_load_mock.return_value;
}

// cartridge_unload
static struct {
    size_t call_count;
    cartridge_t *cartridge;
} cartridge_unload_mock;

void cartridge_unload(cartridge_t *cartridge) {
    cartridge_unload_mock.call_count++;
    cartridge_unload_mock.cartridge = cartridge;
}

// bus_init
static struct {
    size_t call_count;
    bus_t *bus;
    cartridge_t *cartridge;
    int return_value;
} bus_init_mock;

int bus_init(bus_t *bus, cartridge_t *cartridge) {
    bus_init_mock.call_count++;
    bus_init_mock.bus = bus;
    bus_init_mock.cartridge = cartridge;
    return bus_init_mock.return_value;
}

// bus_free
static struct {
    size_t call_count;
    bus_t *bus;
} bus_free_mock;

void bus_free(bus_t *bus) {
    bus_free_mock.call_count++;
    bus_free_mock.bus = bus;
}

// cpu_init
static struct {
    size_t call_count;
    cpu_t *cpu;
} cpu_init_mock;

void cpu_init(cpu_t *cpu) {
    cpu_init_mock.call_count++;
    cpu_init_mock.cpu = cpu;
}

// cpu_step
static struct {
    size_t call_count;
    cpu_t *cpu;
    bus_t *bus;
    int return_value;
} cpu_step_mock;

int cpu_step(cpu_t *cpu, bus_t *bus) {
    cpu_step_mock.call_count++;
    cpu_step_mock.cpu = cpu;
    cpu_step_mock.bus = bus;
    return cpu_step_mock.return_value;
}

// ppu_init
static struct {
    size_t call_count;
    ppu_t *ppu;
    lcd_regs_t *lcd;
    mem_t *vram;
    mem_t *oam;
} ppu_init_mock;

void ppu_init(ppu_t *ppu, lcd_regs_t *lcd, mem_t *vram, mem_t *oam) {
    ppu_init_mock.call_count++;
    ppu_init_mock.ppu = ppu;
    ppu_init_mock.lcd = lcd;
    ppu_init_mock.vram = vram;
    ppu_init_mock.oam = oam;
}

// clock_init
static struct {
    size_t call_count;
    gb_clock_t *clock;
    ppu_t *ppu;
    bus_t *bus;
} clock_init_mock;

void clock_init(gb_clock_t *clock, ppu_t *ppu, bus_t *bus) {
    clock_init_mock.call_count++;
    clock_init_mock.clock = clock;
    clock_init_mock.ppu = ppu;
    clock_init_mock.bus = bus;
}

// clock_tick
static struct {
    size_t call_count;
    gb_clock_t *clock;
    int cycles;
} clock_tick_mock;

void clock_tick(gb_clock_t *clock, int cycles) {
    clock_tick_mock.call_count++;
    clock_tick_mock.clock = clock;
    clock_tick_mock.cycles = cycles;
}

void setUp(void) {
    suppress_logs();

    gb = (gameboy_t){ 0 };
    cartridge_load_mock = (typeof(cartridge_load_mock)){ 0 };
    cartridge_unload_mock = (typeof(cartridge_unload_mock)){ 0 };
    bus_init_mock = (typeof(bus_init_mock)){ 0 };
    bus_free_mock = (typeof(bus_free_mock)){ 0 };
    cpu_init_mock = (typeof(cpu_init_mock)){ 0 };
    cpu_step_mock = (typeof(cpu_step_mock)){ 0 };
    ppu_init_mock = (typeof(ppu_init_mock)){ 0 };
    clock_init_mock = (typeof(clock_init_mock)){ 0 };
    clock_tick_mock = (typeof(clock_tick_mock)){ 0 };
}

void tearDown(void) {
    restore_logs();
}

// ---- gameboy_init ----

void test_gameboy_init_wires_components(void) {
    int result = gameboy_init(&gb, "rom.gb");

    TEST_ASSERT_EQUAL_INT(0, result);

    // The cartridge is loaded from the given path into the owned cartridge.
    TEST_ASSERT_EQUAL_size_t(1, cartridge_load_mock.call_count);
    TEST_ASSERT_EQUAL_PTR(&gb.cartridge, cartridge_load_mock.cartridge);
    TEST_ASSERT_EQUAL_STRING("rom.gb", cartridge_load_mock.filename);

    // The bus is wired to the owned cartridge.
    TEST_ASSERT_EQUAL_size_t(1, bus_init_mock.call_count);
    TEST_ASSERT_EQUAL_PTR(&gb.bus, bus_init_mock.bus);
    TEST_ASSERT_EQUAL_PTR(&gb.cartridge, bus_init_mock.cartridge);

    TEST_ASSERT_EQUAL_size_t(1, cpu_init_mock.call_count);
    TEST_ASSERT_EQUAL_PTR(&gb.cpu, cpu_init_mock.cpu);

    // The PPU is given direct references into the owned bus.
    TEST_ASSERT_EQUAL_size_t(1, ppu_init_mock.call_count);
    TEST_ASSERT_EQUAL_PTR(&gb.ppu, ppu_init_mock.ppu);
    TEST_ASSERT_EQUAL_PTR(&gb.bus.io_reg.lcd, ppu_init_mock.lcd);
    TEST_ASSERT_EQUAL_PTR(&gb.bus.vram, ppu_init_mock.vram);
    TEST_ASSERT_EQUAL_PTR(&gb.bus.oam, ppu_init_mock.oam);

    // The clock is wired to the owned ppu and bus.
    TEST_ASSERT_EQUAL_size_t(1, clock_init_mock.call_count);
    TEST_ASSERT_EQUAL_PTR(&gb.clock, clock_init_mock.clock);
    TEST_ASSERT_EQUAL_PTR(&gb.ppu, clock_init_mock.ppu);
    TEST_ASSERT_EQUAL_PTR(&gb.bus, clock_init_mock.bus);
}

void test_gameboy_init_fails_when_cartridge_load_fails(void) {
    cartridge_load_mock.return_value = -1;

    int result = gameboy_init(&gb, "rom.gb");

    TEST_ASSERT_EQUAL_INT(-1, result);
    // Nothing further is set up, and there is nothing to unload.
    TEST_ASSERT_EQUAL_size_t(0, bus_init_mock.call_count);
    TEST_ASSERT_EQUAL_size_t(0, cartridge_unload_mock.call_count);
}

void test_gameboy_init_unloads_cartridge_when_bus_init_fails(void) {
    bus_init_mock.return_value = -1;

    int result = gameboy_init(&gb, "rom.gb");

    TEST_ASSERT_EQUAL_INT(-1, result);
    // The already-loaded cartridge is released; the rest is not wired.
    TEST_ASSERT_EQUAL_size_t(1, cartridge_unload_mock.call_count);
    TEST_ASSERT_EQUAL_PTR(&gb.cartridge, cartridge_unload_mock.cartridge);
    TEST_ASSERT_EQUAL_size_t(0, cpu_init_mock.call_count);
    TEST_ASSERT_EQUAL_size_t(0, ppu_init_mock.call_count);
    TEST_ASSERT_EQUAL_size_t(0, clock_init_mock.call_count);
}

// ---- gameboy_step ----

void test_gameboy_step_advances_clock_by_cpu_cycles(void) {
    cpu_step_mock.return_value = 12;

    int cycles = gameboy_step(&gb);

    TEST_ASSERT_EQUAL_INT(12, cycles);
    TEST_ASSERT_EQUAL_size_t(1, cpu_step_mock.call_count);
    TEST_ASSERT_EQUAL_PTR(&gb.cpu, cpu_step_mock.cpu);
    TEST_ASSERT_EQUAL_PTR(&gb.bus, cpu_step_mock.bus);

    TEST_ASSERT_EQUAL_size_t(1, clock_tick_mock.call_count);
    TEST_ASSERT_EQUAL_PTR(&gb.clock, clock_tick_mock.clock);
    TEST_ASSERT_EQUAL_INT(12, clock_tick_mock.cycles);
}

void test_gameboy_step_halt_returns_minus1_without_ticking_clock(void) {
    cpu_step_mock.return_value = -1;

    int cycles = gameboy_step(&gb);

    TEST_ASSERT_EQUAL_INT(-1, cycles);
    TEST_ASSERT_EQUAL_size_t(0, clock_tick_mock.call_count);
}

// ---- gameboy_free ----

void test_gameboy_free_releases_cartridge_and_bus(void) {
    gameboy_free(&gb);

    TEST_ASSERT_EQUAL_size_t(1, cartridge_unload_mock.call_count);
    TEST_ASSERT_EQUAL_PTR(&gb.cartridge, cartridge_unload_mock.cartridge);
    TEST_ASSERT_EQUAL_size_t(1, bus_free_mock.call_count);
    TEST_ASSERT_EQUAL_PTR(&gb.bus, bus_free_mock.bus);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_gameboy_init_wires_components);
    RUN_TEST(test_gameboy_init_fails_when_cartridge_load_fails);
    RUN_TEST(test_gameboy_init_unloads_cartridge_when_bus_init_fails);

    RUN_TEST(test_gameboy_step_advances_clock_by_cpu_cycles);
    RUN_TEST(test_gameboy_step_halt_returns_minus1_without_ticking_clock);

    RUN_TEST(test_gameboy_free_releases_cartridge_and_bus);

    return UNITY_END();
}
