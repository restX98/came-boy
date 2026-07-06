/*-------------------------------------------------------
 * Private Opcode Interrupt-Related Instructions Definitions
 *-------------------------------------------------------*/

static int op_halt(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    (void)opcode;
    (void)bus;

    uint8_t ie = bus_read(bus, 0xFFFF);
    uint8_t if_ = bus_read(bus, 0xFF0F);
    bool irq_pending = (ie & if_ & 0b00011111) != 0;

    if (cpu->ime.enabled) {
        // Case 1: IME=1. Sleep until any enabled IRQ is pending, then
        // the interrupt servicing routine wakes us and dispatches.
        cpu->halted = true;
    } else if (!irq_pending) {
        // Case 2: IME=0, no IRQ pending. Sleep until an IRQ becomes pending,
        // then wake WITHOUT servicing, execution resumes after HALT.
        cpu->halted = true;
    } else {
        // Case 3: IME=0 AND IRQ already pending -> HALT bug.
        // CPU doesn't actually halt; instead, the byte after HALT is read
        // but PC fails to increment, causing it to be executed twice.
        cpu->halt_bug = true;
    }

    LOG_DEBUG("HALT at PC=0x%04X (IME=%d IE=0x%02X IF=0x%02X halted=%d halt_bug=%d)",
        cpu->pc - 1, cpu->ime.enabled, ie, if_, cpu->halted, cpu->halt_bug);

    return 4; // HALT takes 4 cycles
}

static int op_di(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    (void)opcode;
    (void)bus;

    cpu->ime.enabled = false;
    cpu->ime.scheduled = false;

    LOG_DEBUG("DI at PC=0x%04X", cpu->pc - 1);

    return 4; // DI takes 4 cycles
}

static int op_ei(cpu_t *cpu, bus_t *bus, uint8_t opcode) {
    (void)opcode;
    (void)bus;

    cpu->ime.scheduled = true;

    LOG_DEBUG("EI at PC=0x%04X", cpu->pc - 1);

    return 4; // EI takes 4 cycles
}
