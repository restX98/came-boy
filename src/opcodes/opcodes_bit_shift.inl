/*-------------------------------------------------------
 * Private Opcode Bit Shift Instructions Definitions
 *-------------------------------------------------------*/

static int op_rlca(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    (void)bus;
    uint16_t instr_pc = cpu->pc - 1;

    uint8_t old_a = cpu->af.hi;

    bool carry = (old_a >> 7) == 1;

    cpu->af.hi = (old_a << 1) | (old_a >> 7);

    flag_clear(cpu, FLAG_Z);
    flag_clear(cpu, FLAG_N);
    flag_clear(cpu, FLAG_H);
    if (carry) flag_set(cpu, FLAG_C); else flag_clear(cpu, FLAG_C);

    LOG_DEBUG("RLCA 0x%02X -> 0x%02X at PC=0x%04X (opcode=0x%02X)",
        old_a, cpu->af.hi, instr_pc, opcode);

    return 4; // RLCA takes 4 cycles
}

static int op_rrca(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    (void)bus;
    uint16_t instr_pc = cpu->pc - 1;

    uint8_t old_a = cpu->af.hi;

    bool carry = (old_a & 0x01) == 1;

    cpu->af.hi = (old_a >> 1) | ((old_a & 0x01) << 7);

    flag_clear(cpu, FLAG_Z);
    flag_clear(cpu, FLAG_N);
    flag_clear(cpu, FLAG_H);
    if (carry) flag_set(cpu, FLAG_C); else flag_clear(cpu, FLAG_C);

    LOG_DEBUG("RRCA 0x%02X -> 0x%02X at PC=0x%04X (opcode=0x%02X)",
        old_a, cpu->af.hi, instr_pc, opcode);

    return 4; // RRCA takes 4 cycles
}

static int op_rla(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    (void)bus;
    uint16_t instr_pc = cpu->pc - 1;

    uint8_t old_a = cpu->af.hi;

    bool carry = (old_a >> 7) == 1;

    cpu->af.hi = (old_a << 1) | (flag_get(cpu, FLAG_C) ? 1 : 0);

    flag_clear(cpu, FLAG_Z);
    flag_clear(cpu, FLAG_N);
    flag_clear(cpu, FLAG_H);
    if (carry) flag_set(cpu, FLAG_C); else flag_clear(cpu, FLAG_C);

    LOG_DEBUG("RLA 0x%02X -> 0x%02X at PC=0x%04X (opcode=0x%02X)",
        old_a, cpu->af.hi, instr_pc, opcode);

    return 4; // RLA takes 4 cycles
}

static int op_rra(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    (void)bus;
    uint16_t instr_pc = cpu->pc - 1;

    uint8_t old_a = cpu->af.hi;

    bool carry = (old_a & 0x01) == 1;

    cpu->af.hi = (flag_get(cpu, FLAG_C) ? 0x80 : 0x00) | (old_a >> 1);

    flag_clear(cpu, FLAG_Z);
    flag_clear(cpu, FLAG_N);
    flag_clear(cpu, FLAG_H);
    if (carry) flag_set(cpu, FLAG_C); else flag_clear(cpu, FLAG_C);

    LOG_DEBUG("RRA 0x%02X -> 0x%02X at PC=0x%04X (opcode=0x%02X)",
        old_a, cpu->af.hi, instr_pc, opcode);

    return 4; // RRA takes 4 cycles
}

static int op_rlc_r8(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 2; // PC is past prefix + opcode

    r8_operand_t register_code = opcode & 0b111;

    uint8_t reg_value = read_r8(cpu, bus, register_code);
    uint8_t rotated_value = (reg_value << 1) | (reg_value >> 7);

    bool carry = (reg_value >> 7) == 1;
    bool zero = rotated_value == 0;

    write_r8(cpu, bus, register_code, rotated_value);

    flag_clear(cpu, FLAG_N);
    flag_clear(cpu, FLAG_H);
    if (zero)  flag_set(cpu, FLAG_Z); else flag_clear(cpu, FLAG_Z);
    if (carry) flag_set(cpu, FLAG_C); else flag_clear(cpu, FLAG_C);

    LOG_DEBUG("RLC %s 0x%02X -> 0x%02X at PC=0x%04X (opcode=0x%02X)",
        get_r8_name(register_code), reg_value, rotated_value, instr_pc, opcode);

    return (register_code == OP_MEM_HL) ? 16 : 8;
}

