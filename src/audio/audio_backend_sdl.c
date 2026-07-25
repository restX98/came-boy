#include "audio/audio_backend_sdl.h"

#include <SDL2/SDL.h>

#include "logger.h"

#define SDL_AUDIO_SAMPLE_RATE 44100
#define SDL_AUDIO_CHANNELS    2
#define SDL_AUDIO_DEVICE_BUFFER_FRAMES 1024 // SDL's own device buffer

// Frames accumulated locally before one SDL_QueueAudio call.
#define STAGING_BUFFER_FRAMES 256

typedef struct {
    SDL_AudioDeviceID device;
    int16_t staging[STAGING_BUFFER_FRAMES * SDL_AUDIO_CHANNELS];
    int staging_count; // frames currently buffered
} sdl_audio_ctx_t;

static sdl_audio_ctx_t g_sdl_audio_ctx;

static int audio_backend_sdl_init(audio_backend_t *self) {
    sdl_audio_ctx_t *ctx = self->ctx;
    ctx->staging_count = 0;
    ctx->device = 0;

    if (SDL_WasInit(SDL_INIT_AUDIO) == 0 && SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
        // No audio hardware/driver available (e.g. headless/CI environments):
        // log and continue silently rather than requiring audio to run.
        LOG_WARN("audio_backend_sdl: SDL_InitSubSystem(AUDIO) failed: %s", SDL_GetError());
        return 0;
    }

    SDL_AudioSpec want = { 0 };
    SDL_AudioSpec have = { 0 };
    want.freq = SDL_AUDIO_SAMPLE_RATE;
    want.format = AUDIO_S16SYS;
    want.channels = SDL_AUDIO_CHANNELS;
    want.samples = SDL_AUDIO_DEVICE_BUFFER_FRAMES;
    want.callback = NULL; // we push via SDL_QueueAudio rather than a pull callback

    // allowed_changes=0: force SDL to give us exactly what we asked for (it
    // resamples internally if the real device differs), so our fixed-rate
    // downsampling in audio_tick stays valid.
    ctx->device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if (ctx->device == 0) {
        LOG_WARN("audio_backend_sdl: SDL_OpenAudioDevice failed: %s", SDL_GetError());
        return 0;
    }

    SDL_PauseAudioDevice(ctx->device, 0); // start playback
    LOG_INFO("audio_backend_sdl: opened device at %d Hz, %d channel(s)", have.freq, have.channels);
    return 0;
}

static void audio_backend_sdl_flush(sdl_audio_ctx_t *ctx) {
    if (ctx->device == 0 || ctx->staging_count == 0) return;

    Uint32 bytes = (Uint32)(ctx->staging_count * SDL_AUDIO_CHANNELS * sizeof(int16_t));
    if (SDL_QueueAudio(ctx->device, ctx->staging, bytes) != 0) {
        LOG_WARN("audio_backend_sdl: SDL_QueueAudio failed: %s", SDL_GetError());
    }
    ctx->staging_count = 0;
}

static void audio_backend_sdl_queue_sample(audio_backend_t *self, int16_t left, int16_t right) {
    sdl_audio_ctx_t *ctx = self->ctx;
    if (ctx->device == 0) return; // no device available; drop samples silently

    ctx->staging[ctx->staging_count * SDL_AUDIO_CHANNELS + 0] = left;
    ctx->staging[ctx->staging_count * SDL_AUDIO_CHANNELS + 1] = right;
    ctx->staging_count++;

    if (ctx->staging_count == STAGING_BUFFER_FRAMES) {
        audio_backend_sdl_flush(ctx);
    }

    // If the emulator ever outruns real time (e.g. fast-forward), don't let
    // SDL's internal queue grow unbounded — drop stale audio instead of
    // letting latency creep up.
    Uint32 queued_bytes = SDL_GetQueuedAudioSize(ctx->device);
    Uint32 max_bytes = SDL_AUDIO_DEVICE_BUFFER_FRAMES * SDL_AUDIO_CHANNELS * sizeof(int16_t) * 4;
    if (queued_bytes > max_bytes) {
        SDL_ClearQueuedAudio(ctx->device);
    }
}

static void audio_backend_sdl_deinit(audio_backend_t *self) {
    sdl_audio_ctx_t *ctx = self->ctx;
    if (ctx->device == 0) return;

    audio_backend_sdl_flush(ctx);
    SDL_CloseAudioDevice(ctx->device);
    ctx->device = 0;
}

audio_backend_t audio_backend_sdl(void) {
    g_sdl_audio_ctx = (sdl_audio_ctx_t){ 0 };
    return (audio_backend_t) {
        .init = audio_backend_sdl_init,
        .queue_sample = audio_backend_sdl_queue_sample,
        .deinit = audio_backend_sdl_deinit,
        .ctx = &g_sdl_audio_ctx,
    };
}
