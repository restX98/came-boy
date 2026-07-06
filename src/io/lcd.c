#include "lcd.h"

#include <assert.h>

#include "logger.h"

void lcd_init(lcd_regs_t *lcd) {
    lcd->ctrl.reg = 0x91;
    lcd->stat.reg = 0x85;
    lcd->scy = 0x00;
    lcd->scx = 0x00;
    lcd->ly = 0x00;
    lcd->lyc = 0x00;
    lcd->dma = 0xFF;
    lcd->bgp = 0xFC;
    lcd->obp0 = 0x00;
    lcd->obp1 = 0x00;
    lcd->wy = 0x00;
    lcd->wx = 0x00;
}

uint8_t lcd_read(lcd_regs_t *lcd, uint16_t addr) {
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
        default:
            assert(0 && "io_lcd_read: unhandled LCD register");
            break;
    }

}

void lcd_write(lcd_regs_t *lcd, uint16_t addr, uint8_t value) {
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
            lcd_update_stat(lcd, interrupts);
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
