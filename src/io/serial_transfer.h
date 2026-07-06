#ifndef SERIAL_TRANSFER_H
#define SERIAL_TRANSFER_H

#include <stdint.h>

typedef struct {
    uint8_t sb;          // 0xFF01 — Serial transfer data
    uint8_t sc;          // 0xFF02 — Serial transfer control
    uint8_t shift_count; // bits shifted in the active transfer (0-7)
} st_regs_t;

void serial_transfer_init(st_regs_t *serial);

uint8_t serial_transfer_read(st_regs_t *serial, uint16_t addr);
void serial_transfer_write(st_regs_t *serial, uint16_t addr, uint8_t value);

#endif // SERIAL_TRANSFER_H
