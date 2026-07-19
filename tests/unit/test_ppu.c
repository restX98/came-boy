#include "unity.h"
#include "log_helpers.h"

#include <assert.h>
#include <string.h>

#include "ppu.h"
#include "io/lcd.h"

static ppu_t ppu;
static bus_t bus;
static uint8_t vram_buf[VRAM_SIZE];
static uint8_t oam_buf[OAM_SIZE];

// ---- Mock functions ----
// ppu.c only reaches outside itself through the LCD status helpers.

typedef struct {
    lcd_regs_t *lcd;
    ppu_mode_t mode;
    interrupt_regs_t *interrupts;
} lcd_set_mode_call_t;

typedef struct {
    size_t call_count;
    bool return_value; // returned by every call
    lcd_set_mode_call_t calls[256];
} lcd_set_mode_stats_t;

static lcd_set_mode_stats_t lcd_set_mode_stats;

bool lcd_set_mode(lcd_regs_t *lcd, ppu_mode_t mode, interrupt_regs_t *interrupts) {
    if (lcd_set_mode_stats.call_count == 256) {
        assert(0 && "Exceeded maximum call count for lcd_set_mode_stats");
    }

    lcd_set_mode_call_t *call = &lcd_set_mode_stats.calls[lcd_set_mode_stats.call_count];
    call->lcd = lcd;
    call->mode = mode;
    call->interrupts = interrupts;

    lcd_set_mode_stats.call_count++;

    return lcd_set_mode_stats.return_value;
}

typedef struct {
    lcd_regs_t *lcd;
    interrupt_regs_t *interrupts;
} lcd_update_stat_call_t;

typedef struct {
    size_t call_count;
    lcd_update_stat_call_t calls[256];
} lcd_update_stat_stats_t;

static lcd_update_stat_stats_t lcd_update_stat_stats;

void lcd_update_stat(lcd_regs_t *lcd, interrupt_regs_t *interrupts) {
    if (lcd_update_stat_stats.call_count == 256) {
        assert(0 && "Exceeded maximum call count for lcd_update_stat_stats");
    }

    lcd_update_stat_call_t *call = &lcd_update_stat_stats.calls[lcd_update_stat_stats.call_count];
    call->lcd = lcd;
    call->interrupts = interrupts;

    lcd_update_stat_stats.call_count++;
}

void setUp(void) {
    suppress_logs();

    ppu = (ppu_t){ 0 };
    bus = (bus_t){ 0 };

    memset(vram_buf, 0, sizeof(vram_buf));
    memset(oam_buf, 0, sizeof(oam_buf));
    bus.vram.mem = vram_buf;
    bus.vram.size = VRAM_SIZE;
    bus.oam.mem = oam_buf;
    bus.oam.size = OAM_SIZE;

    lcd_set_mode_stats = (lcd_set_mode_stats_t){ 0 };
    lcd_set_mode_stats.return_value = true; // by default the mode change "takes"
    lcd_update_stat_stats = (lcd_update_stat_stats_t){ 0 };
}

void tearDown(void) {
    restore_logs();
}

// Return the mode of the most recent lcd_set_mode() call.
static ppu_mode_t last_mode(void) {
    TEST_ASSERT_GREATER_THAN_size_t(0, lcd_set_mode_stats.call_count);
    return lcd_set_mode_stats.calls[lcd_set_mode_stats.call_count - 1].mode;
}

// ---- ppu_init ----

void test_ppu_init_resets_state(void) {
    ppu.dot = 123;
    ppu.frame_ready = true;
    memset(ppu.framebuffer, 0xAB, sizeof(ppu.framebuffer));

    ppu_init(&ppu);

    TEST_ASSERT_EQUAL_UINT16(0, ppu.dot);
    TEST_ASSERT_FALSE(ppu.frame_ready);
    TEST_ASSERT_EACH_EQUAL_UINT8(0, ppu.framebuffer, sizeof(ppu.framebuffer));
}

// ---- ppu_step: LCD disabled ----

void test_ppu_step_disabled_resets_dot_and_ly(void) {
    bus.io_reg.lcd.ctrl.lcd_enable = 0;
    ppu.dot = 200;
    bus.io_reg.lcd.ly = 42;

    ppu_step(&ppu, &bus, 8);

    TEST_ASSERT_EQUAL_UINT16(0, ppu.dot);
    TEST_ASSERT_EQUAL_UINT8(0, bus.io_reg.lcd.ly);
}

