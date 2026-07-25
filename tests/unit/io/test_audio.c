#include "unity.h"
#include "log_helpers.h"

#include <string.h>

#include "io/audio.h"

static audio_regs_t audio;
static uint8_t wave_ram[16];

void setUp(void) {
    suppress_logs();

    audio = (audio_regs_t){ 0 };
    audio.nr52 = 0x80; // powered on by default; audio_write ignores nearly
                       // everything while off, see the power-off tests below
    memset(wave_ram, 0, sizeof(wave_ram));
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

    TEST_ASSERT_EQUAL_UINT8(0, audio.frame_seq_step);
    TEST_ASSERT_FALSE(audio.frame_seq_prev_bit);
    TEST_ASSERT_EQUAL_HEX16(0x7FFF, audio.ch4_lfsr);
}

// ---- audio_read ----

void test_audio_read_returns_raw_registers(void) {
    audio.nr10 = 0xA5; // 0xFF10 — bit 7 already set, so the read mask is a no-op here
    audio.nr12 = 0xA5; // 0xFF12
    audio.nr22 = 0xA5; // 0xFF17
    audio.nr42 = 0xA5; // 0xFF21
    audio.nr43 = 0xA5; // 0xFF22
    audio.nr50 = 0xA5; // 0xFF24
    audio.nr51 = 0xA5; // 0xFF25
    audio.nr52 = 0xA5; // 0xFF26

    TEST_ASSERT_EQUAL_HEX8(0xA5, audio_read(&audio, 0xFF10));
    TEST_ASSERT_EQUAL_HEX8(0xA5, audio_read(&audio, 0xFF12));
    TEST_ASSERT_EQUAL_HEX8(0xA5, audio_read(&audio, 0xFF17));
    TEST_ASSERT_EQUAL_HEX8(0xA5, audio_read(&audio, 0xFF21));
    TEST_ASSERT_EQUAL_HEX8(0xA5, audio_read(&audio, 0xFF22));
    TEST_ASSERT_EQUAL_HEX8(0xA5, audio_read(&audio, 0xFF24));
    TEST_ASSERT_EQUAL_HEX8(0xA5, audio_read(&audio, 0xFF25));
    TEST_ASSERT_EQUAL_HEX8(0xA5, audio_read(&audio, 0xFF26));
}

void test_audio_read_applies_read_masks(void) {
    // Each stored bit sits outside its register's mask, so the result is the
    // stored value OR'd with the always-set bits. These bits must always read
    // high — even right after NR52 clears the underlying byte to 0 on
    // power-off — so the mask is applied here rather than baked in at write
    // time.
    audio.nr10 = 0x00; // 0xFF10 — value | 0x80
    audio.nr11 = 0x40; // 0xFF11 — value | 0x3F
    audio.nr14 = 0x40; // 0xFF14 — value | 0xBF
    audio.nr21 = 0x40; // 0xFF16 — value | 0x3F
    audio.nr24 = 0x40; // 0xFF19 — value | 0xBF
    audio.nr30 = 0x00; // 0xFF1A — value | 0x7F
    audio.nr32 = 0x00; // 0xFF1C — value | 0x9F
    audio.nr34 = 0x40; // 0xFF1E — value | 0xBF
    audio.nr44 = 0x40; // 0xFF23 — value | 0xBF

    TEST_ASSERT_EQUAL_HEX8(0x80, audio_read(&audio, 0xFF10));
    TEST_ASSERT_EQUAL_HEX8(0x7F, audio_read(&audio, 0xFF11));
    TEST_ASSERT_EQUAL_HEX8(0xFF, audio_read(&audio, 0xFF14));
    TEST_ASSERT_EQUAL_HEX8(0x7F, audio_read(&audio, 0xFF16));
    TEST_ASSERT_EQUAL_HEX8(0xFF, audio_read(&audio, 0xFF19));
    TEST_ASSERT_EQUAL_HEX8(0x7F, audio_read(&audio, 0xFF1A));
    TEST_ASSERT_EQUAL_HEX8(0x9F, audio_read(&audio, 0xFF1C));
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
    audio_write(&audio, 0xFF11, 0xA5, wave_ram);
    TEST_ASSERT_EQUAL_HEX8(0xA5, audio.nr11);
    audio_write(&audio, 0xFF12, 0xA5, wave_ram);
    TEST_ASSERT_EQUAL_HEX8(0xA5, audio.nr12);
    audio_write(&audio, 0xFF13, 0xA5, wave_ram);
    TEST_ASSERT_EQUAL_HEX8(0xA5, audio.nr13);
    audio_write(&audio, 0xFF16, 0xA5, wave_ram);
    TEST_ASSERT_EQUAL_HEX8(0xA5, audio.nr21);
    audio_write(&audio, 0xFF17, 0xA5, wave_ram);
    TEST_ASSERT_EQUAL_HEX8(0xA5, audio.nr22);
    audio_write(&audio, 0xFF18, 0xA5, wave_ram);
    TEST_ASSERT_EQUAL_HEX8(0xA5, audio.nr23);
    audio_write(&audio, 0xFF1B, 0xA5, wave_ram);
    TEST_ASSERT_EQUAL_HEX8(0xA5, audio.nr31);
    audio_write(&audio, 0xFF1D, 0xA5, wave_ram);
    TEST_ASSERT_EQUAL_HEX8(0xA5, audio.nr33);
    audio_write(&audio, 0xFF21, 0xA5, wave_ram);
    TEST_ASSERT_EQUAL_HEX8(0xA5, audio.nr42);
    audio_write(&audio, 0xFF22, 0xA5, wave_ram);
    TEST_ASSERT_EQUAL_HEX8(0xA5, audio.nr43);
    audio_write(&audio, 0xFF24, 0xA5, wave_ram);
    TEST_ASSERT_EQUAL_HEX8(0xA5, audio.nr50);
    audio_write(&audio, 0xFF25, 0xA5, wave_ram);
    TEST_ASSERT_EQUAL_HEX8(0xA5, audio.nr51);
}

