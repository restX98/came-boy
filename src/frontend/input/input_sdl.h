#ifndef INPUT_SDL_H
#define INPUT_SDL_H

#include "frontend/input/input.h"

// Builds an input backend that reads real keyboard events through SDL,
// sharing its window with renderer_sdl (src/renderer/renderer_sdl.c) via
// frontend/sdl_context.h. poll() returns true (quit requested) on window
// close or Escape.
input_t input_sdl(void);

#endif // INPUT_SDL_H
