/*-------------------------------------------------------
 * Private Opcode Stack Manipulation Instructions Definitions
 *-------------------------------------------------------*/

static int op_ld_imm16mem_sp(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 1;

    uint16_t address = read_imm16(cpu, bus);

    bus_write(bus, address, cpu->sp & 0xFF);
    bus_write(bus, address + 1, cpu->sp >> 8);

    LOG_DEBUG("LD [0x%04X],SP SP=0x%04X at PC=0x%04X (opcode=0x%02X)",
        address, cpu->sp, instr_pc, opcode);

    return 20; // LD [imm16],sp takes 20 cycles
}
