#include "unity.h"
#include "log_helpers.h"

#include <assert.h>

#include "io/lcd.h"

typedef struct {
    size_t call_count;
    interrupt_t interrupts[10];
} interrupts_request_stats_t;

static interrupts_request_stats_t interrupts_request_stats;

void interrupts_request(interrupt_regs_t *interrupts, interrupt_t interrupt) {
    (void)interrupts;
    if (interrupts_request_stats.call_count == 10) {
        assert(0 && "Exceeded maximum call count for interrupts_request_stats");
    }

    interrupts_request_stats.interrupts[interrupts_request_stats.call_count] = interrupt;
    interrupts_request_stats.call_count++;
}

static lcd_regs_t lcd;
static interrupt_regs_t interrupts;

void setUp(void) {
    suppress_logs();

    lcd = (lcd_regs_t){ 0 };
    interrupts = (interrupt_regs_t){ 0 };
    interrupts_request_stats = (interrupts_request_stats_t){ 0 };
}

void tearDown(void) {
    restore_logs();
}

// ---- lcd_init ----

void test_lcd_init_sets_boot_values(void) {
    lcd_init(&lcd);

    TEST_ASSERT_EQUAL_HEX8(0x91, lcd.ctrl.reg);
    TEST_ASSERT_EQUAL_HEX8(0x85, lcd.stat.reg);
    TEST_ASSERT_EQUAL_HEX8(0x00, lcd.scy);
    TEST_ASSERT_EQUAL_HEX8(0x00, lcd.scx);
    TEST_ASSERT_EQUAL_HEX8(0x00, lcd.ly);
    TEST_ASSERT_EQUAL_HEX8(0x00, lcd.lyc);
    TEST_ASSERT_EQUAL_HEX8(0xFF, lcd.dma);
    TEST_ASSERT_EQUAL_HEX8(0xFC, lcd.bgp);
    TEST_ASSERT_EQUAL_HEX8(0x00, lcd.obp0);
    TEST_ASSERT_EQUAL_HEX8(0x00, lcd.obp1);
    TEST_ASSERT_EQUAL_HEX8(0x00, lcd.wy);
    TEST_ASSERT_EQUAL_HEX8(0x00, lcd.wx);
}

// ---- lcd_read ----

void test_lcd_read_returns_registers(void) {
    // Distinct values so a wrong field mapping would be caught.
    lcd.ctrl.reg = 0x11; // 0xFF40
    lcd.stat.reg = 0x22; // 0xFF41
    lcd.scy = 0x33;      // 0xFF42
    lcd.scx = 0x44;      // 0xFF43
    lcd.ly = 0x55;       // 0xFF44
    lcd.lyc = 0x66;      // 0xFF45
    lcd.dma = 0x77;      // 0xFF46
    lcd.bgp = 0x88;      // 0xFF47
    lcd.obp0 = 0x99;     // 0xFF48
    lcd.obp1 = 0xAA;     // 0xFF49
    lcd.wy = 0xBB;       // 0xFF4A
    lcd.wx = 0xCC;       // 0xFF4B

    TEST_ASSERT_EQUAL_HEX8(0x11, lcd_read(&lcd, 0xFF40));
    TEST_ASSERT_EQUAL_HEX8(0x22, lcd_read(&lcd, 0xFF41));
    TEST_ASSERT_EQUAL_HEX8(0x33, lcd_read(&lcd, 0xFF42));
    TEST_ASSERT_EQUAL_HEX8(0x44, lcd_read(&lcd, 0xFF43));
    TEST_ASSERT_EQUAL_HEX8(0x55, lcd_read(&lcd, 0xFF44));
    TEST_ASSERT_EQUAL_HEX8(0x66, lcd_read(&lcd, 0xFF45));
    TEST_ASSERT_EQUAL_HEX8(0x77, lcd_read(&lcd, 0xFF46));
    TEST_ASSERT_EQUAL_HEX8(0x88, lcd_read(&lcd, 0xFF47));
    TEST_ASSERT_EQUAL_HEX8(0x99, lcd_read(&lcd, 0xFF48));
    TEST_ASSERT_EQUAL_HEX8(0xAA, lcd_read(&lcd, 0xFF49));
    TEST_ASSERT_EQUAL_HEX8(0xBB, lcd_read(&lcd, 0xFF4A));
    TEST_ASSERT_EQUAL_HEX8(0xCC, lcd_read(&lcd, 0xFF4B));
}

