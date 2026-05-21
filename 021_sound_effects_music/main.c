#include <SDL.h>
#include <SDL_mixer.h>

#define LAZYFOO_LESSON 21

#include <hal/debug.h>
#include <hal/video.h>
#include <hal/xbox.h>
#include <windows.h>

#include <math.h>
#include <stdint.h>
#include <stdio.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define FPS 60
#define FRAME_MS (1000 / FPS)

#define SAMPLE_RATE 22050
#define CHANNELS 1
#define BITS_PER_SAMPLE 16
#define EFFECT_MS 180
#define MUSIC_MS 4200

typedef struct GeneratedWav {
    Uint8 *data;
    int size;
} GeneratedWav;

typedef struct DemoState {
    SDL_Window *window;
    SDL_Renderer *renderer;
    Mix_Chunk *scratch;
    Mix_Chunk *high;
    Mix_Chunk *medium;
    Mix_Chunk *low;
    Mix_Music *music;
    GeneratedWav scratch_wav;
    GeneratedWav high_wav;
    GeneratedWav medium_wav;
    GeneratedWav low_wav;
    GeneratedWav music_wav;
    Uint32 start_ticks;
    Uint32 last_effect_ticks;
    int effect_index;
    int music_ok;
    int running;
} DemoState;

static void write_le16(Uint8 *dst, Uint16 value)
{
    dst[0] = (Uint8)(value & 0xff);
    dst[1] = (Uint8)(value >> 8);
}

static void write_le32(Uint8 *dst, Uint32 value)
{
    dst[0] = (Uint8)(value & 0xff);
    dst[1] = (Uint8)((value >> 8) & 0xff);
    dst[2] = (Uint8)((value >> 16) & 0xff);
    dst[3] = (Uint8)(value >> 24);
}

static GeneratedWav make_tone_wav(int duration_ms, double base_hz, double wobble_hz, int sweep)
{
    const Uint32 samples = (SAMPLE_RATE * duration_ms) / 1000;
    const Uint16 block_align = (CHANNELS * BITS_PER_SAMPLE) / 8;
    const Uint32 data_size = samples * block_align;
    const Uint32 byte_rate = SAMPLE_RATE * block_align;
    GeneratedWav wav;
    Uint32 i;

    wav.data = (Uint8 *)SDL_malloc(44 + data_size);
    wav.size = wav.data ? (int)(44 + data_size) : 0;
    if (!wav.data) {
        return wav;
    }

    SDL_memcpy(wav.data + 0, "RIFF", 4);
    write_le32(wav.data + 4, 36 + data_size);
    SDL_memcpy(wav.data + 8, "WAVE", 4);
    SDL_memcpy(wav.data + 12, "fmt ", 4);
    write_le32(wav.data + 16, 16);
    write_le16(wav.data + 20, 1);
    write_le16(wav.data + 22, CHANNELS);
    write_le32(wav.data + 24, SAMPLE_RATE);
    write_le32(wav.data + 28, byte_rate);
    write_le16(wav.data + 32, block_align);
    write_le16(wav.data + 34, BITS_PER_SAMPLE);
    SDL_memcpy(wav.data + 36, "data", 4);
    write_le32(wav.data + 40, data_size);

    for (i = 0; i < samples; ++i) {
        const double t = (double)i / (double)SAMPLE_RATE;
        const double env = sin(3.14159265358979323846 * (double)i / (double)samples);
        const double hz = base_hz + (sweep ? wobble_hz * t : wobble_hz * sin(t * 18.0));
        const double sample = sin(2.0 * 3.14159265358979323846 * hz * t) * env * 11800.0;
        write_le16(wav.data + 44 + (i * block_align), (Uint16)((int16_t)sample));
    }

    return wav;
}

