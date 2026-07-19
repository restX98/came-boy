#include "unity.h"
#include "log_helpers.h"

#include <assert.h>
#include <string.h>

#include "bus.h"
#include "io/oam_dma.h"

static bus_t bus;
static uint8_t oam_mem[OAM_SIZE];

// ---- Mock functions ----

// Mock bus_read
typedef struct {
    bus_t *bus;
    uint16_t addr;
    uint8_t return_value;
} bus_read_call_t;

typedef struct {
    size_t call_count;
    bus_read_call_t calls[256];
} bus_read_stats_t;

static bus_read_stats_t bus_read_stats;

uint8_t bus_read(bus_t *b, uint16_t addr) {
    if (bus_read_stats.call_count == 256) {
        assert(0 && "Exceeded maximum call count for bus_read_stats");
    }

    bus_read_call_t *call = &bus_read_stats.calls[bus_read_stats.call_count];
    call->bus = b;
    call->addr = addr;

    bus_read_stats.call_count++;

    return bus_read_stats.calls[bus_read_stats.call_count - 1].return_value;
}

void setUp(void) {
    suppress_logs();

    bus = (bus_t){ 0 };
    bus.oam.mem = oam_mem;
    bus.oam.size = OAM_SIZE;
    memset(oam_mem, 0, sizeof(oam_mem));

    bus_read_stats = (bus_read_stats_t){ 0 };
}

void tearDown(void) {
    restore_logs();
}

// ---- oam_dma_init ----

void test_oam_dma_init_sets_defaults(void) {
    oam_dma_reg_t dma;
    oam_dma_init(&dma);

    TEST_ASSERT_FALSE(dma.active);
    TEST_ASSERT_EQUAL_UINT16(0xFFFF, dma.source);
    TEST_ASSERT_EQUAL_UINT8(0, dma.index);
    TEST_ASSERT_EQUAL_UINT8(0, dma.start_delay);
}

// ---- oam_dma_read / oam_dma_write ----

void test_oam_dma_read_returns_high_byte_of_source(void) {
    oam_dma_reg_t dma = { .source = 0xAB00 };
    TEST_ASSERT_EQUAL_UINT8(0xAB, oam_dma_read(&dma));
}

void test_oam_dma_write_starts_transfer(void) {
    oam_dma_reg_t dma = { 0 };
    oam_dma_write(&dma, 0xC0);

    TEST_ASSERT_TRUE(dma.active);
    TEST_ASSERT_EQUAL_UINT16(0xC000, dma.source);
    TEST_ASSERT_EQUAL_UINT8(0, dma.index);
    TEST_ASSERT_EQUAL_UINT8(4, dma.start_delay);
}

// ---- oam_dma_tick ----

void test_oam_dma_tick_inactive_does_nothing(void) {
    bus.io_reg.oam_dma = (oam_dma_reg_t){ .active = false };

    oam_dma_tick(&bus, 4);

    TEST_ASSERT_EQUAL_size_t(0, bus_read_stats.call_count);
}

void test_oam_dma_tick_consumes_start_delay_before_transferring(void) {
    bus.io_reg.oam_dma = (oam_dma_reg_t){
        .active = true, .source = 0x8000, .index = 0, .start_delay = 4
    };

    oam_dma_tick(&bus, 4);

    // The whole tick is spent draining the start delay; no bytes copied yet.
    TEST_ASSERT_EQUAL_UINT8(0, bus.io_reg.oam_dma.start_delay);
    TEST_ASSERT_EQUAL_size_t(0, bus_read_stats.call_count);
    TEST_ASSERT_EQUAL_UINT8(0, bus.io_reg.oam_dma.index);
}

void test_oam_dma_tick_copies_byte_from_source(void) {
    bus.io_reg.oam_dma = (oam_dma_reg_t){
        .active = true, .source = 0x8000, .index = 0, .start_delay = 0
    };
    bus_read_stats.calls[0].return_value = 0x5A;

    oam_dma_tick(&bus, 4);

    TEST_ASSERT_EQUAL_size_t(1, bus_read_stats.call_count);
    TEST_ASSERT_EQUAL_PTR(&bus, bus_read_stats.calls[0].bus);
    TEST_ASSERT_EQUAL_UINT16(0x8000, bus_read_stats.calls[0].addr);
    TEST_ASSERT_EQUAL_UINT8(0x5A, oam_mem[0]);
    TEST_ASSERT_EQUAL_UINT8(1, bus.io_reg.oam_dma.index);
}

void test_oam_dma_tick_mirrors_wram_for_high_source(void) {
    // A DMA source in 0xE000-0xFFFF is mirrored down to WRAM (addr - 0x2000).
    bus.io_reg.oam_dma = (oam_dma_reg_t){
        .active = true, .source = 0xE000, .index = 0, .start_delay = 0
    };

    oam_dma_tick(&bus, 4);

    TEST_ASSERT_EQUAL_size_t(1, bus_read_stats.call_count);
    TEST_ASSERT_EQUAL_UINT16(0xC000, bus_read_stats.calls[0].addr);
}

void test_oam_dma_tick_does_not_mirror_source_below_echo(void) {
    // The byte just below the echo region must not be mirrored.
    bus.io_reg.oam_dma = (oam_dma_reg_t){
        .active = true, .source = 0xDFFF, .index = 0, .start_delay = 0
    };

    oam_dma_tick(&bus, 4);

    TEST_ASSERT_EQUAL_size_t(1, bus_read_stats.call_count);
    TEST_ASSERT_EQUAL_UINT16(0xDFFF, bus_read_stats.calls[0].addr);
}

void test_oam_dma_tick_completes_after_oam_size_bytes(void) {
    bus.io_reg.oam_dma = (oam_dma_reg_t){
        .active = true, .source = 0x8000, .index = 0, .start_delay = 0
    };

    // Enough cycles to transfer every OAM byte in a single tick.
    oam_dma_tick(&bus, OAM_SIZE * 4);

    TEST_ASSERT_EQUAL_size_t(OAM_SIZE, bus_read_stats.call_count);
    TEST_ASSERT_FALSE(bus.io_reg.oam_dma.active);
    TEST_ASSERT_EQUAL_UINT8(OAM_SIZE, bus.io_reg.oam_dma.index);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_oam_dma_init_sets_defaults);

    RUN_TEST(test_oam_dma_read_returns_high_byte_of_source);
    RUN_TEST(test_oam_dma_write_starts_transfer);

    RUN_TEST(test_oam_dma_tick_inactive_does_nothing);
    RUN_TEST(test_oam_dma_tick_consumes_start_delay_before_transferring);
    RUN_TEST(test_oam_dma_tick_copies_byte_from_source);
    RUN_TEST(test_oam_dma_tick_mirrors_wram_for_high_source);
    RUN_TEST(test_oam_dma_tick_does_not_mirror_source_below_echo);
    RUN_TEST(test_oam_dma_tick_completes_after_oam_size_bytes);

    return UNITY_END();
}
