#include "unity.h"
#include "log_helpers.h"

#include "alu.h"

void setUp(void) {
    suppress_logs();
}

void tearDown(void) {
    restore_logs();
}

// ---- alu_add8 ----

void test_alu_add8_basic_addition(void) {
    alu8_result_t r = alu_add8(0x10, 0x20, 0);
    TEST_ASSERT_EQUAL_UINT8(0x30, r.value);
    TEST_ASSERT_FALSE(r.status.zero);
    TEST_ASSERT_FALSE(r.status.half_carry);
    TEST_ASSERT_FALSE(r.status.carry);
}

void test_alu_add8_zero_result_sets_zero_flag(void) {
    alu8_result_t r = alu_add8(0x00, 0x00, 0);
    TEST_ASSERT_EQUAL_UINT8(0x00, r.value);
    TEST_ASSERT_TRUE(r.status.zero);
    TEST_ASSERT_FALSE(r.status.carry);
}

void test_alu_add8_wraps_on_byte_overflow_and_sets_carry(void) {
    alu8_result_t r = alu_add8(0xFF, 0x01, 0);
    TEST_ASSERT_EQUAL_UINT8(0x00, r.value);
    TEST_ASSERT_TRUE(r.status.zero);
    TEST_ASSERT_TRUE(r.status.carry);
}

void test_alu_add8_sets_half_carry_on_lower_nibble_overflow(void) {
    alu8_result_t r = alu_add8(0x0F, 0x01, 0);
    TEST_ASSERT_EQUAL_UINT8(0x10, r.value);
    TEST_ASSERT_TRUE(r.status.half_carry);
    TEST_ASSERT_FALSE(r.status.carry);
}

void test_alu_add8_no_half_carry_when_no_lower_nibble_overflow(void) {
    alu8_result_t r = alu_add8(0x01, 0x01, 0);
    TEST_ASSERT_EQUAL_UINT8(0x02, r.value);
    TEST_ASSERT_FALSE(r.status.half_carry);
}

void test_alu_add8_with_carry_in(void) {
    alu8_result_t r = alu_add8(0x10, 0x20, 1);
    TEST_ASSERT_EQUAL_UINT8(0x31, r.value);
    TEST_ASSERT_FALSE(r.status.carry);
}

void test_alu_add8_carry_in_triggers_carry_out(void) {
    alu8_result_t r = alu_add8(0xFF, 0x00, 1);
    TEST_ASSERT_EQUAL_UINT8(0x00, r.value);
    TEST_ASSERT_TRUE(r.status.zero);
    TEST_ASSERT_TRUE(r.status.carry);
    TEST_ASSERT_TRUE(r.status.half_carry);
}

void test_alu_add8_carry_in_triggers_half_carry(void) {
    alu8_result_t r = alu_add8(0x0F, 0x00, 1);
    TEST_ASSERT_EQUAL_UINT8(0x10, r.value);
    TEST_ASSERT_TRUE(r.status.half_carry);
    TEST_ASSERT_FALSE(r.status.carry);
}

void test_alu_add8_max_values(void) {
    alu8_result_t r = alu_add8(0xFF, 0xFF, 0);
    TEST_ASSERT_EQUAL_UINT8(0xFE, r.value);
    TEST_ASSERT_TRUE(r.status.carry);
    TEST_ASSERT_TRUE(r.status.half_carry);
}

void test_alu_add8_zero_plus_zero_with_carry(void) {
    alu8_result_t r = alu_add8(0x00, 0x00, 1);
    TEST_ASSERT_EQUAL_UINT8(0x01, r.value);
    TEST_ASSERT_FALSE(r.status.zero);
    TEST_ASSERT_FALSE(r.status.carry);
    TEST_ASSERT_FALSE(r.status.half_carry);
}

// ---- alu_sub8 ----

void test_alu_sub8_basic_subtraction(void) {
    alu8_result_t r = alu_sub8(0x30, 0x10, 0);
    TEST_ASSERT_EQUAL_UINT8(0x20, r.value);
    TEST_ASSERT_FALSE(r.status.zero);
    TEST_ASSERT_FALSE(r.status.half_carry);
    TEST_ASSERT_FALSE(r.status.carry);
}

