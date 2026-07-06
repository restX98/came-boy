#include "io_reg.h"

#include <stddef.h>

#include "logger.h"

static uint8_t read_joypad(io_reg_t *io_reg, uint16_t addr);
static void write_joypad(io_reg_t *io_reg, uint16_t addr, uint8_t value);
static uint8_t read_serial(io_reg_t *io_reg, uint16_t addr);
static void write_serial(io_reg_t *io_reg, uint16_t addr, uint8_t value);
static uint8_t read_timer(io_reg_t *io_reg, uint16_t addr);
static void write_timer(io_reg_t *io_reg, uint16_t addr, uint8_t value);
static uint8_t read_interrupts(io_reg_t *io_reg, uint16_t addr);
static void write_interrupts(io_reg_t *io_reg, uint16_t addr, uint8_t value);
static uint8_t read_audio(io_reg_t *io_reg, uint16_t addr);
static void write_audio(io_reg_t *io_reg, uint16_t addr, uint8_t value);
static uint8_t read_wave_pattern(io_reg_t *io_reg, uint16_t addr);
static void write_wave_pattern(io_reg_t *io_reg, uint16_t addr, uint8_t value);
static uint8_t read_lcd(io_reg_t *io_reg, uint16_t addr);
static void write_lcd(io_reg_t *io_reg, uint16_t addr, uint8_t value);
static uint8_t read_oam_dma(io_reg_t *io_reg, uint16_t addr);
static void write_oam_dma(io_reg_t *io_reg, uint16_t addr, uint8_t value);
static uint8_t read_boot_rom_disable(io_reg_t *io_reg, uint16_t addr);
static void write_boot_rom_disable(io_reg_t *io_reg, uint16_t addr, uint8_t value);

typedef struct {
    const char *name;
    uint16_t start;
    uint16_t end;
    uint8_t(*read_fn)(io_reg_t *io_reg, uint16_t addr);
    void (*write_fn)(io_reg_t *io_reg, uint16_t addr, uint8_t value);
} io_reg_region_t;

static const io_reg_region_t register_map[] = {
    {"Joypad", 0xFF00, 0xFF00, read_joypad, write_joypad},                                       // 0xFF00 — Joypad
    {"Serial Transfer", 0xFF01, 0xFF02, read_serial, write_serial},                              // 0xFF01–0xFF02 — Serial transfer
    {"Timer", 0xFF04, 0xFF07, read_timer, write_timer},                                          // 0xFF04–0xFF07 — Timer and divider
    {"Interrupt Flag", 0xFF0F, 0xFF0F, read_interrupts, write_interrupts},                       // 0xFF0F — Interrupt flag (IF)
    {"Audio", 0xFF10, 0xFF26, read_audio, write_audio},                                          // 0xFF10–0xFF26 — Audio
    {"Wave Pattern", 0xFF30, 0xFF3F, read_wave_pattern, write_wave_pattern},                     // 0xFF30–0xFF3F — Wave pattern
    {"LCD", 0xFF40, 0xFF4B, read_lcd, write_lcd},                                                // 0xFF40–0xFF4B — LCD
    {"OAM DMA transfer", 0xFF46, 0xFF46, read_oam_dma, write_oam_dma},                           // 0xFF46 — OAM DMA transfer
    {"Boot ROM mapping control", 0xFF50, 0xFF50, read_boot_rom_disable, write_boot_rom_disable}, // 0xFF50 — Boot ROM mapping control
    {"Interrupt Enable", 0xFFFF, 0xFFFF, read_interrupts, write_interrupts}                      // 0xFFFF — Interrupt enable (IE)
};

void io_reg_init(io_reg_t *io_reg) {
    joypad_init(&io_reg->joyp);
    serial_transfer_init(&io_reg->serial_transfer);
    interrupts_init(&io_reg->interrupts);
    timer_init(&io_reg->timer);
    audio_init(&io_reg->audio);
    lcd_init(&io_reg->lcd);
}

