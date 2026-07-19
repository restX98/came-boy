#include "unity.h"
#include "log_helpers.h"

#include <string.h>

#include "bus.h"
#include "io/oam_dma.h"
#include "memory/mbc/mbc.h"
#include "memory/mem.h"

static bus_t       bus;
static cartridge_t cartridge;
static uint8_t     fake_rom[0x8000];

// Cycles needed for a full transfer: 4-cycle start delay + 4 cycles/byte.
#define DMA_FULL_CYCLES (4 + OAM_SIZE * 4)

void setUp(void) {
    suppress_logs();

    memset(fake_rom, 0, sizeof(fake_rom));
    cartridge = (cartridge_t){ .rom = fake_rom, .size = sizeof(fake_rom), .mbc = &no_mbc_ops };
    bus_init(&bus, &cartridge);
}

void tearDown(void) {
    bus_free(&bus);
    restore_logs();
}

// ---- helpers ----

// Fill WRAM page $C000+page*0x100 with a recognizable ramp so each byte
// is distinct, then kick off a DMA from that page via $FF46.
static void fill_wram_page(uint8_t page) {
    for (int i = 0; i < OAM_SIZE; i++) {
        bus_write(&bus, 0xC000 + page * 0x100 + i, (uint8_t)(i ^ 0xA5));
    }
}

// ---- writing $FF46 arms the DMA source register ----

void test_oam_dma_read_returns_source_high_byte(void) {
    bus_write(&bus, 0xFF46, 0xC2);
    // $FF46 reads back the high byte of the source address.
    TEST_ASSERT_EQUAL_UINT8(0xC2, bus_read(&bus, 0xFF46));
}

// ---- a full transfer copies the source page into OAM ----

void test_oam_dma_copies_full_page_into_oam(void) {
    fill_wram_page(0x00); // source page $C000

    bus_write(&bus, 0xFF46, 0xC0);
    oam_dma_tick(&bus, DMA_FULL_CYCLES);

    for (int i = 0; i < OAM_SIZE; i++) {
        TEST_ASSERT_EQUAL_UINT8((uint8_t)(i ^ 0xA5), bus_read(&bus, 0xFE00 + i));
    }
}

// ---- the 4-cycle start delay copies nothing yet ----

void test_oam_dma_start_delay_copies_no_bytes(void) {
    fill_wram_page(0x00);
    bus_write(&bus, 0xFE00, 0x00); // known OAM byte before DMA

    bus_write(&bus, 0xFF46, 0xC0);
    oam_dma_tick(&bus, 4); // exactly the start delay

    // Nothing transferred yet; first OAM byte is still its pre-DMA value.
    TEST_ASSERT_EQUAL_UINT8(0x00, bus_read(&bus, 0xFE00));
}

// ---- transfer advances one byte per 4 cycles ----

void test_oam_dma_transfers_one_byte_per_four_cycles(void) {
    fill_wram_page(0x00);
    bus_write(&bus, 0xFE03, 0x00); // sentinel: 4th OAM byte, distinct from its ramp value

    bus_write(&bus, 0xFF46, 0xC0);
    oam_dma_tick(&bus, 4);     // consume start delay
    oam_dma_tick(&bus, 4 * 3); // three bytes

    TEST_ASSERT_EQUAL_UINT8((uint8_t)(0 ^ 0xA5), bus_read(&bus, 0xFE00));
    TEST_ASSERT_EQUAL_UINT8((uint8_t)(1 ^ 0xA5), bus_read(&bus, 0xFE01));
    TEST_ASSERT_EQUAL_UINT8((uint8_t)(2 ^ 0xA5), bus_read(&bus, 0xFE02));
    // 4th byte not yet copied: still the sentinel, not (3 ^ 0xA5) == 0xA6.
    TEST_ASSERT_EQUAL_UINT8(0x00, bus_read(&bus, 0xFE03));
}

// ---- extra cycles after completion do nothing ----

void test_oam_dma_deactivates_after_completion(void) {
    fill_wram_page(0x00);

    bus_write(&bus, 0xFF46, 0xC0);
    oam_dma_tick(&bus, DMA_FULL_CYCLES);
    TEST_ASSERT_FALSE(bus.io_reg.oam_dma.active);

    // Change the source data and tick again: OAM must not change.
    bus_write(&bus, 0xC000, 0xEE);
    oam_dma_tick(&bus, DMA_FULL_CYCLES);
    TEST_ASSERT_EQUAL_UINT8((uint8_t)(0 ^ 0xA5), bus_read(&bus, 0xFE00));
}

// ---- WRAM is mirrored on the DMA source bus for $E000-$FFFF ----

void test_oam_dma_source_above_e000_mirrors_wram(void) {
    fill_wram_page(0x00); // data lives at $C000

    // Source $E000 must read through to WRAM $C000 on the DMA bus.
    bus_write(&bus, 0xFF46, 0xE0);
    oam_dma_tick(&bus, DMA_FULL_CYCLES);

    for (int i = 0; i < OAM_SIZE; i++) {
        TEST_ASSERT_EQUAL_UINT8((uint8_t)(i ^ 0xA5), bus_read(&bus, 0xFE00 + i));
    }
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_oam_dma_read_returns_source_high_byte);
    RUN_TEST(test_oam_dma_copies_full_page_into_oam);
    RUN_TEST(test_oam_dma_start_delay_copies_no_bytes);
    RUN_TEST(test_oam_dma_transfers_one_byte_per_four_cycles);
    RUN_TEST(test_oam_dma_deactivates_after_completion);
    RUN_TEST(test_oam_dma_source_above_e000_mirrors_wram);

    return UNITY_END();
}
