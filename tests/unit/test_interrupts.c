#include "unity.h"
#include "log_helpers.h"

#include "interrupts.h"

static interrupt_regs_t interrupts;

void setUp(void) {
    suppress_logs();
    interrupts = (interrupt_regs_t){ 0 };
}

void tearDown(void) {
    restore_logs();
}

// ---- interrupts_init ----

void test_interrupts_init_sets_initial_register_values(void) {
    interrupts_init(&interrupts);

    TEST_ASSERT_EQUAL_UINT8(0xE1, interrupts.flag.reg);
    TEST_ASSERT_EQUAL_UINT8(0x00, interrupts.enable.reg);
}

// ---- interrupts_request ----

void test_interrupts_request_sets_correct_bit_for_each_interrupt(void) {
    const interrupt_t interrupt_types[] = { INT_VBLANK, INT_LCD, INT_TIMER, INT_SERIAL, INT_JOYPAD };
    const uint8_t expected_bits[] = { 0x01, 0x02, 0x04, 0x08, 0x10 };

    for (size_t i = 0; i < 5; i++) {
        interrupts.flag.reg = 0;
        interrupts_request(&interrupts, interrupt_types[i]);
        TEST_ASSERT_EQUAL_UINT8(expected_bits[i], interrupts.flag.reg);
    }
}

void test_interrupts_request_preserves_other_bits(void) {
    interrupts.flag.reg = 0xE1; // upper 3 bits + vblank
    interrupts_request(&interrupts, INT_TIMER);
    TEST_ASSERT_EQUAL_UINT8(0xE5, interrupts.flag.reg);
}

// ---- interrupts_acknowledge ----

void test_interrupts_acknowledge_clears_correct_bit_for_each_interrupt(void) {
    const interrupt_t interrupt_types[] = { INT_VBLANK, INT_LCD, INT_TIMER, INT_SERIAL, INT_JOYPAD };
    const uint8_t expected_after[] = { 0xFE, 0xFD, 0xFB, 0xF7, 0xEF };

    for (size_t i = 0; i < 5; i++) {
        interrupts.flag.reg = 0xFF;
        interrupts_acknowledge(&interrupts, interrupt_types[i]);
        TEST_ASSERT_EQUAL_UINT8(expected_after[i], interrupts.flag.reg);
    }
}

void test_interrupts_acknowledge_preserves_other_bits(void) {
    interrupts.flag.reg = 0x1F; // all 5 interrupts pending
    interrupts_acknowledge(&interrupts, INT_LCD);
    TEST_ASSERT_EQUAL_UINT8(0x1D, interrupts.flag.reg);
}

// ---- interrupts_pending ----

void test_interrupts_pending_returns_minus_1_when_nothing_pending(void) {
    interrupts.enable.reg = 0x00;
    interrupts.flag.reg = 0x00;
    TEST_ASSERT_EQUAL_INT(-1, interrupts_pending(&interrupts));
}

void test_interrupts_pending_returns_minus_1_when_flagged_but_not_enabled(void) {
    interrupts.enable.reg = 0x00;
    interrupts.flag.reg = 0xFF;
    TEST_ASSERT_EQUAL_INT(-1, interrupts_pending(&interrupts));
}

void test_interrupts_pending_returns_minus_1_when_enabled_but_not_flagged(void) {
    interrupts.enable.reg = 0xFF;
    interrupts.flag.reg = 0x00;
    TEST_ASSERT_EQUAL_INT(-1, interrupts_pending(&interrupts));
}

void test_interrupts_pending_returns_index_for_each_interrupt(void) {
    const interrupt_t interrupt_types[] = { INT_VBLANK, INT_LCD, INT_TIMER, INT_SERIAL, INT_JOYPAD };
    const uint8_t masks[] = { 0x01, 0x02, 0x04, 0x08, 0x10 };

    for (size_t i = 0; i < 5; i++) {
        interrupts.enable.reg = masks[i];
        interrupts.flag.reg = masks[i];
        TEST_ASSERT_EQUAL_INT(interrupt_types[i], interrupts_pending(&interrupts));
    }
}