void test_audio_write_applies_or_masks(void) {
    // Writing 0x00 leaves only the register's forced (always-set) bits.
    // NR10/NR30/NR32 are not included here: their forced-high bits are
    // applied on read (see test_audio_read_applies_read_masks) rather than
    // baked into the stored byte, so they keep reading correctly even after
    // NR52 zeroes the raw register on power-off.
    audio_write(&audio, 0xFF14, 0x00, wave_ram);
    TEST_ASSERT_EQUAL_HEX8(0x38, audio.nr14);
    audio_write(&audio, 0xFF19, 0x00, wave_ram);
    TEST_ASSERT_EQUAL_HEX8(0x38, audio.nr24);
    audio_write(&audio, 0xFF1E, 0x00, wave_ram);
    TEST_ASSERT_EQUAL_HEX8(0x38, audio.nr34);
    audio_write(&audio, 0xFF20, 0x00, wave_ram);
    TEST_ASSERT_EQUAL_HEX8(0xC0, audio.nr41);
    audio_write(&audio, 0xFF23, 0x00, wave_ram);
    TEST_ASSERT_EQUAL_HEX8(0x3F, audio.nr44);
    audio_write(&audio, 0xFF26, 0x00, wave_ram);
    TEST_ASSERT_EQUAL_HEX8(0x70, audio.nr52); // bits 0-3 are now live status, not forced high
}

void test_audio_write_unhandled_is_ignored(void) {
    audio_regs_t before = audio;

    audio_write(&audio, 0xFF15, 0xAA, wave_ram);

    TEST_ASSERT_EQUAL_INT(0, memcmp(&audio, &before, sizeof(audio_regs_t)));
}

// ---- audio_write: powering off (NR52) ----

void test_audio_write_power_off_clears_other_registers(void) {
    audio.nr52 = 0x8F; // powered on, all channels active
    audio.nr10 = 0xFF;
    audio.nr11 = 0xFF;
    audio.nr12 = 0xFF;
    audio.nr50 = 0xFF;
    audio.nr51 = 0xFF;

    audio_write(&audio, 0xFF26, 0x00, wave_ram); // power off

    TEST_ASSERT_EQUAL_HEX8(0x00, audio.nr10);
    TEST_ASSERT_EQUAL_HEX8(0x00, audio.nr11);
    TEST_ASSERT_EQUAL_HEX8(0x00, audio.nr12);
    TEST_ASSERT_EQUAL_HEX8(0x00, audio.nr50);
    TEST_ASSERT_EQUAL_HEX8(0x00, audio.nr51);
}

void test_audio_write_power_off_preserves_length_counters(void) {
    audio.nr52 = 0x81;
    audio.ch1_length_counter = 42;

    audio_write(&audio, 0xFF26, 0x00, wave_ram);

    TEST_ASSERT_EQUAL_UINT16(42, audio.ch1_length_counter);
}

void test_audio_write_ignores_other_registers_while_powered_off(void) {
    audio.nr52 = 0x00; // powered off

    audio_write(&audio, 0xFF12, 0xF0, wave_ram);

    TEST_ASSERT_EQUAL_HEX8(0x00, audio.nr12);
}

void test_audio_write_ignores_trigger_while_powered_off(void) {
    audio.nr52 = 0x00; // powered off
    audio.nr12 = 0xF0; // would be DAC-on, if the write took effect

    audio_write(&audio, 0xFF14, 0x80, wave_ram); // trigger, but ignored while off

    TEST_ASSERT_EQUAL_UINT8(0x00, audio.nr52 & 0x01);
}

void test_audio_write_power_on_reallows_writes(void) {
    audio.nr52 = 0x00; // powered off

    audio_write(&audio, 0xFF26, 0x80, wave_ram); // power back on
    audio_write(&audio, 0xFF12, 0xF0, wave_ram);

    TEST_ASSERT_EQUAL_HEX8(0xF0, audio.nr12);
}