void test_ppu_step_disabled_forces_hblank_mode(void) {
    bus.io_reg.lcd.ctrl.lcd_enable = 0;

    ppu_step(&ppu, &bus, 1);

    TEST_ASSERT_EQUAL_size_t(1, lcd_set_mode_stats.call_count);
    TEST_ASSERT_EQUAL_UINT8(PPU_MODE_HBLANK, lcd_set_mode_stats.calls[0].mode);
    TEST_ASSERT_EQUAL_PTR(&bus.io_reg.lcd, lcd_set_mode_stats.calls[0].lcd);
    TEST_ASSERT_EQUAL_PTR(&bus.io_reg.interrupts, lcd_set_mode_stats.calls[0].interrupts);
}

// ---- ppu_step: visible-line mode machine ----

void test_ppu_step_enters_drawing_at_dot_80(void) {
    bus.io_reg.lcd.ctrl.lcd_enable = 1;

    ppu_step(&ppu, &bus, 80);

    TEST_ASSERT_EQUAL_UINT16(80, ppu.dot);
    TEST_ASSERT_EQUAL_size_t(1, lcd_set_mode_stats.call_count);
    TEST_ASSERT_EQUAL_UINT8(PPU_MODE_DRAWING, lcd_set_mode_stats.calls[0].mode);
}

void test_ppu_step_enters_hblank_after_drawing(void) {
    bus.io_reg.lcd.ctrl.lcd_enable = 1;

    ppu_step(&ppu, &bus, 80 + 172);

    TEST_ASSERT_EQUAL_UINT16(252, ppu.dot);
    TEST_ASSERT_EQUAL_size_t(2, lcd_set_mode_stats.call_count);
    TEST_ASSERT_EQUAL_UINT8(PPU_MODE_DRAWING, lcd_set_mode_stats.calls[0].mode);
    TEST_ASSERT_EQUAL_UINT8(PPU_MODE_HBLANK, lcd_set_mode_stats.calls[1].mode);
}

void test_ppu_step_drawing_blocks_oam_and_vram(void) {
    bus.io_reg.lcd.ctrl.lcd_enable = 1;
    bus.oam_accessible = true;
    bus.vram_accessible = true;

    ppu_step(&ppu, &bus, 80); // enter DRAWING

    TEST_ASSERT_FALSE(bus.oam_accessible);
    TEST_ASSERT_FALSE(bus.vram_accessible);
}

void test_ppu_step_hblank_restores_oam_and_vram(void) {
    bus.io_reg.lcd.ctrl.lcd_enable = 1;
    bus.oam_accessible = false;
    bus.vram_accessible = false;

    ppu_step(&ppu, &bus, 80 + 172); // through DRAWING into HBLANK

    TEST_ASSERT_EQUAL_UINT8(PPU_MODE_HBLANK, last_mode());
    TEST_ASSERT_TRUE(bus.oam_accessible);
    TEST_ASSERT_TRUE(bus.vram_accessible);
}

void test_ppu_step_mode_change_ignored_when_lcd_set_mode_returns_false(void) {
    bus.io_reg.lcd.ctrl.lcd_enable = 1;
    bus.oam_accessible = true;
    bus.vram_accessible = true;
    lcd_set_mode_stats.return_value = false; // mode change does not take effect

    ppu_step(&ppu, &bus, 80); // would enter DRAWING, but rejected

    // Accessibility flags must be left untouched when the mode change is rejected.
    TEST_ASSERT_TRUE(bus.oam_accessible);
    TEST_ASSERT_TRUE(bus.vram_accessible);
}

// ---- ppu_step: scanline advance ----

void test_ppu_step_advances_ly_and_updates_stat_after_scanline(void) {
    bus.io_reg.lcd.ctrl.lcd_enable = 1;

    ppu_step(&ppu, &bus, DOTS_PER_SCANLINE);

    TEST_ASSERT_EQUAL_UINT16(0, ppu.dot);
    TEST_ASSERT_EQUAL_UINT8(1, bus.io_reg.lcd.ly);
    TEST_ASSERT_EQUAL_size_t(1, lcd_update_stat_stats.call_count);
    TEST_ASSERT_EQUAL_PTR(&bus.io_reg.lcd, lcd_update_stat_stats.calls[0].lcd);
    // A new visible line begins with OAM scan.
    TEST_ASSERT_EQUAL_UINT8(PPU_MODE_OAM_SCAN, last_mode());
}