void test_alu_sub8_zero_result_sets_zero_flag(void) {
    alu8_result_t r = alu_sub8(0x42, 0x42, 0);
    TEST_ASSERT_EQUAL_UINT8(0x00, r.value);
    TEST_ASSERT_TRUE(r.status.zero);
    TEST_ASSERT_FALSE(r.status.carry);
}

void test_alu_sub8_borrow_sets_carry(void) {
    alu8_result_t r = alu_sub8(0x00, 0x01, 0);
    TEST_ASSERT_EQUAL_UINT8(0xFF, r.value);
    TEST_ASSERT_TRUE(r.status.carry);
}

void test_alu_sub8_sets_half_carry_on_lower_nibble_borrow(void) {
    alu8_result_t r = alu_sub8(0x10, 0x01, 0);
    TEST_ASSERT_EQUAL_UINT8(0x0F, r.value);
    TEST_ASSERT_TRUE(r.status.half_carry);
    TEST_ASSERT_FALSE(r.status.carry);
}

void test_alu_sub8_no_half_carry_when_no_nibble_borrow(void) {
    alu8_result_t r = alu_sub8(0x22, 0x11, 0);
    TEST_ASSERT_EQUAL_UINT8(0x11, r.value);
    TEST_ASSERT_FALSE(r.status.half_carry);
    TEST_ASSERT_FALSE(r.status.carry);
}

void test_alu_sub8_with_carry_in(void) {
    alu8_result_t r = alu_sub8(0x30, 0x10, 1);
    TEST_ASSERT_EQUAL_UINT8(0x1F, r.value);
}

void test_alu_sub8_carry_in_sets_half_carry(void) {
    // 0x1F - 0x0F - 1: lower nibble borrow must be detected
    alu8_result_t r = alu_sub8(0x1F, 0x0F, 1);
    TEST_ASSERT_EQUAL_UINT8(0x0F, r.value);
    TEST_ASSERT_TRUE(r.status.half_carry);
    TEST_ASSERT_FALSE(r.status.carry);
}

void test_alu_sub8_carry_in_causes_borrow(void) {
    // 0x00 - 0x00 - 1 = 0xFF with borrow
    alu8_result_t r = alu_sub8(0x00, 0x00, 1);
    TEST_ASSERT_EQUAL_UINT8(0xFF, r.value);
    TEST_ASSERT_TRUE(r.status.carry);
    TEST_ASSERT_TRUE(r.status.half_carry);
}

void test_alu_sub8_carry_in_produces_zero(void) {
    // 0x42 - 0x41 - 1 = 0x00
    alu8_result_t r = alu_sub8(0x42, 0x41, 1);
    TEST_ASSERT_EQUAL_UINT8(0x00, r.value);
    TEST_ASSERT_TRUE(r.status.zero);
    TEST_ASSERT_FALSE(r.status.carry);
}

void test_alu_sub8_max_borrow(void) {
    alu8_result_t r = alu_sub8(0x00, 0xFF, 0);
    TEST_ASSERT_EQUAL_UINT8(0x01, r.value);
    TEST_ASSERT_TRUE(r.status.carry);
    TEST_ASSERT_TRUE(r.status.half_carry);
}

// ---- alu_inc8 ----

void test_alu_inc8_basic_increment(void) {
    alu8_result_t r = alu_inc8(0x01);
    TEST_ASSERT_EQUAL_UINT8(0x02, r.value);
    TEST_ASSERT_FALSE(r.status.zero);
    TEST_ASSERT_FALSE(r.status.half_carry);
    TEST_ASSERT_FALSE(r.status.carry);
}

void test_alu_inc8_wraps_to_zero(void) {
    alu8_result_t r = alu_inc8(0xFF);
    TEST_ASSERT_EQUAL_UINT8(0x00, r.value);
    TEST_ASSERT_TRUE(r.status.zero);
    TEST_ASSERT_TRUE(r.status.half_carry);
}

void test_alu_inc8_sets_half_carry_on_nibble_overflow(void) {
    alu8_result_t r = alu_inc8(0x0F);
    TEST_ASSERT_EQUAL_UINT8(0x10, r.value);
    TEST_ASSERT_TRUE(r.status.half_carry);
    TEST_ASSERT_FALSE(r.status.zero);
}