// ---- lcd_write ----

void test_lcd_write_stores_registers(void) {
    lcd_write(&lcd, 0xFF40, 0xA5, &interrupts);
    TEST_ASSERT_EQUAL_HEX8(0xA5, lcd.ctrl.reg);
    lcd_write(&lcd, 0xFF42, 0xA5, &interrupts);
    TEST_ASSERT_EQUAL_HEX8(0xA5, lcd.scy);
    lcd_write(&lcd, 0xFF43, 0xA5, &interrupts);
    TEST_ASSERT_EQUAL_HEX8(0xA5, lcd.scx);
    lcd_write(&lcd, 0xFF45, 0xA5, &interrupts);
    TEST_ASSERT_EQUAL_HEX8(0xA5, lcd.lyc);
    lcd_write(&lcd, 0xFF46, 0xA5, &interrupts);
    TEST_ASSERT_EQUAL_HEX8(0xA5, lcd.dma);
    lcd_write(&lcd, 0xFF47, 0xA5, &interrupts);
    TEST_ASSERT_EQUAL_HEX8(0xA5, lcd.bgp);
    lcd_write(&lcd, 0xFF48, 0xA5, &interrupts);
    TEST_ASSERT_EQUAL_HEX8(0xA5, lcd.obp0);
    lcd_write(&lcd, 0xFF49, 0xA5, &interrupts);
    TEST_ASSERT_EQUAL_HEX8(0xA5, lcd.obp1);
    lcd_write(&lcd, 0xFF4A, 0xA5, &interrupts);
    TEST_ASSERT_EQUAL_HEX8(0xA5, lcd.wy);
    lcd_write(&lcd, 0xFF4B, 0xA5, &interrupts);
    TEST_ASSERT_EQUAL_HEX8(0xA5, lcd.wx);
}

void test_lcd_write_stat_preserves_read_only_bits(void) {
    // Bits 0-2 (PPU mode + LYC=LY) are read-only and kept from the old value;
    // bits 3-6 come from the write; bit 7 is forced to 1.
    lcd.stat.reg = 0x05; // read-only bits 0 and 2 set

    lcd_write(&lcd, 0xFF41, 0xFF, &interrupts);

    // (0x05 & 0x07) | (0xFF & 0x78) | 0x80 == 0x05 | 0x78 | 0x80 == 0xFD
    TEST_ASSERT_EQUAL_HEX8(0xFD, lcd.stat.reg);
}

void test_lcd_write_stat_ignores_low_bits_of_value(void) {
    lcd.stat.reg = 0x00;
    lcd.lyc = 0x01; // ly (0) != lyc so lcd_update_stat leaves the LYC=LY flag (bit 2) clear

    lcd_write(&lcd, 0xFF41, 0x07, &interrupts); // low 3 bits should not be written

    // (0x00 & 0x07) | (0x07 & 0x78) | 0x80 == 0x80
    TEST_ASSERT_EQUAL_HEX8(0x80, lcd.stat.reg);
}

void test_lcd_write_ly_is_read_only(void) {
    lcd.ly = 0xAB;

    lcd_write(&lcd, 0xFF44, 0x55, &interrupts);

    TEST_ASSERT_EQUAL_HEX8(0xAB, lcd.ly);
}

// ---- lcd_set_mode ----

void test_lcd_set_mode_changes_mode_and_returns_true(void) {
    lcd.stat.ppu_mode = PPU_MODE_HBLANK;

    bool changed = lcd_set_mode(&lcd, PPU_MODE_OAM_SCAN, &interrupts);

    TEST_ASSERT_TRUE(changed);
    TEST_ASSERT_EQUAL_UINT8(PPU_MODE_OAM_SCAN, lcd.stat.ppu_mode);
}

void test_lcd_set_mode_same_mode_returns_false_without_side_effects(void) {
    lcd.stat.ppu_mode = PPU_MODE_VBLANK;

    bool changed = lcd_set_mode(&lcd, PPU_MODE_VBLANK, &interrupts);

    // Guard should short-circuit before touching the register or interrupts.
    TEST_ASSERT_FALSE(changed);
    TEST_ASSERT_EQUAL_UINT8(PPU_MODE_VBLANK, lcd.stat.ppu_mode);
    TEST_ASSERT_EQUAL_size_t(0, interrupts_request_stats.call_count);
}