void test_ppu_step_enters_vblank_and_sets_frame_ready(void) {
    bus.io_reg.lcd.ctrl.lcd_enable = 1;
    bus.io_reg.lcd.ly = VBLANK_START_LINE - 1; // 143
    ppu.dot = DOTS_PER_SCANLINE - 1;

    ppu_step(&ppu, &bus, 1); // roll over into line 144

    TEST_ASSERT_EQUAL_UINT8(VBLANK_START_LINE, bus.io_reg.lcd.ly);
    TEST_ASSERT_TRUE(ppu.frame_ready);
    TEST_ASSERT_EQUAL_UINT8(PPU_MODE_VBLANK, last_mode());
}

void test_ppu_step_ly_wraps_at_frame_end(void) {
    bus.io_reg.lcd.ctrl.lcd_enable = 1;
    bus.io_reg.lcd.ly = SCANLINES_PER_FRAME - 1; // 153
    ppu.dot = DOTS_PER_SCANLINE - 1;

    ppu_step(&ppu, &bus, 1);

    TEST_ASSERT_EQUAL_UINT8(0, bus.io_reg.lcd.ly);
}

// ---- ppu_step: background rendering ----

// Configure a minimal, fully-visible BG: unsigned $8000 tile data, tile map at
// $9800, identity palette, no scroll, sprites off.
static void setup_bg_only(void) {
    lcd_regs_t *lcd = &bus.io_reg.lcd;
    lcd->ctrl.lcd_enable = 1;
    lcd->ctrl.bg_window_enable = 1;
    lcd->ctrl.bg_window_tile_data = 1; // unsigned addressing from $8000
    lcd->ctrl.bg_tile_map = 0;         // $9800
    lcd->ctrl.window_enable = 0;
    lcd->ctrl.obj_enable = 0;
    lcd->scx = 0;
    lcd->scy = 0;
    lcd->bgp = 0xE4; // 11 10 01 00 -> identity: color id == shade
}

void test_ppu_renders_bg_tile_to_framebuffer(void) {
    setup_bg_only();

    // Tile 0 at $8000, row 0: low=0xA0, high=0xC0 gives color ids 3,2,1,0 for the
    // first four pixels (rest 0). Tile map is all zeroes -> every cell uses tile 0.
    vram_buf[0] = 0xA0; // low plane, row 0
    vram_buf[1] = 0xC0; // high plane, row 0

    ppu_step(&ppu, &bus, 80 + 172); // render line 0 at HBlank entry

    TEST_ASSERT_EQUAL_UINT8(3, ppu.framebuffer[0]);
    TEST_ASSERT_EQUAL_UINT8(2, ppu.framebuffer[1]);
    TEST_ASSERT_EQUAL_UINT8(1, ppu.framebuffer[2]);
    TEST_ASSERT_EQUAL_UINT8(0, ppu.framebuffer[3]);
}

void test_ppu_renders_bg_through_palette(void) {
    setup_bg_only();
    // Palette maps color id 3 -> shade 1 (bits 7:6 = 01).
    bus.io_reg.lcd.bgp = 0x40; // 01 00 00 00

    vram_buf[0] = 0x80; // low: bit7 set
    vram_buf[1] = 0x80; // high: bit7 set -> pixel 0 color id 3

    ppu_step(&ppu, &bus, 80 + 172);

    TEST_ASSERT_EQUAL_UINT8(1, ppu.framebuffer[0]);
}

void test_ppu_blanks_line_when_bg_disabled(void) {
    bus.io_reg.lcd.ctrl.lcd_enable = 1;
    bus.io_reg.lcd.ctrl.bg_window_enable = 0;
    bus.io_reg.lcd.ctrl.obj_enable = 0;
    ppu.framebuffer[5] = 0xAA; // pre-existing garbage on line 0

    ppu_step(&ppu, &bus, 80 + 172);

    TEST_ASSERT_EQUAL_UINT8(0, ppu.framebuffer[5]);
}

// ---- ppu_step: sprite rendering ----

void test_ppu_renders_sprite_over_blank_bg(void) {
    lcd_regs_t *lcd = &bus.io_reg.lcd;
    lcd->ctrl.lcd_enable = 1;
    lcd->ctrl.bg_window_enable = 0; // blank BG so the sprite draws freely
    lcd->ctrl.obj_enable = 1;
    lcd->ctrl.obj_size = 0; // 8x8
    lcd->obp0 = 0xE4;       // identity palette

    // OAM entry 0: y=16 (screen y 0), x=8 (screen x 0), tile 1, no flags.
    oam_buf[0] = 16;
    oam_buf[1] = 8;
    oam_buf[2] = 1;
    oam_buf[3] = 0;

    // Tile 1 lives at $8000 + 16. Row 0: low=0x80, high=0x80 -> pixel 0 color id 3.
    vram_buf[16] = 0x80;
    vram_buf[17] = 0x80;

    ppu_step(&ppu, &bus, 80 + 172); // render line 0

    TEST_ASSERT_EQUAL_UINT8(3, ppu.framebuffer[0]); // sprite pixel
    TEST_ASSERT_EQUAL_UINT8(0, ppu.framebuffer[1]); // transparent -> blank BG
}