void test_alu_inc8_no_half_carry_when_no_nibble_overflow(void) {
    alu8_result_t r = alu_inc8(0x10);
    TEST_ASSERT_EQUAL_UINT8(0x11, r.value);
    TEST_ASSERT_FALSE(r.status.half_carry);
}

void test_alu_inc8_zero_increments_to_one(void) {
    alu8_result_t r = alu_inc8(0x00);
    TEST_ASSERT_EQUAL_UINT8(0x01, r.value);
    TEST_ASSERT_FALSE(r.status.zero);
    TEST_ASSERT_FALSE(r.status.half_carry);
}

void test_alu_inc8_carry_is_always_false(void) {
    // INC does not affect the carry flag — it is always reported false
    alu8_result_t r = alu_inc8(0xFF);
    TEST_ASSERT_FALSE(r.status.carry);
}

// ---- alu_dec8 ----

void test_alu_dec8_basic_decrement(void) {
    alu8_result_t r = alu_dec8(0x05);
    TEST_ASSERT_EQUAL_UINT8(0x04, r.value);
    TEST_ASSERT_FALSE(r.status.zero);
    TEST_ASSERT_FALSE(r.status.half_carry);
    TEST_ASSERT_FALSE(r.status.carry);
}

void test_alu_dec8_one_becomes_zero(void) {
    alu8_result_t r = alu_dec8(0x01);
    TEST_ASSERT_EQUAL_UINT8(0x00, r.value);
    TEST_ASSERT_TRUE(r.status.zero);
    TEST_ASSERT_FALSE(r.status.half_carry);
}

void test_alu_dec8_wraps_to_0xFF(void) {
    alu8_result_t r = alu_dec8(0x00);
    TEST_ASSERT_EQUAL_UINT8(0xFF, r.value);
    TEST_ASSERT_FALSE(r.status.zero);
    TEST_ASSERT_TRUE(r.status.half_carry);
}

void test_alu_dec8_sets_half_carry_on_nibble_borrow(void) {
    alu8_result_t r = alu_dec8(0x10);
    TEST_ASSERT_EQUAL_UINT8(0x0F, r.value);
    TEST_ASSERT_TRUE(r.status.half_carry);
}

void test_alu_dec8_no_half_carry_when_no_nibble_borrow(void) {
    alu8_result_t r = alu_dec8(0x08);
    TEST_ASSERT_EQUAL_UINT8(0x07, r.value);
    TEST_ASSERT_FALSE(r.status.half_carry);
}

void test_alu_dec8_carry_is_always_false(void) {
    alu8_result_t r = alu_dec8(0x00);
    TEST_ASSERT_FALSE(r.status.carry);
}

// ---- alu_and8 ----

void test_alu_and8_basic_and(void) {
    alu8_result_t r = alu_and8(0xF0, 0x0F);
    TEST_ASSERT_EQUAL_UINT8(0x00, r.value);
    TEST_ASSERT_TRUE(r.status.zero);
}

void test_alu_and8_non_zero_result(void) {
    alu8_result_t r = alu_and8(0xFF, 0xAA);
    TEST_ASSERT_EQUAL_UINT8(0xAA, r.value);
    TEST_ASSERT_FALSE(r.status.zero);
}

void test_alu_and8_with_itself(void) {
    alu8_result_t r = alu_and8(0x5A, 0x5A);
    TEST_ASSERT_EQUAL_UINT8(0x5A, r.value);
    TEST_ASSERT_FALSE(r.status.zero);
}

void test_alu_and8_zero_operand_produces_zero(void) {
    alu8_result_t r = alu_and8(0xFF, 0x00);
    TEST_ASSERT_EQUAL_UINT8(0x00, r.value);
    TEST_ASSERT_TRUE(r.status.zero);
}

void test_alu_and8_zero_with_zero(void) {
    alu8_result_t r = alu_and8(0x00, 0x00);
    TEST_ASSERT_EQUAL_UINT8(0x00, r.value);
    TEST_ASSERT_TRUE(r.status.zero);
}

// ---- alu_or8 ----

void test_alu_or8_basic_or(void) {
    alu8_result_t r = alu_or8(0xF0, 0x0F);
    TEST_ASSERT_EQUAL_UINT8(0xFF, r.value);
    TEST_ASSERT_FALSE(r.status.zero);
}