static GeneratedWav make_music_wav(void)
{
    const Uint32 samples = (SAMPLE_RATE * MUSIC_MS) / 1000;
    const Uint16 block_align = (CHANNELS * BITS_PER_SAMPLE) / 8;
    const Uint32 data_size = samples * block_align;
    const Uint32 byte_rate = SAMPLE_RATE * block_align;
    GeneratedWav wav;
    Uint32 i;

    wav.data = (Uint8 *)SDL_malloc(44 + data_size);
    wav.size = wav.data ? (int)(44 + data_size) : 0;
    if (!wav.data) {
        return wav;
    }

    SDL_memcpy(wav.data + 0, "RIFF", 4);
    write_le32(wav.data + 4, 36 + data_size);
    SDL_memcpy(wav.data + 8, "WAVE", 4);
    SDL_memcpy(wav.data + 12, "fmt ", 4);
    write_le32(wav.data + 16, 16);
    write_le16(wav.data + 20, 1);
    write_le16(wav.data + 22, CHANNELS);
    write_le32(wav.data + 24, SAMPLE_RATE);
    write_le32(wav.data + 28, byte_rate);
    write_le16(wav.data + 32, block_align);
    write_le16(wav.data + 34, BITS_PER_SAMPLE);
    SDL_memcpy(wav.data + 36, "data", 4);
    write_le32(wav.data + 40, data_size);

    for (i = 0; i < samples; ++i) {
        const double t = (double)i / (double)SAMPLE_RATE;
        const int step = (int)(t * 4.0) % 8;
        const double notes[8] = {220.0, 277.18, 329.63, 277.18, 246.94, 293.66, 369.99, 293.66};
        const double beat = fmod(t * 4.0, 1.0);
        const double env = beat < 0.12 ? beat / 0.12 : 1.0;
        const double tone = sin(2.0 * 3.14159265358979323846 * notes[step] * t) * 4800.0;
        const double bass = sin(2.0 * 3.14159265358979323846 * 55.0 * t) * 2600.0;
        const double sample = (tone + bass) * env;
        write_le16(wav.data + 44 + (i * block_align), (Uint16)((int16_t)sample));
    }

    return wav;
}

static void fail(const char *stage)
{
    debugPrint("LazyFoo 21 failed at %s\nSDL: %s\nMix: %s\n",
               stage, SDL_GetError(), Mix_GetError());
    Sleep(5000);
    XReboot();
}

static Mix_Chunk *load_chunk(GeneratedWav *wav)
{
    SDL_RWops *rw;
    Mix_Chunk *chunk;
    if (!wav->data || wav->size <= 0) {
        fail("generate WAV chunk");
    }
    rw = SDL_RWFromConstMem(wav->data, wav->size);
    if (!rw) {
        fail("SDL_RWFromConstMem");
    }
    chunk = Mix_LoadWAV_RW(rw, 1);
    if (!chunk) {
        fail("Mix_LoadWAV_RW");
    }
    return chunk;
}

static Mix_Music *load_music(GeneratedWav *wav)
{
    SDL_RWops *rw;
    Mix_Music *music;
    if (!wav->data || wav->size <= 0) {
        fail("generate music WAV");
    }
    rw = SDL_RWFromConstMem(wav->data, wav->size);
    if (!rw) {
        fail("SDL_RWFromConstMem music");
    }
    music = Mix_LoadMUS_RW(rw, 1);
    if (!music) {
        fail("Mix_LoadMUS_RW");
    }
    return music;
}

static void free_wav(GeneratedWav *wav)
{
    if (wav->data) {
        SDL_free(wav->data);
        wav->data = NULL;
        wav->size = 0;
    }
}

static void draw_bar(SDL_Renderer *renderer, int x, int y, int w, int h, Uint8 r, Uint8 g, Uint8 b)
{
    SDL_Rect rect = {x, y, w, h};
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    SDL_RenderFillRect(renderer, &rect);
}

