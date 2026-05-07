/*-------------------------------------------------------
 * Private Opcode Interrupt-Related Instructions Definitions
 *-------------------------------------------------------*/

static int op_halt(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    (void)opcode;
    (void)bus;

    // TODO: Understand how HALT actually works

    LOG_DEBUG("HALT at PC=0x%04X", cpu->pc - 1);

    return 4; // HALT takes 4 cycles
}

static int op_di(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    (void)opcode;
    (void)bus;

    cpu->ime = false;

    LOG_DEBUG("DI at PC=0x%04X", cpu->pc - 1);

    return 4; // DI takes 4 cycles
}

static int op_ei(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    (void)opcode;
    (void)bus;

    cpu->ime_scheduled = true;

    LOG_DEBUG("EI at PC=0x%04X", cpu->pc - 1);

    return 4; // EI takes 4 cycles
}