void test_interrupts_pending_returns_lowest_bit_when_multiple_pending(void) {
    interrupts.enable.reg = 0xFF;
    interrupts.flag.reg = 0xFF;
    TEST_ASSERT_EQUAL_INT(INT_VBLANK, interrupts_pending(&interrupts));
}

void test_interrupts_pending_returns_next_priority_when_higher_disabled(void) {
    interrupts.enable.reg = 0xFC; // disable vblank + lcd
    interrupts.flag.reg = 0xFF;
    TEST_ASSERT_EQUAL_INT(INT_TIMER, interrupts_pending(&interrupts));
}

void test_interrupts_pending_ignores_upper_3_bits(void) {
    interrupts.enable.reg = 0xE0;
    interrupts.flag.reg = 0xE0;
    TEST_ASSERT_EQUAL_INT(-1, interrupts_pending(&interrupts));
}

// ---- interrupts_read ----

void test_interrupts_read_returns_flag_register_at_0xFF0F(void) {
    interrupts.flag.reg = 0xAB;
    TEST_ASSERT_EQUAL_UINT8(0xAB, interrupts_read(&interrupts, 0xFF0F));
}

void test_interrupts_read_returns_enable_register_at_0xFFFF(void) {
    interrupts.enable.reg = 0xCD;
    TEST_ASSERT_EQUAL_UINT8(0xCD, interrupts_read(&interrupts, 0xFFFF));
}

// ---- interrupts_write ----

void test_interrupts_write_sets_flag_register_at_0xFF0F(void) {
    interrupts_write(&interrupts, 0xFF0F, 0x1F);
    TEST_ASSERT_EQUAL_UINT8(0xFF, interrupts.flag.reg);
}

void test_interrupts_write_forces_upper_3_bits_on_flag(void) {
    interrupts_write(&interrupts, 0xFF0F, 0x00);
    TEST_ASSERT_EQUAL_UINT8(0xE0, interrupts.flag.reg);
}

void test_interrupts_write_sets_enable_register_at_0xFFFF(void) {
    interrupts_write(&interrupts, 0xFFFF, 0x12);
    TEST_ASSERT_EQUAL_UINT8(0x12, interrupts.enable.reg);
}

void test_interrupts_write_does_not_force_upper_bits_on_enable(void) {
    interrupts_write(&interrupts, 0xFFFF, 0x00);
    TEST_ASSERT_EQUAL_UINT8(0x00, interrupts.enable.reg);
}

int main(void) {
    UNITY_BEGIN();

    // ---- interrupts_init ----
    RUN_TEST(test_interrupts_init_sets_initial_register_values);

    // ---- interrupts_request ----
    RUN_TEST(test_interrupts_request_sets_correct_bit_for_each_interrupt);
    RUN_TEST(test_interrupts_request_preserves_other_bits);

    // ---- interrupts_acknowledge ----
    RUN_TEST(test_interrupts_acknowledge_clears_correct_bit_for_each_interrupt);
    RUN_TEST(test_interrupts_acknowledge_preserves_other_bits);

    // ---- interrupts_pending ----
    RUN_TEST(test_interrupts_pending_returns_minus_1_when_nothing_pending);
    RUN_TEST(test_interrupts_pending_returns_minus_1_when_flagged_but_not_enabled);
    RUN_TEST(test_interrupts_pending_returns_minus_1_when_enabled_but_not_flagged);
    RUN_TEST(test_interrupts_pending_returns_index_for_each_interrupt);
    RUN_TEST(test_interrupts_pending_returns_lowest_bit_when_multiple_pending);
    RUN_TEST(test_interrupts_pending_returns_next_priority_when_higher_disabled);
    RUN_TEST(test_interrupts_pending_ignores_upper_3_bits);

    // ---- interrupts_read ----
    RUN_TEST(test_interrupts_read_returns_flag_register_at_0xFF0F);
    RUN_TEST(test_interrupts_read_returns_enable_register_at_0xFFFF);

    // ---- interrupts_write ----
    RUN_TEST(test_interrupts_write_sets_flag_register_at_0xFF0F);
    RUN_TEST(test_interrupts_write_forces_upper_3_bits_on_flag);
    RUN_TEST(test_interrupts_write_sets_enable_register_at_0xFFFF);
    RUN_TEST(test_interrupts_write_does_not_force_upper_bits_on_enable);

    return UNITY_END();
}