// DMG quirk: NRx1 length writes still reload the length counter while the
// APU is off, even though the register byte itself stays 0 (and reads back
// as 0, like every other register while off).

void test_audio_write_nr11_reloads_length_counter_while_powered_off(void) {
    audio.nr52 = 0x00; // powered off

    audio_write(&audio, 0xFF11, 0x05, wave_ram); // length field = 5

    TEST_ASSERT_EQUAL_UINT16(59, audio.ch1_length_counter); // 64 - 5
    TEST_ASSERT_EQUAL_HEX8(0x00, audio.nr11); // register itself stays 0
}

void test_audio_write_nr21_reloads_length_counter_while_powered_off(void) {
    audio.nr52 = 0x00;
    audio_write(&audio, 0xFF16, 0x05, wave_ram);
    TEST_ASSERT_EQUAL_UINT16(59, audio.ch2_length_counter);
    TEST_ASSERT_EQUAL_HEX8(0x00, audio.nr21);
}

void test_audio_write_nr31_reloads_length_counter_while_powered_off_8bit(void) {
    audio.nr52 = 0x00;
    audio_write(&audio, 0xFF1B, 0x05, wave_ram);
    TEST_ASSERT_EQUAL_UINT16(251, audio.ch3_length_counter); // 256 - 5
    TEST_ASSERT_EQUAL_HEX8(0x00, audio.nr31);
}

void test_audio_write_nr41_reloads_length_counter_while_powered_off(void) {
    audio.nr52 = 0x00;
    audio_write(&audio, 0xFF20, 0x05, wave_ram);
    TEST_ASSERT_EQUAL_UINT16(59, audio.ch4_length_counter);
    TEST_ASSERT_EQUAL_HEX8(0x00, audio.nr41);
}

// ---- audio_write: length counter reload ----

void test_audio_write_nr11_loads_ch1_length_counter(void) {
    audio_write(&audio, 0xFF11, 0x05, wave_ram); // length field = 5
    TEST_ASSERT_EQUAL_UINT16(59, audio.ch1_length_counter); // 64 - 5
}

void test_audio_write_nr21_loads_ch2_length_counter(void) {
    audio_write(&audio, 0xFF16, 0x05, wave_ram);
    TEST_ASSERT_EQUAL_UINT16(59, audio.ch2_length_counter);
}

void test_audio_write_nr31_loads_ch3_length_counter_8bit(void) {
    audio_write(&audio, 0xFF1B, 0x05, wave_ram);
    TEST_ASSERT_EQUAL_UINT16(251, audio.ch3_length_counter); // 256 - 5
}

void test_audio_write_nr41_loads_ch4_length_counter(void) {
    audio_write(&audio, 0xFF20, 0x05, wave_ram);
    TEST_ASSERT_EQUAL_UINT16(59, audio.ch4_length_counter);
}

// ---- audio_write: DAC-off clears the channel-active status immediately ----

void test_audio_write_nr12_dac_off_clears_ch1_active(void) {
    audio.nr52 = 0x81; // powered on, CH1 active
    audio_write(&audio, 0xFF12, 0x00, wave_ram); // volume=0, increase=0 -> DAC off
    TEST_ASSERT_EQUAL_UINT8(0x00, audio.nr52 & 0x01);
}

void test_audio_write_nr22_dac_off_clears_ch2_active(void) {
    audio.nr52 = 0x82;
    audio_write(&audio, 0xFF17, 0x00, wave_ram);
    TEST_ASSERT_EQUAL_UINT8(0x00, audio.nr52 & 0x02);
}

void test_audio_write_nr30_dac_off_clears_ch3_active(void) {
    audio.nr52 = 0x84;
    audio_write(&audio, 0xFF1A, 0x00, wave_ram); // bit 7 clear -> DAC off
    TEST_ASSERT_EQUAL_UINT8(0x00, audio.nr52 & 0x04);
}

void test_audio_write_nr42_dac_off_clears_ch4_active(void) {
    audio.nr52 = 0x88;
    audio_write(&audio, 0xFF21, 0x00, wave_ram);
    TEST_ASSERT_EQUAL_UINT8(0x00, audio.nr52 & 0x08);
}

// ---- triggers ----

void test_audio_trigger_ch1_marks_active_in_nr52(void) {
    audio.nr12 = 0xF0; // DAC on
    audio_write(&audio, 0xFF14, 0x80, wave_ram);
    TEST_ASSERT_EQUAL_UINT8(0x01, audio.nr52 & 0x01);
}

void test_audio_trigger_ch1_reloads_length_when_zero(void) {
    audio.nr12 = 0xF0;
    audio.ch1_length_counter = 0;
    audio_write(&audio, 0xFF14, 0x80, wave_ram);
    TEST_ASSERT_EQUAL_UINT16(64, audio.ch1_length_counter);
}