void test_ppu_sprite_transparent_color0_leaves_bg(void) {
    lcd_regs_t *lcd = &bus.io_reg.lcd;
    lcd->ctrl.lcd_enable = 1;
    lcd->ctrl.bg_window_enable = 0;
    lcd->ctrl.obj_enable = 1;
    lcd->ctrl.obj_size = 0;
    lcd->obp0 = 0xE4;

    oam_buf[0] = 16;
    oam_buf[1] = 8;
    oam_buf[2] = 1;
    oam_buf[3] = 0;

    // All-zero tile -> every sprite pixel is color id 0 (transparent).
    vram_buf[16] = 0x00;
    vram_buf[17] = 0x00;

    ppu_step(&ppu, &bus, 80 + 172);

    TEST_ASSERT_EQUAL_UINT8(0, ppu.framebuffer[0]);
}

void test_ppu_renders_bg_signed_tile_addressing(void) {
    setup_bg_only();
    bus.io_reg.lcd.ctrl.bg_window_tile_data = 0; // signed addressing from $9000

    // Tile index 0 -> $9000 (vram offset 0x1000). Row 0: low=0x80 gives color id 1.
    vram_buf[0x1000] = 0x80;
    vram_buf[0x1001] = 0x00;

    ppu_step(&ppu, &bus, 80 + 172);

    TEST_ASSERT_EQUAL_UINT8(1, ppu.framebuffer[0]);
}

void test_ppu_renders_window(void) {
    setup_bg_only();
    bus.io_reg.lcd.ctrl.window_enable = 1;
    bus.io_reg.lcd.ctrl.window_tile_map = 0; // $9800
    bus.io_reg.lcd.wy = 0;
    bus.io_reg.lcd.wx = 7; // window starts at screen x = 0

    // Window map is all zeroes -> tile 0; give tile 0 an opaque pixel 0.
    vram_buf[0] = 0x80;
    vram_buf[1] = 0x80; // color id 3 -> shade 3 (identity palette)

    ppu_step(&ppu, &bus, 80 + 172);

    TEST_ASSERT_EQUAL_UINT8(3, ppu.framebuffer[0]);
}

void test_ppu_sprite_y_flip(void) {
    lcd_regs_t *lcd = &bus.io_reg.lcd;
    lcd->ctrl.lcd_enable = 1;
    lcd->ctrl.bg_window_enable = 0;
    lcd->ctrl.obj_enable = 1;
    lcd->ctrl.obj_size = 0;
    lcd->obp0 = 0xE4;

    oam_buf[0] = 16;
    oam_buf[1] = 8;
    oam_buf[2] = 1;
    oam_buf[3] = OAM_FLAG_Y_FLIP;

    // With Y-flip, line 0 of the sprite reads row 7 of the tile. Make row 0
    // transparent and row 7 opaque so a hit proves the flip happened.
    vram_buf[16 + 0] = 0x00; // row 0 low
    vram_buf[16 + 1] = 0x00; // row 0 high
    vram_buf[16 + 14] = 0x80; // row 7 low
    vram_buf[16 + 15] = 0x80; // row 7 high -> color id 3

    ppu_step(&ppu, &bus, 80 + 172);

    TEST_ASSERT_EQUAL_UINT8(3, ppu.framebuffer[0]);
}

void test_ppu_sprite_8x16_lower_tile(void) {
    lcd_regs_t *lcd = &bus.io_reg.lcd;
    lcd->ctrl.lcd_enable = 1;
    lcd->ctrl.bg_window_enable = 0;
    lcd->ctrl.obj_enable = 1;
    lcd->ctrl.obj_size = 1; // 8x16
    lcd->obp0 = 0xE4;
    lcd->ly = 8; // second half of the sprite

    // y=16 -> screen y 0; at ly=8 line_in_sprite=8 selects the lower tile:
    // (tile & 0xFE) | 1. tile=2 -> lower tile index 3 ($8000 + 48).
    oam_buf[0] = 16;
    oam_buf[1] = 8;
    oam_buf[2] = 2;
    oam_buf[3] = 0;

    vram_buf[48] = 0x80; // tile 3, row 0 low
    vram_buf[49] = 0x80; // tile 3, row 0 high -> color id 3

    ppu_step(&ppu, &bus, 80 + 172); // render line 8

    TEST_ASSERT_EQUAL_UINT8(3, ppu.framebuffer[8 * LCD_WIDTH + 0]);
}

