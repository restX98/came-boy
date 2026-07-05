#include "io_reg.h"

#include <assert.h>

#include "logger.h"

static uint8_t io_lcd_read(lcd_regs_t *lcd, uint16_t addr);
static void io_lcd_write(lcd_regs_t *lcd, uint16_t addr, uint8_t value);

int io_reg_init(io_reg_t *io_reg) {
    joypad_init(&io_reg->joyp);

    serial_transfer_init(&io_reg->serial_transfer);

    interrupts_init(&io_reg->interrupts);

    timer_init(&io_reg->timer);

    audio_init(&io_reg->audio);

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
        return io_lcd_read(&io_reg->lcd, addr);
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
        io_lcd_write(&io_reg->lcd, addr, value);
    } else {
        LOG_WARN("io_reg_write: unimplemented register 0x%04X", addr);
        assert(0 && "io_reg_write: unimplemented register");
    }
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
