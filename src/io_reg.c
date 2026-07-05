#include "io_reg.h"

#include <assert.h>

#include "logger.h"

static uint8_t io_joyp_read(joypad_reg_t *joyp);
static void io_joyp_write(joypad_reg_t *joyp, uint8_t value);
static uint8_t io_lcd_read(lcd_regs_t *lcd, uint16_t addr);
static void io_lcd_write(lcd_regs_t *lcd, uint16_t addr, uint8_t value);
static uint8_t io_st_read(st_regs_t *serial_transfer, uint16_t addr);
static void io_st_write(st_regs_t *serial_transfer, uint16_t addr, uint8_t value);
static uint8_t io_audio_read(audio_regs_t *audio, uint16_t addr);
static void io_audio_write(audio_regs_t *audio, uint16_t addr, uint8_t value);

int io_reg_init(io_reg_t *io_reg) {
    joypad_init(&io_reg->joyp);

    io_reg->serial_transfer.sb = 0x00;
    io_reg->serial_transfer.sc = 0x7E;

    interrupts_init(&io_reg->interrupts);

    timer_init(&io_reg->timer);

    io_reg->audio.nr10 = 0x80;
    io_reg->audio.nr11 = 0xBF;
    io_reg->audio.nr12 = 0xF3;
    io_reg->audio.nr13 = 0xFF;
    io_reg->audio.nr14 = 0xBF;
    io_reg->audio.nr21 = 0x3F;
    io_reg->audio.nr22 = 0x00;
    io_reg->audio.nr23 = 0xFF;
    io_reg->audio.nr24 = 0xBF;
    io_reg->audio.nr30 = 0x7F;
    io_reg->audio.nr31 = 0xFF;
    io_reg->audio.nr32 = 0x9F;
    io_reg->audio.nr33 = 0xFF;
    io_reg->audio.nr34 = 0xBF;
    io_reg->audio.nr41 = 0xFF;
    io_reg->audio.nr42 = 0x00;
    io_reg->audio.nr43 = 0x00;
    io_reg->audio.nr44 = 0xBF;
    io_reg->audio.nr50 = 0x77;
    io_reg->audio.nr51 = 0xF3;
    io_reg->audio.nr52 = 0xF1;

    io_reg->lcd.ctrl.reg = 0x91;
    io_reg->lcd.stat.reg = 0x85;
    io_reg->lcd.scy = 0x00;
    io_reg->lcd.scx = 0x00;
    io_reg->lcd.ly = 0x00;
    io_reg->lcd.lyc = 0x00;
    io_reg->lcd.dma = 0xFF;
    io_reg->lcd.bgp = 0xFC;
    io_reg->lcd.obp0 = 0x00;
    io_reg->lcd.obp1 = 0x00;
    io_reg->lcd.wy = 0x00;
    io_reg->lcd.wx = 0x00;

    return 0;
}

// TODO: Implement a map like for bus
uint8_t io_reg_read(io_reg_t *io_reg, uint16_t addr) {
    (void)io_reg; (void)addr;

    if (addr == 0xFF00) {
        return io_joyp_read(&io_reg->joyp);
    } else if (addr >= 0xFF01 && addr <= 0xFF02) {
        return io_st_read(&io_reg->serial_transfer, addr);
    } else if (addr >= 0xFF04 && addr <= 0xFF07) {
        return timer_read(&io_reg->timer, addr);
    } else if (addr == 0xFF0F || addr == 0xFFFF) {
        return interrupts_read(&io_reg->interrupts, addr);
    } else if (addr >= 0xFF10 && addr <= 0xFF26) {
        return io_audio_read(&io_reg->audio, addr);
    } else if (addr >= 0xFF40 && addr <= 0xFF4B) {
        return io_lcd_read(&io_reg->lcd, addr);
    }

    LOG_WARN("io_reg_read: unimplemented register 0x%04X", addr);
    assert(0 && "io_reg_read: unimplemented register");
}