uint8_t io_reg_read(io_reg_t *io_reg, uint16_t addr) {
    for (size_t i = 0; i < sizeof(register_map) / sizeof(io_reg_region_t); i++) {
        io_reg_region_t region = register_map[i];
        if (addr >= region.start && addr <= region.end) {
            return region.read_fn(io_reg, addr);
        }
    }

    LOG_WARN("io_reg_read: unimplemented register 0x%04X", addr);
    return 0xFF;
}

void io_reg_write(io_reg_t *io_reg, uint16_t addr, uint8_t value) {
    for (size_t i = 0; i < sizeof(register_map) / sizeof(io_reg_region_t); i++) {
        io_reg_region_t region = register_map[i];
        if (addr >= region.start && addr <= region.end) {
            region.write_fn(io_reg, addr, value);
            return;
        }
    }

    LOG_WARN("io_reg_write: unimplemented register 0x%04X", addr);
}

static uint8_t read_joypad(io_reg_t *io_reg, uint16_t addr) {
    (void)addr;
    return joypad_read(&io_reg->joyp);
}

static void write_joypad(io_reg_t *io_reg, uint16_t addr, uint8_t value) {
    (void)addr;
    joypad_write(&io_reg->joyp, value);
}

static uint8_t read_serial(io_reg_t *io_reg, uint16_t addr) {
    return serial_transfer_read(&io_reg->serial_transfer, addr);
}

static void write_serial(io_reg_t *io_reg, uint16_t addr, uint8_t value) {
    serial_transfer_write(&io_reg->serial_transfer, addr, value);
}

static uint8_t read_timer(io_reg_t *io_reg, uint16_t addr) {
    return timer_read(&io_reg->timer, addr);
}

static void write_timer(io_reg_t *io_reg, uint16_t addr, uint8_t value) {
    timer_write(&io_reg->timer, addr, value);
}

static uint8_t read_interrupts(io_reg_t *io_reg, uint16_t addr) {
    return interrupts_read(&io_reg->interrupts, addr);
}

static void write_interrupts(io_reg_t *io_reg, uint16_t addr, uint8_t value) {
    interrupts_write(&io_reg->interrupts, addr, value);
}

static uint8_t read_audio(io_reg_t *io_reg, uint16_t addr) {
    return audio_read(&io_reg->audio, addr);
}

static void write_audio(io_reg_t *io_reg, uint16_t addr, uint8_t value) {
    audio_write(&io_reg->audio, addr, value);
}

static uint8_t read_wave_pattern(io_reg_t *io_reg, uint16_t addr) {
    (void)io_reg;
    LOG_WARN("io_reg_read: unimplemented wave pattern register 0x%04X", addr);
    return 0xFF;
}
static void write_wave_pattern(io_reg_t *io_reg, uint16_t addr, uint8_t value) {
    (void)io_reg;
    (void)value;
    LOG_WARN("io_reg_write: unimplemented wave pattern register 0x%04X", addr);
}

static uint8_t read_lcd(io_reg_t *io_reg, uint16_t addr) {
    return lcd_read(&io_reg->lcd, addr);
}

static void write_lcd(io_reg_t *io_reg, uint16_t addr, uint8_t value) {
    lcd_write(&io_reg->lcd, addr, value);
}

static uint8_t read_oam_dma(io_reg_t *io_reg, uint16_t addr) {
    (void)io_reg;
    LOG_WARN("io_reg_read: unimplemented OAM DMA register 0x%04X", addr);
    return 0xFF;
}

static void write_oam_dma(io_reg_t *io_reg, uint16_t addr, uint8_t value) {
    (void)io_reg;
    (void)value;
    LOG_WARN("io_reg_write: unimplemented OAM DMA register 0x%04X", addr);
}

static uint8_t read_boot_rom_disable(io_reg_t *io_reg, uint16_t addr) {
    (void)io_reg;
    LOG_WARN("io_reg_read: unimplemented boot ROM disable register 0x%04X", addr);
    return 0xFF;
}

static void write_boot_rom_disable(io_reg_t *io_reg, uint16_t addr, uint8_t value) {
    (void)io_reg;
    (void)value;
    LOG_WARN("io_reg_write: unimplemented boot ROM disable register 0x%04X", addr);
}
