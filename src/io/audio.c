#include "audio.h"

#include "logger.h"

#define CPU_CLOCK_HZ 4194304
#define AUDIO_SAMPLE_RATE_HZ 44100

static void audio_trigger_ch1(audio_regs_t *audio);
static void audio_trigger_ch2(audio_regs_t *audio);
static void audio_trigger_ch3(audio_regs_t *audio);
static void audio_trigger_ch4(audio_regs_t *audio);

static void audio_clock_length(audio_regs_t *audio);
static void audio_clock_sweep(audio_regs_t *audio);
static void audio_clock_envelope(audio_regs_t *audio);
static void audio_frame_sequencer_step(audio_regs_t *audio);
static uint16_t audio_sweep_calculate(audio_regs_t *audio, uint8_t shift, bool *overflow);

void audio_init(audio_regs_t *audio) {
    audio->nr10 = 0x80;
    audio->nr11 = 0xBF;
    audio->nr12 = 0xF3;
    audio->nr13 = 0xFF;
    audio->nr14 = 0xBF;
    audio->nr21 = 0x3F;
    audio->nr22 = 0x00;
    audio->nr23 = 0xFF;
    audio->nr24 = 0xBF;
    audio->nr30 = 0x7F;
    audio->nr31 = 0xFF;
    audio->nr32 = 0x9F;
    audio->nr33 = 0xFF;
    audio->nr34 = 0xBF;
    audio->nr41 = 0xFF;
    audio->nr42 = 0x00;
    audio->nr43 = 0x00;
    audio->nr44 = 0xBF;
    audio->nr50 = 0x77;
    audio->nr51 = 0xF3;
    audio->nr52 = 0xF1;

    audio->frame_seq_step = 0;
    audio->frame_seq_prev_bit = false;

    audio->ch1_length_counter = 0;
    audio->ch1_env_volume = 0;
    audio->ch1_env_timer = 0;
    audio->ch1_sweep_shadow = 0;
    audio->ch1_sweep_timer = 0;
    audio->ch1_sweep_enabled = false;

    audio->ch2_length_counter = 0;
    audio->ch2_env_volume = 0;
    audio->ch2_env_timer = 0;

    audio->ch3_length_counter = 0;

    audio->ch4_length_counter = 0;
    audio->ch4_env_volume = 0;
    audio->ch4_env_timer = 0;

    audio->sample_cycle_acc = 0;
    audio->sample_ready = false;
    audio->sample_left = 0;
    audio->sample_right = 0;
}

uint8_t audio_read(audio_regs_t *audio, uint16_t addr) {
    switch (addr) {
        case 0xFF10:
            return audio->nr10;
        case 0xFF11:
            return audio->nr11 | 0b00111111;
        case 0xFF12:
            return audio->nr12;
        case 0xFF13:
            return 0xFF; // read-only, always returns 0xFF
        case 0xFF14:
            return audio->nr14 | 0b10111111;
        case 0xFF16:
            return audio->nr21 | 0b00111111;
        case 0xFF17:
            return audio->nr22;
        case 0xFF18:
            return 0xFF; // read-only, always returns 0xFF
        case 0xFF19:
            return audio->nr24 | 0b10111111;
        case 0xFF1A:
            return audio->nr30;
        case 0xFF1B:
            return 0xFF; // read-only, always returns 0xFF
        case 0xFF1C:
            return audio->nr32;
        case 0xFF1D:
            return 0xFF; // read-only, always returns 0xFF
        case 0xFF1E:
            return audio->nr34 | 0b10111111;
        case 0xFF20:
            return 0xFF; // read-only, always returns 0xFF
        case 0xFF21:
            return audio->nr42;
        case 0xFF22:
            return audio->nr43;
        case 0xFF23:
            return audio->nr44 | 0b10111111;
        case 0xFF24:
            return audio->nr50;
        case 0xFF25:
            return audio->nr51;
        case 0xFF26:
            return audio->nr52;
        default:
            LOG_WARN("Unhandled audio register 0x%04X", addr);
            return 0xFF;
    }
}

