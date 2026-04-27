#include "bus.h"
#include "logger.h"

void bus_init(bus_t *bus, cartridge_t *cartridge) {
    LOG_INFO("Initializing bus");

    bus->cartridge = cartridge;

    // TODO: Later add other components like RAM, PPU, APU, etc.
}

uint8_t bus_read(bus_t *bus, uint16_t addr) {
    if (addr <= 0x3FFF) {
        return bus->cartridge->rom[addr];
    }

    if (addr <= 0x7FFF) {
        // TODO: Implement MBC support to read from switchable ROM banks
        return bus->cartridge->rom[addr];
    }

    LOG_WARN("Attempted to read from unsupported address: 0x%04X", addr);
    return 0xFF; // Return 0xFF for unsupported addresses
}
