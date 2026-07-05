#ifndef AUDIO_H
#define AUDIO_H

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
} audio_regs_t;

void audio_init(audio_regs_t *audio);

uint8_t audio_read(audio_regs_t *audio, uint16_t addr);
void audio_write(audio_regs_t *audio, uint16_t addr, uint8_t value);

#endif // AUDIO_H