void test_ppu_sprite_x_priority_lower_x_wins(void) {
    lcd_regs_t *lcd = &bus.io_reg.lcd;
    lcd->ctrl.lcd_enable = 1;
    lcd->ctrl.bg_window_enable = 0;
    lcd->ctrl.obj_enable = 1;
    lcd->ctrl.obj_size = 0;
    lcd->obp0 = 0xE4;

    // Sprite 0 at screen x 0..7 (color id 3), sprite 1 at screen x 1..8 (color
    // id 2). They overlap at x 1..7; the lower-X sprite 0 has priority there.
    oam_buf[0] = 16; oam_buf[1] = 8;  oam_buf[2] = 1; oam_buf[3] = 0; // sprite 0
    oam_buf[4] = 16; oam_buf[5] = 9;  oam_buf[6] = 2; oam_buf[7] = 0; // sprite 1

    vram_buf[16] = 0xFF; vram_buf[17] = 0xFF; // tile 1: all color id 3
    vram_buf[32] = 0x00; vram_buf[33] = 0xFF; // tile 2: all color id 2

    ppu_step(&ppu, &bus, 80 + 172);

    TEST_ASSERT_EQUAL_UINT8(3, ppu.framebuffer[1]); // overlap -> sprite 0 wins
    TEST_ASSERT_EQUAL_UINT8(2, ppu.framebuffer[8]); // only sprite 1 reaches here
}

void test_ppu_sprite_equal_x_lower_oam_index_wins(void) {
    lcd_regs_t *lcd = &bus.io_reg.lcd;
    lcd->ctrl.lcd_enable = 1;
    lcd->ctrl.bg_window_enable = 0;
    lcd->ctrl.obj_enable = 1;
    lcd->ctrl.obj_size = 0;
    lcd->obp0 = 0xE4;

    // Two sprites at the same X: the OAM-index tie-break gives sprite 0 priority.
    oam_buf[0] = 16; oam_buf[1] = 8; oam_buf[2] = 1; oam_buf[3] = 0; // sprite 0
    oam_buf[4] = 16; oam_buf[5] = 8; oam_buf[6] = 2; oam_buf[7] = 0; // sprite 1

    vram_buf[16] = 0xFF; vram_buf[17] = 0xFF; // tile 1: color id 3
    vram_buf[32] = 0x00; vram_buf[33] = 0xFF; // tile 2: color id 2

    ppu_step(&ppu, &bus, 80 + 172);

    TEST_ASSERT_EQUAL_UINT8(3, ppu.framebuffer[0]); // sprite 0 wins the tie
}

void test_ppu_sprite_behind_bg_when_priority_set(void) {
    lcd_regs_t *lcd = &bus.io_reg.lcd;
    lcd->ctrl.lcd_enable = 1;
    lcd->ctrl.bg_window_enable = 1;
    lcd->ctrl.bg_window_tile_data = 1;
    lcd->ctrl.bg_tile_map = 0;
    lcd->ctrl.obj_enable = 1;
    lcd->ctrl.obj_size = 0;
    lcd->bgp = 0xE4;
    lcd->obp0 = 0xE4;

    // BG pixel 0 is non-zero (color id 1).
    vram_buf[0] = 0x80;
    vram_buf[1] = 0x00;

    // Opaque sprite at screen x 0 but with BG-priority flag set -> hidden behind
    // non-zero BG.
    oam_buf[0] = 16;
    oam_buf[1] = 8;
    oam_buf[2] = 1;
    oam_buf[3] = OAM_FLAG_PRIORITY;
    vram_buf[16] = 0x80;
    vram_buf[17] = 0x80; // sprite color id 3

    ppu_step(&ppu, &bus, 80 + 172);

    TEST_ASSERT_EQUAL_UINT8(1, ppu.framebuffer[0]); // BG shows through
}

