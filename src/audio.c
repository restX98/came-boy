#include "audio.h"

#include "logger.h"

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
}

uint8_t audio_read(audio_regs_t *audio, uint16_t addr) {
    if (addr == 0xFF10) {
        return audio->nr10;
    } else if (addr == 0xFF11) {
        return audio->nr11 | 0b00111111;
    } else if (addr == 0xFF12) {
        return audio->nr12;
    } else if (addr == 0xFF13) {
        return 0xFF; // audio->nr13 | 0b11111111; // read-only, always returns 0xFF
    } else if (addr == 0xFF14) {
        return audio->nr14 | 0b10111111;
    } else if (addr == 0xFF16) {
        return audio->nr21 | 0b00111111;
    } else if (addr == 0xFF17) {
        return audio->nr22;
    } else if (addr == 0xFF18) {
        return 0xFF; // audio->nr23 | 0b11111111; // read-only, always returns 0xFF
    } else if (addr == 0xFF19) {
        return audio->nr24 | 0b10111111;
    } else if (addr == 0xFF1A) {
        return audio->nr30;
    } else if (addr == 0xFF1B) {
        return 0xFF; // audio->nr31 | 0b11111111; // read-only, always returns 0xFF
    } else if (addr == 0xFF1C) {
        return audio->nr32;
    } else if (addr == 0xFF1D) {
        return 0xFF; // audio->nr33 | 0b11111111; // read-only, always returns 0xFF
    } else if (addr == 0xFF1E) {
        return audio->nr34 | 0b10111111;
    } else if (addr == 0xFF20) {
        return 0xFF; // audio->nr41 | 0b11111111; // read-only, always returns 0xFF
    } else if (addr == 0xFF21) {
        return audio->nr42;
    } else if (addr == 0xFF22) {
        return audio->nr43;
    } else if (addr == 0xFF23) {
        return audio->nr44 | 0b10111111;
    } else if (addr == 0xFF24) {
        return audio->nr50;
    } else if (addr == 0xFF25) {
        return audio->nr51;
    } else if (addr == 0xFF26) {
        return audio->nr52;
    } else {
        LOG_WARN("Unhandled audio register 0x%04X", addr);
        return 0xFF;
    }
}

void audio_write(audio_regs_t *audio, uint16_t addr, uint8_t value) {
    if (addr == 0xFF10) {
        audio->nr10 = value | 0b10000000;
    } else if (addr == 0xFF11) {
        audio->nr11 = value;
    } else if (addr == 0xFF12) {
        audio->nr12 = value;
    } else if (addr == 0xFF13) {
        audio->nr13 = value;
    } else if (addr == 0xFF14) {
        audio->nr14 = value | 0b00111000;
    } else if (addr == 0xFF16) {
        audio->nr21 = value;
    } else if (addr == 0xFF17) {
        audio->nr22 = value;
    } else if (addr == 0xFF18) {
        audio->nr23 = value;
    } else if (addr == 0xFF19) {
        audio->nr24 = value | 0b00111000;
    } else if (addr == 0xFF1A) {
        audio->nr30 = value | 0b01111111;
    } else if (addr == 0xFF1B) {
        audio->nr31 = value;
    } else if (addr == 0xFF1C) {
        audio->nr32 = value | 0b10011111;
    } else if (addr == 0xFF1D) {
        audio->nr33 = value;
    } else if (addr == 0xFF1E) {
        audio->nr34 = value | 0b00111000;
    } else if (addr == 0xFF20) {
        audio->nr41 = value | 0b11000000;
    } else if (addr == 0xFF21) {
        audio->nr42 = value;
    } else if (addr == 0xFF22) {
        audio->nr43 = value;
    } else if (addr == 0xFF23) {
        audio->nr44 = value | 0b00111111;
    } else if (addr == 0xFF24) {
        audio->nr50 = value;
    } else if (addr == 0xFF25) {
        audio->nr51 = value;
    } else if (addr == 0xFF26) {
        audio->nr52 = value | 0b01111111;
    } else {
        LOG_WARN("Unhandled audio register 0x%04X", addr);
    }
}
