/*-------------------------------------------------------
 * Private Opcode 8-bit Arithmetic Instructions Definitions
 *-------------------------------------------------------*/

static int op_add_a_r8(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 1;

    r8_operand_t register_code = opcode & 0b111; // Extract the register code from the opcode

    uint8_t reg_value = read_r8(cpu, bus, register_code);
    uint8_t a = cpu->af.hi;

    alu8_result_t alu_result = alu_add8(a, reg_value, 0);
    cpu->af.hi = alu_result.value;

    // N cleared, Z, H and C set according to result
    flag_clear(cpu, FLAG_N);
    if (alu_result.status.zero)       flag_set(cpu, FLAG_Z); else flag_clear(cpu, FLAG_Z);
    if (alu_result.status.half_carry) flag_set(cpu, FLAG_H); else flag_clear(cpu, FLAG_H);
    if (alu_result.status.carry)      flag_set(cpu, FLAG_C); else flag_clear(cpu, FLAG_C);

    LOG_DEBUG("ADD A,%s: 0x%02X + 0x%02X = 0x%02X at PC=0x%04X (opcode=0x%02X)",
        get_r8_name(register_code), a, reg_value, alu_result.value, instr_pc, opcode);

    return (register_code == OP_MEM_HL) ? 8 : 4; // ADD A,r8 takes 4 cycles for normal r8 register and 8 for [HL]
}

static int op_adc_a_r8(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 1;

    r8_operand_t register_code = opcode & 0b111; // Extract the register code from the opcode

    uint8_t reg_value = read_r8(cpu, bus, register_code);
    uint8_t a = cpu->af.hi;
    uint8_t carry_in = flag_get(cpu, FLAG_C) ? 1 : 0;

    alu8_result_t alu_result = alu_add8(a, reg_value, carry_in);
    cpu->af.hi = alu_result.value;

    // N cleared, Z, H and C set according to result
    flag_clear(cpu, FLAG_N);
    if (alu_result.status.zero)       flag_set(cpu, FLAG_Z); else flag_clear(cpu, FLAG_Z);
    if (alu_result.status.half_carry) flag_set(cpu, FLAG_H); else flag_clear(cpu, FLAG_H);
    if (alu_result.status.carry)      flag_set(cpu, FLAG_C); else flag_clear(cpu, FLAG_C);

    LOG_DEBUG("ADC A,%s: 0x%02X + 0x%02X + %d = 0x%02X at PC=0x%04X (opcode=0x%02X)",
        get_r8_name(register_code), a, reg_value, carry_in, alu_result.value, instr_pc, opcode);

    return (register_code == OP_MEM_HL) ? 8 : 4; // ADC A,r8 takes 4 cycles for normal r8 register and 8 for [HL]
}

static int op_add_a_imm8(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 1;

    uint8_t immediate_value = read_imm8(cpu, bus);
    uint8_t a = cpu->af.hi;

    alu8_result_t result = alu_add8(a, immediate_value, 0);
    cpu->af.hi = result.value;

    // N cleared, Z, H and C set according to result
    flag_clear(cpu, FLAG_N);
    if (result.status.zero)       flag_set(cpu, FLAG_Z); else flag_clear(cpu, FLAG_Z);
    if (result.status.half_carry) flag_set(cpu, FLAG_H); else flag_clear(cpu, FLAG_H);
    if (result.status.carry)      flag_set(cpu, FLAG_C); else flag_clear(cpu, FLAG_C);

    LOG_DEBUG("ADD A,imm8: 0x%02X + 0x%02X = 0x%02X at PC=0x%04X (opcode=0x%02X)",
        a, immediate_value, result.value, instr_pc, opcode);

    return 8;  // ADD A,imm8 takes 8 cycles
}

static int op_adc_a_imm8(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 1;

    uint8_t immediate_value = read_imm8(cpu, bus);
    uint8_t a = cpu->af.hi;
    uint8_t carry_in = flag_get(cpu, FLAG_C) ? 1 : 0;

    alu8_result_t alu_result = alu_add8(a, immediate_value, carry_in);
    cpu->af.hi = alu_result.value;

    // N cleared, Z, H and C set according to result
    flag_clear(cpu, FLAG_N);
    if (alu_result.status.zero)       flag_set(cpu, FLAG_Z); else flag_clear(cpu, FLAG_Z);
    if (alu_result.status.half_carry) flag_set(cpu, FLAG_H); else flag_clear(cpu, FLAG_H);
    if (alu_result.status.carry)      flag_set(cpu, FLAG_C); else flag_clear(cpu, FLAG_C);

    LOG_DEBUG("ADC A,imm8: 0x%02X + 0x%02X + %d = 0x%02X at PC=0x%04X (opcode=0x%02X)",
        a, immediate_value, carry_in, alu_result.value, instr_pc, opcode);

    return 8; // ADC A,imm8 takes 8 cycles
}