static void draw_digit(SDL_Renderer *renderer, int digit, int x, int y, int scale)
{
    static const unsigned char bits[10] = {
        0x77, 0x12, 0x5D, 0x5B, 0x3A, 0x6B, 0x6F, 0x52, 0x7F, 0x7B
    };
    unsigned char mask = bits[digit % 10];
    int t = scale;
    int w = scale * 4;
    int h = scale * 7;
    if (mask & 0x40) draw_bar(renderer, x + t, y, w, t, 250, 204, 21);
    if (mask & 0x20) draw_bar(renderer, x, y + t, t, h / 2 - t, 250, 204, 21);
    if (mask & 0x10) draw_bar(renderer, x + w + t, y + t, t, h / 2 - t, 250, 204, 21);
    if (mask & 0x08) draw_bar(renderer, x + t, y + h / 2, w, t, 250, 204, 21);
    if (mask & 0x04) draw_bar(renderer, x, y + h / 2 + t, t, h / 2 - t, 250, 204, 21);
    if (mask & 0x02) draw_bar(renderer, x + w + t, y + h / 2 + t, t, h / 2 - t, 250, 204, 21);
    if (mask & 0x01) draw_bar(renderer, x + t, y + h, w, t, 250, 204, 21);
}

static void draw_number(SDL_Renderer *renderer, int value, int digits, int x, int y, int scale)
{
    int divisor = 1;
    int i;
    if (value < 0) value = 0;
    for (i = 1; i < digits; ++i) {
        divisor *= 10;
    }
    for (i = 0; i < digits; ++i) {
        draw_digit(renderer, (value / divisor) % 10, x + i * scale * 7, y, scale);
        divisor /= 10;
    }
}

static void play_effect(DemoState *state, int index)
{
    Mix_Chunk *chunks[4];
    chunks[0] = state->scratch;
    chunks[1] = state->high;
    chunks[2] = state->medium;
    chunks[3] = state->low;
    Mix_PlayChannel(-1, chunks[index & 3], 0);
}

static void setup(DemoState *state)
{
    SDL_memset(state, 0, sizeof(*state));
    state->running = 1;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER | SDL_INIT_JOYSTICK | SDL_INIT_TIMER) != 0) {
        fail("SDL_Init");
    }

    state->window = SDL_CreateWindow("Lazy Foo 21 - Sound Effects and Music",
                                     SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                     SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!state->window) {
        fail("SDL_CreateWindow");
    }
    state->renderer = SDL_CreateRenderer(state->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!state->renderer) {
        state->renderer = SDL_CreateRenderer(state->window, -1, SDL_RENDERER_SOFTWARE);
    }
    if (!state->renderer) {
        fail("SDL_CreateRenderer");
    }

    if (Mix_OpenAudio(SAMPLE_RATE, AUDIO_S16SYS, 2, 1024) < 0) {
        fail("Mix_OpenAudio");
    }
    Mix_AllocateChannels(8);
    Mix_Volume(-1, MIX_MAX_VOLUME / 2);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 3);

    state->scratch_wav = make_tone_wav(EFFECT_MS, 280.0, 900.0, 1);
    state->high_wav = make_tone_wav(EFFECT_MS, 880.0, 120.0, 0);
    state->medium_wav = make_tone_wav(EFFECT_MS, 520.0, 80.0, 0);
    state->low_wav = make_tone_wav(EFFECT_MS, 180.0, 40.0, 0);
    state->music_wav = make_music_wav();

    state->scratch = load_chunk(&state->scratch_wav);
    state->high = load_chunk(&state->high_wav);
    state->medium = load_chunk(&state->medium_wav);
    state->low = load_chunk(&state->low_wav);
    state->music = load_music(&state->music_wav);
    state->music_ok = (Mix_PlayMusic(state->music, -1) == 0);

    state->start_ticks = SDL_GetTicks();
    state->last_effect_ticks = state->start_ticks;
}

static void cleanup(DemoState *state)
{
    Mix_HaltMusic();
    Mix_HaltChannel(-1);
    if (state->music) Mix_FreeMusic(state->music);
    if (state->scratch) Mix_FreeChunk(state->scratch);
    if (state->high) Mix_FreeChunk(state->high);
    if (state->medium) Mix_FreeChunk(state->medium);
    if (state->low) Mix_FreeChunk(state->low);
    free_wav(&state->scratch_wav);
    free_wav(&state->high_wav);
    free_wav(&state->medium_wav);
    free_wav(&state->low_wav);
    free_wav(&state->music_wav);
    Mix_CloseAudio();
    if (state->renderer) SDL_DestroyRenderer(state->renderer);
    if (state->window) SDL_DestroyWindow(state->window);
    SDL_Quit();
}