void test_ppu_sprite_uses_obp1_palette(void) {
    lcd_regs_t *lcd = &bus.io_reg.lcd;
    lcd->ctrl.lcd_enable = 1;
    lcd->ctrl.bg_window_enable = 0;
    lcd->ctrl.obj_enable = 1;
    lcd->ctrl.obj_size = 0;
    lcd->obp0 = 0xE4;
    lcd->obp1 = 0x40; // color id 3 -> shade 1

    oam_buf[0] = 16;
    oam_buf[1] = 8;
    oam_buf[2] = 1;
    oam_buf[3] = OAM_FLAG_PALETTE; // select OBP1
    vram_buf[16] = 0x80;
    vram_buf[17] = 0x80; // color id 3

    ppu_step(&ppu, &bus, 80 + 172);

    TEST_ASSERT_EQUAL_UINT8(1, ppu.framebuffer[0]);
}

void test_ppu_sprite_x_flip(void) {
    lcd_regs_t *lcd = &bus.io_reg.lcd;
    lcd->ctrl.lcd_enable = 1;
    lcd->ctrl.bg_window_enable = 0;
    lcd->ctrl.obj_enable = 1;
    lcd->ctrl.obj_size = 0;
    lcd->obp0 = 0xE4;

    oam_buf[0] = 16;
    oam_buf[1] = 8;
    oam_buf[2] = 1;
    oam_buf[3] = OAM_FLAG_X_FLIP;
    // Only bit 7 set -> unflipped that is pixel 0; flipped it lands at pixel 7.
    vram_buf[16] = 0x80;
    vram_buf[17] = 0x80;

    ppu_step(&ppu, &bus, 80 + 172);

    TEST_ASSERT_EQUAL_UINT8(0, ppu.framebuffer[0]); // pixel 0 now transparent
    TEST_ASSERT_EQUAL_UINT8(3, ppu.framebuffer[7]); // opaque pixel mirrored here
}

void test_ppu_sprite_clipped_at_screen_edges(void) {
    lcd_regs_t *lcd = &bus.io_reg.lcd;
    lcd->ctrl.lcd_enable = 1;
    lcd->ctrl.bg_window_enable = 0;
    lcd->ctrl.obj_enable = 1;
    lcd->ctrl.obj_size = 0;
    lcd->obp0 = 0xE4;

    // Sprite A straddles the left edge (screen x -4..3), sprite B the right edge
    // (screen x 156..163). Off-screen pixels must be skipped without corruption.
    oam_buf[0] = 16; oam_buf[1] = 4;   oam_buf[2] = 1; oam_buf[3] = 0; // screen -4
    oam_buf[4] = 16; oam_buf[5] = 164; oam_buf[6] = 1; oam_buf[7] = 0; // screen 156
    vram_buf[16] = 0xFF; vram_buf[17] = 0xFF; // tile 1: all color id 3

    ppu_step(&ppu, &bus, 80 + 172);

    TEST_ASSERT_EQUAL_UINT8(3, ppu.framebuffer[0]);   // A, first on-screen pixel
    TEST_ASSERT_EQUAL_UINT8(3, ppu.framebuffer[156]); // B, first pixel
    TEST_ASSERT_EQUAL_UINT8(3, ppu.framebuffer[159]); // B, last on-screen pixel
}

void test_ppu_sprite_priority_draws_over_bg_color0(void) {
    lcd_regs_t *lcd = &bus.io_reg.lcd;
    lcd->ctrl.lcd_enable = 1;
    lcd->ctrl.bg_window_enable = 1;
    lcd->ctrl.bg_window_tile_data = 1;
    lcd->ctrl.bg_tile_map = 0;
    lcd->ctrl.obj_enable = 1;
    lcd->ctrl.obj_size = 0;
    lcd->bgp = 0xE4;
    lcd->obp0 = 0xE4;

    // BG pixel 0 is color id 0 (blank tile) -> an OBJ-priority sprite still wins.
    vram_buf[0] = 0x00;
    vram_buf[1] = 0x00;

    oam_buf[0] = 16;
    oam_buf[1] = 8;
    oam_buf[2] = 1;
    oam_buf[3] = OAM_FLAG_PRIORITY;
    vram_buf[16] = 0x80;
    vram_buf[17] = 0x80; // sprite color id 3

    ppu_step(&ppu, &bus, 80 + 172);

    TEST_ASSERT_EQUAL_UINT8(3, ppu.framebuffer[0]);
}