static int op_sub_a_r8(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 1;

    r8_operand_t register_code = opcode & 0b111; // Extract the register code from the opcode

    uint8_t reg_value = read_r8(cpu, bus, register_code);
    uint8_t a = cpu->af.hi;

    alu8_result_t result = alu_sub8(a, reg_value, 0);
    cpu->af.hi = result.value;

    // N set, Z, H and C set according to result
    flag_set(cpu, FLAG_N);
    if (result.status.zero)       flag_set(cpu, FLAG_Z); else flag_clear(cpu, FLAG_Z);
    if (result.status.half_carry) flag_set(cpu, FLAG_H); else flag_clear(cpu, FLAG_H);
    if (result.status.carry)      flag_set(cpu, FLAG_C); else flag_clear(cpu, FLAG_C);

    LOG_DEBUG("SUB A,%s: 0x%02X - 0x%02X = 0x%02X at PC=0x%04X (opcode=0x%02X)",
        get_r8_name(register_code), a, reg_value, result.value, instr_pc, opcode);

    return (register_code == OP_MEM_HL) ? 8 : 4; // SUB A,r8 takes 4 cycles for normal r8 register and 8 for [HL]
}

static int op_sbc_a_r8(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 1;

    r8_operand_t register_code = opcode & 0b111; // Extract the register code from the opcode

    uint8_t reg_value = read_r8(cpu, bus, register_code);
    uint8_t a = cpu->af.hi;
    uint8_t carry_in = flag_get(cpu, FLAG_C) ? 1 : 0;

    alu8_result_t result = alu_sub8(a, reg_value, carry_in);
    cpu->af.hi = result.value;

    // N set, Z, H and C set according to result
    flag_set(cpu, FLAG_N);
    if (result.status.zero)        flag_set(cpu, FLAG_Z); else flag_clear(cpu, FLAG_Z);
    if (result.status.half_carry) flag_set(cpu, FLAG_H); else flag_clear(cpu, FLAG_H);
    if (result.status.carry)      flag_set(cpu, FLAG_C); else flag_clear(cpu, FLAG_C);

    LOG_DEBUG("SBC A,%s: 0x%02X - 0x%02X - %d = 0x%02X at PC=0x%04X (opcode=0x%02X)",
        get_r8_name(register_code), a, reg_value, carry_in, result.value, instr_pc, opcode);

    return (register_code == OP_MEM_HL) ? 8 : 4; // SBC A,r8 takes 4 cycles for normal r8 register and 8 for [HL]
}

static int op_sub_a_imm8(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 1;

    uint8_t immediate_value = read_imm8(cpu, bus);
    uint8_t a = cpu->af.hi;

    alu8_result_t result = alu_sub8(a, immediate_value, 0);
    cpu->af.hi = result.value;

    // N set, Z, H and C set according to result
    flag_set(cpu, FLAG_N);
    if (result.status.zero)       flag_set(cpu, FLAG_Z); else flag_clear(cpu, FLAG_Z);
    if (result.status.half_carry) flag_set(cpu, FLAG_H); else flag_clear(cpu, FLAG_H);
    if (result.status.carry)      flag_set(cpu, FLAG_C); else flag_clear(cpu, FLAG_C);

    LOG_DEBUG("SUB A,imm8: 0x%02X - 0x%02X = 0x%02X at PC=0x%04X (opcode=0x%02X)",
        a, immediate_value, result.value, instr_pc, opcode);

    return 8;  // SUB A,imm8 takes 8 cycles
}

static int op_sbc_a_imm8(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 1;

    uint8_t immediate_value = read_imm8(cpu, bus);
    uint8_t a = cpu->af.hi;
    uint8_t carry_in = flag_get(cpu, FLAG_C) ? 1 : 0;

    alu8_result_t alu_result = alu_sub8(a, immediate_value, carry_in);
    cpu->af.hi = alu_result.value;

    // N set, Z, H and C set according to result
    flag_set(cpu, FLAG_N);
    if (alu_result.status.zero)       flag_set(cpu, FLAG_Z); else flag_clear(cpu, FLAG_Z);
    if (alu_result.status.half_carry) flag_set(cpu, FLAG_H); else flag_clear(cpu, FLAG_H);
    if (alu_result.status.carry)      flag_set(cpu, FLAG_C); else flag_clear(cpu, FLAG_C);

    LOG_DEBUG("SBC A,imm8: 0x%02X - 0x%02X - %d = 0x%02X at PC=0x%04X (opcode=0x%02X)",
        a, immediate_value, carry_in, alu_result.value, instr_pc, opcode);

    return 8; // SBC A,imm8 takes 8 cycles
}

