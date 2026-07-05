#include "unity.h"
#include "log_helpers.h"

#include "serial_transfer.h"

static st_regs_t serial;

void setUp(void) {
    suppress_logs();

    serial = (st_regs_t){ 0 };
}

void tearDown(void) {
    restore_logs();
}

// ---- serial_transfer_init ----

void test_serial_transfer_init_sets_defaults(void) {
    serial_transfer_init(&serial);

    TEST_ASSERT_EQUAL_HEX8(0x00, serial.sb);
    TEST_ASSERT_EQUAL_HEX8(0x7E, serial.sc);
    TEST_ASSERT_EQUAL_UINT8(0, serial.shift_count);
}

// ---- serial_transfer_read ----

void test_serial_transfer_read_sb(void) {
    serial.sb = 0xA5;
    TEST_ASSERT_EQUAL_HEX8(0xA5, serial_transfer_read(&serial, 0xFF01));
}

void test_serial_transfer_read_sc(void) {
    serial.sc = 0xA5;
    TEST_ASSERT_EQUAL_HEX8(0xA5, serial_transfer_read(&serial, 0xFF02));
}

// ---- serial_transfer_write ----

void test_serial_transfer_write_sb(void) {
    serial_transfer_write(&serial, 0xFF01, 0xA5);
    TEST_ASSERT_EQUAL_HEX8(0xA5, serial.sb);
}

void test_serial_transfer_write_sc_sets_unused_bits(void) {
    // Bit 7 clear: the unused bits 1-6 always read as 1.
    serial_transfer_write(&serial, 0xFF02, 0x01);

    TEST_ASSERT_EQUAL_HEX8(0x7F, serial.sc);
}

void test_serial_transfer_write_sc_zero_keeps_unused_bits(void) {
    serial_transfer_write(&serial, 0xFF02, 0x00);

    TEST_ASSERT_EQUAL_HEX8(0x7E, serial.sc);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_serial_transfer_init_sets_defaults);

    RUN_TEST(test_serial_transfer_read_sb);
    RUN_TEST(test_serial_transfer_read_sc);

    RUN_TEST(test_serial_transfer_write_sb);
    RUN_TEST(test_serial_transfer_write_sc_sets_unused_bits);
    RUN_TEST(test_serial_transfer_write_sc_zero_keeps_unused_bits);

    return UNITY_END();
}
