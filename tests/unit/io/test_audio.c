#include "unity.h"
#include "log_helpers.h"

#include <string.h>

#include "io/audio.h"

static audio_regs_t audio;

void setUp(void) {
    suppress_logs();

    audio = (audio_regs_t){ 0 };
}

void tearDown(void) {
    restore_logs();
}

// ---- audio_init ----

void test_audio_init_sets_boot_values(void) {
    audio_init(&audio);

    TEST_ASSERT_EQUAL_HEX8(0x80, audio.nr10);
    TEST_ASSERT_EQUAL_HEX8(0xBF, audio.nr11);
    TEST_ASSERT_EQUAL_HEX8(0xF3, audio.nr12);
    TEST_ASSERT_EQUAL_HEX8(0xFF, audio.nr13);
    TEST_ASSERT_EQUAL_HEX8(0xBF, audio.nr14);
    TEST_ASSERT_EQUAL_HEX8(0x3F, audio.nr21);
    TEST_ASSERT_EQUAL_HEX8(0x00, audio.nr22);
    TEST_ASSERT_EQUAL_HEX8(0xFF, audio.nr23);
    TEST_ASSERT_EQUAL_HEX8(0xBF, audio.nr24);
    TEST_ASSERT_EQUAL_HEX8(0x7F, audio.nr30);
    TEST_ASSERT_EQUAL_HEX8(0xFF, audio.nr31);
    TEST_ASSERT_EQUAL_HEX8(0x9F, audio.nr32);
    TEST_ASSERT_EQUAL_HEX8(0xFF, audio.nr33);
    TEST_ASSERT_EQUAL_HEX8(0xBF, audio.nr34);
    TEST_ASSERT_EQUAL_HEX8(0xFF, audio.nr41);
    TEST_ASSERT_EQUAL_HEX8(0x00, audio.nr42);
    TEST_ASSERT_EQUAL_HEX8(0x00, audio.nr43);
    TEST_ASSERT_EQUAL_HEX8(0xBF, audio.nr44);
    TEST_ASSERT_EQUAL_HEX8(0x77, audio.nr50);
    TEST_ASSERT_EQUAL_HEX8(0xF3, audio.nr51);
    TEST_ASSERT_EQUAL_HEX8(0xF1, audio.nr52);
}

// ---- audio_read ----

void test_audio_read_returns_raw_registers(void) {
    audio.nr10 = 0xA5; // 0xFF10
    audio.nr12 = 0xA5; // 0xFF12
    audio.nr22 = 0xA5; // 0xFF17
    audio.nr30 = 0xA5; // 0xFF1A
    audio.nr32 = 0xA5; // 0xFF1C
    audio.nr42 = 0xA5; // 0xFF21
    audio.nr43 = 0xA5; // 0xFF22
    audio.nr50 = 0xA5; // 0xFF24
    audio.nr51 = 0xA5; // 0xFF25
    audio.nr52 = 0xA5; // 0xFF26

    TEST_ASSERT_EQUAL_HEX8(0xA5, audio_read(&audio, 0xFF10));
    TEST_ASSERT_EQUAL_HEX8(0xA5, audio_read(&audio, 0xFF12));
    TEST_ASSERT_EQUAL_HEX8(0xA5, audio_read(&audio, 0xFF17));
    TEST_ASSERT_EQUAL_HEX8(0xA5, audio_read(&audio, 0xFF1A));
    TEST_ASSERT_EQUAL_HEX8(0xA5, audio_read(&audio, 0xFF1C));
    TEST_ASSERT_EQUAL_HEX8(0xA5, audio_read(&audio, 0xFF21));
    TEST_ASSERT_EQUAL_HEX8(0xA5, audio_read(&audio, 0xFF22));
    TEST_ASSERT_EQUAL_HEX8(0xA5, audio_read(&audio, 0xFF24));
    TEST_ASSERT_EQUAL_HEX8(0xA5, audio_read(&audio, 0xFF25));
    TEST_ASSERT_EQUAL_HEX8(0xA5, audio_read(&audio, 0xFF26));
}

void test_audio_read_applies_read_masks(void) {
    // Each stored bit sits outside its register's mask, so the result is the
    // stored value OR'd with the always-set bits.
    audio.nr11 = 0x40; // 0xFF11 — value | 0x3F
    audio.nr14 = 0x40; // 0xFF14 — value | 0xBF
    audio.nr21 = 0x40; // 0xFF16 — value | 0x3F
    audio.nr24 = 0x40; // 0xFF19 — value | 0xBF
    audio.nr34 = 0x40; // 0xFF1E — value | 0xBF
    audio.nr44 = 0x40; // 0xFF23 — value | 0xBF

    TEST_ASSERT_EQUAL_HEX8(0x7F, audio_read(&audio, 0xFF11));
    TEST_ASSERT_EQUAL_HEX8(0xFF, audio_read(&audio, 0xFF14));
    TEST_ASSERT_EQUAL_HEX8(0x7F, audio_read(&audio, 0xFF16));
    TEST_ASSERT_EQUAL_HEX8(0xFF, audio_read(&audio, 0xFF19));
    TEST_ASSERT_EQUAL_HEX8(0xFF, audio_read(&audio, 0xFF1E));
    TEST_ASSERT_EQUAL_HEX8(0xFF, audio_read(&audio, 0xFF23));
}