void test_alu_or8_zero_result(void) {
    alu8_result_t r = alu_or8(0x00, 0x00);
    TEST_ASSERT_EQUAL_UINT8(0x00, r.value);
    TEST_ASSERT_TRUE(r.status.zero);
}

void test_alu_or8_with_zero_operand(void) {
    alu8_result_t r = alu_or8(0xAB, 0x00);
    TEST_ASSERT_EQUAL_UINT8(0xAB, r.value);
    TEST_ASSERT_FALSE(r.status.zero);
}

void test_alu_or8_with_itself(void) {
    alu8_result_t r = alu_or8(0x5A, 0x5A);
    TEST_ASSERT_EQUAL_UINT8(0x5A, r.value);
    TEST_ASSERT_FALSE(r.status.zero);
}

void test_alu_or8_overlapping_bits(void) {
    alu8_result_t r = alu_or8(0xAA, 0x55);
    TEST_ASSERT_EQUAL_UINT8(0xFF, r.value);
    TEST_ASSERT_FALSE(r.status.zero);
}

// ---- alu_xor8 ----

void test_alu_xor8_basic_xor(void) {
    alu8_result_t r = alu_xor8(0xFF, 0x0F);
    TEST_ASSERT_EQUAL_UINT8(0xF0, r.value);
    TEST_ASSERT_FALSE(r.status.zero);
}

void test_alu_xor8_with_itself_produces_zero(void) {
    alu8_result_t r = alu_xor8(0xAB, 0xAB);
    TEST_ASSERT_EQUAL_UINT8(0x00, r.value);
    TEST_ASSERT_TRUE(r.status.zero);
}

void test_alu_xor8_with_zero_operand(void) {
    alu8_result_t r = alu_xor8(0xCD, 0x00);
    TEST_ASSERT_EQUAL_UINT8(0xCD, r.value);
    TEST_ASSERT_FALSE(r.status.zero);
}

void test_alu_xor8_zero_with_zero(void) {
    alu8_result_t r = alu_xor8(0x00, 0x00);
    TEST_ASSERT_EQUAL_UINT8(0x00, r.value);
    TEST_ASSERT_TRUE(r.status.zero);
}

void test_alu_xor8_alternating_bits(void) {
    alu8_result_t r = alu_xor8(0xAA, 0x55);
    TEST_ASSERT_EQUAL_UINT8(0xFF, r.value);
    TEST_ASSERT_FALSE(r.status.zero);
}

// ---- alu_add16 ----

void test_alu_add16_basic_addition(void) {
    alu16_result_t r = alu_add16(0x0003, 0x0005);
    TEST_ASSERT_EQUAL_UINT16(0x0008, r.value);
    TEST_ASSERT_FALSE(r.status.half_carry);
    TEST_ASSERT_FALSE(r.status.carry);
}

void test_alu_add16_zero_plus_zero(void) {
    alu16_result_t r = alu_add16(0x0000, 0x0000);
    TEST_ASSERT_EQUAL_UINT16(0x0000, r.value);
    TEST_ASSERT_FALSE(r.status.half_carry);
    TEST_ASSERT_FALSE(r.status.carry);
}

void test_alu_add16_wraps_on_overflow_and_sets_carry(void) {
    alu16_result_t r = alu_add16(0xFFFF, 0x0001);
    TEST_ASSERT_EQUAL_UINT16(0x0000, r.value);
    TEST_ASSERT_TRUE(r.status.carry);
}

void test_alu_add16_no_carry_below_max(void) {
    alu16_result_t r = alu_add16(0xFFFE, 0x0001);
    TEST_ASSERT_EQUAL_UINT16(0xFFFF, r.value);
    TEST_ASSERT_FALSE(r.status.carry);
}

void test_alu_add16_max_values_sets_carry(void) {
    alu16_result_t r = alu_add16(0xFFFF, 0xFFFF);
    TEST_ASSERT_EQUAL_UINT16(0xFFFE, r.value);
    TEST_ASSERT_TRUE(r.status.carry);
    TEST_ASSERT_TRUE(r.status.half_carry);
}

