#include "io_reg.h"

#include <assert.h>

#include "logger.h"

void io_reg_init(io_reg_t *io_reg) {
    joypad_init(&io_reg->joyp);
    serial_transfer_init(&io_reg->serial_transfer);
    interrupts_init(&io_reg->interrupts);
    timer_init(&io_reg->timer);
    audio_init(&io_reg->audio);
    lcd_init(&io_reg->lcd);
}

// TODO: Implement a map like for bus
uint8_t io_reg_read(io_reg_t *io_reg, uint16_t addr) {
    if (addr == 0xFF00) {
        return joypad_read(&io_reg->joyp);
    } else if (addr >= 0xFF01 && addr <= 0xFF02) {
        return serial_transfer_read(&io_reg->serial_transfer, addr);
    } else if (addr >= 0xFF04 && addr <= 0xFF07) {
        return timer_read(&io_reg->timer, addr);
    } else if (addr == 0xFF0F || addr == 0xFFFF) {
        return interrupts_read(&io_reg->interrupts, addr);
    } else if (addr >= 0xFF10 && addr <= 0xFF26) {
        return audio_read(&io_reg->audio, addr);
    } else if (addr >= 0xFF40 && addr <= 0xFF4B) {
        return lcd_read(&io_reg->lcd, addr);
    }

    assert(0 && "io_reg_read: unimplemented register");
    return 0xFF;
}

void io_reg_write(io_reg_t *io_reg, uint16_t addr, uint8_t value) {
    (void)io_reg; (void)addr; (void)value;

    if (addr == 0xFF00) {
        joypad_write(&io_reg->joyp, value);
    } else if (addr >= 0xFF01 && addr <= 0xFF02) {
        serial_transfer_write(&io_reg->serial_transfer, addr, value);
    } else if (addr >= 0xFF04 && addr <= 0xFF07) {
        timer_write(&io_reg->timer, addr, value);
    } else if (addr == 0xFF0F || addr == 0xFFFF) {
        interrupts_write(&io_reg->interrupts, addr, value);
    } else if (addr >= 0xFF10 && addr <= 0xFF26) {
        audio_write(&io_reg->audio, addr, value);
    } else if (addr >= 0xFF40 && addr <= 0xFF4B) {
        lcd_write(&io_reg->lcd, addr, value);
    } else {
        assert(0 && "io_reg_write: unimplemented register");
    }
}