static int op_inc_r8(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 1;

    r8_operand_t register_code = (opcode >> 3) & 0b111; // Extract the register code from the opcode

    uint8_t reg_value = read_r8(cpu, bus, register_code);

    alu8_result_t result = alu_inc8(reg_value);

    write_r8(cpu, bus, register_code, result.value);

    flag_clear(cpu, FLAG_N);
    if (result.status.zero) flag_set(cpu, FLAG_Z); else flag_clear(cpu, FLAG_Z);
    if (result.status.half_carry) flag_set(cpu, FLAG_H); else flag_clear(cpu, FLAG_H);

    LOG_DEBUG("INC %s 0x%02X -> 0x%02X at PC=0x%04X (opcode=0x%02X)",
        get_r8_name(register_code), reg_value, result.value, instr_pc, opcode);

    return (register_code == OP_MEM_HL) ? 12 : 4; // INC r8 takes 4 cycles for normal r8 register and 12 for [HL]
}

static int op_dec_r8(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 1;

    r8_operand_t register_code = (opcode >> 3) & 0b111; // Extract the register code from the opcode

    uint8_t reg_value = read_r8(cpu, bus, register_code);

    alu8_result_t result = alu_dec8(reg_value);

    write_r8(cpu, bus, register_code, result.value);

    flag_set(cpu, FLAG_N);
    if (result.status.zero)       flag_set(cpu, FLAG_Z); else flag_clear(cpu, FLAG_Z);
    if (result.status.half_carry) flag_set(cpu, FLAG_H); else flag_clear(cpu, FLAG_H);

    LOG_DEBUG("DEC %s 0x%02X -> 0x%02X at PC=0x%04X (opcode=0x%02X)",
        get_r8_name(register_code), reg_value, result.value, instr_pc, opcode);

    return (register_code == OP_MEM_HL) ? 12 : 4; // DEC r8 takes 4 cycles for normal r8 register and 12 for [HL]
}


static int op_cp_a_r8(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 1;

    r8_operand_t register_code = opcode & 0b111; // Extract the register code from the opcode

    uint8_t reg_value = read_r8(cpu, bus, register_code);
    uint8_t a = cpu->af.hi;

    alu8_result_t result = alu_sub8(a, reg_value, 0);

    // N set, Z, H and C set according to result
    flag_set(cpu, FLAG_N);
    if (result.status.zero)       flag_set(cpu, FLAG_Z); else flag_clear(cpu, FLAG_Z);
    if (result.status.half_carry) flag_set(cpu, FLAG_H); else flag_clear(cpu, FLAG_H);
    if (result.status.carry)      flag_set(cpu, FLAG_C); else flag_clear(cpu, FLAG_C);

    LOG_DEBUG("CP A,%s: 0x%02X - 0x%02X at PC=0x%04X (opcode=0x%02X)",
        get_r8_name(register_code), a, reg_value, instr_pc, opcode);

    return (register_code == OP_MEM_HL) ? 8 : 4; // CP A,r8 takes 4 cycles for normal r8 register and 8 for [HL]
}

static int op_cp_a_imm8(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 1;

    uint8_t immediate_value = read_imm8(cpu, bus);
    uint8_t a = cpu->af.hi;

    alu8_result_t result = alu_sub8(a, immediate_value, 0);

    // N set, Z, H and C set according to result
    flag_set(cpu, FLAG_N);
    if (result.status.zero)       flag_set(cpu, FLAG_Z); else flag_clear(cpu, FLAG_Z);
    if (result.status.half_carry) flag_set(cpu, FLAG_H); else flag_clear(cpu, FLAG_H);
    if (result.status.carry)      flag_set(cpu, FLAG_C); else flag_clear(cpu, FLAG_C);

    LOG_DEBUG("CP A,imm8: 0x%02X - 0x%02X at PC=0x%04X (opcode=0x%02X)",
        a, immediate_value, instr_pc, opcode);

    return 8; // CP A,imm8 takes 8 cycles
}