void test_alu_add16_sets_half_carry_on_bit11_overflow(void) {
    // 0x0FFF + 0x0001 = 0x1000: overflow from bit 11
    alu16_result_t r = alu_add16(0x0FFF, 0x0001);
    TEST_ASSERT_EQUAL_UINT16(0x1000, r.value);
    TEST_ASSERT_TRUE(r.status.half_carry);
    TEST_ASSERT_FALSE(r.status.carry);
}

void test_alu_add16_no_half_carry_just_below_bit11_overflow(void) {
    // 0x0FFE + 0x0001 = 0x0FFF: no overflow from bit 11
    alu16_result_t r = alu_add16(0x0FFE, 0x0001);
    TEST_ASSERT_EQUAL_UINT16(0x0FFF, r.value);
    TEST_ASSERT_FALSE(r.status.half_carry);
    TEST_ASSERT_FALSE(r.status.carry);
}

void test_alu_add16_half_carry_ignores_upper_nibble(void) {
    // Lower 12 bits: 0x0FFF + 0x0001 overflow, upper bits should not affect it
    alu16_result_t r = alu_add16(0x1FFF, 0x0001);
    TEST_ASSERT_EQUAL_UINT16(0x2000, r.value);
    TEST_ASSERT_TRUE(r.status.half_carry);
    TEST_ASSERT_FALSE(r.status.carry);
}

void test_alu_add16_carry_without_half_carry(void) {
    // 0xF000 + 0x1000 = 0x10000: carry set, lower 12 bits don't overflow
    alu16_result_t r = alu_add16(0xF000, 0x1000);
    TEST_ASSERT_EQUAL_UINT16(0x0000, r.value);
    TEST_ASSERT_TRUE(r.status.carry);
    TEST_ASSERT_FALSE(r.status.half_carry);
}

void test_alu_add16_add_with_itself(void) {
    alu16_result_t r = alu_add16(0x0800, 0x0800);
    TEST_ASSERT_EQUAL_UINT16(0x1000, r.value);
    TEST_ASSERT_TRUE(r.status.half_carry);
    TEST_ASSERT_FALSE(r.status.carry);
}

// ---- alu_add16_s8 ----

void test_alu_add16_s8_basic_positive_offset(void) {
    alu16_result_t r = alu_add16_s8(0x1000, 0x05);
    TEST_ASSERT_EQUAL_UINT16(0x1005, r.value);
    TEST_ASSERT_FALSE(r.status.half_carry);
    TEST_ASSERT_FALSE(r.status.carry);
}

void test_alu_add16_s8_basic_negative_offset(void) {
    alu16_result_t r = alu_add16_s8(0x1010, -0x05);
    TEST_ASSERT_EQUAL_UINT16(0x100B, r.value);
}

void test_alu_add16_s8_zero_offset_clears_flags(void) {
    alu16_result_t r = alu_add16_s8(0x1234, 0);
    TEST_ASSERT_EQUAL_UINT16(0x1234, r.value);
    TEST_ASSERT_FALSE(r.status.half_carry);
    TEST_ASSERT_FALSE(r.status.carry);
}

void test_alu_add16_s8_sets_half_carry_on_bit3_overflow(void) {
    // Lower nibble overflows: 0x0F + 0x01 = 0x10
    alu16_result_t r = alu_add16_s8(0x000F, 0x01);
    TEST_ASSERT_EQUAL_UINT16(0x0010, r.value);
    TEST_ASSERT_TRUE(r.status.half_carry);
    TEST_ASSERT_FALSE(r.status.carry);
}

void test_alu_add16_s8_no_half_carry_just_below_bit3_overflow(void) {
    alu16_result_t r = alu_add16_s8(0x000E, 0x01);
    TEST_ASSERT_EQUAL_UINT16(0x000F, r.value);
    TEST_ASSERT_FALSE(r.status.half_carry);
    TEST_ASSERT_FALSE(r.status.carry);
}

void test_alu_add16_s8_sets_carry_on_bit7_overflow(void) {
    // Lower byte overflows: 0xFF + 0x01 = 0x100
    alu16_result_t r = alu_add16_s8(0x00FF, 0x01);
    TEST_ASSERT_EQUAL_UINT16(0x0100, r.value);
    TEST_ASSERT_TRUE(r.status.half_carry);
    TEST_ASSERT_TRUE(r.status.carry);
}