void test_ppu_sprite_8x16_upper_tile(void) {
    lcd_regs_t *lcd = &bus.io_reg.lcd;
    lcd->ctrl.lcd_enable = 1;
    lcd->ctrl.bg_window_enable = 0;
    lcd->ctrl.obj_enable = 1;
    lcd->ctrl.obj_size = 1; // 8x16
    lcd->obp0 = 0xE4;

    // At ly 0 the upper tile (tile & 0xFE) is used. tile=2 -> tile 2 ($8000+32).
    oam_buf[0] = 16;
    oam_buf[1] = 8;
    oam_buf[2] = 2;
    oam_buf[3] = 0;
    vram_buf[32] = 0x80;
    vram_buf[33] = 0x80; // color id 3

    ppu_step(&ppu, &bus, 80 + 172);

    TEST_ASSERT_EQUAL_UINT8(3, ppu.framebuffer[0]);
}

void test_ppu_limits_to_ten_sprites_per_line(void) {
    lcd_regs_t *lcd = &bus.io_reg.lcd;
    lcd->ctrl.lcd_enable = 1;
    lcd->ctrl.bg_window_enable = 0;
    lcd->ctrl.obj_enable = 1;
    lcd->ctrl.obj_size = 0;
    lcd->obp0 = 0xE4;

    // 11 sprites on line 0, each in its own 8px column. Only the first 10 (by OAM
    // index) are drawn; the 11th (column at screen 80) is dropped.
    for (int i = 0; i < 11; i++) {
        oam_buf[i * 4 + 0] = 16;
        oam_buf[i * 4 + 1] = (uint8_t)(8 + i * 8); // screen x = i * 8
        oam_buf[i * 4 + 2] = 1;
        oam_buf[i * 4 + 3] = 0;
    }
    vram_buf[16] = 0xFF;
    vram_buf[17] = 0xFF; // tile 1: color id 3

    ppu_step(&ppu, &bus, 80 + 172);

    TEST_ASSERT_EQUAL_UINT8(3, ppu.framebuffer[72]); // sprite 9 (10th) drawn
    TEST_ASSERT_EQUAL_UINT8(0, ppu.framebuffer[80]); // sprite 10 (11th) dropped
}

void test_ppu_sprite_off_scanline_is_skipped(void) {
    lcd_regs_t *lcd = &bus.io_reg.lcd;
    lcd->ctrl.lcd_enable = 1;
    lcd->ctrl.bg_window_enable = 0;
    lcd->ctrl.obj_enable = 1;
    lcd->ctrl.obj_size = 0;
    lcd->obp0 = 0xE4;

    // Sprite 0 covers line 0; sprite 1 sits far below and must be culled.
    oam_buf[0] = 16;  oam_buf[1] = 8; oam_buf[2] = 1; oam_buf[3] = 0;
    oam_buf[4] = 100; oam_buf[5] = 8; oam_buf[6] = 1; oam_buf[7] = 0; // screen y 84
    vram_buf[16] = 0x80;
    vram_buf[17] = 0x80;

    ppu_step(&ppu, &bus, 80 + 172); // render line 0

    TEST_ASSERT_EQUAL_UINT8(3, ppu.framebuffer[0]);
}

void test_ppu_bg_uses_alternate_tile_map(void) {
    setup_bg_only();
    bus.io_reg.lcd.ctrl.bg_tile_map = 1; // $9C00

    // $9800 (unused map) stays 0 -> blank tile 0. $9C00 map cell 0 -> tile 5.
    vram_buf[0x1C00] = 5;
    vram_buf[5 * 16 + 0] = 0x80;
    vram_buf[5 * 16 + 1] = 0x80; // tile 5 pixel 0 -> color id 3

    ppu_step(&ppu, &bus, 80 + 172);

    TEST_ASSERT_EQUAL_UINT8(3, ppu.framebuffer[0]);
}

void test_ppu_window_uses_alternate_tile_map(void) {
    setup_bg_only();
    bus.io_reg.lcd.ctrl.window_enable = 1;
    bus.io_reg.lcd.ctrl.window_tile_map = 1; // $9C00
    bus.io_reg.lcd.wy = 0;
    bus.io_reg.lcd.wx = 7;

    vram_buf[0x1C00] = 7;
    vram_buf[7 * 16 + 0] = 0x80;
    vram_buf[7 * 16 + 1] = 0x80; // tile 7 pixel 0 -> color id 3

    ppu_step(&ppu, &bus, 80 + 172);

    TEST_ASSERT_EQUAL_UINT8(3, ppu.framebuffer[0]);
}

