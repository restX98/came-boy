#include "interrupts.h"

#include <assert.h>

void interrupts_init(interrupt_regs_t *interrupts) {
    interrupts->flag.reg = 0xE1;
    interrupts->enable.reg = 0x00;
}

void interrupts_request(interrupt_regs_t *interrupts, interrupt_t interrupt) {
    interrupts->flag.reg |= (1 << interrupt);
}

uint8_t interrupts_read(interrupt_regs_t *interrupts, uint16_t addr) {
    if (addr == 0xFF0F) {
        return interrupts->flag.reg;
    } else if (addr == 0xFFFF) {
        return interrupts->enable.reg;
    }

    assert(0 && "Assert something");
}

void interrupts_write(interrupt_regs_t *interrupts, uint16_t addr, uint8_t value) {
    value |= 0b11100000;
    if (addr == 0xFF0F) {
        interrupts->flag.reg = value;
    } else if (addr == 0xFFFF) {
        interrupts->enable.reg = value;
    } else {
        assert(0 && "Assert something");
    }
}