void test_alu_add16_s8_no_carry_below_bit7_overflow(void) {
    alu16_result_t r = alu_add16_s8(0x007F, 0x01);
    TEST_ASSERT_EQUAL_UINT16(0x0080, r.value);
    TEST_ASSERT_FALSE(r.status.carry);
}

void test_alu_add16_s8_flags_use_lower_byte_only(void) {
    // Upper byte of base must not influence flags
    alu16_result_t r = alu_add16_s8(0xFF00, 0x01);
    TEST_ASSERT_EQUAL_UINT16(0xFF01, r.value);
    TEST_ASSERT_FALSE(r.status.half_carry);
    TEST_ASSERT_FALSE(r.status.carry);
}

void test_alu_add16_s8_max_positive_offset(void) {
    alu16_result_t r = alu_add16_s8(0x1000, 0x7F);
    TEST_ASSERT_EQUAL_UINT16(0x107F, r.value);
    TEST_ASSERT_FALSE(r.status.half_carry);
    TEST_ASSERT_FALSE(r.status.carry);
}

void test_alu_add16_s8_max_negative_offset(void) {
    // offset = -128 (0x80 as unsigned): low nibble is 0, low byte is 0x80
    alu16_result_t r = alu_add16_s8(0x0100, -128);
    TEST_ASSERT_EQUAL_UINT16(0x0080, r.value);
    TEST_ASSERT_FALSE(r.status.half_carry);
    TEST_ASSERT_FALSE(r.status.carry);
}

void test_alu_add16_s8_negative_offset_sets_both_flags(void) {
    // 0x0001 + (-1): low byte 0x01 + 0xFF = 0x100, low nibble 0x01 + 0x0F = 0x10
    alu16_result_t r = alu_add16_s8(0x0001, -1);
    TEST_ASSERT_EQUAL_UINT16(0x0000, r.value);
    TEST_ASSERT_TRUE(r.status.half_carry);
    TEST_ASSERT_TRUE(r.status.carry);
}

void test_alu_add16_s8_negative_offset_no_flags_when_low_byte_zero(void) {
    // 0x0100 + (-1) = 0x00FF: low byte 0x00 + 0xFF = 0xFF (no carry/half-carry)
    alu16_result_t r = alu_add16_s8(0x0100, -1);
    TEST_ASSERT_EQUAL_UINT16(0x00FF, r.value);
    TEST_ASSERT_FALSE(r.status.half_carry);
    TEST_ASSERT_FALSE(r.status.carry);
}

void test_alu_add16_s8_wraps_around_16bit_boundary(void) {
    // 0xFFFF + 1 = 0x0000 (16-bit wrap); flags reflect lower-byte arithmetic
    alu16_result_t r = alu_add16_s8(0xFFFF, 0x01);
    TEST_ASSERT_EQUAL_UINT16(0x0000, r.value);
    TEST_ASSERT_TRUE(r.status.half_carry);
    TEST_ASSERT_TRUE(r.status.carry);
}

