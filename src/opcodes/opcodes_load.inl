/*-------------------------------------------------------
 * Private Opcode Load Instructions Definitions
 *-------------------------------------------------------*/

static int op_ld_r8_r8(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 1;

    r8_operand_t dest_reg_code = (opcode >> 3) & 0b111;
    r8_operand_t source_reg_code = opcode & 0b111;

    write_r8(cpu, bus, dest_reg_code, read_r8(cpu, bus, source_reg_code));

    LOG_DEBUG("LD %s,%s at PC=0x%04X (opcode=0x%02X)",
        get_r8_name(dest_reg_code), get_r8_name(source_reg_code), instr_pc, opcode);

    return (dest_reg_code == OP_MEM_HL || source_reg_code == OP_MEM_HL) ? 8 : 4;
}

static int op_ld_r8_imm8(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 1;

    uint8_t immediate_value = read_imm8(cpu, bus);

    r8_operand_t register_code = (opcode >> 3) & 0b111; // Extract the register code from the opcode

    write_r8(cpu, bus, register_code, immediate_value);

    LOG_DEBUG("LD %s,imm8 value=0x%02X at PC=0x%04X (opcode=0x%02X)",
        get_r8_name(register_code), immediate_value, instr_pc, opcode);

    return (register_code == OP_MEM_HL) ? 12 : 4; // LD r8,imm8 takes 4 cycles for normal r8 register and 12 for [HL]
}

static int op_ld_r16_imm16(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 1;

    uint16_t immediate_value = read_imm16(cpu, bus);

    r16_operand_t register_code = (opcode >> 4) & 0b11; // Extract the register code from the opcode

    write_r16(cpu, register_code, immediate_value);

    LOG_DEBUG("LD %s,imm16 value=0x%04X at PC=0x%04X (opcode=0x%02X)",
        get_r16_name(register_code), immediate_value, instr_pc, opcode);

    return 12; // LD r16,imm16 takes 12 cycles
}

static int op_ld_r16mem_a(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 1;

    r16mem_operand_t register_code = (opcode >> 4) & 0b11; // Extract the register code from the opcode

    uint16_t reg_value = read_r16mem(cpu, register_code);
    const char *reg_name = get_r16mem_name(register_code);

    bus_write(bus, reg_value, cpu->af.hi);

    LOG_DEBUG("LD [%s],A %s=0x%04X A=%u at PC=0x%04X (opcode=0x%02X)",
        reg_name, reg_name, reg_value, cpu->af.hi, instr_pc, opcode);

    return 8; // LD [r16mem],a takes 8 cycles
}

static int op_ld_a_r16mem(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 1;

    r16mem_operand_t register_code = (opcode >> 4) & 0b11; // Extract the register code from the opcode

    uint16_t reg_value = read_r16mem(cpu, register_code);
    const char *reg_name = get_r16mem_name(register_code);

    cpu->af.hi = bus_read(bus, reg_value);

    LOG_DEBUG("LD A,[%s] %s=0x%04X [%s]=%u at PC=0x%04X (opcode=0x%02X)",
        reg_name, reg_name, reg_value, reg_name, cpu->af.hi, instr_pc, opcode);

    return 8; // LD a,[r16mem] takes 8 cycles
}

static int op_ldh_imm8mem_a(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 1;

    uint8_t immediate_value = read_imm8(cpu, bus);
    uint16_t address = 0xFF00 | immediate_value;
    uint8_t a = cpu->af.hi;

    bus_write(bus, address, a);

    LOG_DEBUG("LDH [0x%02X],A address=0x%04X A=0x%02X at PC=0x%04X (opcode=0x%02X)",
        immediate_value, address, a, instr_pc, opcode);

    return 12;
}

static int op_ldh_c_mem_a(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 1;

    uint16_t address = 0xFF00 | cpu->bc.lo;
    uint8_t a = cpu->af.hi;

    bus_write(bus, address, a);

    LOG_DEBUG("LDH [C],A C=0x%02X address=0x%04X A=0x%02X at PC=0x%04X (opcode=0x%02X)",
        cpu->bc.lo, address, a, instr_pc, opcode);

    return 8;
}

static int op_ld_imm16mem_a(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 1;

    uint16_t address = read_imm16(cpu, bus);
    uint8_t a = cpu->af.hi;

    bus_write(bus, address, a);

    LOG_DEBUG("LD [imm16],A address=0x%04X A=0x%02X at PC=0x%04X (opcode=0x%02X)",
        address, a, instr_pc, opcode);

    return 16;
}

static int op_ldh_a_imm8mem(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 1;

    uint8_t immediate_value = read_imm8(cpu, bus);
    uint16_t address = 0xFF00 | immediate_value;

    cpu->af.hi = bus_read(bus, address);

    LOG_DEBUG("LDH A, [0x%02X] address=0x%04X value=0x%02X at PC=0x%04X (opcode=0x%02X)",
        immediate_value, address, cpu->af.hi, instr_pc, opcode);

    return 12;
}

static int op_ldh_a_c_mem(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 1;

    uint16_t address = 0xFF00 | cpu->bc.lo;

    cpu->af.hi = bus_read(bus, address);

    LOG_DEBUG("LDH A, [C] address=0x%04X value=0x%02X at PC=0x%04X (opcode=0x%02X)",
        address, cpu->af.hi, instr_pc, opcode);

    return 8;
}

static int op_ld_a_imm16mem(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 1;

    uint16_t address = read_imm16(cpu, bus);

    cpu->af.hi = bus_read(bus, address);

    LOG_DEBUG("LD A, [imm16] address=0x%04X value=0x%02X at PC=0x%04X (opcode=0x%02X)",
        address, cpu->af.hi, instr_pc, opcode);

    return 16;
}

static int op_ld_imm16mem_sp(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    uint16_t instr_pc = cpu->pc - 1;

    uint16_t address = read_imm16(cpu, bus);

    bus_write(bus, address, cpu->sp & 0xFF);
    bus_write(bus, address + 1, cpu->sp >> 8);

    LOG_DEBUG("LD [0x%04X],SP SP=0x%04X at PC=0x%04X (opcode=0x%02X)",
        address, cpu->sp, instr_pc, opcode);

    return 20; // LD [imm16],sp takes 20 cycles
}
