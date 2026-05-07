/*-------------------------------------------------------
 * Private Opcode Carry Flag Instructions Definitions
 *-------------------------------------------------------*/

static int op_scf(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    (void)bus;
    uint16_t instr_pc = cpu->pc - 1;

    flag_clear(cpu, FLAG_N);
    flag_clear(cpu, FLAG_H);
    flag_set(cpu, FLAG_C);

    LOG_DEBUG("SCF at PC=0x%04X (opcode=0x%02X)", instr_pc, opcode);

    return 4; // SCF takes 4 cycles
}

static int op_ccf(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    (void)bus;
    uint16_t instr_pc = cpu->pc - 1;

    flag_clear(cpu, FLAG_N);
    flag_clear(cpu, FLAG_H);

    if (flag_get(cpu, FLAG_C)) flag_clear(cpu, FLAG_C); else flag_set(cpu, FLAG_C);

    LOG_DEBUG("CCF at PC=0x%04X (opcode=0x%02X)", instr_pc, opcode);

    return 4; // CCF takes 4 cycles
}
