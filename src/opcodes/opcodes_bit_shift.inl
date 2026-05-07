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
