#ifndef RENDERER_ASCII_H
#define RENDERER_ASCII_H

#include "renderer/renderer.h"

// Build a renderer that draws the framebuffer into a text terminal using
// half-block glyphs (▀): each character cell stacks two vertical pixels (top =
// foreground colour, bottom = background colour), so a 160x144 frame needs only
// 160x72 character cells and keeps square-ish pixels. Colours are 24-bit ANSI.
renderer_t renderer_ascii(const char *tty_path);

#endif // RENDERER_ASCII_H
