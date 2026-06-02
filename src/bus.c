#include "bus.h"

#include <assert.h>

#include "logger.h"

static uint8_t read_rom(bus_t *bus, uint16_t addr);
static void write_rom(bus_t *bus, uint16_t addr, uint8_t value);
static uint8_t read_vram(bus_t *bus, uint16_t addr);
static void write_vram(bus_t *bus, uint16_t addr, uint8_t value);
static uint8_t read_ext_ram(bus_t *bus, uint16_t addr);
static void write_ext_ram(bus_t *bus, uint16_t addr, uint8_t value);
static uint8_t read_wram(bus_t *bus, uint16_t addr);
static void write_wram(bus_t *bus, uint16_t addr, uint8_t value);
static uint8_t read_echo_ram(bus_t *bus, uint16_t addr);
static uint8_t read_oam(bus_t *bus, uint16_t addr);
static uint8_t read_not_usable(bus_t *bus, uint16_t addr);
static uint8_t read_io_reg(bus_t *bus, uint16_t addr);
static void write_io_reg(bus_t *bus, uint16_t addr, uint8_t value);
static uint8_t read_hram(bus_t *bus, uint16_t addr);
static void write_hram(bus_t *bus, uint16_t addr, uint8_t value);

static const mem_region_t memory_map[] = {
    {"ROM", 0x0000, 0x7FFF, read_rom, write_rom},                   // 16 KiB ROM bank 00 (fixed) + 16 KiB ROM bank 01~NN (switchable, if supported)
    {"VRAM", 0x8000, 0x9FFF, read_vram, write_vram},                         // 8 KiB Video RAM (VRAM)
    {"External RAM", 0xA000, 0xBFFF, read_ext_ram, write_ext_ram},           // 8 KiB External RAM (if supported)
    {"WRAM", 0xC000, 0xDFFF, read_wram, write_wram},                         // 8 KiB Work RAM (WRAM)
    {"Echo RAM", 0xE000, 0xFDFF, read_echo_ram, NULL},                       // Echo RAM (mirrors 0xC000~0xDDFF)
    {"OAM", 0xFE00, 0xFE9F, read_oam, NULL},                                 // Object Attribute Memory (OAM)
    {"Not Usable", 0xFEA0, 0xFEFF, read_not_usable, NULL},                   // Not usable
    {"I/O Registers", 0xFF00, 0xFF7F, read_io_reg, write_io_reg},            // I/O Registers
    {"HRAM", 0xFF80, 0xFFFE, read_hram, write_hram},                         // High RAM (HRAM) - 127 bytes
    {"Interrupt Enable Register", 0xFFFF, 0xFFFF, read_io_reg, write_io_reg} // Interrupt Enable Register (IE)
};

int bus_init(bus_t *bus, cartridge_t *cartridge) {
    LOG_INFO("Initializing bus");

    bus->cartridge = cartridge;

    io_reg_init(&bus->io_reg);

    if (mem_init(&bus->wram, WRAM_SIZE, "WRAM") != 0) {
        LOG_ERROR("Could not initialize WRAM");
        return -1;
    }
    if (mem_init(&bus->vram, VRAM_SIZE, "VRAM") != 0) {
        LOG_ERROR("Could not initialize VRAM");
        mem_free(&bus->wram);
        return -1;
    }
    if (mem_init(&bus->hram, HRAM_SIZE, "HRAM") != 0) {
        LOG_ERROR("Could not initialize HRAM");
        mem_free(&bus->wram);
        mem_free(&bus->vram);
        return -1;
    }

    // TODO: Later add other components like RAM, PPU, APU, etc.

    return 0;
}

void bus_free(bus_t *bus) {
    LOG_INFO("Freeing bus resources");

    mem_free(&bus->wram);
    mem_free(&bus->vram);
    mem_free(&bus->hram);
}

uint8_t bus_read(bus_t *bus, uint16_t addr) {
    LOG_DEBUG("Reading from address: 0x%04X", addr);

    for (size_t i = 0; i < sizeof(memory_map) / sizeof(mem_region_t); i++) {
        mem_region_t region = memory_map[i];
        if (addr >= region.start && addr <= region.end) {
            return region.read_fn(bus, addr);
        }
    }

    LOG_WARN("bus_read: address 0x%04X not mapped - this should never happen", addr);
    assert(0 && "Address not mapped in bus_read");
}