void test_audio_trigger_ch1_seeds_envelope_from_nr12(void) {
    audio.nr12 = 0xA3; // volume=10, increase, pace=3
    audio_write(&audio, 0xFF14, 0x80, wave_ram);
    TEST_ASSERT_EQUAL_UINT8(10, audio.ch1_env_volume);
    TEST_ASSERT_EQUAL_UINT8(3, audio.ch1_env_timer);
}

void test_audio_trigger_ch1_seeds_sweep_shadow_from_period(void) {
    audio.nr12 = 0xF0;
    audio.nr13 = 0x34; // period low
    // Trigger write goes to NR14 itself, so the period-high bits (2) must be
    // included in the same write — a separate prior write to nr14 would be
    // clobbered by NR14's write mask.
    audio_write(&audio, 0xFF14, 0x82, wave_ram);
    TEST_ASSERT_EQUAL_UINT16(0x0234, audio.ch1_sweep_shadow);
}

void test_audio_trigger_ch1_disables_on_immediate_sweep_overflow(void) {
    audio.nr12 = 0xF0;
    audio.nr10 = 0x01; // pace=0, shift=1
    audio.nr13 = 0xFF;
    // period = 0x7FF (2047), max — any add overflows; period-high bits (7)
    // must be included in the trigger write itself, see comment above.
    audio_write(&audio, 0xFF14, 0x87, wave_ram);
    TEST_ASSERT_EQUAL_UINT8(0x00, audio.nr52 & 0x01);
}

void test_audio_trigger_ch1_reloads_period_timer(void) {
    audio.nr12 = 0xF0;
    audio.nr13 = 0x00;
    audio.nr14 = 0x00; // period = 0 -> timer = (2048 - 0) * 4
    audio_write(&audio, 0xFF14, 0x80, wave_ram);
    TEST_ASSERT_EQUAL_INT32(2048 * 4, audio.ch1_period_timer);
}

void test_audio_trigger_ch1_dac_off_does_not_set_active(void) {
    audio.nr12 = 0x00; // DAC off
    audio_write(&audio, 0xFF14, 0x80, wave_ram);
    TEST_ASSERT_EQUAL_UINT8(0x00, audio.nr52 & 0x01);
}

// ---- length-enable quirk (NRx4 bit 6 going 0 -> 1), verified against
// gb-test-roms dmg_sound 03-trigger ----

void test_audio_write_nr14_enable_during_odd_step_clocks_length_immediately(void) {
    audio.frame_seq_step = 1;
    audio.ch1_length_counter = 5;
    audio_write(&audio, 0xFF14, 0x40, wave_ram); // enable only, no trigger
    TEST_ASSERT_EQUAL_UINT16(4, audio.ch1_length_counter);
}

void test_audio_write_nr14_enable_during_even_step_does_not_clock(void) {
    audio.frame_seq_step = 0;
    audio.ch1_length_counter = 5;
    audio_write(&audio, 0xFF14, 0x40, wave_ram);
    TEST_ASSERT_EQUAL_UINT16(5, audio.ch1_length_counter);
}

void test_audio_write_nr14_enable_quirk_disables_channel_on_reaching_zero(void) {
    audio.frame_seq_step = 1;
    audio.ch1_length_counter = 1;
    audio.nr52 = 0x81; // powered on, CH1 active
    audio_write(&audio, 0xFF14, 0x40, wave_ram); // enable only, no trigger
    TEST_ASSERT_EQUAL_UINT16(0, audio.ch1_length_counter);
    TEST_ASSERT_EQUAL_UINT8(0x00, audio.nr52 & 0x01);
}

void test_audio_write_nr14_enable_quirk_does_not_disable_when_also_triggered(void) {
    audio.nr12 = 0xF0; // DAC on
    audio.frame_seq_step = 1;
    audio.ch1_length_counter = 1;
    audio.nr52 = 0x81;
    audio_write(&audio, 0xFF14, 0xC0, wave_ram); // enable + trigger together
    // Enable quirk clocks 1 -> 0 (channel not disabled since triggering);
    // trigger then reloads to max and re-clocks it once more (see
    // test_audio_trigger_ch1_unfreezes_and_reclocks_length below).
    TEST_ASSERT_EQUAL_UINT16(63, audio.ch1_length_counter);
    TEST_ASSERT_EQUAL_UINT8(0x01, audio.nr52 & 0x01);
}

void test_audio_write_nr14_repeated_enable_write_does_not_reclock(void) {
    audio.frame_seq_step = 1;
    audio.nr14 = 0x40; // already enabled
    audio.ch1_length_counter = 5;
    audio_write(&audio, 0xFF14, 0x40, wave_ram); // still enabled, no 0 -> 1 transition
    TEST_ASSERT_EQUAL_UINT16(5, audio.ch1_length_counter);
}

void test_audio_trigger_ch1_unfreezes_and_reclocks_length(void) {
    // Length enable already set (no 0 -> 1 transition on this write) and the
    // counter previously ran down to 0; triggering while enabled during an
    // odd frame-sequencer step reloads to max and immediately re-clocks it.
    audio.nr12 = 0xF0;
    audio.nr14 = 0x40;
    audio.frame_seq_step = 1;
    audio.ch1_length_counter = 0;
    audio_write(&audio, 0xFF14, 0xC0, wave_ram);
    TEST_ASSERT_EQUAL_UINT16(63, audio.ch1_length_counter);
}