void test_ppu_window_skipped_when_ly_below_wy(void) {
    setup_bg_only();
    bus.io_reg.lcd.ctrl.window_enable = 1;
    bus.io_reg.lcd.wy = 10; // window starts below line 0
    bus.io_reg.lcd.wx = 7;

    // BG tile 0 pixel 0 -> color id 1; window would differ, but is inactive here.
    vram_buf[0] = 0x80;
    vram_buf[1] = 0x00;

    ppu_step(&ppu, &bus, 80 + 172); // line 0, below WY

    TEST_ASSERT_EQUAL_UINT8(1, ppu.framebuffer[0]); // BG, not window
}

void test_ppu_pixels_left_of_wx_use_bg(void) {
    setup_bg_only();
    bus.io_reg.lcd.ctrl.window_enable = 1;
    bus.io_reg.lcd.wy = 0;
    bus.io_reg.lcd.wx = 80; // window covers screen x >= 73

    vram_buf[0] = 0x80;
    vram_buf[1] = 0x00; // tile 0 pixel 0 -> color id 1

    ppu_step(&ppu, &bus, 80 + 172);

    TEST_ASSERT_EQUAL_UINT8(1, ppu.framebuffer[0]); // left of window -> BG
}

void test_ppu_vblank_lines_do_not_reenter_oam_scan(void) {
    bus.io_reg.lcd.ctrl.lcd_enable = 1;
    bus.io_reg.lcd.ly = 150; // mid-VBlank
    ppu.dot = DOTS_PER_SCANLINE - 1;

    ppu_step(&ppu, &bus, 1); // advance to line 151

    TEST_ASSERT_EQUAL_UINT8(151, bus.io_reg.lcd.ly);
    TEST_ASSERT_EQUAL_size_t(1, lcd_update_stat_stats.call_count);
    // Lines 145..153 neither re-enter VBlank nor start an OAM scan.
    TEST_ASSERT_EQUAL_size_t(0, lcd_set_mode_stats.call_count);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_ppu_init_resets_state);

    RUN_TEST(test_ppu_step_disabled_resets_dot_and_ly);
    RUN_TEST(test_ppu_step_disabled_forces_hblank_mode);

    RUN_TEST(test_ppu_step_enters_drawing_at_dot_80);
    RUN_TEST(test_ppu_step_enters_hblank_after_drawing);
    RUN_TEST(test_ppu_step_drawing_blocks_oam_and_vram);
    RUN_TEST(test_ppu_step_hblank_restores_oam_and_vram);
    RUN_TEST(test_ppu_step_mode_change_ignored_when_lcd_set_mode_returns_false);

    RUN_TEST(test_ppu_step_advances_ly_and_updates_stat_after_scanline);
    RUN_TEST(test_ppu_step_enters_vblank_and_sets_frame_ready);
    RUN_TEST(test_ppu_step_ly_wraps_at_frame_end);

    RUN_TEST(test_ppu_renders_bg_tile_to_framebuffer);
    RUN_TEST(test_ppu_renders_bg_through_palette);
    RUN_TEST(test_ppu_blanks_line_when_bg_disabled);

    RUN_TEST(test_ppu_renders_bg_signed_tile_addressing);
    RUN_TEST(test_ppu_renders_window);

    RUN_TEST(test_ppu_renders_sprite_over_blank_bg);
    RUN_TEST(test_ppu_sprite_transparent_color0_leaves_bg);
    RUN_TEST(test_ppu_sprite_y_flip);
    RUN_TEST(test_ppu_sprite_8x16_lower_tile);
    RUN_TEST(test_ppu_sprite_x_priority_lower_x_wins);
    RUN_TEST(test_ppu_sprite_equal_x_lower_oam_index_wins);
    RUN_TEST(test_ppu_sprite_behind_bg_when_priority_set);
    RUN_TEST(test_ppu_sprite_uses_obp1_palette);
    RUN_TEST(test_ppu_sprite_x_flip);
    RUN_TEST(test_ppu_sprite_clipped_at_screen_edges);
    RUN_TEST(test_ppu_sprite_priority_draws_over_bg_color0);
    RUN_TEST(test_ppu_sprite_8x16_upper_tile);
    RUN_TEST(test_ppu_limits_to_ten_sprites_per_line);
    RUN_TEST(test_ppu_sprite_off_scanline_is_skipped);

    RUN_TEST(test_ppu_bg_uses_alternate_tile_map);
    RUN_TEST(test_ppu_window_uses_alternate_tile_map);
    RUN_TEST(test_ppu_window_skipped_when_ly_below_wy);
    RUN_TEST(test_ppu_pixels_left_of_wx_use_bg);

    RUN_TEST(test_ppu_vblank_lines_do_not_reenter_oam_scan);

    return UNITY_END();
}
