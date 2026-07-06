#include "unity.h"
#include "log_helpers.h"

#include "io/joypad.h"

static joypad_reg_t jp;

void setUp(void) {
    suppress_logs();

    jp = (joypad_reg_t){ 0 };
}

void tearDown(void) {
    restore_logs();
}

// ---- joypad_init ----

void test_joypad_init_sets_power_on_state(void) {
    joypad_init(&jp);

    TEST_ASSERT_EQUAL_HEX8(0x00, jp.select_bits);
    TEST_ASSERT_EQUAL_HEX8(0x0F, jp.dpad_state);
    TEST_ASSERT_EQUAL_HEX8(0x0F, jp.button_state);
    // Reads of $FF00 at power-on must return 0xCF.
    TEST_ASSERT_EQUAL_HEX8(0xCF, joypad_read(&jp));
}

// ---- joypad_read ----

void test_joypad_read_no_row_selected_returns_high_nibble(void) {
    // Both select lines high (=not selected); pressed keys must be ignored.
    jp.select_bits = JOYP_SELECT_DPAD | JOYP_SELECT_BUTTONS; // 0x30
    jp.dpad_state = 0x00;   // everything "pressed"
    jp.button_state = 0x00;

    // 0xC0 | 0x30 | 0x0F
    TEST_ASSERT_EQUAL_HEX8(0xFF, joypad_read(&jp));
}

void test_joypad_read_dpad_selected(void) {
    jp.select_bits = JOYP_SELECT_BUTTONS; // 0x20: bit4=0 selects D-Pad
    jp.dpad_state = 0x0E;                  // Right pressed
    jp.button_state = 0x00;                // ignored (not selected)

    // 0xC0 | 0x20 | 0x0E
    TEST_ASSERT_EQUAL_HEX8(0xEE, joypad_read(&jp));
}

void test_joypad_read_buttons_selected(void) {
    jp.select_bits = JOYP_SELECT_DPAD; // 0x10: bit5=0 selects Buttons
    jp.button_state = 0x0D;            // B pressed
    jp.dpad_state = 0x00;              // ignored (not selected)

    // 0xC0 | 0x10 | 0x0D
    TEST_ASSERT_EQUAL_HEX8(0xDD, joypad_read(&jp));
}

void test_joypad_read_both_rows_selected_ands_states(void) {
    jp.select_bits = 0x00; // both rows selected
    jp.dpad_state = 0x0E;
    jp.button_state = 0x0D;

    // nibble = 0x0F & 0x0E & 0x0D = 0x0C; 0xC0 | 0x00 | 0x0C
    TEST_ASSERT_EQUAL_HEX8(0xCC, joypad_read(&jp));
}

// ---- joypad_write ----

void test_joypad_write_only_keeps_select_bits(void) {
    joypad_write(&jp, 0xFF); // high bits and low nibble ignored
    TEST_ASSERT_EQUAL_HEX8(0x30, jp.select_bits);

    joypad_write(&jp, 0x0F); // low nibble ignored -> no select bits
    TEST_ASSERT_EQUAL_HEX8(0x00, jp.select_bits);

    joypad_write(&jp, 0xD0); // 0xD0 & 0x30 == 0x10
    TEST_ASSERT_EQUAL_HEX8(0x10, jp.select_bits);
}

// ---- joypad_press_dpad ----

void test_joypad_press_dpad_selected_fires_interrupt(void) {
    jp.select_bits = 0x00; // D-Pad selected
    jp.dpad_state = 0x0F;  // Right released

    bool interrupt = joypad_press_dpad(&jp, JOYPAD_RIGHT);

    TEST_ASSERT_TRUE(interrupt);
    TEST_ASSERT_EQUAL_HEX8(0x0E, jp.dpad_state); // press clears the bit
}

void test_joypad_press_dpad_not_selected_no_interrupt_but_updates_state(void) {
    jp.select_bits = JOYP_SELECT_DPAD; // 0x10: D-Pad NOT selected
    jp.dpad_state = 0x0F;

    bool interrupt = joypad_press_dpad(&jp, JOYPAD_RIGHT);

    TEST_ASSERT_FALSE(interrupt);
    TEST_ASSERT_EQUAL_HEX8(0x0E, jp.dpad_state); // still recorded
}

void test_joypad_press_dpad_already_pressed_no_interrupt(void) {
    jp.select_bits = 0x00;
    jp.dpad_state = 0x0E; // Right already pressed

    bool interrupt = joypad_press_dpad(&jp, JOYPAD_RIGHT);

    TEST_ASSERT_FALSE(interrupt); // no high->low transition
    TEST_ASSERT_EQUAL_HEX8(0x0E, jp.dpad_state);
}

// ---- joypad_press_button ----

void test_joypad_press_button_selected_fires_interrupt(void) {
    jp.select_bits = 0x00; // Buttons selected
    jp.button_state = 0x0F;

    bool interrupt = joypad_press_button(&jp, JOYPAD_A);

    TEST_ASSERT_TRUE(interrupt);
    TEST_ASSERT_EQUAL_HEX8(0x0E, jp.button_state);
}

void test_joypad_press_button_not_selected_no_interrupt(void) {
    jp.select_bits = JOYP_SELECT_BUTTONS; // 0x20: Buttons NOT selected
    jp.button_state = 0x0F;

    bool interrupt = joypad_press_button(&jp, JOYPAD_A);

    TEST_ASSERT_FALSE(interrupt);
    TEST_ASSERT_EQUAL_HEX8(0x0E, jp.button_state);
}

// ---- joypad_release ----

void test_joypad_release_dpad_sets_bit(void) {
    jp.dpad_state = 0x0E; // Right pressed

    joypad_release_dpad(&jp, JOYPAD_RIGHT);

    TEST_ASSERT_EQUAL_HEX8(0x0F, jp.dpad_state);
}

void test_joypad_release_button_sets_bit(void) {
    jp.button_state = 0x0E; // A pressed

    joypad_release_button(&jp, JOYPAD_A);

    TEST_ASSERT_EQUAL_HEX8(0x0F, jp.button_state);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_joypad_init_sets_power_on_state);

    RUN_TEST(test_joypad_read_no_row_selected_returns_high_nibble);
    RUN_TEST(test_joypad_read_dpad_selected);
    RUN_TEST(test_joypad_read_buttons_selected);
    RUN_TEST(test_joypad_read_both_rows_selected_ands_states);

    RUN_TEST(test_joypad_write_only_keeps_select_bits);

    RUN_TEST(test_joypad_press_dpad_selected_fires_interrupt);
    RUN_TEST(test_joypad_press_dpad_not_selected_no_interrupt_but_updates_state);
    RUN_TEST(test_joypad_press_dpad_already_pressed_no_interrupt);

    RUN_TEST(test_joypad_press_button_selected_fires_interrupt);
    RUN_TEST(test_joypad_press_button_not_selected_no_interrupt);

    RUN_TEST(test_joypad_release_dpad_sets_bit);
    RUN_TEST(test_joypad_release_button_sets_bit);

    return UNITY_END();
}
