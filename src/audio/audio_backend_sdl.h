#ifndef AUDIO_BACKEND_SDL_H
#define AUDIO_BACKEND_SDL_H

#include "audio/audio_backend.h"

// SDL2 audio backend. Stages incoming stereo samples locally and flushes
// them to an SDL audio device in chunks via SDL_QueueAudio, rather than
// syscalling once per sample (44100 times/sec).
audio_backend_t audio_backend_sdl(void);

#endif // AUDIO_BACKEND_SDL_H
