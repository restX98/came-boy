#include "io_reg.h"

#include <assert.h>

#include "logger.h"

static uint8_t io_interrupt_read(interrupt_regs_t *interrupts, uint16_t addr);
static void io_interrupt_write(interrupt_regs_t *interrupts, uint16_t addr, uint8_t value);
static uint8_t io_lcd_read(lcd_regs_t *lcd, uint16_t addr);
static void io_lcd_write(lcd_regs_t *lcd, uint16_t addr, uint8_t value);
static uint8_t io_st_read(st_regs_t *serial_transfer, uint16_t addr);
static void io_st_write(st_regs_t *serial_transfer, uint16_t addr, uint8_t value);

int io_reg_init(io_reg_t *io_reg) {
    io_reg->joyp = 0xCF;
    io_reg->serial_transfer.sb = 0x00;
    io_reg->serial_transfer.sc = 0x7E;

    io_reg->interrupts.flag.reg = 0xE1;
    io_reg->interrupts.enable.reg = 0x00;

    io_reg->timer.div = 0xAB;
    io_reg->timer.tima = 0x00;
    io_reg->timer.tma = 0x00;
    io_reg->timer.tac = 0xF8;

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

    io_reg->lcd.lcdc = 0x91;
    io_reg->lcd.stat = 0x85;
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

    if (addr == 0xFF0F || addr == 0xFFFF) {
        return io_interrupt_read(&io_reg->interrupts, addr);
    } else if (addr >= 0xFF01 && addr <= 0xFF02) {
        return io_st_read(&io_reg->serial_transfer, addr);
    } else if (addr >= 0xFF40 && addr <= 0xFF4B) {
        return io_lcd_read(&io_reg->lcd, addr);
    }

    LOG_WARN("io_reg_read: unimplemented register 0x%04X", addr);
    assert(0 && "io_reg_read: unimplemented register");
}

void io_reg_write(io_reg_t *io_reg, uint16_t addr, uint8_t value) {
    (void)io_reg; (void)addr; (void)value;

    if (addr == 0xFF0F || addr == 0xFFFF) {
        io_interrupt_write(&io_reg->interrupts, addr, value);
    } else if (addr >= 0xFF01 && addr <= 0xFF02) {
        io_st_write(&io_reg->serial_transfer, addr, value);
    } else if (addr >= 0xFF40 && addr <= 0xFF4B) {
        io_lcd_write(&io_reg->lcd, addr, value);
    } else {
        LOG_WARN("io_reg_write: unimplemented register 0x%04X", addr);
        assert(0 && "io_reg_write: unimplemented register");
    }
}

static uint8_t io_interrupt_read(interrupt_regs_t *interrupts, uint16_t addr) {
    if (addr == 0xFF0F) {
        return interrupts->flag.reg;
    } else if (addr == 0xFFFF) {
        return interrupts->enable.reg;
    }

    assert(0 && "Assert something");
}

static void io_interrupt_write(interrupt_regs_t *interrupts, uint16_t addr, uint8_t value) {
    value |= 0b11100000;
    if (addr == 0xFF0F) {
        interrupts->flag.reg = value;
    } else if (addr == 0xFFFF) {
        interrupts->enable.reg = value;
    } else {
        assert(0 && "Assert something");
    }
}

static uint8_t io_lcd_read(lcd_regs_t *lcd, uint16_t addr) {
    switch (addr) {
        case 0xFF40:
            return lcd->lcdc;
        case 0xFF41:
            return lcd->stat;
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

    assert(0 && "Assert something");
}

static void io_lcd_write(lcd_regs_t *lcd, uint16_t addr, uint8_t value) {
    switch (addr) {
        case 0xFF40:
            lcd->lcdc = value;
            break;
        case 0xFF41:
            // TODO: fix writability (0, 1, 2, bits are read only)
            lcd->stat = value;
            break;
        case 0xFF42:
            lcd->scy = value;
            break;
        case 0xFF43:
            lcd->scx = value;
            break;
        case 0xFF44:
            assert(0 && "Register 0xFF44 is read only");
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
            assert(0 && "Assert something");
            break;
    }
}

static uint8_t io_st_read(st_regs_t *serial_transfer, uint16_t addr) {
    if (addr == 0xFF01) {
        return serial_transfer->sb;
    } else if (addr == 0xFF02) {
        return serial_transfer->sc;
    }

    assert(0 && "Assert something");
}

static void io_st_write(st_regs_t *serial_transfer, uint16_t addr, uint8_t value) {
    if (addr == 0xFF01) {
        serial_transfer->sb = value;
    } else if (addr == 0xFF02) {
        serial_transfer->sc = value;
    } else {
        assert(0 && "Assert something");
    }
}
