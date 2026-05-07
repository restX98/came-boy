/*-------------------------------------------------------
 * Private Opcode Bitwise Logic Instructions Definitions
 *-------------------------------------------------------*/

static int op_and_a_r8(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 1;

    r8_operand_t register_code = opcode & 0b111; // Extract the register code from the opcode

    uint8_t reg_value = read_r8(cpu, bus, register_code);
    uint8_t a = cpu->af.hi;

    alu8_result_t result = alu_and8(a, reg_value);
    cpu->af.hi = result.value;

    if (result.status.zero) flag_set(cpu, FLAG_Z); else flag_clear(cpu, FLAG_Z);
    flag_clear(cpu, FLAG_N);
    flag_set(cpu, FLAG_H);
    flag_clear(cpu, FLAG_C);

    LOG_DEBUG("AND A,%s: 0x%02X & 0x%02X = 0x%02X at PC=0x%04X (opcode=0x%02X)",
        get_r8_name(register_code), a, reg_value, result.value, instr_pc, opcode);

    return (register_code == OP_MEM_HL) ? 8 : 4; // AND A,r8 takes 4 cycles for normal r8 register and 8 for [HL]
}

static int op_xor_a_r8(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 1;

    r8_operand_t register_code = opcode & 0b111; // Extract the register code from the opcode

    uint8_t reg_value = read_r8(cpu, bus, register_code);
    uint8_t a = cpu->af.hi;

    alu8_result_t result = alu_xor8(a, reg_value);
    cpu->af.hi = result.value;

    if (result.status.zero) flag_set(cpu, FLAG_Z); else flag_clear(cpu, FLAG_Z);
    flag_clear(cpu, FLAG_N);
    flag_clear(cpu, FLAG_H);
    flag_clear(cpu, FLAG_C);

    LOG_DEBUG("XOR A,%s: 0x%02X ^ 0x%02X = 0x%02X at PC=0x%04X (opcode=0x%02X)",
        get_r8_name(register_code), a, reg_value, result.value, instr_pc, opcode);

    return (register_code == OP_MEM_HL) ? 8 : 4; // XOR A,r8 takes 4 cycles for normal r8 register and 8 for [HL]
}

static int op_or_a_r8(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 1;

    r8_operand_t register_code = opcode & 0b111; // Extract the register code from the opcode

    uint8_t reg_value = read_r8(cpu, bus, register_code);
    uint8_t a = cpu->af.hi;

    alu8_result_t result = alu_or8(a, reg_value);
    cpu->af.hi = result.value;

    if (result.status.zero) flag_set(cpu, FLAG_Z); else flag_clear(cpu, FLAG_Z);
    flag_clear(cpu, FLAG_N);
    flag_clear(cpu, FLAG_H);
    flag_clear(cpu, FLAG_C);

    LOG_DEBUG("OR A,%s: 0x%02X | 0x%02X = 0x%02X at PC=0x%04X (opcode=0x%02X)",
        get_r8_name(register_code), a, reg_value, result.value, instr_pc, opcode);

    return (register_code == OP_MEM_HL) ? 8 : 4; // OR A,r8 takes 4 cycles for normal r8 register and 8 for [HL]
}

static int op_and_a_imm8(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 1;

    uint8_t immediate_value = read_imm8(cpu, bus);
    uint8_t a = cpu->af.hi;

    alu8_result_t result = alu_and8(a, immediate_value);
    cpu->af.hi = result.value;

    if (result.status.zero) flag_set(cpu, FLAG_Z); else flag_clear(cpu, FLAG_Z);
    flag_clear(cpu, FLAG_N);
    flag_set(cpu, FLAG_H);
    flag_clear(cpu, FLAG_C);

    LOG_DEBUG("AND A,imm8: 0x%02X & 0x%02X = 0x%02X at PC=0x%04X (opcode=0x%02X)",
        a, immediate_value, result.value, instr_pc, opcode);

    return 8; // AND A,imm8 takes 8 cycles
}

static int op_xor_a_imm8(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 1;

    uint8_t immediate_value = read_imm8(cpu, bus);
    uint8_t a = cpu->af.hi;

    alu8_result_t result = alu_xor8(a, immediate_value);
    cpu->af.hi = result.value;

    if (result.status.zero) flag_set(cpu, FLAG_Z); else flag_clear(cpu, FLAG_Z);
    flag_clear(cpu, FLAG_N);
    flag_clear(cpu, FLAG_H);
    flag_clear(cpu, FLAG_C);

    LOG_DEBUG("XOR A,imm8: 0x%02X ^ 0x%02X = 0x%02X at PC=0x%04X (opcode=0x%02X)",
        a, immediate_value, result.value, instr_pc, opcode);

    return 8; // XOR A,imm8 takes 8 cycles
}

static int op_or_a_imm8(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 1;

    uint8_t immediate_value = read_imm8(cpu, bus);
    uint8_t a = cpu->af.hi;

    alu8_result_t result = alu_or8(a, immediate_value);
    cpu->af.hi = result.value;

    if (result.status.zero) flag_set(cpu, FLAG_Z); else flag_clear(cpu, FLAG_Z);
    flag_clear(cpu, FLAG_N);
    flag_clear(cpu, FLAG_H);
    flag_clear(cpu, FLAG_C);

    LOG_DEBUG("OR A,imm8: 0x%02X | 0x%02X = 0x%02X at PC=0x%04X (opcode=0x%02X)",
        a, immediate_value, result.value, instr_pc, opcode);

    return 8; // OR A,imm8 takes 8 cycles
}

static int op_cpl(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    (void)bus;
    uint16_t instr_pc = cpu->pc - 1;

    cpu->af.hi = ~cpu->af.hi;

    flag_set(cpu, FLAG_N);
    flag_set(cpu, FLAG_H);

    LOG_DEBUG("CPL A=0x%02X at PC=0x%04X (opcode=0x%02X)",
        cpu->af.hi, instr_pc, opcode);

    return 4; // CPL takes 4 cycles
}
