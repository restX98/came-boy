/*-------------------------------------------------------
 * Private Opcode Jumps and Subroutine Instructions definitions
 *-------------------------------------------------------*/

static int op_jr_imm8(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 1;

    int8_t offset = (int8_t)read_imm8(cpu, bus);

    cpu->pc += offset;

    LOG_DEBUG("JR imm8 offset=%d at PC=0x%04X (opcode=0x%02X)",
        offset, instr_pc, opcode);

    return 12; // JR imm8 takes 12 cycles
}

static int op_jr_cond_imm8(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 1;

    cond_operand_t cond_op = (opcode >> 3) & 0b11; // Extract condition code from opcode
    bool condition = check_condition(cpu, cond_op);

    int8_t offset = (int8_t)read_imm8(cpu, bus);

    if (condition) {
        cpu->pc += offset;
    }

    LOG_DEBUG("JR %s offset=%d (0x%02X) at PC=0x%04X (opcode=0x%02X)",
        get_condition_name(cond_op), offset, (uint8_t)offset, instr_pc, opcode);

    return condition ? 12 : 8;
}