void bus_write(bus_t *bus, uint16_t addr, uint8_t value) {
    LOG_DEBUG("Writing %d to address: 0x%04X", value, addr);

    for (size_t i = 0; i < sizeof(memory_map) / sizeof(mem_region_t); i++) {
        mem_region_t region = memory_map[i];
        if (addr >= region.start && addr <= region.end) {
            region.write_fn(bus, addr, value);
            return;
        }
    }

    LOG_WARN("bus_write: address 0x%04X not mapped - this should never happen", addr);
    assert(0 && "Address not mapped in bus_write");
}

static uint8_t read_rom(bus_t *bus, uint16_t addr) {
    uint8_t value = cartridge_rom_read(bus->cartridge, addr);
    LOG_DEBUG("ROM read: [0x%04X] = 0x%02X", addr, value);

    return value;
}

static void write_rom(bus_t *bus, uint16_t addr, uint8_t value) {
    cartridge_rom_write(bus->cartridge, addr, value);
    LOG_DEBUG("ROM write: [0x%04X] = 0x%02X", addr, value);
}

static uint8_t read_vram(bus_t *bus, uint16_t addr) {
    uint16_t vram_address = addr - 0x8000;
    uint8_t value = bus->vram.mem[vram_address];
    LOG_DEBUG("VRAM read: [0x%04X] = 0x%02X", vram_address, value);

    return value;
}

static void write_vram(bus_t *bus, uint16_t addr, uint8_t value) {
    uint16_t vram_address = addr - 0x8000;
    bus->vram.mem[vram_address] = value;
    LOG_DEBUG("VRAM write: [0x%04X] = 0x%02X", vram_address, value);
}

static uint8_t read_ext_ram(bus_t *bus, uint16_t addr) {
    uint16_t ext_ram_address = addr - 0xA000;
    uint8_t value = cartridge_ext_ram_read(bus->cartridge, ext_ram_address);
    LOG_DEBUG("External RAM read: [0x%04X] = 0x%02X", ext_ram_address, value);
    // TODO: Implement MBC support to read from external RAM (?)

    return value;
}

static void write_ext_ram(bus_t *bus, uint16_t addr, uint8_t value) {
    uint16_t ext_ram_address = addr - 0xA000;
    cartridge_ext_ram_write(bus->cartridge, ext_ram_address, value);
    LOG_DEBUG("External RAM write: [0x%04X] = 0x%02X", ext_ram_address, value);
}

static uint8_t read_wram(bus_t *bus, uint16_t addr) {
    uint16_t wram_address = addr - 0xC000;
    uint8_t value = bus->wram.mem[wram_address];
    LOG_DEBUG("WRAM read: [0x%04X] = 0x%02X", wram_address, value);

    return value;
}

static void write_wram(bus_t *bus, uint16_t addr, uint8_t value) {
    uint16_t wram_address = addr - 0xC000;
    bus->wram.mem[wram_address] = value;
    LOG_DEBUG("WRAM write: [0x%04X] = 0x%02X", wram_address, value);
}

static uint8_t read_echo_ram(bus_t *bus, uint16_t addr) {
    (void)bus;
    (void)addr;
    // TODO: Implement echo RAM support (?) Understand how it works and whether it's necessary to implement
    return 0xFF; // Temporary return 0xFF
}

static uint8_t read_oam(bus_t *bus, uint16_t addr) {
    (void)bus;
    (void)addr;
    // TODO: Implement OAM support to read from sprite attribute memory
    return 0xFF; // Temporary return 0xFF
}

static uint8_t read_not_usable(bus_t *bus, uint16_t addr) {
    (void)bus;
    (void)addr;
    LOG_WARN("Attempted to read from not usable address: 0x%04X", addr);
    // TODO: Implement proper handling for not usable addresses (e.g., return 0xFF, log a warning, etc.)
    // something about OAM corruption, to understand how it works and whether it's necessary to implement
    return 0xFF; // Return 0xFF for not usable addresses
}

static uint8_t read_io_reg(bus_t *bus, uint16_t addr) {
    return io_reg_read(&bus->io_reg, addr);
}

static void write_io_reg(bus_t *bus, uint16_t addr, uint8_t value) {
    io_reg_write(&bus->io_reg, addr, value);
}

static uint8_t read_hram(bus_t *bus, uint16_t addr) {
    uint16_t hram_address = addr - 0xFF80;
    uint8_t value = bus->hram.mem[hram_address];
    LOG_DEBUG("HRAM read: [0x%04X] = 0x%02X", hram_address, value);

    return value;
}

static void write_hram(bus_t *bus, uint16_t addr, uint8_t value) {
    uint16_t hram_address = addr - 0xFF80;
    bus->hram.mem[hram_address] = value;
    LOG_DEBUG("HRAM write: [0x%04X] = 0x%02X", hram_address, value);
}