int main(void) {
    UNITY_BEGIN();

    // alu_add8
    RUN_TEST(test_alu_add8_basic_addition);
    RUN_TEST(test_alu_add8_zero_result_sets_zero_flag);
    RUN_TEST(test_alu_add8_wraps_on_byte_overflow_and_sets_carry);
    RUN_TEST(test_alu_add8_sets_half_carry_on_lower_nibble_overflow);
    RUN_TEST(test_alu_add8_no_half_carry_when_no_lower_nibble_overflow);
    RUN_TEST(test_alu_add8_with_carry_in);
    RUN_TEST(test_alu_add8_carry_in_triggers_carry_out);
    RUN_TEST(test_alu_add8_carry_in_triggers_half_carry);
    RUN_TEST(test_alu_add8_max_values);
    RUN_TEST(test_alu_add8_zero_plus_zero_with_carry);

    // alu_sub8
    RUN_TEST(test_alu_sub8_basic_subtraction);
    RUN_TEST(test_alu_sub8_zero_result_sets_zero_flag);
    RUN_TEST(test_alu_sub8_borrow_sets_carry);
    RUN_TEST(test_alu_sub8_sets_half_carry_on_lower_nibble_borrow);
    RUN_TEST(test_alu_sub8_no_half_carry_when_no_nibble_borrow);
    RUN_TEST(test_alu_sub8_with_carry_in);
    RUN_TEST(test_alu_sub8_carry_in_sets_half_carry);
    RUN_TEST(test_alu_sub8_carry_in_causes_borrow);
    RUN_TEST(test_alu_sub8_carry_in_produces_zero);
    RUN_TEST(test_alu_sub8_max_borrow);

    // alu_inc8
    RUN_TEST(test_alu_inc8_basic_increment);
    RUN_TEST(test_alu_inc8_wraps_to_zero);
    RUN_TEST(test_alu_inc8_sets_half_carry_on_nibble_overflow);
    RUN_TEST(test_alu_inc8_no_half_carry_when_no_nibble_overflow);
    RUN_TEST(test_alu_inc8_zero_increments_to_one);
    RUN_TEST(test_alu_inc8_carry_is_always_false);

    // alu_dec8
    RUN_TEST(test_alu_dec8_basic_decrement);
    RUN_TEST(test_alu_dec8_one_becomes_zero);
    RUN_TEST(test_alu_dec8_wraps_to_0xFF);
    RUN_TEST(test_alu_dec8_sets_half_carry_on_nibble_borrow);
    RUN_TEST(test_alu_dec8_no_half_carry_when_no_nibble_borrow);
    RUN_TEST(test_alu_dec8_carry_is_always_false);

    // alu_and8
    RUN_TEST(test_alu_and8_basic_and);
    RUN_TEST(test_alu_and8_non_zero_result);
    RUN_TEST(test_alu_and8_with_itself);
    RUN_TEST(test_alu_and8_zero_operand_produces_zero);
    RUN_TEST(test_alu_and8_zero_with_zero);

    // alu_or8
    RUN_TEST(test_alu_or8_basic_or);
    RUN_TEST(test_alu_or8_zero_result);
    RUN_TEST(test_alu_or8_with_zero_operand);
    RUN_TEST(test_alu_or8_with_itself);
    RUN_TEST(test_alu_or8_overlapping_bits);

    // alu_xor8
    RUN_TEST(test_alu_xor8_basic_xor);
    RUN_TEST(test_alu_xor8_with_itself_produces_zero);
    RUN_TEST(test_alu_xor8_with_zero_operand);
    RUN_TEST(test_alu_xor8_zero_with_zero);
    RUN_TEST(test_alu_xor8_alternating_bits);

    // alu_add16
    RUN_TEST(test_alu_add16_basic_addition);
    RUN_TEST(test_alu_add16_zero_plus_zero);
    RUN_TEST(test_alu_add16_wraps_on_overflow_and_sets_carry);
    RUN_TEST(test_alu_add16_no_carry_below_max);
    RUN_TEST(test_alu_add16_max_values_sets_carry);
    RUN_TEST(test_alu_add16_sets_half_carry_on_bit11_overflow);
    RUN_TEST(test_alu_add16_no_half_carry_just_below_bit11_overflow);
    RUN_TEST(test_alu_add16_half_carry_ignores_upper_nibble);
    RUN_TEST(test_alu_add16_carry_without_half_carry);
    RUN_TEST(test_alu_add16_add_with_itself);

    // alu_add16_s8
    RUN_TEST(test_alu_add16_s8_basic_positive_offset);
    RUN_TEST(test_alu_add16_s8_basic_negative_offset);
    RUN_TEST(test_alu_add16_s8_zero_offset_clears_flags);
    RUN_TEST(test_alu_add16_s8_sets_half_carry_on_bit3_overflow);
    RUN_TEST(test_alu_add16_s8_no_half_carry_just_below_bit3_overflow);
    RUN_TEST(test_alu_add16_s8_sets_carry_on_bit7_overflow);
    RUN_TEST(test_alu_add16_s8_no_carry_below_bit7_overflow);
    RUN_TEST(test_alu_add16_s8_flags_use_lower_byte_only);
    RUN_TEST(test_alu_add16_s8_max_positive_offset);
    RUN_TEST(test_alu_add16_s8_max_negative_offset);
    RUN_TEST(test_alu_add16_s8_negative_offset_sets_both_flags);
    RUN_TEST(test_alu_add16_s8_negative_offset_no_flags_when_low_byte_zero);
    RUN_TEST(test_alu_add16_s8_wraps_around_16bit_boundary);

    return UNITY_END();
}