void io_reg_write(io_reg_t *io_reg, uint16_t addr, uint8_t value) {
    (void)io_reg; (void)addr; (void)value;

    if (addr == 0xFF00) {
        io_joyp_write(&io_reg->joyp, value);
    } else if (addr >= 0xFF01 && addr <= 0xFF02) {
        io_st_write(&io_reg->serial_transfer, addr, value);
    } else if (addr >= 0xFF04 && addr <= 0xFF07) {
        timer_write(&io_reg->timer, addr, value);
    } else if (addr == 0xFF0F || addr == 0xFFFF) {
        interrupts_write(&io_reg->interrupts, addr, value);
    } else if (addr >= 0xFF10 && addr <= 0xFF26) {
        io_audio_write(&io_reg->audio, addr, value);
    } else if (addr >= 0xFF40 && addr <= 0xFF4B) {
        io_lcd_write(&io_reg->lcd, addr, value);
    } else {
        LOG_WARN("io_reg_write: unimplemented register 0x%04X", addr);
        assert(0 && "io_reg_write: unimplemented register");
    }
}

static uint8_t io_joyp_read(joypad_reg_t *joyp) {
    return joypad_read(joyp);
}

static void io_joyp_write(joypad_reg_t *joyp, uint8_t value) {
    joypad_write(joyp, value);
}

static uint8_t io_lcd_read(lcd_regs_t *lcd, uint16_t addr) {
    switch (addr) {
        case 0xFF40:
            return lcd->ctrl.reg;
        case 0xFF41:
            return lcd->stat.reg;
        case 0xFF42:
            return lcd->scy;
        case 0xFF43:
            return lcd->scx;
        case 0xFF44:
            return lcd->ly;
        case 0xFF45:
            return lcd->lyc;
        case 0xFF46:
            return lcd->dma;
        case 0xFF47:
            return lcd->bgp;
        case 0xFF48:
            return lcd->obp0;
        case 0xFF49:
            return lcd->obp1;
        case 0xFF4A:
            return lcd->wy;
        case 0xFF4B:
            return lcd->wx;
    }

    assert(0 && "io_lcd_read: unhandled LCD register");
}

static void io_lcd_write(lcd_regs_t *lcd, uint16_t addr, uint8_t value) {
    switch (addr) {
        case 0xFF40:
            lcd->ctrl.reg = value;
            break;
        case 0xFF41:
            lcd->stat.reg = (lcd->stat.reg & 0x07) | (value & 0x78) | 0x80;
            break;
        case 0xFF42:
            lcd->scy = value;
            break;
        case 0xFF43:
            lcd->scx = value;
            break;
        case 0xFF44:
            LOG_DEBUG("Attempted to write to read-only register 0xFF44 (LY) ignored", addr, value);
            break;
        case 0xFF45:
            lcd->lyc = value;
            break;
        case 0xFF46:
            lcd->dma = value;
            break;
        case 0xFF47:
            lcd->bgp = value;
            break;
        case 0xFF48:
            lcd->obp0 = value;
            break;
        case 0xFF49:
            lcd->obp1 = value;
            break;
        case 0xFF4A:
            lcd->wy = value;
            break;
        case 0xFF4B:
            lcd->wx = value;
            break;
        default:
            assert(0 && "io_lcd_write: unhandled LCD register");
            break;
    }
}

static uint8_t io_st_read(st_regs_t *serial_transfer, uint16_t addr) {
    if (addr == 0xFF01) {
        return serial_transfer->sb;
    } else if (addr == 0xFF02) {
        return serial_transfer->sc;
    }

    assert(0 && "io_st_read: unhandled serial register");
}

static void io_st_write(st_regs_t *serial_transfer, uint16_t addr, uint8_t value) {
    if (addr == 0xFF01) {
        serial_transfer->sb = value;
    } else if (addr == 0xFF02) {
        serial_transfer->sc = value;
        if ((value & 0x81) == 0x81) {
            // Transfer initiated with internal clock — emit the byte
            log_serial((char)serial_transfer->sb);

            // Clear the transfer-start bit to signal completion
            serial_transfer->sc &= ~0x80;
            // Optional: request serial interrupt
            // request_interrupt(INT_SERIAL);
        }
    } else {
        assert(0 && "io_st_write: unhandled serial register");
    }
}

