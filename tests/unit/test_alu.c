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

    return UNITY_END();
}