void audio_write(audio_regs_t *audio, uint16_t addr, uint8_t value) {
    switch (addr) {
        case 0xFF10:
            audio->nr10 = value | 0b10000000;
            break;
        case 0xFF11:
            audio->nr11 = value;
            audio->ch1_length_counter = 64 - (value & 0x3F);
            break;
        case 0xFF12:
            audio->nr12 = value;
            break;
        case 0xFF13:
            audio->nr13 = value;
            break;
        case 0xFF14:
            audio->nr14 = value | 0b00111000;
            if (value & 0x80) {
                audio_trigger_ch1(audio);
            }
            break;
        case 0xFF16:
            audio->nr21 = value;
            audio->ch2_length_counter = 64 - (value & 0x3F);
            break;
        case 0xFF17:
            audio->nr22 = value;
            break;
        case 0xFF18:
            audio->nr23 = value;
            break;
        case 0xFF19:
            audio->nr24 = value | 0b00111000;
            if (value & 0x80) {
                audio_trigger_ch2(audio);
            }
            break;
        case 0xFF1A:
            audio->nr30 = value | 0b01111111;
            break;
        case 0xFF1B:
            audio->nr31 = value;
            audio->ch3_length_counter = 256 - value;
            break;
        case 0xFF1C:
            audio->nr32 = value | 0b10011111;
            break;
        case 0xFF1D:
            audio->nr33 = value;
            break;
        case 0xFF1E:
            audio->nr34 = value | 0b00111000;
            if (value & 0x80) {
                audio_trigger_ch3(audio);
            }
            break;
        case 0xFF20:
            audio->nr41 = value | 0b11000000;
            audio->ch4_length_counter = 64 - (value & 0x3F);
            break;
        case 0xFF21:
            audio->nr42 = value;
            break;
        case 0xFF22:
            audio->nr43 = value;
            break;
        case 0xFF23:
            audio->nr44 = value | 0b00111111;
            if (value & 0x80) {
                audio_trigger_ch4(audio);
            }
            break;
        case 0xFF24:
            audio->nr50 = value;
            break;
        case 0xFF25:
            audio->nr51 = value;
            break;
        case 0xFF26: {
            // Only bit 7 (master enable) is writable. Bits 0-3 are read-only
            // status flags reflecting live channel state; bits 4-6 always
            // read as 1. Powering off silences all channels immediately.
            bool audio_on = value & 0x80;
            uint8_t status = audio_on ? (audio->nr52 & 0x0F) : 0x00;
            audio->nr52 = status | 0b01110000 | (audio_on ? 0x80 : 0x00);
            // TODO: powering off should also clear/lock most other audio
            // registers until re-enabled; not modeled yet.
            break;
        }
        default:
            LOG_WARN("Unhandled audio register 0x%04X", addr);
            break;
    }
}

void audio_tick(audio_regs_t *audio, int cycles, uint8_t div_register) {
    bool bit = (div_register >> 4) & 1;

    if (audio->frame_seq_prev_bit && !bit) {
        audio_frame_sequencer_step(audio);
    }

    audio->frame_seq_prev_bit = bit;

    // Drift-free downsampling from the CPU clock to the output sample rate
    // (Bresenham-style fractional accumulator).
    audio->sample_cycle_acc += cycles * AUDIO_SAMPLE_RATE_HZ;
    while (audio->sample_cycle_acc >= CPU_CLOCK_HZ) {
        audio->sample_cycle_acc -= CPU_CLOCK_HZ;

        // TODO: mix real channel output (CH1-4 through NR50/NR51/NR52) once
        // channel signal generation exists. Silence for now — this proves
        // the sample-rate plumbing end-to-end, nothing more.
        audio->sample_ready = true;
        audio->sample_left = 0;
        audio->sample_right = 0;
    }
}

static void audio_frame_sequencer_step(audio_regs_t *audio) {
    switch (audio->frame_seq_step) {
        case 0:
        case 4:
            audio_clock_length(audio);
            break;
        case 2:
        case 6:
            audio_clock_length(audio);
            audio_clock_sweep(audio);
            break;
        case 7:
            audio_clock_envelope(audio);
            break;
        default: // 1, 3, 5: nothing happens on these steps
            break;
    }

    audio->frame_seq_step = (audio->frame_seq_step + 1) % 8;
}

// ---- Triggers (NRx4 bit 7) ----

static void audio_trigger_ch1(audio_regs_t *audio) {
    audio->nr52 |= 0x01;

    if (audio->ch1_length_counter == 0) {
        audio->ch1_length_counter = 64;
    }

    audio->ch1_env_volume = (audio->nr12 >> 4) & 0x0F;
    audio->ch1_env_timer = audio->nr12 & 0x07;

    uint16_t period = audio->nr13 | ((uint16_t)(audio->nr14 & 0x07) << 8);
    audio->ch1_sweep_shadow = period;

    uint8_t sweep_pace = (audio->nr10 >> 4) & 0x07;
    uint8_t sweep_shift = audio->nr10 & 0x07;
    audio->ch1_sweep_timer = sweep_pace ? sweep_pace : 8;
    audio->ch1_sweep_enabled = (sweep_pace != 0) || (sweep_shift != 0);

    // Pan Docs: an immediate overflow check runs on trigger if shift != 0,
    // which can disable the channel before the first sweep step ever fires.
    if (sweep_shift != 0) {
        bool overflow;
        audio_sweep_calculate(audio, sweep_shift, &overflow);
        if (overflow) {
            audio->nr52 &= ~0x01;
        }
    }

    // TODO: if NR12's volume+direction bits are all zero, the DAC is off and
    // trigger should not actually re-enable the channel. Not modeled yet
    // since channel output doesn't exist.
}

