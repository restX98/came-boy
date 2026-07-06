#ifndef IO_REG_H
#define IO_REG_H

#include <stdint.h>

#include "io/audio.h"
#include "io/interrupts.h"
#include "io/joypad.h"
#include "io/lcd.h"
#include "io/serial_transfer.h"
#include "io/timer.h"

typedef struct {
    joypad_reg_t joyp;
    interrupt_regs_t interrupts;
    lcd_regs_t lcd;
    st_regs_t serial_transfer;
    timer_regs_t timer;
    audio_regs_t audio;
    uint8_t wp_ram[16];
    uint8_t boot_rom_disable;
} io_reg_t;

void io_reg_init(io_reg_t *io_reg);
uint8_t io_reg_read(io_reg_t *io_reg, uint16_t addr);
void io_reg_write(io_reg_t *io_reg, uint16_t addr, uint8_t value);

#endif // IO_REG_H
