/*-------------------------------------------------------
 * Private helpers definition
 *-------------------------------------------------------*/

static uint8_t read_imm8(cpu_t *cpu, bus_t *bus) {
    uint8_t immediate_value = bus_read(bus, cpu->pc);
    cpu->pc += 1;
    return immediate_value;
}

static uint16_t read_imm16(cpu_t *cpu, bus_t *bus) {
    uint8_t lo = bus_read(bus, cpu->pc);
    uint8_t hi = bus_read(bus, cpu->pc + 1);
    cpu->pc += 2;
    return (hi << 8) | lo;
}

static uint8_t read_r8(cpu_t *cpu, bus_t *bus, r8_operand_t r8_op) {
    switch (r8_op) {
        case OP_REG_B: return cpu->bc.hi;
        case OP_REG_C: return cpu->bc.lo;
        case OP_REG_D: return cpu->de.hi;
        case OP_REG_E: return cpu->de.lo;
        case OP_REG_H: return cpu->hl.hi;
        case OP_REG_L: return cpu->hl.lo;
        case OP_MEM_HL: return bus_read(bus, cpu->hl.reg); // [HL]
        case OP_REG_A: return cpu->af.hi;
        default: assert(0 && "Invalid r8 operand");
    }
}

static void write_r8(cpu_t *cpu, bus_t *bus, r8_operand_t r8_op, uint8_t value) {
    switch (r8_op) {
        case OP_REG_B: cpu->bc.hi = value; break;
        case OP_REG_C: cpu->bc.lo = value; break;
        case OP_REG_D: cpu->de.hi = value; break;
        case OP_REG_E: cpu->de.lo = value; break;
        case OP_REG_H: cpu->hl.hi = value; break;
        case OP_REG_L: cpu->hl.lo = value; break;
        case OP_MEM_HL: bus_write(bus, cpu->hl.reg, value); break; // [HL]
        case OP_REG_A: cpu->af.hi = value; break;
        default: assert(0 && "Invalid r8 operand");
    }
}

static const char *get_r8_name(r8_operand_t r8_op) {
    static const char *names[] = {
        "B","C","D","E","H","L","[HL]","A"
    };
    if (r8_op > OP_REG_A) return "??";
    return names[r8_op];
}

static uint16_t read_r16(cpu_t *cpu, r16_operand_t r16_op) {
    switch (r16_op) {
        case OP_REG_BC: return cpu->bc.reg;
        case OP_REG_DE: return cpu->de.reg;
        case OP_REG_HL: return cpu->hl.reg;
        case OP_REG_SP: return cpu->sp;
        default: assert(0 && "Invalid r16 operand");
    }
}

static void write_r16(cpu_t *cpu, r16_operand_t r16_op, uint16_t value) {
    switch (r16_op) {
        case OP_REG_BC: cpu->bc.reg = value; break;
        case OP_REG_DE: cpu->de.reg = value; break;
        case OP_REG_HL: cpu->hl.reg = value; break;
        case OP_REG_SP: cpu->sp = value; break;
        default: assert(0 && "Invalid r16 operand");
    }
}

static const char *get_r16_name(r16_operand_t r16_op) {
    static const char *names[] = {
        "BC","DE","HL","SP"
    };
    if (r16_op > OP_REG_SP) return "??";
    return names[r16_op];
}

static uint16_t read_r16mem(cpu_t *cpu, r16mem_operand_t r16mem_op) {
    switch (r16mem_op) {
        case OP_REG_BC_MEM: return cpu->bc.reg;
        case OP_REG_DE_MEM: return cpu->de.reg;
        case OP_REG_HLI_MEM: return cpu->hl.reg++;
        case OP_REG_HLD_MEM: return cpu->hl.reg--;
        default: assert(0 && "Invalid r16mem operand");
    }
}

static const char *get_r16mem_name(r16mem_operand_t r16mem_op) {
    static const char *names[] = {
        "BC","DE","HL+","HL-"
    };
    if (r16mem_op > OP_REG_HLD_MEM) return "??";
    return names[r16mem_op];
}

static bool check_condition(cpu_t *cpu, cond_operand_t cond) {
    switch (cond) {
        case OP_NZ: return !flag_get(cpu, FLAG_Z);
        case OP_Z: return  flag_get(cpu, FLAG_Z);
        case OP_NC: return !flag_get(cpu, FLAG_C);
        case OP_C: return  flag_get(cpu, FLAG_C);
        default: assert(0 && "Invalid condition operand");
    }
}

static const char *get_condition_name(cond_operand_t cond) {
    static const char *names[] = {
        "NZ","Z","NC","C"
    };
    if (cond > OP_C) return "??";
    return names[cond];
}
