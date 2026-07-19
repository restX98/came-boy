#include "renderer/renderer_ascii.h"

#include <stdio.h>

#include "logger.h"
#include "ppu.h"

// Upper-half-block: fills the top half of the cell with the foreground colour
// and leaves the bottom half showing the background colour, letting one cell
// represent two vertically-stacked pixels.
#define HALF_BLOCK "▀"

// Game Boy shade -> RGB. 0 = lightest, 3 = darkest (grayscale; swap these for
// the classic DMG greens if you prefer).
static const uint8_t PALETTE[4][3] = {
    { 255, 255, 255 },
    { 170, 170, 170 },
    {  85,  85,  85 },
    {   0,   0,   0 },
};

typedef struct {
    const char *tty_path;  // caller-supplied tty, or NULL to spawn our own window
    FILE *out;
} ascii_ctx_t;

static ascii_ctx_t g_ascii_ctx;

static int renderer_ascii_init(renderer_t *self) {
    ascii_ctx_t *ctx = self->ctx;

    const char *path = ctx->tty_path;
    if (path) {
        ctx->out = fopen(path, "w");
        if (!ctx->out) {
            LOG_WARN("Could not open %s, falling back to stdout", path);
        }
    }
    if (!ctx->out) ctx->out = stdout;

    // A frame is a few hundred KB of escape codes; a big fully-buffered stream
    // turns each frame into essentially one write() instead of thousands.
    setvbuf(ctx->out, NULL, _IOFBF, 1 << 20);

    fprintf(ctx->out, "\x1b[?25l\x1b[2J\x1b[H");  // hide cursor, clear, home
    fflush(ctx->out);
    return 0;
}

static void renderer_ascii_render(renderer_t *self, const uint8_t *framebuffer) {
    ascii_ctx_t *ctx = self->ctx;
    if (!ctx->out) return;

    fprintf(ctx->out, "\x1b[H");  // home; overwrite in place, no scrolling

    // Track the last emitted colours so runs of same-coloured pixels don't
    // re-send the (long) 24-bit escape for every cell. SGR state survives the
    // cursor moves between rows, so we track across the whole frame.
    int last_fg = -1, last_bg = -1;
    for (int cy = 0; cy < LCD_HEIGHT / 2; cy++) {
        fprintf(ctx->out, "\x1b[%d;1H", cy + 1);
        for (int x = 0; x < LCD_WIDTH; x++) {
            int top = framebuffer[(2 * cy) * LCD_WIDTH + x] & 0x3;
            int bot = framebuffer[(2 * cy + 1) * LCD_WIDTH + x] & 0x3;

            if (top != last_fg) {
                fprintf(ctx->out, "\x1b[38;2;%d;%d;%dm",
                    PALETTE[top][0], PALETTE[top][1], PALETTE[top][2]);
                last_fg = top;
            }
            if (bot != last_bg) {
                fprintf(ctx->out, "\x1b[48;2;%d;%d;%dm",
                    PALETTE[bot][0], PALETTE[bot][1], PALETTE[bot][2]);
                last_bg = bot;
            }
            fputs(HALF_BLOCK, ctx->out);
        }
    }

    fprintf(ctx->out, "\x1b[0m");  // reset colours so the shell prompt is normal
    fflush(ctx->out);
}

static void renderer_ascii_deinit(renderer_t *self) {
    ascii_ctx_t *ctx = self->ctx;
    if (!ctx->out) return;
    fprintf(ctx->out, "\x1b[0m\x1b[?25h");  // reset colours, show cursor
    fflush(ctx->out);
    if (ctx->out != stdout) fclose(ctx->out);
    ctx->out = NULL;
}

renderer_t renderer_ascii(const char *tty_path) {
    g_ascii_ctx = (ascii_ctx_t){ .tty_path = tty_path, .out = NULL };
    return (renderer_t) {
        .init = renderer_ascii_init,
            .render = renderer_ascii_render,
            .deinit = renderer_ascii_deinit,
            .ctx = &g_ascii_ctx,
    };
}