void test_lcd_set_mode_entering_vblank_requests_vblank_interrupt(void) {
    lcd.stat.ppu_mode = PPU_MODE_HBLANK;

    lcd_set_mode(&lcd, PPU_MODE_VBLANK, &interrupts);

    TEST_ASSERT_EQUAL_size_t(1, interrupts_request_stats.call_count);
    TEST_ASSERT_EQUAL_INT(INT_VBLANK, interrupts_request_stats.interrupts[0]);
}

void test_lcd_set_mode_entering_non_vblank_requests_no_interrupt(void) {
    lcd.stat.ppu_mode = PPU_MODE_HBLANK;

    lcd_set_mode(&lcd, PPU_MODE_DRAWING, &interrupts);

    // No VBLANK (not mode 1) and no STAT source enabled -> nothing requested.
    TEST_ASSERT_EQUAL_size_t(0, interrupts_request_stats.call_count);
}

void test_lcd_set_mode_vblank_interrupt_fires_only_on_transition(void) {
    lcd.stat.ppu_mode = PPU_MODE_HBLANK;

    lcd_set_mode(&lcd, PPU_MODE_VBLANK, &interrupts); // transition -> requests
    lcd_set_mode(&lcd, PPU_MODE_VBLANK, &interrupts); // no-op -> nothing

    TEST_ASSERT_EQUAL_size_t(1, interrupts_request_stats.call_count);
}

void test_lcd_set_mode_updates_stat_interrupt_line(void) {
    // A mode change runs lcd_update_stat: entering OAM scan with the mode-2
    // STAT source enabled raises the STAT (LCD) interrupt.
    lcd.stat.ppu_mode = PPU_MODE_HBLANK;
    lcd.stat.mode2_int_sel = 1;

    lcd_set_mode(&lcd, PPU_MODE_OAM_SCAN, &interrupts);

    TEST_ASSERT_EQUAL_size_t(1, interrupts_request_stats.call_count);
    TEST_ASSERT_EQUAL_INT(INT_LCD, interrupts_request_stats.interrupts[0]);
}

// ---- lcd_update_stat ----

void test_lcd_update_stat_sets_lyc_eq_ly_when_equal(void) {
    lcd.ly = 0x42;
    lcd.lyc = 0x42;

    lcd_update_stat(&lcd, &interrupts);

    TEST_ASSERT_TRUE(lcd.stat.lyc_eq_ly);
}

void test_lcd_update_stat_clears_lyc_eq_ly_when_not_equal(void) {
    lcd.ly = 0x42;
    lcd.lyc = 0x43;
    lcd.stat.lyc_eq_ly = 1; // stale value that must be recomputed to 0

    lcd_update_stat(&lcd, &interrupts);

    TEST_ASSERT_FALSE(lcd.stat.lyc_eq_ly);
}

void test_lcd_update_stat_requests_interrupt_on_lyc_coincidence(void) {
    lcd.ly = 0x10;
    lcd.lyc = 0x10;
    lcd.stat.lyc_int_sel = 1;

    lcd_update_stat(&lcd, &interrupts);

    TEST_ASSERT_EQUAL_size_t(1, interrupts_request_stats.call_count);
    TEST_ASSERT_EQUAL_INT(INT_LCD, interrupts_request_stats.interrupts[0]);
}

void test_lcd_update_stat_no_lyc_interrupt_without_coincidence(void) {
    // LYC select enabled but LY != LYC: the LYC STAT source stays low.
    lcd.ly = 0x10;
    lcd.lyc = 0x11;
    lcd.stat.lyc_int_sel = 1;

    lcd_update_stat(&lcd, &interrupts);

    TEST_ASSERT_FALSE(lcd.stat.lyc_eq_ly);
    TEST_ASSERT_EQUAL_size_t(0, interrupts_request_stats.call_count);
    TEST_ASSERT_FALSE(lcd.stat_line);
}

void test_lcd_update_stat_requests_interrupt_on_mode_source(void) {
    lcd.stat.ppu_mode = PPU_MODE_HBLANK; // mode 0
    lcd.stat.mode0_int_sel = 1;

    lcd_update_stat(&lcd, &interrupts);

    TEST_ASSERT_EQUAL_size_t(1, interrupts_request_stats.call_count);
    TEST_ASSERT_EQUAL_INT(INT_LCD, interrupts_request_stats.interrupts[0]);
}