void test_audio_trigger_ch2_marks_active_in_nr52(void) {
    audio.nr22 = 0xF0;
    audio_write(&audio, 0xFF19, 0x80, wave_ram);
    TEST_ASSERT_EQUAL_UINT8(0x02, audio.nr52 & 0x02);
}

void test_audio_trigger_ch3_marks_active_and_resets_wave_position(void) {
    audio.nr30 = 0x80; // DAC on
    audio.ch3_wave_pos = 17;
    audio_write(&audio, 0xFF1E, 0x80, wave_ram);
    TEST_ASSERT_EQUAL_UINT8(0x04, audio.nr52 & 0x04);
    TEST_ASSERT_EQUAL_UINT8(0, audio.ch3_wave_pos);
}

void test_audio_trigger_ch3_reloads_length_when_zero(void) {
    audio.nr30 = 0x80;
    audio.ch3_length_counter = 0;
    audio_write(&audio, 0xFF1E, 0x80, wave_ram);
    TEST_ASSERT_EQUAL_UINT16(256, audio.ch3_length_counter);
}

void test_audio_trigger_ch4_marks_active_and_resets_lfsr(void) {
    audio.nr42 = 0xF0;
    audio.ch4_lfsr = 0x0001;
    audio_write(&audio, 0xFF23, 0x80, wave_ram);
    TEST_ASSERT_EQUAL_UINT8(0x08, audio.nr52 & 0x08);
    TEST_ASSERT_EQUAL_HEX16(0x7FFF, audio.ch4_lfsr);
}

// ---- audio_tick: length clocking ----

void test_audio_tick_length_disables_channel_when_it_reaches_zero(void) {
    audio.nr14 = 0x40;            // length enabled
    audio.ch1_length_counter = 1;
    audio.nr52 = 0x01;
    audio.frame_seq_prev_bit = true; // primed for a falling edge

    audio_tick(&audio, 4, 0x00, wave_ram); // bit 4 falls -> step 0 -> clocks length

    TEST_ASSERT_EQUAL_UINT16(0, audio.ch1_length_counter);
    TEST_ASSERT_EQUAL_UINT8(0x00, audio.nr52 & 0x01);
}

void test_audio_tick_length_ignored_when_disabled(void) {
    audio.nr14 = 0x00;            // length disabled
    audio.ch1_length_counter = 1;
    audio.nr52 = 0x01;
    audio.frame_seq_prev_bit = true;

    audio_tick(&audio, 4, 0x00, wave_ram);

    TEST_ASSERT_EQUAL_UINT16(1, audio.ch1_length_counter);
    TEST_ASSERT_EQUAL_UINT8(0x01, audio.nr52 & 0x01);
}

// ---- audio_tick: envelope clocking (step 7) ----

void test_audio_tick_envelope_increases_volume_at_step_7(void) {
    audio.frame_seq_step = 7; // this falling edge runs step 7 (envelope)
    audio.frame_seq_prev_bit = true;
    audio.nr12 = 0x08 | 0x01; // increase, pace=1
    audio.ch1_env_volume = 5;
    audio.ch1_env_timer = 1;

    audio_tick(&audio, 4, 0x00, wave_ram);

    TEST_ASSERT_EQUAL_UINT8(6, audio.ch1_env_volume);
}

void test_audio_tick_envelope_stops_at_max_volume(void) {
    audio.frame_seq_step = 7;
    audio.frame_seq_prev_bit = true;
    audio.nr12 = 0x08 | 0x01; // increase, pace=1
    audio.ch1_env_volume = 15;
    audio.ch1_env_timer = 1;

    audio_tick(&audio, 4, 0x00, wave_ram);

    TEST_ASSERT_EQUAL_UINT8(15, audio.ch1_env_volume);
}

// ---- audio_tick: sweep clocking (steps 2, 6) ----

void test_audio_tick_sweep_updates_period_registers(void) {
    audio.frame_seq_step = 2; // this falling edge runs step 2 (sweep)
    audio.frame_seq_prev_bit = true;
    audio.nr10 = 0x11;              // pace=1, subtract=0, shift=1
    audio.ch1_sweep_shadow = 0x100; // period 256
    audio.ch1_sweep_timer = 1;
    audio.ch1_sweep_enabled = true;
    audio.nr52 = 0x01;

    audio_tick(&audio, 4, 0x00, wave_ram);

    // new_period = 256 + (256 >> 1) = 384 = 0x180
    TEST_ASSERT_EQUAL_UINT16(0x0180, audio.ch1_sweep_shadow);
    TEST_ASSERT_EQUAL_UINT8(0x80, audio.nr13);
    TEST_ASSERT_EQUAL_UINT8(0x01, audio.nr14 & 0x07);
}

