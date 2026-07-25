#ifndef AUDIO_H
#define AUDIO_H

#include <stdbool.h>
#include <stdint.h>

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

    // Frame sequencer: a 512 Hz clock derived from falling edges of DIV
    // register bit 4. Steps length counters (256 Hz), sweep (128 Hz), and
    // envelope (64 Hz) across all channels.
    uint8_t frame_seq_step;   // 0-7, wraps around
    bool frame_seq_prev_bit;  // last-sampled DIV bit 4, for edge detection

    // ---- Internal runtime state (not memory-mapped) ----
    // Channel 1 (pulse + sweep)
    uint16_t ch1_length_counter;
    uint8_t  ch1_env_volume;
    uint8_t  ch1_env_timer;
    uint16_t ch1_sweep_shadow;
    uint8_t  ch1_sweep_timer;
    bool     ch1_sweep_enabled;
    int32_t  ch1_period_timer; // T-cycles remaining until the next duty step
    uint8_t  ch1_duty_pos;     // 0-7, position within the duty waveform

    // Channel 2 (pulse)
    uint16_t ch2_length_counter;
    uint8_t  ch2_env_volume;
    uint8_t  ch2_env_timer;
    int32_t  ch2_period_timer;
    uint8_t  ch2_duty_pos;

    // Channel 3 (wave) — length counter counts up to 256, not 64
    uint16_t ch3_length_counter;
    int32_t  ch3_period_timer;
    uint8_t  ch3_wave_pos; // 0-31, nibble index into wave RAM

    // Channel 4 (noise)
    uint16_t ch4_length_counter;
    uint8_t  ch4_env_volume;
    uint8_t  ch4_env_timer;
    int32_t  ch4_period_timer;
    uint16_t ch4_lfsr; // 15-bit linear feedback shift register

    // ---- Sample clock ----
    // Downsamples the CPU clock (4194304 Hz) to the output sample rate via a
    // drift-free fractional accumulator, staging one stereo sample at a time
    // — mirrors ppu_t's framebuffer/frame_ready handshake consumed in main.c.
    int32_t sample_cycle_acc;
    bool sample_ready;
    int16_t sample_left;
    int16_t sample_right;
} audio_regs_t;

void audio_init(audio_regs_t *audio);

uint8_t audio_read(audio_regs_t *audio, uint16_t addr);
void audio_write(audio_regs_t *audio, uint16_t addr, uint8_t value);

// Advances the frame sequencer, per-channel waveform generators, and the
// sample clock. `cycles` is the elapsed T-cycles for this call; `div_register`
// is the current value of DIV (0xFF04); `wave_ram` is the 16-byte Wave
// Pattern RAM ($FF30-$FF3F), owned by io_reg_t, read here for Channel 3.
void audio_tick(audio_regs_t *audio, int cycles, uint8_t div_register, const uint8_t *wave_ram);

#endif // AUDIO_H
