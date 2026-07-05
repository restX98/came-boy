#ifndef IO_REG_H
#define IO_REG_H

#include <stdint.h>

#include "interrupts.h"
#include "joypad.h"
#include "timer.h"

typedef union {
    uint8_t reg;
    struct {
        uint8_t bg_window_enable : 1;    // bit 0 — BG & Window enable (DMG) / priority (CGB)
        uint8_t obj_enable : 1;          // bit 1 — OBJ (sprite) enable
        uint8_t obj_size : 1;            // bit 2 — OBJ size (0 = 8×8, 1 = 8×16)
        uint8_t bg_tile_map : 1;         // bit 3 — BG tile map area (0 = $9800, 1 = $9C00)
        uint8_t bg_window_tile_data : 1; // bit 4 — BG/Window tile data area (0 = $8800, 1 = $8000)
        uint8_t window_enable : 1;       // bit 5 — Window enable
        uint8_t window_tile_map : 1;     // bit 6 — Window tile map area (0 = $9800, 1 = $9C00)
        uint8_t lcd_enable : 1;          // bit 7 — LCD & PPU enable
    };
} lcd_ctrl_t;

typedef union {
    uint8_t reg;
    struct {
        uint8_t ppu_mode : 2;      // bits 0-1 — current PPU mode (read-only)
        uint8_t lyc_eq_ly : 1;     // bit 2    — LYC == LY flag (read-only, set by PPU)
        uint8_t mode0_int_sel : 1; // bit 3    — Mode 0 (HBlank) STAT interrupt enable
        uint8_t mode1_int_sel : 1; // bit 4    — Mode 1 (VBlank) STAT interrupt enable
        uint8_t mode2_int_sel : 1; // bit 5    — Mode 2 (OAM scan) STAT interrupt enable
        uint8_t lyc_int_sel : 1;   // bit 6    — LYC=LY STAT interrupt enable
        uint8_t _ : 1;             // bit 7    — unused, always 1 on real hardware
    };
} lcd_stat_t;

typedef struct {
    lcd_ctrl_t ctrl;    // 0xFF40 — LCD control
    lcd_stat_t stat;    // 0xFF41 — LCD status
    uint8_t scy;        // 0xFF42 — Background viewport Y position
    uint8_t scx;        // 0xFF43 — Background viewport X position
    uint8_t ly;         // 0xFF44 — LCD Y coordinate [read-only]
    uint8_t lyc;        // 0xFF45 — LY compare
    uint8_t dma;        // 0xFF46 — OAM DMA source address & start
    uint8_t bgp;        // 0xFF47 — BGP (Non-CGB Mode only): BG palette data
    uint8_t obp0;       // 0xFF48 — OBP0 (Non-CGB Mode only): OBJ palette 0 data
    uint8_t obp1;       // 0xFF49 — OBP1 (Non-CGB Mode only): OBJ palette 1 data
    uint8_t wy;         // 0xFF4A — Window Y position
    uint8_t wx;         // 0xFF4B — Window X position plus 7
} lcd_regs_t;

typedef struct {
    uint8_t sb;         // 0xFF01 — Serial transfer data
    uint8_t sc;         // 0xFF02 — Serial transfer control
} st_regs_t;

typedef struct {
    uint8_t nr52;       // 0xFF26 — Audio master control
    uint8_t nr51;       // 0xFF25 — Sound panning
    uint8_t nr50;       // 0xFF24 — Master volume & VIN panning
    uint8_t nr10;       // 0xFF10 — Channel 1 sweep
    uint8_t nr11;       // 0xFF11 — Channel 1 length timer & duty cycle
    uint8_t nr12;       // 0xFF12 — Channel 1 volume & envelope
    uint8_t nr13;       // 0xFF13 — Channel 1 period low [write-only]
    uint8_t nr14;       // 0xFF14 — Channel 1 period high & control
    uint8_t nr21;       // 0xFF16 — NR21 ($FF16) → NR11
    uint8_t nr22;       // 0xFF17 — NR22 ($FF17) → NR12
    uint8_t nr23;       // 0xFF18 — NR23 ($FF18) → NR13
    uint8_t nr24;       // 0xFF19 — NR24 ($FF19) → NR14
    uint8_t nr30;       // 0xFF1A — Channel 3 DAC enable
    uint8_t nr31;       // 0xFF1B — Channel 3 length timer [write-only]
    uint8_t nr32;       // 0xFF1C — Channel 3 output level
    uint8_t nr33;       // 0xFF1D — Channel 3 period low [write-only]
    uint8_t nr34;       // 0xFF1E — Channel 3 period high & control
    uint8_t nr41;       // 0xFF20 — Channel 4 length timer [write-only]
    uint8_t nr42;       // 0xFF21 — Channel 4 volume & envelope
    uint8_t nr43;       // 0xFF22 — Channel 4 frequency & randomness
    uint8_t nr44;       // 0xFF23 — Channel 4 control
} audio_regs_t;

typedef struct {
    joypad_reg_t joyp;        // 0xFF00 — Joypad
    interrupt_regs_t interrupts;
    lcd_regs_t lcd;
    st_regs_t serial_transfer;
    timer_regs_t timer;
    audio_regs_t audio;
    uint8_t wp_ram[16];       // 0xFF30–0xFF3F — Wave pattern RAM
    uint8_t boot_rom_disable; // 0xFF50 — Boot ROM mapping control [read-only]
} io_reg_t;

int io_reg_init(io_reg_t *io_reg);
uint8_t io_reg_read(io_reg_t *io_reg, uint16_t addr);
void io_reg_write(io_reg_t *io_reg, uint16_t addr, uint8_t value);

#endif // IO_REG_H
