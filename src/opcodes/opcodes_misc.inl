/*-------------------------------------------------------
 * Private Opcode Miscellaneous Instructions definitions
 *-------------------------------------------------------*/

static int op_nop(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    (void)bus; (void)opcode;

    LOG_DEBUG("NOP at PC=0x%04X", cpu->pc - 1);

    return 4; // NOP takes 4 cycles
}

static int op_stop(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    (void)opcode;

    // TODO: Understand how STOP actually works

    read_imm8(cpu, bus);

    LOG_DEBUG("STOP at PC=0x%04X", cpu->pc - 1);

    return 4; // STOP takes 4 cycles
}

static int op_daa(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    (void)bus;
    uint16_t instr_pc = cpu->pc - 1;

    bool subtraction_flag = flag_get(cpu, FLAG_N);
    bool half_carry_flag = flag_get(cpu, FLAG_H);
    bool carry_flag = flag_get(cpu, FLAG_C);

    uint8_t adjustment = 0;

    if (subtraction_flag) {
        if (half_carry_flag) adjustment += 0x06;
        if (carry_flag) adjustment += 0x60;
        cpu->af.hi -= adjustment;
    } else {
        if (half_carry_flag || (cpu->af.hi & 0xF) > 0x9) {
            adjustment += 0x06;
        }
        if (carry_flag || cpu->af.hi > 0x99) {
            flag_set(cpu, FLAG_C);
            adjustment += 0x60;
        }
        cpu->af.hi += adjustment;
    }

    flag_clear(cpu, FLAG_H);
    if (cpu->af.hi == 0) flag_set(cpu, FLAG_Z); else flag_clear(cpu, FLAG_Z);

    LOG_DEBUG("DAA A=0x%02X at PC=0x%04X (opcode=0x%02X)",
        cpu->af.hi, instr_pc, opcode);

    return 4; // DAA takes 4 cycles
}
