#include "ppu.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "logger.h"

typedef struct {
    uint8_t oam_index;  // for tie-breaking when X is equal
    int16_t x;          // screen X of left edge (oam_x - 8)
    uint8_t y;          // raw OAM y (screen y is y - 16)
    uint8_t tile;
    uint8_t flags;
} sprite_t;

void ppu_init(ppu_t *ppu) {
    ppu->dot = 0;
    ppu->frame_ready = false;
    memset(ppu->framebuffer, 0, sizeof(ppu->framebuffer));
}

static void set_ppu_mode(bus_t *bus, ppu_mode_t mode) {
    if (!lcd_set_mode(&bus->io_reg.lcd, mode, &bus->io_reg.interrupts)) {
        return;
    }

    switch (mode) {
        case PPU_MODE_HBLANK:
        case PPU_MODE_VBLANK:
            bus->oam_accessible = true;
            bus->vram_accessible = true;
            break;
        case PPU_MODE_OAM_SCAN:
            bus->oam_accessible = false;
            bus->vram_accessible = true;
            break;
        case PPU_MODE_DRAWING:
            bus->oam_accessible = false;
            bus->vram_accessible = false;
            break;
    }
}

static uint8_t fetch_bg_window_pixel(bus_t *bus, uint8_t map_x, uint8_t map_y,
    uint16_t map_base, bool unsigned_addressing) {
    uint8_t *vram = bus->vram.mem;
    uint16_t map_offset = map_base - 0x8000;

    uint8_t tile_row = map_y / 8;
    uint8_t pixel_row = map_y % 8;
    uint8_t tile_col = map_x / 8;
    uint8_t pixel_col = map_x % 8;

    uint8_t tile_index = vram[map_offset + tile_row * 32 + tile_col];

    uint16_t tile_addr;
    if (unsigned_addressing) {
        tile_addr = 0x8000 + tile_index * 16;
    } else {
        tile_addr = 0x9000 + (int8_t)tile_index * 16;
    }
    uint16_t tile_offset = tile_addr - 0x8000;

    uint8_t low = vram[tile_offset + pixel_row * 2];
    uint8_t high = vram[tile_offset + pixel_row * 2 + 1];

    uint8_t bit = 7 - pixel_col;
    return (((high >> bit) & 1) << 1) | ((low >> bit) & 1);
}

static void pixel_fetcher(ppu_t *ppu, bus_t *bus, uint8_t ly) {
    lcd_regs_t *lcd = &bus->io_reg.lcd;

    if (lcd->ctrl.bg_window_enable) {
        bool unsigned_addr = lcd->ctrl.bg_window_tile_data;
        uint16_t bg_map_base = lcd->ctrl.bg_tile_map ? 0x9C00 : 0x9800;
        uint16_t win_map_base = lcd->ctrl.window_tile_map ? 0x9C00 : 0x9800;

        bool window_on_this_line = lcd->ctrl.window_enable && (ly >= lcd->wy);

        for (int x = 0; x < LCD_WIDTH; x++) {
            uint8_t color_id;

            // Window covers this pixel if enabled AND we're past WY AND past WX-7.
            // WX = 7 means the window starts at screen x=0.
            bool in_window = window_on_this_line && (x + 7 >= lcd->wx);

            if (in_window) {
                uint8_t win_x = x + 7 - lcd->wx;
                uint8_t win_y = ly - lcd->wy;
                color_id = fetch_bg_window_pixel(bus, win_x, win_y,
                    win_map_base, unsigned_addr);
            } else {
                uint8_t bg_x = x + lcd->scx;
                uint8_t bg_y = ly + lcd->scy;
                color_id = fetch_bg_window_pixel(bus, bg_x, bg_y,
                    bg_map_base, unsigned_addr);
            }

            uint8_t shade = (lcd->bgp >> (color_id * 2)) & 0x03;
            ppu->framebuffer[ly * LCD_WIDTH + x] = shade;
        }
    } else {
        // BG/Window disabled: blank line, all BG color IDs are 0
        memset(&ppu->framebuffer[ly * LCD_WIDTH], 0, LCD_WIDTH);
    }
}

void ppu_step(ppu_t *ppu, bus_t *bus, int t_cycles) {
    lcd_regs_t *lcd = &bus->io_reg.lcd;

    if (!lcd->ctrl.lcd_enable) {
        ppu->dot = 0;
        lcd->ly = 0;

        set_ppu_mode(bus, PPU_MODE_HBLANK);

        return;
    }

    for (int i = 0; i < t_cycles; i++) {
        ppu->dot++;

        if (lcd->ly < VBLANK_START_LINE) {
            // Visible scanlines: cycle through OAM_SCAN -> DRAWING -> HBLANK
            if (ppu->dot == 80) {
                set_ppu_mode(bus, PPU_MODE_DRAWING);
            } else if (ppu->dot == 80 + 172) {     // simplified: drawing is constant 172
                set_ppu_mode(bus, PPU_MODE_HBLANK);

                pixel_fetcher(ppu, bus, lcd->ly); // draw the whole line at HBlank entry
            }
        }

        if (ppu->dot >= DOTS_PER_SCANLINE) {
            ppu->dot = 0;
            lcd->ly++;

            if (lcd->ly >= SCANLINES_PER_FRAME) {
                lcd->ly = 0;
            }

            lcd_update_stat(lcd, &bus->io_reg.interrupts);

            if (lcd->ly == VBLANK_START_LINE) {
                set_ppu_mode(bus, PPU_MODE_VBLANK);

                ppu->frame_ready = true;
            } else if (lcd->ly < VBLANK_START_LINE) {
                set_ppu_mode(bus, PPU_MODE_OAM_SCAN);
            }
        }
    }
}