static void audio_trigger_ch2(audio_regs_t *audio) {
    audio->nr52 |= 0x02;

    if (audio->ch2_length_counter == 0) {
        audio->ch2_length_counter = 64;
    }

    audio->ch2_env_volume = (audio->nr22 >> 4) & 0x0F;
    audio->ch2_env_timer = audio->nr22 & 0x07;
}

static void audio_trigger_ch3(audio_regs_t *audio) {
    audio->nr52 |= 0x04;

    if (audio->ch3_length_counter == 0) {
        audio->ch3_length_counter = 256;
    }

    // TODO: reset wave RAM read position to 0 once CH3 sample playback exists.
}

static void audio_trigger_ch4(audio_regs_t *audio) {
    audio->nr52 |= 0x08;

    if (audio->ch4_length_counter == 0) {
        audio->ch4_length_counter = 64;
    }

    audio->ch4_env_volume = (audio->nr42 >> 4) & 0x0F;
    audio->ch4_env_timer = audio->nr42 & 0x07;

    // TODO: reset LFSR to all-1s once CH4 noise generation exists.
}

// ---- Length counters (steps 0, 2, 4, 6) ----

static void audio_clock_one_length(audio_regs_t *audio, uint16_t *counter, uint8_t nrx4, uint8_t channel_bit) {
    bool length_enabled = (nrx4 >> 6) & 1;
    if (!length_enabled || *counter == 0) {
        return;
    }

    (*counter)--;
    if (*counter == 0) {
        audio->nr52 &= ~channel_bit;
    }
}

static void audio_clock_length(audio_regs_t *audio) {
    audio_clock_one_length(audio, &audio->ch1_length_counter, audio->nr14, 0x01);
    audio_clock_one_length(audio, &audio->ch2_length_counter, audio->nr24, 0x02);
    audio_clock_one_length(audio, &audio->ch3_length_counter, audio->nr34, 0x04);
    audio_clock_one_length(audio, &audio->ch4_length_counter, audio->nr44, 0x08);
}

// ---- Sweep, CH1 only (steps 2, 6) ----

static uint16_t audio_sweep_calculate(audio_regs_t *audio, uint8_t shift, bool *overflow) {
    uint16_t delta = audio->ch1_sweep_shadow >> shift;
    bool subtract = (audio->nr10 >> 3) & 1;
    uint16_t new_period = subtract
        ? (uint16_t)(audio->ch1_sweep_shadow - delta)
        : (uint16_t)(audio->ch1_sweep_shadow + delta);

    *overflow = new_period > 2047;
    return new_period;
}

static void audio_clock_sweep(audio_regs_t *audio) {
    if (audio->ch1_sweep_timer > 0) {
        audio->ch1_sweep_timer--;
    }

    if (audio->ch1_sweep_timer != 0) {
        return;
    }

    uint8_t pace = (audio->nr10 >> 4) & 0x07;
    audio->ch1_sweep_timer = pace ? pace : 8;

    if (!audio->ch1_sweep_enabled || pace == 0) {
        return;
    }

    uint8_t shift = audio->nr10 & 0x07;
    bool overflow;
    uint16_t new_period = audio_sweep_calculate(audio, shift, &overflow);

    if (overflow) {
        audio->nr52 &= ~0x01;
        return;
    }

    if (shift != 0) {
        audio->ch1_sweep_shadow = new_period;
        audio->nr13 = new_period & 0xFF;
        audio->nr14 = (audio->nr14 & 0xF8) | ((new_period >> 8) & 0x07);

        // Pan Docs: hardware runs a second overflow check here (on the
        // just-written period) without writing back its result. Skipped for
        // now; revisit if a sweep-heavy test ROM misbehaves.
    }
}

// ---- Envelope, CH1/CH2/CH4 (step 7) ----

static void audio_clock_one_envelope(uint8_t *volume, uint8_t *timer, uint8_t nrx2) {
    uint8_t pace = nrx2 & 0x07;
    if (pace == 0) {
        return;
    }

    if (*timer > 0) {
        (*timer)--;
    }

    if (*timer != 0) {
        return;
    }

    *timer = pace;

    bool increase = (nrx2 >> 3) & 1;
    if (increase && *volume < 15) {
        (*volume)++;
    } else if (!increase && *volume > 0) {
        (*volume)--;
    }
}

static void audio_clock_envelope(audio_regs_t *audio) {
    audio_clock_one_envelope(&audio->ch1_env_volume, &audio->ch1_env_timer, audio->nr12);
    audio_clock_one_envelope(&audio->ch2_env_volume, &audio->ch2_env_timer, audio->nr22);
    audio_clock_one_envelope(&audio->ch4_env_volume, &audio->ch4_env_timer, audio->nr42);
}
