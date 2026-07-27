# came-boy

A Game Boy (DMG) emulator written in C, built as a hands-on way to explore low-level programming and emulator development.

## Features

- Full LR35902 CPU (SM83) instruction set, including CB-prefixed opcodes
- PPU (background, window, sprites) with a frame-based renderer
- APU with all four audio channels (pulse x2, wave, noise)
- Timer, serial transfer, joypad, interrupts, OAM DMA
- Cartridge/MBC support: ROM-only, MBC1, MBC2, MBC3 (with RTC), MBC5
- Two frontends: SDL2 (video/audio/input) and an ASCII/terminal renderer for headless or tty use

## How it works

`gameboy_t` ([src/gameboy.h](src/gameboy.h)) is the top-level machine: it owns the cartridge, bus, CPU, PPU, and clock by value and wires them together in `gameboy_init`. Each call to `gameboy_step` executes one CPU instruction and advances the rest of the machine (PPU, timer, APU, DMA) by the resulting t-cycle count. `main.c` drives this loop, pumping input, queuing audio samples, and rendering a frame whenever the PPU signals `frame_ready`.

Source layout:

```
src/
  cpu.c / isa/           CPU core and opcode tables (opcodes/*.inl split by category)
  bus.c                  Memory bus / address dispatch
  memory/                Cartridge loading, MBC1/2/3/5, RAM
  io/                    LCD/PPU registers, timer, audio, serial, joypad, interrupts, OAM DMA
  ppu.c                  Pixel pipeline / frame rendering
  clock.c                Drives PPU/timer/APU off CPU cycles
  frontend/              Renderer, audio backend, and input abstractions
    renderer/            SDL and ASCII renderer backends
    audio/                SDL audio backend
    input/                SDL and tty input backends
  main.c                 Entry point / emulation loop
```

Frontend backends (renderer, audio, input) are small vtable-style structs (`init`/`poll`/`render`/`deinit` function pointers + a `ctx`), so swapping SDL for ASCII/tty is just picking a different constructor in `main.c`.

## Requirements

- GCC with C23 support (`-std=gnu2x`)
- SDL2 (`pkg-config --cflags/--libs sdl2` must resolve), e.g. `apt install libsdl2-dev`
- `lcov`/`genhtml`, only needed for coverage reports (`apt install lcov`)

## Building

```sh
make            # builds build/came-boy
make clean      # removes build/, tests/bin/, and coverage reports
```

## Running

```sh
./build/came-boy <rom.gb>                    # SDL frontend (default)
./build/came-boy <rom.gb> --tty              # ASCII renderer on stdout/stdin
./build/came-boy <rom.gb> --tty=/dev/ttysX   # ASCII renderer pinned to a specific tty
```

ROMs aren't included in the repo (`roms/` is gitignored) — point the emulator at your own `.gb` file.

### Controls (SDL and tty frontends)

| Game Boy | Key |
|---|---|
| D-Pad     | W / A / S / D |
| A         | P |
| B         | L |
| Select    | V |
| Start     | B |
| Quit      | Escape |

### Known limitations

- No save file (`.sav`) persistence — battery-backed cartridge RAM (and MBC3 RTC state) lives in memory only and is lost on exit.
- No GBC (Game Boy Color) support — DMG only.

## Testing

Tests use the bundled [Unity](tests/unity) framework and mirror `src/`'s layout 1:1 (`tests/unit/io/test_lcd.c` covers `src/io/lcd.c`).

```sh
make test              # unit + integration tests
make test_unit         # unit tests only
make test_integration  # integration tests only

make unit_lcd          # build & run a single unit test by stem (tests/unit/io/test_lcd.c)
make integ_cpu_full    # build & run a single integration test the same way
```

Integration tests (`tests/integration/`) link the full emulator and exercise cross-module behavior (e.g. `test_cpu_full`, `test_bus_mem`, `test_oam_dma`).

### Coverage

```sh
make coverage_unit   # unit tests only -> tests/coverage_report/unit/index.html
make coverage        # unit + integration -> tests/coverage_report/global/index.html
```

Both targets build instrumented binaries under `build/coverage`/`tests/bin/coverage`, run them, and generate an lcov/genhtml HTML report.

## TODOs

- [ ] Persist battery-backed cartridge RAM (and MBC3 RTC state) to a save file across runs
- [ ] GBC (Game Boy Color) support — currently DMG-only
- [ ] Fix failing mooneye acceptance timing tests: `add_sp_e_timing`, `call_cc_timing`, `call_cc_timing2`, `call_timing`, `call_timing2`, `halt_ime0_nointr_timing`, `jp_cc_timing`, `jp_timing`, `ld_hl_sp_e_timing`, `oam_dma_restart`, `oam_dma_start`, `oam_dma_timing`, `pop_timing`, `push_timing`, `ret_cc_timing`, `ret_timing`, `reti_timing`, `rst_timing`, and `serial/boot_sclk_align-dmgABCmgb`
- [ ] MBC7 (rumble/tilt sensor) support — currently unimplemented
- [ ] Save states (snapshot/restore emulator state)
