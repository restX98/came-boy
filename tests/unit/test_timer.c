#include "unity.h"
#include "log_helpers.h"

#include <assert.h>
#include <string.h>

#include "timer.h"

static timer_regs_t timer;
static interrupt_regs_t interrupts;

// ---- Mock functions ----

typedef struct {
    interrupt_regs_t *interrupts;
    interrupt_t interrupt;
} interrupts_request_call_t;

typedef struct {
    size_t call_count;
    interrupts_request_call_t calls[10];
} interrupts_request_stats_t;

static interrupts_request_stats_t interrupts_request_stats;

void interrupts_request(interrupt_regs_t *interrupts, interrupt_t interrupt) {
    if (interrupts_request_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for interrupts_request_stats");
    }

    interrupts_request_call_t *call = &interrupts_request_stats.calls[interrupts_request_stats.call_count];
    call->interrupts = interrupts;
    call->interrupt = interrupt;

    interrupts_request_stats.call_count++;
}

void setUp(void) {
    suppress_logs();

    timer = (timer_regs_t){ 0 };
    interrupts = (interrupt_regs_t){ 0 };
    interrupts_request_stats = (interrupts_request_stats_t){ 0 };
}

void tearDown(void) {
    restore_logs();
}

// ---- timer_init ----

void test_timer_init_sets_initial_values(void) {
    timer.div_counter = 0x1234;
    timer.tima = 0xAA;
    timer.tma = 0xBB;
    timer.tac.reg = 0x07;
    timer.tima_counter = 999;
    timer.tima_reload_pending = 3;

    timer_init(&timer);

    TEST_ASSERT_EQUAL_UINT16(0xAB00, timer.div_counter);
    TEST_ASSERT_EQUAL_UINT8(0x00, timer.tima);
    TEST_ASSERT_EQUAL_UINT8(0x00, timer.tma);
    TEST_ASSERT_EQUAL_UINT8(0xF8, timer.tac.reg);
    TEST_ASSERT_EQUAL_UINT16(0, timer.tima_counter);
    TEST_ASSERT_EQUAL_UINT8(0, timer.tima_reload_pending);
}

// ---- timer_tick: DIV ----

void test_timer_tick_increments_div_counter_by_cycles(void) {
    timer.div_counter = 0x0100;

    timer_tick(&timer, &interrupts, 16);

    TEST_ASSERT_EQUAL_UINT16(0x0110, timer.div_counter);
}

void test_timer_tick_div_register_reflects_upper_byte_of_div_counter(void) {
    timer.div_counter = 0x00FF;

    timer_tick(&timer, &interrupts, 1);

    TEST_ASSERT_EQUAL_UINT16(0x0100, timer.div_counter);
    TEST_ASSERT_EQUAL_UINT8(0x01, timer.div);
}

void test_timer_tick_advances_div_even_when_timer_disabled(void) {
    timer.tac.enable = 0;
    timer.div_counter = 0;

    timer_tick(&timer, &interrupts, 256);

    TEST_ASSERT_EQUAL_UINT16(256, timer.div_counter);
}

// ---- timer_tick: TIMA enable gate ----

void test_timer_tick_does_not_advance_tima_when_disabled(void) {
    timer.tac.enable = 0;
    timer.tac.clock_select = 1; // 16 cycles
    timer.tima = 0;

    timer_tick(&timer, &interrupts, 1024);

    TEST_ASSERT_EQUAL_UINT8(0, timer.tima);
    TEST_ASSERT_EQUAL_UINT16(0, timer.tima_counter);
}

// ---- timer_tick: TIMA frequency per clock_select ----

void test_timer_tick_increments_tima_at_correct_cycle_count_per_clock_select(void) {
    const int freq_cycles[] = { 1024, 16, 64, 256 };

    for (int clock_select = 0; clock_select < 4; clock_select++) {
        timer = (timer_regs_t){ 0 };
        timer.tac.enable = 1;
        timer.tac.clock_select = clock_select;

        timer_tick(&timer, &interrupts, freq_cycles[clock_select] - 1);
        TEST_ASSERT_EQUAL_UINT8(0, timer.tima);

        timer_tick(&timer, &interrupts, 1);
        TEST_ASSERT_EQUAL_UINT8(1, timer.tima);
    }
}

// ---- timer_tick: TIMA overflow / delayed reload ----

void test_timer_tick_sets_reload_pending_on_tima_overflow(void) {
    timer.tac.enable = 1;
    timer.tac.clock_select = 1; // 16 cycles
    timer.tima = 0xFF;

    timer_tick(&timer, &interrupts, 16);

    TEST_ASSERT_EQUAL_UINT8(0x00, timer.tima);
    TEST_ASSERT_EQUAL_UINT8(4, timer.tima_reload_pending);
}

void test_timer_tick_does_not_request_interrupt_immediately_on_overflow(void) {
    timer.tac.enable = 1;
    timer.tac.clock_select = 1;
    timer.tima = 0xFF;

    timer_tick(&timer, &interrupts, 16);

    TEST_ASSERT_EQUAL_size_t(0, interrupts_request_stats.call_count);
}

