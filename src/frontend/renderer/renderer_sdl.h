#ifndef RENDERER_SDL_H
#define RENDERER_SDL_H

#include "frontend/renderer/renderer.h"

// Builds a renderer that draws the framebuffer into a real SDL window,
// scaled up from the native 160x144 resolution. Shares its window with
// input_sdl (src/input/input_sdl.c) via frontend/sdl_context.h.
renderer_t renderer_sdl(void);

#endif // RENDERER_SDL_H
