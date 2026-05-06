#include "alu.h"

alu8_result_t alu_add8(uint8_t a, uint8_t value, uint8_t carry) {
    uint16_t r = a + value + carry;

    return (alu8_result_t) {
        .value = (uint8_t)r,
            .status = {
                .zero = ((uint8_t)r == 0),
                .half_carry = ((a & 0xF) + (value & 0xF) + carry) > 0xF,
                .carry = r > 0xFF
        }
    };
}

alu8_result_t alu_sub8(uint8_t a, uint8_t value, uint8_t carry) {
    uint16_t r = a - value - carry;

    // Carry must not be added to reg_value before masking, as doing so
    // can hide a half-borrow due to overflow in the lower nibble.
    // Example:
    // A     = 0001 1111                         |  A           = 0001 1111
    // reg   = 0000 1111                         |  reg + carry = 0001 0000
    // carry = 0000 0001                         |  diff        = 0000 1111 -> half borrow undetected
    // diff  = 0000 1111 -> half borrow detected |
    return (alu8_result_t) {
        .value = (uint8_t)r,
            .status = {
                .zero = ((uint8_t)r == 0),
                .half_carry = (a & 0xF) < ((value & 0xF) + carry),
                .carry = a < (value + carry)
        }
    };
}

alu8_result_t alu_inc8(uint8_t value) {
    uint8_t r = value + 1;

    return (alu8_result_t) {
        .value = (uint8_t)r,
            .status = {
                .zero = (r == 0),
                .half_carry = (value & 0xF) == 0x0F,
                .carry = false
        }
    };
}

alu8_result_t alu_dec8(uint8_t value) {
    uint8_t r = value - 1;

    return (alu8_result_t) {
        .value = (uint8_t)r,
            .status = {
                .zero = (r == 0),
                .half_carry = (value & 0xF) == 0x00,
                .carry = false
        }
    };
}

alu8_result_t alu_and8(uint8_t a, uint8_t value) {
    uint8_t r = a & value;

    return (alu8_result_t) {
        .value = r,
            .status = {
                .zero = (r == 0),
        }
    };
}

alu8_result_t alu_or8(uint8_t a, uint8_t value) {
    uint8_t r = a | value;

    return (alu8_result_t) {
        .value = r,
            .status = {
                .zero = (r == 0),
        }
    };
}

alu8_result_t alu_xor8(uint8_t a, uint8_t value) {
    uint8_t r = a ^ value;

    return (alu8_result_t) {
        .value = r,
            .status = {
                .zero = (r == 0),
        }
    };
}

alu16_result_t alu_add16(uint16_t hl, uint16_t value) {
    uint32_t r = hl + value;

    return (alu16_result_t) {
        .value = (uint16_t)r,
            .status = {
                .half_carry = ((hl & 0x0FFF) + (value & 0x0FFF)) > 0x0FFF,
                .carry = r > 0xFFFF,
        }
    };
}
