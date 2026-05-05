#ifndef ALU_H
#define ALU_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t value;
    struct {
        bool zero;
        bool half_carry;
        bool carry;
    } status;
} alu_result_t;

alu_result_t alu_add(uint8_t a, uint8_t value, uint8_t carry);
alu_result_t alu_sub(uint8_t a, uint8_t value, uint8_t carry);

// uint8_t alu_and(uint8_t a, uint8_t v);
// uint8_t alu_or(uint8_t a, uint8_t v);
// uint8_t alu_xor(uint8_t a, uint8_t v);

#endif // ALU_H
