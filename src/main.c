#include <stdio.h>
#include "logger.h"
#include "cartridge.h"

int main(void) {

    cartridge_t cartridge = { 0 };

    if (cartridge_load(&cartridge, "./roms/Tetris.gb") != 0) {
        LOG_ERROR("Could not load cartridge");
        return -1;
    }

    // free resources
    cartridge_unload(&cartridge);

    return 0;
}