void test_audio_read_write_only_returns_0xFF(void) {
    // Underlying fields are non-zero to prove they are never exposed.
    audio.nr13 = 0x11; // 0xFF13
    audio.nr23 = 0x22; // 0xFF18
    audio.nr31 = 0x33; // 0xFF1B
    audio.nr33 = 0x44; // 0xFF1D
    audio.nr41 = 0x55; // 0xFF20

    TEST_ASSERT_EQUAL_HEX8(0xFF, audio_read(&audio, 0xFF13));
    TEST_ASSERT_EQUAL_HEX8(0xFF, audio_read(&audio, 0xFF18));
    TEST_ASSERT_EQUAL_HEX8(0xFF, audio_read(&audio, 0xFF1B));
    TEST_ASSERT_EQUAL_HEX8(0xFF, audio_read(&audio, 0xFF1D));
    TEST_ASSERT_EQUAL_HEX8(0xFF, audio_read(&audio, 0xFF20));
}

void test_audio_read_unhandled_returns_0xFF(void) {
    // 0xFF15 and 0xFF1F are gaps in the register map; 0xFF27 is past the end.
    TEST_ASSERT_EQUAL_HEX8(0xFF, audio_read(&audio, 0xFF15));
    TEST_ASSERT_EQUAL_HEX8(0xFF, audio_read(&audio, 0xFF1F));
    TEST_ASSERT_EQUAL_HEX8(0xFF, audio_read(&audio, 0xFF27));
}

// ---- audio_write ----

void test_audio_write_stores_raw_registers(void) {
    audio_write(&audio, 0xFF11, 0xA5);
    TEST_ASSERT_EQUAL_HEX8(0xA5, audio.nr11);
    audio_write(&audio, 0xFF12, 0xA5);
    TEST_ASSERT_EQUAL_HEX8(0xA5, audio.nr12);
    audio_write(&audio, 0xFF13, 0xA5);
    TEST_ASSERT_EQUAL_HEX8(0xA5, audio.nr13);
    audio_write(&audio, 0xFF16, 0xA5);
    TEST_ASSERT_EQUAL_HEX8(0xA5, audio.nr21);
    audio_write(&audio, 0xFF17, 0xA5);
    TEST_ASSERT_EQUAL_HEX8(0xA5, audio.nr22);
    audio_write(&audio, 0xFF18, 0xA5);
    TEST_ASSERT_EQUAL_HEX8(0xA5, audio.nr23);
    audio_write(&audio, 0xFF1B, 0xA5);
    TEST_ASSERT_EQUAL_HEX8(0xA5, audio.nr31);
    audio_write(&audio, 0xFF1D, 0xA5);
    TEST_ASSERT_EQUAL_HEX8(0xA5, audio.nr33);
    audio_write(&audio, 0xFF21, 0xA5);
    TEST_ASSERT_EQUAL_HEX8(0xA5, audio.nr42);
    audio_write(&audio, 0xFF22, 0xA5);
    TEST_ASSERT_EQUAL_HEX8(0xA5, audio.nr43);
    audio_write(&audio, 0xFF24, 0xA5);
    TEST_ASSERT_EQUAL_HEX8(0xA5, audio.nr50);
    audio_write(&audio, 0xFF25, 0xA5);
    TEST_ASSERT_EQUAL_HEX8(0xA5, audio.nr51);
}

void test_audio_write_applies_or_masks(void) {
    // Writing 0x00 leaves only the register's forced (always-set) bits.
    audio_write(&audio, 0xFF10, 0x00);
    TEST_ASSERT_EQUAL_HEX8(0x80, audio.nr10);
    audio_write(&audio, 0xFF14, 0x00);
    TEST_ASSERT_EQUAL_HEX8(0x38, audio.nr14);
    audio_write(&audio, 0xFF19, 0x00);
    TEST_ASSERT_EQUAL_HEX8(0x38, audio.nr24);
    audio_write(&audio, 0xFF1A, 0x00);
    TEST_ASSERT_EQUAL_HEX8(0x7F, audio.nr30);
    audio_write(&audio, 0xFF1C, 0x00);
    TEST_ASSERT_EQUAL_HEX8(0x9F, audio.nr32);
    audio_write(&audio, 0xFF1E, 0x00);
    TEST_ASSERT_EQUAL_HEX8(0x38, audio.nr34);
    audio_write(&audio, 0xFF20, 0x00);
    TEST_ASSERT_EQUAL_HEX8(0xC0, audio.nr41);
    audio_write(&audio, 0xFF23, 0x00);
    TEST_ASSERT_EQUAL_HEX8(0x3F, audio.nr44);
    audio_write(&audio, 0xFF26, 0x00);
    TEST_ASSERT_EQUAL_HEX8(0x7F, audio.nr52);
}

void test_audio_write_unhandled_is_ignored(void) {
    audio_regs_t zero = { 0 };

    audio_write(&audio, 0xFF15, 0xAA);

    TEST_ASSERT_EQUAL_INT(0, memcmp(&audio, &zero, sizeof(audio_regs_t)));
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_audio_init_sets_boot_values);

    RUN_TEST(test_audio_read_returns_raw_registers);
    RUN_TEST(test_audio_read_applies_read_masks);
    RUN_TEST(test_audio_read_write_only_returns_0xFF);
    RUN_TEST(test_audio_read_unhandled_returns_0xFF);

    RUN_TEST(test_audio_write_stores_raw_registers);
    RUN_TEST(test_audio_write_applies_or_masks);
    RUN_TEST(test_audio_write_unhandled_is_ignored);

    return UNITY_END();
}
