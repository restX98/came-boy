#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdint.h>

typedef enum {
    INT_VBLANK = 0,
    INT_LCD = 1,
    INT_TIMER = 2,
    INT_SERIAL = 3,
    INT_JOYPAD = 4,
} interrupt_t;

typedef union {
    uint8_t reg;
    struct {
        uint8_t v_blank : 1;
        uint8_t lcd : 1;
        uint8_t timer : 1;
        uint8_t serial : 1;
        uint8_t joypad : 1;
        uint8_t _ : 3;  // bits 5–7, always 1 on real hardware
    };
} int_flag_t;

typedef struct {
    int_flag_t flag;    // 0xFF0F — Interrupt flag (IF)
    int_flag_t enable;  // 0xFFFF — Interrupt enable (IE)
} interrupt_regs_t;

void interrupts_init(interrupt_regs_t *interrupts);

void interrupts_request(interrupt_regs_t *interrupts, interrupt_t interrupt);

uint8_t interrupts_read(interrupt_regs_t *interrupts, uint16_t addr);
void interrupts_write(interrupt_regs_t *interrupts, uint16_t addr, uint8_t value);

#endif // INTERRUPTS_H