static int op_rrc_r8(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 2; // PC is past prefix + opcode

    r8_operand_t register_code = opcode & 0b111;

    uint8_t reg_value = read_r8(cpu, bus, register_code);
    uint8_t rotated_value = (reg_value >> 1) | ((reg_value & 0x01) << 7);

    bool carry = (reg_value & 0x01) == 1;
    bool zero = rotated_value == 0;

    write_r8(cpu, bus, register_code, rotated_value);

    flag_clear(cpu, FLAG_N);
    flag_clear(cpu, FLAG_H);
    if (zero)  flag_set(cpu, FLAG_Z); else flag_clear(cpu, FLAG_Z);
    if (carry) flag_set(cpu, FLAG_C); else flag_clear(cpu, FLAG_C);

    LOG_DEBUG("RRC %s 0x%02X -> 0x%02X at PC=0x%04X (opcode=0x%02X)",
        get_r8_name(register_code), reg_value, rotated_value, instr_pc, opcode);

    return (register_code == OP_MEM_HL) ? 16 : 8;
}

static int op_rl_r8(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 2; // PC is past prefix + opcode

    r8_operand_t register_code = opcode & 0b111;

    uint8_t reg_value = read_r8(cpu, bus, register_code);
    uint8_t carry_in = flag_get(cpu, FLAG_C) ? 1 : 0;
    uint8_t rotated_value = (reg_value << 1) | carry_in;

    bool carry = (reg_value >> 7) == 1;
    bool zero = rotated_value == 0;

    write_r8(cpu, bus, register_code, rotated_value);

    flag_clear(cpu, FLAG_N);
    flag_clear(cpu, FLAG_H);
    if (zero)  flag_set(cpu, FLAG_Z); else flag_clear(cpu, FLAG_Z);
    if (carry) flag_set(cpu, FLAG_C); else flag_clear(cpu, FLAG_C);

    LOG_DEBUG("RL %s 0x%02X -> 0x%02X at PC=0x%04X (opcode=0x%02X)",
        get_r8_name(register_code), reg_value, rotated_value, instr_pc, opcode);

    return (register_code == OP_MEM_HL) ? 16 : 8;
}

static int op_rr_r8(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 2; // PC is past prefix + opcode

    r8_operand_t register_code = opcode & 0b111;

    uint8_t reg_value = read_r8(cpu, bus, register_code);
    uint8_t carry_in = flag_get(cpu, FLAG_C) ? 0x80 : 0x00;
    uint8_t rotated_value = (reg_value >> 1) | carry_in;

    bool carry = (reg_value & 0x01) == 1;
    bool zero = rotated_value == 0;

    write_r8(cpu, bus, register_code, rotated_value);

    flag_clear(cpu, FLAG_N);
    flag_clear(cpu, FLAG_H);
    if (zero)  flag_set(cpu, FLAG_Z); else flag_clear(cpu, FLAG_Z);
    if (carry) flag_set(cpu, FLAG_C); else flag_clear(cpu, FLAG_C);

    LOG_DEBUG("RR %s 0x%02X -> 0x%02X at PC=0x%04X (opcode=0x%02X)",
        get_r8_name(register_code), reg_value, rotated_value, instr_pc, opcode);

    return (register_code == OP_MEM_HL) ? 16 : 8;
}

static int op_sla_r8(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 2;

    r8_operand_t register_code = opcode & 0b111;

    uint8_t reg_value = read_r8(cpu, bus, register_code);
    uint8_t shifted_value = reg_value << 1;

    bool carry = (reg_value >> 7) == 1;
    bool zero = shifted_value == 0;

    write_r8(cpu, bus, register_code, shifted_value);

    flag_clear(cpu, FLAG_N);
    flag_clear(cpu, FLAG_H);
    if (zero)  flag_set(cpu, FLAG_Z); else flag_clear(cpu, FLAG_Z);
    if (carry) flag_set(cpu, FLAG_C); else flag_clear(cpu, FLAG_C);

    LOG_DEBUG("SLA %s 0x%02X -> 0x%02X at PC=0x%04X (opcode=0x%02X)",
        get_r8_name(register_code), reg_value, shifted_value, instr_pc, opcode);

    return (register_code == OP_MEM_HL) ? 16 : 8;
}

static int op_sra_r8(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 2;

    r8_operand_t register_code = opcode & 0b111;

    uint8_t reg_value = read_r8(cpu, bus, register_code);
    uint8_t shifted_value = (reg_value >> 1) | (reg_value & 0x80);

    bool carry = (reg_value & 0x01) == 1;
    bool zero = shifted_value == 0;

    write_r8(cpu, bus, register_code, shifted_value);

    flag_clear(cpu, FLAG_N);
    flag_clear(cpu, FLAG_H);
    if (zero)  flag_set(cpu, FLAG_Z); else flag_clear(cpu, FLAG_Z);
    if (carry) flag_set(cpu, FLAG_C); else flag_clear(cpu, FLAG_C);

    LOG_DEBUG("SRA %s 0x%02X -> 0x%02X at PC=0x%04X (opcode=0x%02X)",
        get_r8_name(register_code), reg_value, shifted_value, instr_pc, opcode);

    return (register_code == OP_MEM_HL) ? 16 : 8;
}