void test_audio_tick_sweep_disables_channel_on_overflow(void) {
    audio.frame_seq_step = 2;
    audio.frame_seq_prev_bit = true;
    audio.nr10 = 0x11;              // pace=1, subtract=0, shift=1
    audio.ch1_sweep_shadow = 0x7FF; // 2047, any addition overflows
    audio.ch1_sweep_timer = 1;
    audio.ch1_sweep_enabled = true;
    audio.nr52 = 0x01;

    audio_tick(&audio, 4, 0x00, wave_ram);

    TEST_ASSERT_EQUAL_UINT8(0x00, audio.nr52 & 0x01);
}

void test_audio_tick_sweep_second_overflow_check_disables_without_rewriting(void) {
    // Pan Docs / gb-test-roms dmg_sound 04-sweep test 5: after writing back a
    // non-overflowing calculation, hardware immediately re-runs the overflow
    // check against the just-written period. If *that* overflows, the
    // channel is disabled anyway, but its (overflowing) result is discarded.
    audio.frame_seq_step = 2;
    audio.frame_seq_prev_bit = true;
    audio.nr10 = 0x11;              // pace=1, subtract=0, shift=1
    audio.ch1_sweep_shadow = 0x500; // first calc: 0x500 + 0x280 = 0x780 (no overflow)
    audio.ch1_sweep_timer = 1;
    audio.ch1_sweep_enabled = true;
    audio.nr52 = 0x01;

    audio_tick(&audio, 4, 0x00, wave_ram);

    // Second calc: 0x780 + 0x3C0 = 0xB40, overflows -> disables the channel,
    // but the shadow/period registers keep the first calc's result.
    TEST_ASSERT_EQUAL_UINT8(0x00, audio.nr52 & 0x01);
    TEST_ASSERT_EQUAL_UINT16(0x0780, audio.ch1_sweep_shadow);
    TEST_ASSERT_EQUAL_UINT8(0x80, audio.nr13);
    TEST_ASSERT_EQUAL_UINT8(0x07, audio.nr14 & 0x07);
}

// ---- NR10 negate-mode exit quirk ----
// gb-test-roms dmg_sound 05-sweep details, tests 4-6: clearing NR10's negate
// bit after a sweep calculation actually ran in negate mode disables the
// channel, even outside of a sweep clock; a fresh trigger resets the tracking.

void test_audio_write_nr10_exiting_negate_after_calculation_disables_channel(void) {
    audio.nr10 = 0x08; // negate mode, shift=0
    audio.ch1_sweep_negate_used = true;
    audio.nr52 = 0x81; // powered on, CH1 active

    audio_write(&audio, 0xFF10, 0x00, wave_ram); // negate -> positive

    TEST_ASSERT_EQUAL_UINT8(0x00, audio.nr52 & 0x01);
}

void test_audio_write_nr10_exiting_negate_without_calculation_does_not_disable(void) {
    audio.nr10 = 0x08; // negate mode, but no calculation has run since trigger
    audio.ch1_sweep_negate_used = false;
    audio.nr52 = 0x81;

    audio_write(&audio, 0xFF10, 0x00, wave_ram);

    TEST_ASSERT_EQUAL_UINT8(0x01, audio.nr52 & 0x01);
}

void test_audio_write_nr10_staying_in_negate_mode_does_not_disable(void) {
    audio.nr10 = 0x09; // negate mode, shift=1
    audio.ch1_sweep_negate_used = true;
    audio.nr52 = 0x81;

    audio_write(&audio, 0xFF10, 0x0A, wave_ram); // still negate, shift changes 1 -> 2

    TEST_ASSERT_EQUAL_UINT8(0x01, audio.nr52 & 0x01);
}

void test_audio_trigger_ch1_resets_negate_used_flag(void) {
    audio.nr12 = 0xF0; // DAC on
    audio.nr10 = 0x08;  // negate mode, shift=0, so trigger doesn't recalculate
    audio.ch1_sweep_negate_used = true;

    audio_write(&audio, 0xFF14, 0x80, wave_ram); // trigger

    TEST_ASSERT_FALSE(audio.ch1_sweep_negate_used);
}

void test_audio_sweep_calculate_marks_negate_used_without_overflow(void) {
    audio.nr10 = 0x19; // pace=1, negate=1, shift=1
    audio.ch1_sweep_shadow = 0x100;
    audio.ch1_sweep_timer = 1;
    audio.ch1_sweep_enabled = true;
    audio.ch1_sweep_negate_used = false;
    audio.nr52 = 0x01;
    audio.frame_seq_step = 2;
    audio.frame_seq_prev_bit = true;

    audio_tick(&audio, 4, 0x00, wave_ram);

    TEST_ASSERT_TRUE(audio.ch1_sweep_negate_used);
    TEST_ASSERT_EQUAL_UINT8(0x01, audio.nr52 & 0x01); // no overflow, stays active
}

// ---- audio_tick: sample clock ----