static void handle_input(DemoState *state)
{
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            state->running = 0;
        } else if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
            case SDLK_ESCAPE:
                state->running = 0;
                break;
            case SDLK_1:
            case SDLK_UP:
                play_effect(state, 1);
                break;
            case SDLK_2:
            case SDLK_RIGHT:
                play_effect(state, 2);
                break;
            case SDLK_3:
            case SDLK_DOWN:
                play_effect(state, 3);
                break;
            case SDLK_4:
            case SDLK_LEFT:
                play_effect(state, 0);
                break;
            case SDLK_p:
                if (Mix_PausedMusic()) {
                    Mix_ResumeMusic();
                } else {
                    Mix_PauseMusic();
                }
                break;
            default:
                break;
            }
        } else if (e.type == SDL_CONTROLLERBUTTONDOWN) {
            if (e.cbutton.button == SDL_CONTROLLER_BUTTON_BACK) {
                state->running = 0;
            } else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_UP) {
                play_effect(state, 1);
            } else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT) {
                play_effect(state, 2);
            } else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_DOWN) {
                play_effect(state, 3);
            } else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_LEFT) {
                play_effect(state, 0);
            } else if (e.cbutton.button == SDL_CONTROLLER_BUTTON_A) {
                if (Mix_PausedMusic()) {
                    Mix_ResumeMusic();
                } else {
                    Mix_PauseMusic();
                }
            }
        }
    }
}

static void render(DemoState *state, Uint32 ticks)
{
    int i;
    int music_phase = (ticks / 40) % 384;
    int playing_channels = 0;

    SDL_SetRenderDrawColor(state->renderer, 8, 13, 24, 255);
    SDL_RenderClear(state->renderer);

    draw_bar(state->renderer, 88, 80, 464, 96, 15, 23, 42);
    draw_bar(state->renderer, 112, 112, state->music_ok && !Mix_PausedMusic() ? 416 : 96, 32, 56, 189, 248);
    draw_bar(state->renderer, 112, 148, music_phase, 8, 250, 204, 21);

    for (i = 0; i < 4; ++i) {
        const int x = 112 + i * 108;
        const int active = Mix_Playing(i);
        Uint8 r = i == 0 ? 249 : (i == 1 ? 96 : (i == 2 ? 74 : 248));
        Uint8 g = i == 0 ? 115 : (i == 1 ? 165 : (i == 2 ? 222 : 113));
        Uint8 b = i == 0 ? 22 : (i == 1 ? 250 : (i == 2 ? 128 : 113));
        if (active) {
            playing_channels++;
        }
        draw_bar(state->renderer, x, 232, 76, active ? 120 : 44, r, g, b);
        draw_digit(state->renderer, i + 1, x + 22, active ? 258 : 246, 6);
    }

    draw_bar(state->renderer, 120, 398, 400, 20, 30, 41, 59);
    draw_bar(state->renderer, 120, 398, (ticks / 12) % 400, 20, 148, 163, 184);
    draw_number(state->renderer, playing_channels, 1, 302, 334, 10);

    SDL_RenderPresent(state->renderer);
}

int main(void)
{
    DemoState state;

    XVideoSetMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, REFRESH_DEFAULT);
    debugPrint("Lazy Foo SDL lesson 21 starting\n");
    setup(&state);

    while (state.running) {
        Uint32 frame_start = SDL_GetTicks();
        Uint32 ticks;

        handle_input(&state);
        ticks = SDL_GetTicks() - state.start_ticks;

        if (ticks - state.last_effect_ticks >= 650) {
            play_effect(&state, state.effect_index++);
            state.last_effect_ticks = ticks;
        }

        render(&state, ticks);

        if (SDL_GetTicks() - frame_start < FRAME_MS) {
            SDL_Delay(FRAME_MS - (SDL_GetTicks() - frame_start));
        }
    }

    cleanup(&state);
    return 0;
}
