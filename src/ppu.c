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
    ppu->window_line = 0;
    ppu->wy_condition = false;

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

static int compare_sprites_dmg(const void *a, const void *b) {
    const sprite_t *sa = a;
    const sprite_t *sb = b;
    if (sa->x != sb->x) return sa->x - sb->x;
    return (int)sa->oam_index - (int)sb->oam_index;
}

static void render_sprites_scanline(ppu_t *ppu, bus_t *bus, uint8_t ly, const uint8_t *bg_color_id) {
    lcd_regs_t *lcd = &bus->io_reg.lcd;
    uint8_t *oam = bus->oam.mem;
    uint8_t *vram = bus->vram.mem;

    uint8_t sprite_height = lcd->ctrl.obj_size ? 16 : 8;

    // OAM scan: collect up to 10 sprites overlapping this scanline (Y check only).
    sprite_t visible[MAX_SPRITES_PER_LINE];
    int num_visible = 0;

    for (int i = 0; i < OAM_ENTRY_COUNT && num_visible < MAX_SPRITES_PER_LINE; i++) {
        uint8_t y_pos = oam[i * 4];
        int16_t screen_y = (int16_t)y_pos - 16;
        if ((int16_t)ly >= screen_y && (int16_t)ly < screen_y + sprite_height) {
            visible[num_visible].oam_index = (uint8_t)i;
            visible[num_visible].y = y_pos;
            visible[num_visible].x = (int16_t)oam[i * 4 + 1] - 8;
            visible[num_visible].tile = oam[i * 4 + 2];
            visible[num_visible].flags = oam[i * 4 + 3];
            num_visible++;
        }
    }

    // DMG priority: smaller X wins; OAM index breaks ties. Sort ascending so
    // iteration is in highest-priority-first order; we then skip pixels that
    // an earlier (higher priority) sprite already claimed.
    qsort(visible, num_visible, sizeof(sprite_t), compare_sprites_dmg);

    bool pixel_taken[LCD_WIDTH] = { false };

    for (int i = 0; i < num_visible; i++) {
        sprite_t *s = &visible[i];

        int line_in_sprite = (int)ly - ((int)s->y - 16);  // 0..sprite_height-1
        if (s->flags & OAM_FLAG_Y_FLIP) {
            line_in_sprite = sprite_height - 1 - line_in_sprite;
        }

        // 8×16 tiles always come in pairs; bit 0 of the tile index is ignored.
        uint8_t tile = s->tile;
        if (sprite_height == 16) {
            tile &= 0xFE;
            if (line_in_sprite >= 8) {
                tile |= 0x01;
                line_in_sprite -= 8;
            }
        }

        // Sprites always use $8000 unsigned addressing.
        uint16_t tile_offset = (uint16_t)tile * 16;
        uint8_t low = vram[tile_offset + line_in_sprite * 2];
        uint8_t high = vram[tile_offset + line_in_sprite * 2 + 1];

        uint8_t palette = (s->flags & OAM_FLAG_PALETTE) ? lcd->obp1 : lcd->obp0;
        bool bg_priority = s->flags & OAM_FLAG_PRIORITY;

        for (int px = 0; px < 8; px++) {
            int screen_x = (int)s->x + px;
            if (screen_x < 0 || screen_x >= LCD_WIDTH) continue;
            if (pixel_taken[screen_x]) continue;

            int bit = (s->flags & OAM_FLAG_X_FLIP) ? px : (7 - px);
            uint8_t color_id = (((high >> bit) & 1) << 1) | ((low >> bit) & 1);
            if (color_id == 0) continue;  // sprite color 0 = transparent

            // Mark taken: even when this sprite loses to BG, lower-priority
            // sprites in this column must not draw over it.
            pixel_taken[screen_x] = true;

            if (bg_priority && bg_color_id[screen_x] != 0) continue;

            uint8_t shade = (palette >> (color_id * 2)) & 0x03;
            ppu->framebuffer[ly * LCD_WIDTH + screen_x] = shade;
        }
    }
}

static void pixel_fetcher(ppu_t *ppu, bus_t *bus, uint8_t ly) {
    lcd_regs_t *lcd = &bus->io_reg.lcd;

    uint8_t bg_color_id[LCD_WIDTH] = { 0 };

    // Y condition latches on exact equality, checked every scanline,
    // independent of LCDC bits.
    if (ly == lcd->wy) {
        ppu->wy_condition = true;
    }

    if (lcd->ctrl.bg_window_enable) {
        bool unsigned_addr = lcd->ctrl.bg_window_tile_data;
        uint16_t bg_map_base = lcd->ctrl.bg_tile_map ? 0x9C00 : 0x9800;
        uint16_t win_map_base = lcd->ctrl.window_tile_map ? 0x9C00 : 0x9800;

        bool window_active = lcd->ctrl.window_enable
            && ppu->wy_condition
            && lcd->wx <= 166;
        bool window_drawn = false;

        for (int x = 0; x < LCD_WIDTH; x++) {
            uint8_t color_id;

            // Window covers this pixel if enabled AND we're past WY AND past WX-7.
            // WX = 7 means the window starts at screen x=0.
            bool in_window = window_active && (x + 7 >= lcd->wx);

            if (in_window) {
                uint8_t win_x = x + 7 - lcd->wx;
                uint8_t win_y = ppu->window_line;
                color_id = fetch_bg_window_pixel(bus, win_x, win_y, win_map_base, unsigned_addr);
                window_drawn = true;
            } else {
                uint8_t bg_x = x + lcd->scx;
                uint8_t bg_y = ly + lcd->scy;
                color_id = fetch_bg_window_pixel(bus, bg_x, bg_y,
                    bg_map_base, unsigned_addr);
            }

            bg_color_id[x] = color_id;
            uint8_t shade = (lcd->bgp >> (color_id * 2)) & 0x03;
            ppu->framebuffer[ly * LCD_WIDTH + x] = shade;
        }

        if (window_drawn) {
            ppu->window_line++;
        }
    } else {
        // BG/Window disabled: blank line, all BG color IDs are 0
        memset(&ppu->framebuffer[ly * LCD_WIDTH], 0, LCD_WIDTH);
    }

    if (lcd->ctrl.obj_enable) {
        render_sprites_scanline(ppu, bus, ly, bg_color_id);
    }
}

void ppu_step(ppu_t *ppu, bus_t *bus, int t_cycles) {
    lcd_regs_t *lcd = &bus->io_reg.lcd;

    if (!lcd->ctrl.lcd_enable) {
        ppu->dot = 0;
        ppu->window_line = 0;
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
                ppu->window_line = 0;
            }

            lcd_update_stat(lcd, &bus->io_reg.interrupts);

            if (lcd->ly == VBLANK_START_LINE) {
                set_ppu_mode(bus, PPU_MODE_VBLANK);

                ppu->window_line = 0;
                ppu->wy_condition = false;
                ppu->frame_ready = true;
            } else if (lcd->ly < VBLANK_START_LINE) {
                set_ppu_mode(bus, PPU_MODE_OAM_SCAN);
            }
        }
    }
}
