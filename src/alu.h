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
} alu8_result_t;

alu8_result_t alu_add8(uint8_t a, uint8_t value, uint8_t carry);
alu8_result_t alu_sub8(uint8_t a, uint8_t value, uint8_t carry);

alu8_result_t alu_inc8(uint8_t value);
alu8_result_t alu_dec8(uint8_t value);

alu8_result_t alu_and8(uint8_t a, uint8_t value);
alu8_result_t alu_or8(uint8_t a, uint8_t value);
alu8_result_t alu_xor8(uint8_t a, uint8_t value);

#endif // ALU_H
