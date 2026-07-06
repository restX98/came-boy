#include "serial_transfer.h"

#include <assert.h>

#include "logger.h"

void serial_transfer_init(st_regs_t *serial) {
    serial->sb = 0x00;
    serial->sc = 0x7E;
    serial->shift_count = 0;
}

uint8_t serial_transfer_read(st_regs_t *serial, uint16_t addr) {
    if (addr == 0xFF01) {
        return serial->sb;
    } else if (addr == 0xFF02) {
        return serial->sc;
    }

    assert(0 && "serial_transfer_read: unhandled serial register");
    return 0xFF;
}

void serial_transfer_write(st_regs_t *serial, uint16_t addr, uint8_t value) {
    if (addr == 0xFF01) {
        serial->sb = value;
    } else if (addr == 0xFF02) {
        serial->sc = value | 0x7E;
    } else {
        assert(0 && "serial_transfer_write: unhandled serial register");
    }
}