static uint8_t io_audio_read(audio_regs_t *audio, uint16_t addr) {
    if (addr == 0xFF10) {
        return audio->nr10;
    } else if (addr == 0xFF11) {
        return audio->nr11 | 0b00111111;
    } else if (addr == 0xFF12) {
        return audio->nr12;
    } else if (addr == 0xFF13) {
        return 0xFF; // audio->nr13 | 0b11111111; // read-only, always returns 0xFF
    } else if (addr == 0xFF14) {
        return audio->nr14 | 0b10111111;
    } else if (addr == 0xFF16) {
        return audio->nr21 | 0b00111111;
    } else if (addr == 0xFF17) {
        return audio->nr22;
    } else if (addr == 0xFF18) {
        return 0xFF; // audio->nr23 | 0b11111111; // read-only, always returns 0xFF
    } else if (addr == 0xFF19) {
        return audio->nr24 | 0b10111111;
    } else if (addr == 0xFF1A) {
        return audio->nr30;
    } else if (addr == 0xFF1B) {
        return 0xFF; // audio->nr31 | 0b11111111; // read-only, always returns 0xFF
    } else if (addr == 0xFF1C) {
        return audio->nr32;
    } else if (addr == 0xFF1D) {
        return 0xFF; // audio->nr33 | 0b11111111; // read-only, always returns 0xFF
    } else if (addr == 0xFF1E) {
        return audio->nr34 | 0b10111111;
    } else if (addr == 0xFF20) {
        return 0xFF; // audio->nr41 | 0b11111111; // read-only, always returns 0xFF
    } else if (addr == 0xFF21) {
        return audio->nr42;
    } else if (addr == 0xFF22) {
        return audio->nr43;
    } else if (addr == 0xFF23) {
        return audio->nr44 | 0b10111111;
    } else if (addr == 0xFF24) {
        return audio->nr50;
    } else if (addr == 0xFF25) {
        return audio->nr51;
    } else if (addr == 0xFF26) {
        return audio->nr52;
    } else {
        LOG_WARN("Unhandled audio register 0x%04X", addr);
        return 0xFF;
    }
}

static void io_audio_write(audio_regs_t *audio, uint16_t addr, uint8_t value) {
    if (addr == 0xFF10) {
        audio->nr10 = value | 0b10000000;
    } else if (addr == 0xFF11) {
        audio->nr11 = value;
    } else if (addr == 0xFF12) {
        audio->nr12 = value;
    } else if (addr == 0xFF13) {
        audio->nr13 = value;
    } else if (addr == 0xFF14) {
        audio->nr14 = value | 0b00111000;
    } else if (addr == 0xFF16) {
        audio->nr21 = value;
    } else if (addr == 0xFF17) {
        audio->nr22 = value;
    } else if (addr == 0xFF18) {
        audio->nr23 = value;
    } else if (addr == 0xFF19) {
        audio->nr24 = value | 0b00111000;
    } else if (addr == 0xFF1A) {
        audio->nr30 = value | 0b01111111;
    } else if (addr == 0xFF1B) {
        audio->nr31 = value;
    } else if (addr == 0xFF1C) {
        audio->nr32 = value | 0b10011111;
    } else if (addr == 0xFF1D) {
        audio->nr33 = value;
    } else if (addr == 0xFF1E) {
        audio->nr34 = value | 0b00111000;
    } else if (addr == 0xFF20) {
        audio->nr41 = value | 0b11000000;
    } else if (addr == 0xFF21) {
        audio->nr42 = value;
    } else if (addr == 0xFF22) {
        audio->nr43 = value;
    } else if (addr == 0xFF23) {
        audio->nr44 = value | 0b00111111;
    } else if (addr == 0xFF24) {
        audio->nr50 = value;
    } else if (addr == 0xFF25) {
        audio->nr51 = value;
    } else if (addr == 0xFF26) {
        audio->nr52 = value | 0b01111111;
    } else {
        LOG_WARN("Unhandled audio register 0x%04X", addr);
    }
}
