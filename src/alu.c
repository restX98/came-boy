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

    return (alu_result_t) {
        .value = (uint8_t)r,
            .status = {
                .zero = ((uint8_t)r == 0),
                .half_carry = (a & 0xF) < ((value & 0xF) + carry),
                .carry = a < (value + carry)
        }
    };
}