void test_lcd_update_stat_mode1_interrupt_in_vblank(void) {
    lcd.ly = 0x00;
    lcd.lyc = 0x01; // avoid LYC coincidence
    lcd.stat.ppu_mode = PPU_MODE_VBLANK; // mode 1
    lcd.stat.mode1_int_sel = 1;

    lcd_update_stat(&lcd, &interrupts);

    TEST_ASSERT_EQUAL_size_t(1, interrupts_request_stats.call_count);
    TEST_ASSERT_EQUAL_INT(INT_LCD, interrupts_request_stats.interrupts[0]);
}

void test_lcd_update_stat_no_interrupt_in_drawing_mode(void) {
    // Mode 3 (drawing) has no STAT source bit, so even with every mode-select
    // enabled the STAT line stays low.
    lcd.ly = 0x01;
    lcd.lyc = 0x02; // avoid LYC coincidence
    lcd.stat.ppu_mode = PPU_MODE_DRAWING;
    lcd.stat.mode0_int_sel = 1;
    lcd.stat.mode1_int_sel = 1;
    lcd.stat.mode2_int_sel = 1;

    lcd_update_stat(&lcd, &interrupts);

    TEST_ASSERT_EQUAL_size_t(0, interrupts_request_stats.call_count);
    TEST_ASSERT_FALSE(lcd.stat_line);
}

void test_lcd_update_stat_fires_only_on_rising_edge(void) {
    lcd.stat.ppu_mode = PPU_MODE_HBLANK;
    lcd.stat.mode0_int_sel = 1;

    lcd_update_stat(&lcd, &interrupts); // low -> high: fires
    lcd_update_stat(&lcd, &interrupts); // high -> high: no new interrupt

    TEST_ASSERT_EQUAL_size_t(1, interrupts_request_stats.call_count);
    TEST_ASSERT_TRUE(lcd.stat_line);
}

void test_lcd_update_stat_refires_after_line_goes_low(void) {
    lcd.stat.ppu_mode = PPU_MODE_HBLANK;
    lcd.stat.mode0_int_sel = 1;

    lcd_update_stat(&lcd, &interrupts);   // rising edge: fires (count 1)

    lcd.stat.ppu_mode = PPU_MODE_DRAWING; // condition no longer holds
    lcd_update_stat(&lcd, &interrupts);   // line falls, no fire
    TEST_ASSERT_FALSE(lcd.stat_line);

    lcd.stat.ppu_mode = PPU_MODE_HBLANK;  // condition holds again
    lcd_update_stat(&lcd, &interrupts);   // rising edge again: fires (count 2)

    TEST_ASSERT_EQUAL_size_t(2, interrupts_request_stats.call_count);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_lcd_init_sets_boot_values);

    RUN_TEST(test_lcd_read_returns_registers);

    RUN_TEST(test_lcd_write_stores_registers);
    RUN_TEST(test_lcd_write_stat_preserves_read_only_bits);
    RUN_TEST(test_lcd_write_stat_ignores_low_bits_of_value);
    RUN_TEST(test_lcd_write_ly_is_read_only);

    RUN_TEST(test_lcd_set_mode_changes_mode_and_returns_true);
    RUN_TEST(test_lcd_set_mode_same_mode_returns_false_without_side_effects);
    RUN_TEST(test_lcd_set_mode_entering_vblank_requests_vblank_interrupt);
    RUN_TEST(test_lcd_set_mode_entering_non_vblank_requests_no_interrupt);
    RUN_TEST(test_lcd_set_mode_vblank_interrupt_fires_only_on_transition);
    RUN_TEST(test_lcd_set_mode_updates_stat_interrupt_line);

    RUN_TEST(test_lcd_update_stat_sets_lyc_eq_ly_when_equal);
    RUN_TEST(test_lcd_update_stat_clears_lyc_eq_ly_when_not_equal);
    RUN_TEST(test_lcd_update_stat_requests_interrupt_on_lyc_coincidence);
    RUN_TEST(test_lcd_update_stat_no_lyc_interrupt_without_coincidence);
    RUN_TEST(test_lcd_update_stat_requests_interrupt_on_mode_source);
    RUN_TEST(test_lcd_update_stat_mode1_interrupt_in_vblank);
    RUN_TEST(test_lcd_update_stat_no_interrupt_in_drawing_mode);
    RUN_TEST(test_lcd_update_stat_fires_only_on_rising_edge);
    RUN_TEST(test_lcd_update_stat_refires_after_line_goes_low);

    return UNITY_END();
}
