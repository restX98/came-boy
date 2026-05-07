/*-------------------------------------------------------
 * Private Opcode 16-bit Arithmetic Instructions Definitions
 *-------------------------------------------------------*/

static int op_add_hl_r16(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    (void)bus;
    uint16_t instr_pc = cpu->pc - 1;

    r16_operand_t register_code = (opcode >> 4) & 0b11; // Extract the register code from the opcode

    uint16_t reg_value = read_r16(cpu, register_code);
    uint16_t hl = cpu->hl.reg;

    alu16_result_t result = alu_add16(hl, reg_value);
    cpu->hl.reg = result.value;

    // N cleared, H and C set according to result, Z unaffected
    flag_clear(cpu, FLAG_N);
    if (result.status.half_carry) flag_set(cpu, FLAG_H); else flag_clear(cpu, FLAG_H);
    if (result.status.carry)      flag_set(cpu, FLAG_C); else flag_clear(cpu, FLAG_C);

    LOG_DEBUG("ADD HL,%s HL=0x%04X at PC=0x%04X (opcode=0x%02X)",
        get_r16_name(register_code), cpu->hl.reg, instr_pc, opcode);

    return 8; // ADD HL,r16 takes 8 cycles
}

static int op_inc_r16(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    (void)bus;
    uint16_t instr_pc = cpu->pc - 1;

    r16_operand_t register_code = (opcode >> 4) & 0b11; // Extract the register code from the opcode

    write_r16(cpu, register_code, read_r16(cpu, register_code) + 1);

    LOG_DEBUG("INC %s at PC=0x%04X (opcode=0x%02X)",
        get_r16_name(register_code), instr_pc, opcode);

    return 8; // INC r16 takes 8 cycles
}

static int op_dec_r16(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    (void)bus;
    uint16_t instr_pc = cpu->pc - 1;

    r16_operand_t register_code = (opcode >> 4) & 0b11; // Extract the register code from the opcode

    write_r16(cpu, register_code, read_r16(cpu, register_code) - 1);

    LOG_DEBUG("DEC %s at PC=0x%04X (opcode=0x%02X)",
        get_r16_name(register_code), instr_pc, opcode);

    return 8; // DEC r16 takes 8 cycles
}