void test_audio_tick_produces_sample_after_enough_cycles(void) {
    audio.sample_cycle_acc = 0;
    audio_tick(&audio, 96, 0x00, wave_ram); // 96 cycles crosses one 44100 Hz period
    TEST_ASSERT_TRUE(audio.sample_ready);
}

void test_audio_tick_no_sample_before_enough_cycles(void) {
    audio.sample_cycle_acc = 0;
    audio_tick(&audio, 10, 0x00, wave_ram);
    TEST_ASSERT_FALSE(audio.sample_ready);
}

// ---- audio_tick: waveform generation ----

void test_audio_tick_ch1_advances_duty_position(void) {
    audio.nr12 = 0xF0;
    audio.nr13 = 0x00;
    audio.nr14 = 0x00; // period 0 -> period timer reload = 2048*4
    audio.ch1_period_timer = 4; // about to reload

    audio_tick(&audio, 4, 0x00, wave_ram);

    TEST_ASSERT_EQUAL_UINT8(1, audio.ch1_duty_pos);
}

void test_audio_tick_ch1_silent_when_dac_off(void) {
    audio.nr12 = 0x00; // DAC off
    audio.nr52 = 0x01;
    audio.nr11 = 0xC0; // duty=50%, first step is high
    audio.ch1_env_volume = 15;
    audio.sample_cycle_acc = 0;

    audio_tick(&audio, 96, 0x00, wave_ram);

    TEST_ASSERT_EQUAL_INT16(0, audio.sample_left);
    TEST_ASSERT_EQUAL_INT16(0, audio.sample_right);
}

void test_audio_tick_ch1_produces_nonzero_sample_when_active(void) {
    audio.nr52 = 0x81;  // master on, CH1 active
    audio.nr12 = 0xF0;  // DAC on, max volume
    audio.nr11 = 0x80;  // duty=50% (index 2 -> first table entry high)
    audio.nr51 = 0x11;  // CH1 -> both left and right
    audio.nr50 = 0x77;  // max volume both sides
    audio.ch1_env_volume = 15;
    audio.ch1_duty_pos = 2; // duty_table[2][2] == 1 -> loud
    audio.sample_cycle_acc = 0;

    audio_tick(&audio, 96, 0x00, wave_ram);

    TEST_ASSERT_TRUE(audio.sample_ready);
    TEST_ASSERT_NOT_EQUAL_INT16(0, audio.sample_left);
    TEST_ASSERT_NOT_EQUAL_INT16(0, audio.sample_right);
}

void test_audio_tick_ch3_reads_wave_ram_nibble(void) {
    audio.nr52 = 0x84;  // master on, CH3 active
    audio.nr30 = 0x80;  // DAC on
    audio.nr32 = 0x20;  // output level = 100%
    audio.nr51 = 0x44;  // CH3 -> both left and right
    audio.nr50 = 0x77;
    wave_ram[0] = 0xF0; // high nibble = 0xF (max), low nibble = 0
    audio.ch3_wave_pos = 0;
    audio.sample_cycle_acc = 0;

    audio_tick(&audio, 96, 0x00, wave_ram);

    TEST_ASSERT_TRUE(audio.sample_ready);
    TEST_ASSERT_NOT_EQUAL_INT16(0, audio.sample_left);
}

void test_audio_tick_ch3_output_level_mute(void) {
    audio.nr52 = 0x84;
    audio.nr30 = 0x80;
    audio.nr32 = 0x00; // output level = mute
    audio.nr51 = 0x44;
    audio.nr50 = 0x77;
    wave_ram[0] = 0xFF;
    audio.sample_cycle_acc = 0;

    audio_tick(&audio, 96, 0x00, wave_ram);

    TEST_ASSERT_EQUAL_INT16(0, audio.sample_left);
    TEST_ASSERT_EQUAL_INT16(0, audio.sample_right);
}

void test_audio_tick_ch4_produces_nonzero_sample_when_active(void) {
    audio.nr52 = 0x88;  // master on, CH4 active
    audio.nr42 = 0xF0;  // DAC on, max volume
    audio.nr51 = 0x88;  // CH4 -> both left and right
    audio.nr50 = 0x77;
    audio.ch4_env_volume = 15;
    audio.ch4_lfsr = 0x7FFE; // bit 0 clear -> high output
    audio.sample_cycle_acc = 0;

    audio_tick(&audio, 96, 0x00, wave_ram);

    TEST_ASSERT_TRUE(audio.sample_ready);
    TEST_ASSERT_NOT_EQUAL_INT16(0, audio.sample_left);
}

