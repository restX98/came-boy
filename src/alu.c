#include "alu.h"

alu_result_t alu_add(uint8_t a, uint8_t value, uint8_t carry) {
    uint16_t r = a + value + carry;

    return (alu_result_t) {
        .value = (uint8_t)r,
            .status = {
                .zero = ((uint8_t)r == 0),
                .half_carry = ((a & 0xF) + (value & 0xF) + carry) > 0xF,
                .carry = r > 0xFF
        }
    };
}

alu_result_t alu_sub(uint8_t a, uint8_t value, uint8_t carry) {
    uint16_t r = a - value - carry;

    // Carry must not be added to reg_value before masking, as doing so
    // can hide a half-borrow due to overflow in the lower nibble.
    // Example:
    // A     = 0001 1111                         |  A           = 0001 1111
    // reg   = 0000 1111                         |  reg + carry = 0001 0000
    // carry = 0000 0001                         |  diff        = 0000 1111 -> half borrow undetected
    // diff  = 0000 1111 -> half borrow detected |
    return (alu_result_t) {
        .value = (uint8_t)r,
            .status = {
                .zero = ((uint8_t)r == 0),
                .half_carry = (a & 0xF) < ((value & 0xF) + carry),
                .carry = a < (value + carry)
        }
    };
}
