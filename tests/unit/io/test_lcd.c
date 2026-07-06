#include "unity.h"
#include "log_helpers.h"

#include "io/lcd.h"

static lcd_regs_t lcd;

void setUp(void) {
    suppress_logs();

    lcd = (lcd_regs_t){ 0 };
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
    lcd_write(&lcd, 0xFF40, 0xA5);
    TEST_ASSERT_EQUAL_HEX8(0xA5, lcd.ctrl.reg);
    lcd_write(&lcd, 0xFF42, 0xA5);
    TEST_ASSERT_EQUAL_HEX8(0xA5, lcd.scy);
    lcd_write(&lcd, 0xFF43, 0xA5);
    TEST_ASSERT_EQUAL_HEX8(0xA5, lcd.scx);
    lcd_write(&lcd, 0xFF45, 0xA5);
    TEST_ASSERT_EQUAL_HEX8(0xA5, lcd.lyc);
    lcd_write(&lcd, 0xFF46, 0xA5);
    TEST_ASSERT_EQUAL_HEX8(0xA5, lcd.dma);
    lcd_write(&lcd, 0xFF47, 0xA5);
    TEST_ASSERT_EQUAL_HEX8(0xA5, lcd.bgp);
    lcd_write(&lcd, 0xFF48, 0xA5);
    TEST_ASSERT_EQUAL_HEX8(0xA5, lcd.obp0);
    lcd_write(&lcd, 0xFF49, 0xA5);
    TEST_ASSERT_EQUAL_HEX8(0xA5, lcd.obp1);
    lcd_write(&lcd, 0xFF4A, 0xA5);
    TEST_ASSERT_EQUAL_HEX8(0xA5, lcd.wy);
    lcd_write(&lcd, 0xFF4B, 0xA5);
    TEST_ASSERT_EQUAL_HEX8(0xA5, lcd.wx);
}

void test_lcd_write_stat_preserves_read_only_bits(void) {
    // Bits 0-2 (PPU mode + LYC=LY) are read-only and kept from the old value;
    // bits 3-6 come from the write; bit 7 is forced to 1.
    lcd.stat.reg = 0x05; // read-only bits 0 and 2 set

    lcd_write(&lcd, 0xFF41, 0xFF);

    // (0x05 & 0x07) | (0xFF & 0x78) | 0x80 == 0x05 | 0x78 | 0x80 == 0xFD
    TEST_ASSERT_EQUAL_HEX8(0xFD, lcd.stat.reg);
}

void test_lcd_write_stat_ignores_low_bits_of_value(void) {
    lcd.stat.reg = 0x00;

    lcd_write(&lcd, 0xFF41, 0x07); // low 3 bits should not be written

    // (0x00 & 0x07) | (0x07 & 0x78) | 0x80 == 0x80
    TEST_ASSERT_EQUAL_HEX8(0x80, lcd.stat.reg);
}

void test_lcd_write_ly_is_read_only(void) {
    lcd.ly = 0xAB;

    lcd_write(&lcd, 0xFF44, 0x55);

    TEST_ASSERT_EQUAL_HEX8(0xAB, lcd.ly);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_lcd_init_sets_boot_values);

    RUN_TEST(test_lcd_read_returns_registers);

    RUN_TEST(test_lcd_write_stores_registers);
    RUN_TEST(test_lcd_write_stat_preserves_read_only_bits);
    RUN_TEST(test_lcd_write_stat_ignores_low_bits_of_value);
    RUN_TEST(test_lcd_write_ly_is_read_only);

    return UNITY_END();
}