void test_audio_tick_master_off_forces_silence(void) {
    audio.nr52 = 0x01; // master off (bit 7 clear), but CH1 status bit still set
    audio.nr12 = 0xF0;
    audio.nr11 = 0xC0;
    audio.ch1_env_volume = 15;
    audio.ch1_duty_pos = 2;
    audio.sample_cycle_acc = 0;

    audio_tick(&audio, 96, 0x00, wave_ram);

    TEST_ASSERT_EQUAL_INT16(0, audio.sample_left);
    TEST_ASSERT_EQUAL_INT16(0, audio.sample_right);
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

    RUN_TEST(test_audio_write_power_off_clears_other_registers);
    RUN_TEST(test_audio_write_power_off_preserves_length_counters);
    RUN_TEST(test_audio_write_ignores_other_registers_while_powered_off);
    RUN_TEST(test_audio_write_ignores_trigger_while_powered_off);
    RUN_TEST(test_audio_write_power_on_reallows_writes);

    RUN_TEST(test_audio_write_nr11_reloads_length_counter_while_powered_off);
    RUN_TEST(test_audio_write_nr21_reloads_length_counter_while_powered_off);
    RUN_TEST(test_audio_write_nr31_reloads_length_counter_while_powered_off_8bit);
    RUN_TEST(test_audio_write_nr41_reloads_length_counter_while_powered_off);

    RUN_TEST(test_audio_write_nr11_loads_ch1_length_counter);
    RUN_TEST(test_audio_write_nr21_loads_ch2_length_counter);
    RUN_TEST(test_audio_write_nr31_loads_ch3_length_counter_8bit);
    RUN_TEST(test_audio_write_nr41_loads_ch4_length_counter);

    RUN_TEST(test_audio_write_nr12_dac_off_clears_ch1_active);
    RUN_TEST(test_audio_write_nr22_dac_off_clears_ch2_active);
    RUN_TEST(test_audio_write_nr30_dac_off_clears_ch3_active);
    RUN_TEST(test_audio_write_nr42_dac_off_clears_ch4_active);

    RUN_TEST(test_audio_trigger_ch1_marks_active_in_nr52);
    RUN_TEST(test_audio_trigger_ch1_reloads_length_when_zero);
    RUN_TEST(test_audio_trigger_ch1_seeds_envelope_from_nr12);
    RUN_TEST(test_audio_trigger_ch1_seeds_sweep_shadow_from_period);
    RUN_TEST(test_audio_trigger_ch1_disables_on_immediate_sweep_overflow);
    RUN_TEST(test_audio_trigger_ch1_reloads_period_timer);
    RUN_TEST(test_audio_trigger_ch1_dac_off_does_not_set_active);

    RUN_TEST(test_audio_write_nr14_enable_during_odd_step_clocks_length_immediately);
    RUN_TEST(test_audio_write_nr14_enable_during_even_step_does_not_clock);
    RUN_TEST(test_audio_write_nr14_enable_quirk_disables_channel_on_reaching_zero);
    RUN_TEST(test_audio_write_nr14_enable_quirk_does_not_disable_when_also_triggered);
    RUN_TEST(test_audio_write_nr14_repeated_enable_write_does_not_reclock);
    RUN_TEST(test_audio_trigger_ch1_unfreezes_and_reclocks_length);

    RUN_TEST(test_audio_trigger_ch2_marks_active_in_nr52);
    RUN_TEST(test_audio_trigger_ch3_marks_active_and_resets_wave_position);
    RUN_TEST(test_audio_trigger_ch3_reloads_length_when_zero);
    RUN_TEST(test_audio_trigger_ch4_marks_active_and_resets_lfsr);

    RUN_TEST(test_audio_tick_length_disables_channel_when_it_reaches_zero);
    RUN_TEST(test_audio_tick_length_ignored_when_disabled);

    RUN_TEST(test_audio_tick_envelope_increases_volume_at_step_7);
    RUN_TEST(test_audio_tick_envelope_stops_at_max_volume);

    RUN_TEST(test_audio_tick_sweep_updates_period_registers);
    RUN_TEST(test_audio_tick_sweep_disables_channel_on_overflow);
    RUN_TEST(test_audio_tick_sweep_second_overflow_check_disables_without_rewriting);

    RUN_TEST(test_audio_write_nr10_exiting_negate_after_calculation_disables_channel);
    RUN_TEST(test_audio_write_nr10_exiting_negate_without_calculation_does_not_disable);
    RUN_TEST(test_audio_write_nr10_staying_in_negate_mode_does_not_disable);
    RUN_TEST(test_audio_trigger_ch1_resets_negate_used_flag);
    RUN_TEST(test_audio_sweep_calculate_marks_negate_used_without_overflow);

    RUN_TEST(test_audio_tick_produces_sample_after_enough_cycles);
    RUN_TEST(test_audio_tick_no_sample_before_enough_cycles);

    RUN_TEST(test_audio_tick_ch1_advances_duty_position);
    RUN_TEST(test_audio_tick_ch1_silent_when_dac_off);
    RUN_TEST(test_audio_tick_ch1_produces_nonzero_sample_when_active);
    RUN_TEST(test_audio_tick_ch3_reads_wave_ram_nibble);
    RUN_TEST(test_audio_tick_ch3_output_level_mute);
    RUN_TEST(test_audio_tick_ch4_produces_nonzero_sample_when_active);
    RUN_TEST(test_audio_tick_master_off_forces_silence);

    return UNITY_END();
}
