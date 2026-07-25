#include "unity.h"
#include "log_helpers.h"

#include <assert.h>

#include "clock.h"

static gb_clock_t clk;
static ppu_t ppu;
static bus_t bus;

// ---- Mock functions ----
// clock.c fans a tick out to the PPU, the OAM DMA engine and the timer; stub
// each and record how it was called.

typedef struct {
    ppu_t *ppu;
    int t_cycles;
} ppu_step_call_t;

typedef struct {
    size_t call_count;
    ppu_step_call_t calls[10];
} ppu_step_stats_t;

static ppu_step_stats_t ppu_step_stats;

void ppu_step(ppu_t *ppu, int t_cycles) {
    if (ppu_step_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for ppu_step_stats");
    }
    ppu_step_call_t *call = &ppu_step_stats.calls[ppu_step_stats.call_count];
    call->ppu = ppu;
    call->t_cycles = t_cycles;
    ppu_step_stats.call_count++;
}

typedef struct {
    bus_t *bus;
    int cycles;
} oam_dma_tick_call_t;

typedef struct {
    size_t call_count;
    oam_dma_tick_call_t calls[10];
} oam_dma_tick_stats_t;

static oam_dma_tick_stats_t oam_dma_tick_stats;

void oam_dma_tick(bus_t *bus, int cycles) {
    if (oam_dma_tick_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for oam_dma_tick_stats");
    }
    oam_dma_tick_call_t *call = &oam_dma_tick_stats.calls[oam_dma_tick_stats.call_count];
    call->bus = bus;
    call->cycles = cycles;
    oam_dma_tick_stats.call_count++;
}

typedef struct {
    timer_regs_t *timer;
    int cycles;
} timer_tick_call_t;

typedef struct {
    size_t call_count;
    timer_tick_call_t calls[10];
} timer_tick_stats_t;

static timer_tick_stats_t timer_tick_stats;

void timer_tick(timer_regs_t *timer, int cycles) {
    if (timer_tick_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for timer_tick_stats");
    }
    timer_tick_call_t *call = &timer_tick_stats.calls[timer_tick_stats.call_count];
    call->timer = timer;
    call->cycles = cycles;
    timer_tick_stats.call_count++;
}

typedef struct {
    audio_regs_t *audio;
    int cycles;
    uint8_t div_register;
    const uint8_t *wave_ram;
} audio_tick_call_t;

typedef struct {
    size_t call_count;
    audio_tick_call_t calls[10];
} audio_tick_stats_t;

static audio_tick_stats_t audio_tick_stats;

void audio_tick(audio_regs_t *audio, int cycles, uint8_t div_register, const uint8_t *wave_ram) {
    if (audio_tick_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for audio_tick_stats");
    }
    audio_tick_call_t *call = &audio_tick_stats.calls[audio_tick_stats.call_count];
    call->audio = audio;
    call->cycles = cycles;
    call->div_register = div_register;
    call->wave_ram = wave_ram;
    audio_tick_stats.call_count++;
}

void setUp(void) {
    suppress_logs();

    clk = (gb_clock_t){ 0 };
    ppu = (ppu_t){ 0 };
    bus = (bus_t){ 0 };

    ppu_step_stats = (ppu_step_stats_t){ 0 };
    oam_dma_tick_stats = (oam_dma_tick_stats_t){ 0 };
    timer_tick_stats = (timer_tick_stats_t){ 0 };
    audio_tick_stats = (audio_tick_stats_t){ 0 };
}

void tearDown(void) {
    restore_logs();
}

// ---- clock_init ----

void test_clock_init_stores_references(void) {
    clock_init(&clk, &ppu, &bus);

    TEST_ASSERT_EQUAL_PTR(&ppu, clk.ppu);
    TEST_ASSERT_EQUAL_PTR(&bus, clk.bus);
}

// ---- clock_tick ----

void test_clock_tick_advances_ppu_with_cycles(void) {
    clock_init(&clk, &ppu, &bus);

    clock_tick(&clk, 20);

    TEST_ASSERT_EQUAL_size_t(1, ppu_step_stats.call_count);
    TEST_ASSERT_EQUAL_PTR(&ppu, ppu_step_stats.calls[0].ppu);
    TEST_ASSERT_EQUAL_INT(20, ppu_step_stats.calls[0].t_cycles);
}

void test_clock_tick_advances_oam_dma_with_bus_and_cycles(void) {
    clock_init(&clk, &ppu, &bus);

    clock_tick(&clk, 20);

    TEST_ASSERT_EQUAL_size_t(1, oam_dma_tick_stats.call_count);
    TEST_ASSERT_EQUAL_PTR(&bus, oam_dma_tick_stats.calls[0].bus);
    TEST_ASSERT_EQUAL_INT(20, oam_dma_tick_stats.calls[0].cycles);
}

void test_clock_tick_advances_timer_with_bus_timer_and_cycles(void) {
    clock_init(&clk, &ppu, &bus);

    clock_tick(&clk, 20);

    TEST_ASSERT_EQUAL_size_t(1, timer_tick_stats.call_count);
    TEST_ASSERT_EQUAL_PTR(&bus.io_reg.timer, timer_tick_stats.calls[0].timer);
    TEST_ASSERT_EQUAL_INT(20, timer_tick_stats.calls[0].cycles);
}

void test_clock_tick_advances_audio_with_cycles_div_register_and_wave_ram(void) {
    clock_init(&clk, &ppu, &bus);
    bus.io_reg.timer.div = 0x3C; // timer_tick is mocked, so set it directly

    clock_tick(&clk, 20);

    TEST_ASSERT_EQUAL_size_t(1, audio_tick_stats.call_count);
    TEST_ASSERT_EQUAL_PTR(&bus.io_reg.audio, audio_tick_stats.calls[0].audio);
    TEST_ASSERT_EQUAL_INT(20, audio_tick_stats.calls[0].cycles);
    TEST_ASSERT_EQUAL_UINT8(0x3C, audio_tick_stats.calls[0].div_register);
    TEST_ASSERT_EQUAL_PTR(bus.io_reg.wp_ram, audio_tick_stats.calls[0].wave_ram);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_clock_init_stores_references);

    RUN_TEST(test_clock_tick_advances_ppu_with_cycles);
    RUN_TEST(test_clock_tick_advances_oam_dma_with_bus_and_cycles);
    RUN_TEST(test_clock_tick_advances_timer_with_bus_timer_and_cycles);
    RUN_TEST(test_clock_tick_advances_audio_with_cycles_div_register_and_wave_ram);

    return UNITY_END();
}
