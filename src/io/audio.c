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
    switch (addr) {
        case 0xFF10:
            return audio->nr10;
        case 0xFF11:
            return audio->nr11 | 0b00111111;
        case 0xFF12:
            return audio->nr12;
        case 0xFF13:
            return 0xFF; // audio->nr13 | 0b11111111; // read-only, always returns 0xFF
        case 0xFF14:
            return audio->nr14 | 0b10111111;
        case 0xFF16:
            return audio->nr21 | 0b00111111;
        case 0xFF17:
            return audio->nr22;
        case 0xFF18:
            return 0xFF; // audio->nr23 | 0b11111111; // read-only, always returns 0xFF
        case 0xFF19:
            return audio->nr24 | 0b10111111;
        case 0xFF1A:
            return audio->nr30;
        case 0xFF1B:
            return 0xFF; // audio->nr31 | 0b11111111; // read-only, always returns 0xFF
        case 0xFF1C:
            return audio->nr32;
        case 0xFF1D:
            return 0xFF; // audio->nr33 | 0b11111111; // read-only, always returns 0xFF
        case 0xFF1E:
            return audio->nr34 | 0b10111111;
        case 0xFF20:
            return 0xFF; // audio->nr41 | 0b11111111; // read-only, always returns 0xFF
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
            break;
        case 0xFF12:
            audio->nr12 = value;
            break;
        case 0xFF13:
            audio->nr13 = value;
            break;
        case 0xFF14:
            audio->nr14 = value | 0b00111000;
            break;
        case 0xFF16:
            audio->nr21 = value;
            break;
        case 0xFF17:
            audio->nr22 = value;
            break;
        case 0xFF18:
            audio->nr23 = value;
            break;
        case 0xFF19:
            audio->nr24 = value | 0b00111000;
            break;
        case 0xFF1A:
            audio->nr30 = value | 0b01111111;
            break;
        case 0xFF1B:
            audio->nr31 = value;
            break;
        case 0xFF1C:
            audio->nr32 = value | 0b10011111;
            break;
        case 0xFF1D:
            audio->nr33 = value;
            break;
        case 0xFF1E:
            audio->nr34 = value | 0b00111000;
            break;
        case 0xFF20:
            audio->nr41 = value | 0b11000000;
            break;
        case 0xFF21:
            audio->nr42 = value;
            break;
        case 0xFF22:
            audio->nr43 = value;
            break;
        case 0xFF23:
            audio->nr44 = value | 0b00111111;
            break;
        case 0xFF24:
            audio->nr50 = value;
            break;
        case 0xFF25:
            audio->nr51 = value;
            break;
        case 0xFF26:
            audio->nr52 = value | 0b01111111; // Bit 7 is read-only, always returns the status of the sound controller (1 if enabled, otherwise 0)
            break;
        default:
            LOG_WARN("Unhandled audio register 0x%04X", addr);
            break;
    }
}