void test_timer_tick_decrements_reload_pending_when_cycles_below_threshold(void) {
    timer.tima_reload_pending = 4;
    timer.tima = 0;
    timer.tma = 0x42;

    timer_tick(&timer, &interrupts, 2);

    TEST_ASSERT_EQUAL_UINT8(2, timer.tima_reload_pending);
    TEST_ASSERT_EQUAL_UINT8(0, timer.tima);
    TEST_ASSERT_EQUAL_size_t(0, interrupts_request_stats.call_count);
}

void test_timer_tick_reloads_tima_and_requests_interrupt_when_pending_expires(void) {
    timer.tima_reload_pending = 4;
    timer.tima = 0;
    timer.tma = 0x42;

    timer_tick(&timer, &interrupts, 4);

    TEST_ASSERT_EQUAL_UINT8(0x42, timer.tima);
    TEST_ASSERT_EQUAL_UINT8(0, timer.tima_reload_pending);
    TEST_ASSERT_EQUAL_size_t(1, interrupts_request_stats.call_count);
    TEST_ASSERT_EQUAL_INT(INT_TIMER, interrupts_request_stats.calls[0].interrupt);
    TEST_ASSERT_EQUAL_PTR(&interrupts, interrupts_request_stats.calls[0].interrupts);
}

// ---- timer_read ----

void test_timer_read_returns_div_at_0xFF04(void) {
    timer.div_counter = 0xAB00;
    TEST_ASSERT_EQUAL_UINT8(0xAB, timer_read(&timer, 0xFF04));
}

void test_timer_read_returns_tima_at_0xFF05(void) {
    timer.tima = 0x42;
    TEST_ASSERT_EQUAL_UINT8(0x42, timer_read(&timer, 0xFF05));
}

void test_timer_read_returns_tma_at_0xFF06(void) {
    timer.tma = 0x99;
    TEST_ASSERT_EQUAL_UINT8(0x99, timer_read(&timer, 0xFF06));
}

void test_timer_read_returns_tac_at_0xFF07(void) {
    timer.tac.reg = 0xFD;
    TEST_ASSERT_EQUAL_UINT8(0xFD, timer_read(&timer, 0xFF07));
}

// ---- timer_write ----

void test_timer_write_resets_div_counter_at_0xFF04(void) {
    timer.div_counter = 0x1234;

    timer_write(&timer, 0xFF04, 0xAB);

    TEST_ASSERT_EQUAL_UINT16(0, timer.div_counter);
}

void test_timer_write_sets_tima_at_0xFF05(void) {
    timer_write(&timer, 0xFF05, 0x42);
    TEST_ASSERT_EQUAL_UINT8(0x42, timer.tima);
}

void test_timer_write_sets_tma_at_0xFF06(void) {
    timer_write(&timer, 0xFF06, 0x99);
    TEST_ASSERT_EQUAL_UINT8(0x99, timer.tma);
}

void test_timer_write_sets_tac_at_0xFF07(void) {
    timer_write(&timer, 0xFF07, 0xFD);
    TEST_ASSERT_EQUAL_UINT8(0xFD, timer.tac.reg);
}

int main(void) {
    UNITY_BEGIN();

    // ---- timer_init ----
    RUN_TEST(test_timer_init_sets_initial_values);

    // ---- timer_tick: DIV ----
    RUN_TEST(test_timer_tick_increments_div_counter_by_cycles);
    RUN_TEST(test_timer_tick_div_register_reflects_upper_byte_of_div_counter);
    RUN_TEST(test_timer_tick_advances_div_even_when_timer_disabled);

    // ---- timer_tick: TIMA enable gate ----
    RUN_TEST(test_timer_tick_does_not_advance_tima_when_disabled);

    // ---- timer_tick: TIMA frequency per clock_select ----
    RUN_TEST(test_timer_tick_increments_tima_at_correct_cycle_count_per_clock_select);

    // ---- timer_tick: TIMA overflow / delayed reload ----
    RUN_TEST(test_timer_tick_sets_reload_pending_on_tima_overflow);
    RUN_TEST(test_timer_tick_does_not_request_interrupt_immediately_on_overflow);
    RUN_TEST(test_timer_tick_decrements_reload_pending_when_cycles_below_threshold);
    RUN_TEST(test_timer_tick_reloads_tima_and_requests_interrupt_when_pending_expires);

    // ---- timer_read ----
    RUN_TEST(test_timer_read_returns_div_at_0xFF04);
    RUN_TEST(test_timer_read_returns_tima_at_0xFF05);
    RUN_TEST(test_timer_read_returns_tma_at_0xFF06);
    RUN_TEST(test_timer_read_returns_tac_at_0xFF07);

    // ---- timer_write ----
    RUN_TEST(test_timer_write_resets_div_counter_at_0xFF04);
    RUN_TEST(test_timer_write_sets_tima_at_0xFF05);
    RUN_TEST(test_timer_write_sets_tma_at_0xFF06);
    RUN_TEST(test_timer_write_sets_tac_at_0xFF07);

    return UNITY_END();
}
