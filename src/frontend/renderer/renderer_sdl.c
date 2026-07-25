#include "frontend/renderer/renderer_sdl.h"

#include "frontend/sdl_context.h"
#include "logger.h"
#include "ppu.h"

// Game Boy shade -> RGB. Mirrors renderer_ascii's palette (0 = lightest, 3 =
// darkest; swap these for the classic DMG greens if you prefer).
static const uint8_t PALETTE[4][3] = {
    { 255, 255, 255 },
    { 170, 170, 170 },
    {  85,  85,  85 },
    {   0,   0,   0 },
};

typedef struct {
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    uint32_t pixels[LCD_WIDTH * LCD_HEIGHT]; // ARGB8888, rebuilt from the shade framebuffer each frame
} sdl_renderer_ctx_t;

static sdl_renderer_ctx_t g_ctx;

static int renderer_sdl_init(renderer_t *self) {
    sdl_renderer_ctx_t *ctx = self->ctx;

    SDL_Window *window;
    if (sdl_context_acquire(&window, &ctx->renderer) != 0) {
        ctx->texture = NULL; // no display available; render() becomes a no-op
        return 0;
    }

    ctx->texture = SDL_CreateTexture(
        ctx->renderer, SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING, LCD_WIDTH, LCD_HEIGHT
    );
    if (!ctx->texture) {
        LOG_WARN("renderer_sdl: SDL_CreateTexture failed: %s", SDL_GetError());
    }
    return 0;
}

static void renderer_sdl_render(renderer_t *self, const uint8_t *framebuffer) {
    sdl_renderer_ctx_t *ctx = self->ctx;
    if (!ctx->texture) return;

    for (int i = 0; i < LCD_WIDTH * LCD_HEIGHT; i++) {
        const uint8_t *rgb = PALETTE[framebuffer[i] & 0x3];
        ctx->pixels[i] = 0xFF000000u | ((uint32_t)rgb[0] << 16) | ((uint32_t)rgb[1] << 8) | rgb[2];
    }

    SDL_UpdateTexture(ctx->texture, NULL, ctx->pixels, LCD_WIDTH * (int)sizeof(uint32_t));
    SDL_RenderClear(ctx->renderer);
    SDL_RenderCopy(ctx->renderer, ctx->texture, NULL, NULL);
    SDL_RenderPresent(ctx->renderer); // no vsync -- see sdl_context.c
}

static void renderer_sdl_deinit(renderer_t *self) {
    sdl_renderer_ctx_t *ctx = self->ctx;
    if (ctx->texture) {
        SDL_DestroyTexture(ctx->texture);
        ctx->texture = NULL;
    }
    sdl_context_release();
}

renderer_t renderer_sdl(void) {
    g_ctx = (sdl_renderer_ctx_t){ 0 };
    return (renderer_t) {
        .init = renderer_sdl_init,
        .render = renderer_sdl_render,
        .deinit = renderer_sdl_deinit,
        .ctx = &g_ctx,
    };
}
