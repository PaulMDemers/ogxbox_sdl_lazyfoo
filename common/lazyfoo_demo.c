#include "lazyfoo_demo.h"

#include <SDL.h>
#include <SDL_image.h>
#include <hal/debug.h>
#include <hal/video.h>
#include <hal/xbox.h>
#include <windows.h>

#include <math.h>
#include <stdio.h>
#include <string.h>

#ifndef LAZYFOO_LESSON
#define LAZYFOO_LESSON 1
#endif

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define FPS 60
#define FRAME_MS (1000 / FPS)

static const LazyFooLessonInfo k_lessons[] = {
    {1, "hello_sdl", "Hello SDL"},
    {2, "image_on_screen", "Getting an Image on the Screen"},
    {3, "event_driven_programming", "Event Driven Programming"},
    {4, "key_presses", "Key Presses"},
    {5, "optimized_surface_loading", "Optimized Surface Loading"},
    {6, "extension_libraries", "Extension Libraries"},
    {7, "texture_loading_rendering", "Texture Loading and Rendering"},
    {8, "geometry_rendering", "Geometry Rendering"},
    {9, "viewport", "The Viewport"},
    {10, "color_keying", "Color Keying"},
    {11, "clip_rendering_sprite_sheets", "Clip Rendering and Sprite Sheets"},
    {12, "color_modulation", "Color Modulation"},
    {13, "alpha_blending", "Alpha Blending"},
    {14, "animated_sprites_vsync", "Animated Sprites and VSync"},
    {15, "rotation_flipping", "Rotation and Flipping"},
    {17, "mouse_events", "Mouse Events"},
    {18, "key_states", "Key States"},
    {19, "gamepads_joysticks", "Gamepads and Joysticks"},
    {20, "force_feedback", "Force Feedback"},
    {22, "timing", "Timing"},
    {23, "advanced_timers", "Advanced Timers"},
    {24, "calculating_frame_rate", "Calculating Frame Rate"},
    {25, "capping_frame_rate", "Capping Frame Rate"},
    {26, "motion", "Motion"},
    {27, "collision_detection", "Collision Detection"},
    {28, "per_pixel_collision", "Per-Pixel Collision Detection"},
    {29, "circular_collision", "Circular Collision Detection"},
    {30, "scrolling", "Scrolling"},
    {31, "scrolling_backgrounds", "Scrolling Backgrounds"},
    {32, "text_input_clipboard", "Text Input and Clipboard Handling"},
    {33, "file_reading_writing", "File Reading and Writing"},
    {35, "window_events", "Window Events"},
    {36, "multiple_windows", "Multiple Windows"},
    {37, "multiple_displays", "Multiple Displays"},
    {38, "particle_engines", "Particle Engines"},
    {39, "tiling", "Tiling"},
    {40, "texture_manipulation", "Texture Manipulation"},
    {41, "bitmap_fonts", "Bitmap Fonts"},
    {42, "texture_streaming", "Texture Streaming"},
    {43, "render_to_texture", "Render to Texture"},
    {44, "frame_independent_movement", "Frame Independent Movement"},
    {45, "timer_callbacks", "Timer Callbacks"},
    {46, "multithreading", "Multithreading"},
    {47, "semaphores", "Semaphores"},
    {48, "atomic_operations", "Atomic Operations"},
    {49, "mutexes_conditions", "Mutexes and Conditions"},
};

static const char *k_xpm_image[] = {
    "32 32 5 1",
    "  c #202030",
    ". c #38BDF8",
    "+ c #F97316",
    "* c #FACC15",
    "# c #FFFFFF",
    "################################",
    "#..............................#",
    "#..++++....++++....++++....++..#",
    "#..+**+....+**+....+**+....++..#",
    "#..++++....++++....++++....++..#",
    "#..............................#",
    "#......##################......#",
    "#......#................#......#",
    "#......#..++++..++++....#......#",
    "#......#..+**+..+**+....#......#",
    "#......#..++++..++++....#......#",
    "#......#................#......#",
    "#......##################......#",
    "#..............................#",
    "#..++++....++++....++++....++..#",
    "#..+**+....+**+....+**+....++..#",
    "#..++++....++++....++++....++..#",
    "#..............................#",
    "#......##################......#",
    "#......#................#......#",
    "#......#....++++..++++..#......#",
    "#......#....+**+..+**+..#......#",
    "#......#....++++..++++..#......#",
    "#......#................#......#",
    "#......##################......#",
    "#..............................#",
    "#..++++....++++....++++....++..#",
    "#..+**+....+**+....+**+....++..#",
    "#..++++....++++....++++....++..#",
    "#..............................#",
    "#..............................#",
    "################################"
};

typedef struct DemoState {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Surface *window_surface;
    SDL_Surface *surface;
    SDL_Surface *sprite_surface;
    SDL_Texture *texture;
    SDL_Texture *sprite_texture;
    SDL_Texture *color_key_texture;
    SDL_Rect sprite_clips[4];
    int input_x;
    int input_y;
    int cursor_x;
    int cursor_y;
    float dot_x;
    float dot_y;
    Uint32 last_ticks;
    int timer_count;
    int thread_count;
    int sync_count;
    int file_ok;
    SDL_TimerID timer_id;
    SDL_Thread *thread;
    SDL_sem *sem;
    SDL_mutex *mutex;
    SDL_cond *cond;
    SDL_atomic_t atomic_count;
    int running;
    Uint32 start_ticks;
} DemoState;

static Uint32 s_timer_tick(Uint32 interval, void *param)
{
    DemoState *state = (DemoState *)param;
    state->timer_count++;
    return interval;
}

static int s_thread_main(void *param)
{
    DemoState *state = (DemoState *)param;
    int i;
    for (i = 0; i < 240; ++i) {
        state->thread_count++;
        if (state->sem) {
            SDL_SemPost(state->sem);
        }
        if (state->mutex && state->cond) {
            SDL_LockMutex(state->mutex);
            state->sync_count++;
            SDL_CondSignal(state->cond);
            SDL_UnlockMutex(state->mutex);
        }
        SDL_AtomicAdd(&state->atomic_count, 1);
        SDL_Delay(16);
    }
    return 0;
}

const LazyFooLessonInfo *lazyfoo_get_lesson_info(int lesson)
{
    size_t i;
    for (i = 0; i < sizeof(k_lessons) / sizeof(k_lessons[0]); ++i) {
        if (k_lessons[i].lesson == lesson) {
            return &k_lessons[i];
        }
    }
    return NULL;
}

static void fail(const char *stage)
{
    debugPrint("LazyFoo %02d failed at %s\nSDL: %s\nIMG: %s\n",
               LAZYFOO_LESSON, stage, SDL_GetError(), IMG_GetError());
    Sleep(5000);
    XReboot();
}

static SDL_Surface *make_surface(int w, int h, Uint8 r, Uint8 g, Uint8 b)
{
    SDL_Surface *s = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_ARGB8888);
    SDL_Rect rect;
    if (!s) {
        fail("SDL_CreateRGBSurfaceWithFormat");
    }
    rect.x = 0;
    rect.y = 0;
    rect.w = w;
    rect.h = h;
    SDL_FillRect(s, &rect, SDL_MapRGB(s->format, r, g, b));
    return s;
}

static SDL_Surface *make_checker_surface(int w, int h)
{
    SDL_Surface *s = make_surface(w, h, 24, 30, 42);
    SDL_Rect rect;
    int x, y;
    for (y = 0; y < h; y += 32) {
        for (x = 0; x < w; x += 32) {
            rect.x = x;
            rect.y = y;
            rect.w = 30;
            rect.h = 30;
            SDL_FillRect(s, &rect, SDL_MapRGB(s->format,
                                              (Uint8)(64 + (x * 3) % 160),
                                              (Uint8)(80 + (y * 5) % 140),
                                              (Uint8)(180 - ((x + y) % 80))));
        }
    }
    return s;
}

static SDL_Surface *make_keyed_surface(void)
{
    SDL_Surface *s = make_surface(192, 144, 255, 0, 255);
    SDL_Rect rect;
    Uint32 cyan = SDL_MapRGB(s->format, 45, 212, 191);
    Uint32 orange = SDL_MapRGB(s->format, 249, 115, 22);
    Uint32 white = SDL_MapRGB(s->format, 255, 255, 255);

    rect.x = 16;
    rect.y = 16;
    rect.w = 160;
    rect.h = 112;
    SDL_FillRect(s, &rect, cyan);
    rect.x = 48;
    rect.y = 40;
    rect.w = 96;
    rect.h = 64;
    SDL_FillRect(s, &rect, orange);
    rect.x = 78;
    rect.y = 18;
    rect.w = 36;
    rect.h = 108;
    SDL_FillRect(s, &rect, white);
    SDL_SetColorKey(s, SDL_TRUE, SDL_MapRGB(s->format, 255, 0, 255));
    return s;
}

static SDL_Surface *make_sprite_sheet(void)
{
    SDL_Surface *s = make_surface(128, 128, 8, 13, 24);
    SDL_Rect rect;
    int i;
    Uint32 colors[4];
    colors[0] = SDL_MapRGB(s->format, 248, 113, 113);
    colors[1] = SDL_MapRGB(s->format, 74, 222, 128);
    colors[2] = SDL_MapRGB(s->format, 96, 165, 250);
    colors[3] = SDL_MapRGB(s->format, 250, 204, 21);

    for (i = 0; i < 4; ++i) {
        rect.x = (i % 2) * 64 + 8;
        rect.y = (i / 2) * 64 + 8;
        rect.w = 48;
        rect.h = 48;
        SDL_FillRect(s, &rect, colors[i]);
        rect.x += 12;
        rect.y += 12;
        rect.w = 24;
        rect.h = 24;
        SDL_FillRect(s, &rect, SDL_MapRGB(s->format, 15, 23, 42));
    }
    return s;
}

static SDL_Texture *texture_from_surface(DemoState *state, SDL_Surface *surface)
{
    SDL_Texture *texture = SDL_CreateTextureFromSurface(state->renderer, surface);
    if (!texture) {
        fail("SDL_CreateTextureFromSurface");
    }
    return texture;
}

static void create_renderer(DemoState *state)
{
    state->renderer = SDL_CreateRenderer(state->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!state->renderer) {
        state->renderer = SDL_CreateRenderer(state->window, -1, SDL_RENDERER_SOFTWARE);
    }
    if (!state->renderer) {
        fail("SDL_CreateRenderer");
    }
    SDL_SetRenderDrawBlendMode(state->renderer, SDL_BLENDMODE_BLEND);
}

static void setup(DemoState *state)
{
    const LazyFooLessonInfo *info = lazyfoo_get_lesson_info(LAZYFOO_LESSON);
    char title[96];
    memset(state, 0, sizeof(*state));
    state->running = 1;
    state->cursor_x = SCREEN_WIDTH / 2;
    state->cursor_y = SCREEN_HEIGHT / 2;
    state->dot_x = 96.0f;
    state->dot_y = 220.0f;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_JOYSTICK | SDL_INIT_TIMER) != 0) {
        fail("SDL_Init");
    }
    if ((IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) & (IMG_INIT_PNG | IMG_INIT_JPG)) == 0) {
        debugPrint("SDL_image initialized without PNG/JPG flags; using built-in XPM path.\n");
    }

    snprintf(title, sizeof(title), "Lazy Foo %02d - %s", LAZYFOO_LESSON, info ? info->title : "SDL");
    state->window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                     SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!state->window) {
        fail("SDL_CreateWindow");
    }

    state->window_surface = SDL_GetWindowSurface(state->window);
    if (!state->window_surface) {
        fail("SDL_GetWindowSurface");
    }

    if (LAZYFOO_LESSON >= 7) {
        create_renderer(state);
    }

    state->surface = make_checker_surface(256, 192);
    state->sprite_surface = make_sprite_sheet();

    if (state->renderer) {
        SDL_Surface *keyed;
        state->texture = texture_from_surface(state, state->surface);
        state->sprite_texture = texture_from_surface(state, state->sprite_surface);
        keyed = make_keyed_surface();
        state->color_key_texture = texture_from_surface(state, keyed);
        SDL_FreeSurface(keyed);
        SDL_SetTextureBlendMode(state->color_key_texture, SDL_BLENDMODE_BLEND);
    }

    state->sprite_clips[0] = (SDL_Rect){0, 0, 64, 64};
    state->sprite_clips[1] = (SDL_Rect){64, 0, 64, 64};
    state->sprite_clips[2] = (SDL_Rect){0, 64, 64, 64};
    state->sprite_clips[3] = (SDL_Rect){64, 64, 64, 64};
    state->start_ticks = SDL_GetTicks();
    state->last_ticks = state->start_ticks;

    if (LAZYFOO_LESSON == 33) {
        SDL_RWops *rw = SDL_RWFromFile("E:\\lazyfoo_save.bin", "wb");
        if (rw) {
            const char payload[] = "Lazy Foo SDL on OG Xbox";
            state->file_ok = (SDL_RWwrite(rw, payload, 1, sizeof(payload)) == sizeof(payload));
            SDL_RWclose(rw);
        }
    } else if (LAZYFOO_LESSON == 23 || LAZYFOO_LESSON == 45) {
        state->timer_id = SDL_AddTimer(250, s_timer_tick, state);
    } else if (LAZYFOO_LESSON >= 46 && LAZYFOO_LESSON <= 49) {
        if (LAZYFOO_LESSON == 47) {
            state->sem = SDL_CreateSemaphore(0);
        }
        if (LAZYFOO_LESSON == 49) {
            state->mutex = SDL_CreateMutex();
            state->cond = SDL_CreateCond();
        }
        SDL_AtomicSet(&state->atomic_count, 0);
        state->thread = SDL_CreateThread(s_thread_main, "lazyfoo_worker", state);
    }
}

static void cleanup(DemoState *state)
{
    if (state->timer_id) SDL_RemoveTimer(state->timer_id);
    if (state->thread) SDL_WaitThread(state->thread, NULL);
    if (state->cond) SDL_DestroyCond(state->cond);
    if (state->mutex) SDL_DestroyMutex(state->mutex);
    if (state->sem) SDL_DestroySemaphore(state->sem);
    if (state->color_key_texture) SDL_DestroyTexture(state->color_key_texture);
    if (state->sprite_texture) SDL_DestroyTexture(state->sprite_texture);
    if (state->texture) SDL_DestroyTexture(state->texture);
    if (state->sprite_surface) SDL_FreeSurface(state->sprite_surface);
    if (state->surface) SDL_FreeSurface(state->surface);
    if (state->renderer) SDL_DestroyRenderer(state->renderer);
    if (state->window) SDL_DestroyWindow(state->window);
    IMG_Quit();
    SDL_Quit();
}

static void handle_input(DemoState *state)
{
    SDL_Event e;
    state->input_x = 0;
    state->input_y = 0;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            state->running = 0;
        } else if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
            case SDLK_ESCAPE:
                state->running = 0;
                break;
            case SDLK_LEFT:
                state->input_x = -1;
                break;
            case SDLK_RIGHT:
                state->input_x = 1;
                break;
            case SDLK_UP:
                state->input_y = -1;
                break;
            case SDLK_DOWN:
                state->input_y = 1;
                break;
            default:
                break;
            }
        } else if (e.type == SDL_CONTROLLERBUTTONDOWN) {
            if (e.cbutton.button == SDL_CONTROLLER_BUTTON_BACK) {
                state->running = 0;
            }
            if (e.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_LEFT) state->input_x = -1;
            if (e.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT) state->input_x = 1;
            if (e.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_UP) state->input_y = -1;
            if (e.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_DOWN) state->input_y = 1;
        } else if (e.type == SDL_JOYAXISMOTION) {
            if (e.jaxis.axis == 0) {
                if (e.jaxis.value < -12000) state->input_x = -1;
                if (e.jaxis.value > 12000) state->input_x = 1;
            }
            if (e.jaxis.axis == 1) {
                if (e.jaxis.value < -12000) state->input_y = -1;
                if (e.jaxis.value > 12000) state->input_y = 1;
            }
        } else if (e.type == SDL_MOUSEMOTION) {
            state->cursor_x = e.motion.x;
            state->cursor_y = e.motion.y;
        }
    }

    state->cursor_x += state->input_x * 8;
    state->cursor_y += state->input_y * 8;
    if (state->cursor_x < 0) state->cursor_x = 0;
    if (state->cursor_y < 0) state->cursor_y = 0;
    if (state->cursor_x > SCREEN_WIDTH) state->cursor_x = SCREEN_WIDTH;
    if (state->cursor_y > SCREEN_HEIGHT) state->cursor_y = SCREEN_HEIGHT;
}

static void clear_renderer(DemoState *state, Uint8 r, Uint8 g, Uint8 b)
{
    SDL_SetRenderDrawColor(state->renderer, r, g, b, 255);
    SDL_RenderClear(state->renderer);
}

static void render_label_bars(DemoState *state, Uint32 ticks)
{
    SDL_Rect bar = {0, 0, SCREEN_WIDTH, 26};
    SDL_SetRenderDrawColor(state->renderer, 15, 23, 42, 230);
    SDL_RenderFillRect(state->renderer, &bar);
    bar.y = SCREEN_HEIGHT - 26;
    SDL_RenderFillRect(state->renderer, &bar);
    bar.x = 12;
    bar.y = 8;
    bar.w = (int)(80 + (ticks / 8) % 200);
    bar.h = 10;
    SDL_SetRenderDrawColor(state->renderer, 56, 189, 248, 255);
    SDL_RenderFillRect(state->renderer, &bar);
}

static void draw_circle(DemoState *state, int cx, int cy, int radius, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    int y;
    SDL_SetRenderDrawColor(state->renderer, r, g, b, a);
    for (y = -radius; y <= radius; ++y) {
        int x_extent = (int)sqrt((double)(radius * radius - y * y));
        SDL_RenderDrawLine(state->renderer, cx - x_extent, cy + y, cx + x_extent, cy + y);
    }
}

static void draw_bar(DemoState *state, int x, int y, int w, int h, Uint8 r, Uint8 g, Uint8 b)
{
    SDL_Rect rect = {x, y, w, h};
    SDL_SetRenderDrawColor(state->renderer, r, g, b, 255);
    SDL_RenderFillRect(state->renderer, &rect);
}

static void draw_digit(DemoState *state, int digit, int x, int y, int scale)
{
    static const unsigned char bits[10] = {
        0x7E, 0x30, 0x6D, 0x79, 0x33, 0x5B, 0x5F, 0x70, 0x7F, 0x7B
    };
    unsigned char mask = bits[digit % 10];
    int t = scale;
    int w = scale * 4;
    int h = scale * 7;
    if (mask & 0x40) draw_bar(state, x + t, y, w, t, 250, 204, 21);
    if (mask & 0x20) draw_bar(state, x, y + t, t, h / 2 - t, 250, 204, 21);
    if (mask & 0x10) draw_bar(state, x + w + t, y + t, t, h / 2 - t, 250, 204, 21);
    if (mask & 0x08) draw_bar(state, x + t, y + h / 2, w, t, 250, 204, 21);
    if (mask & 0x04) draw_bar(state, x, y + h / 2 + t, t, h / 2 - t, 250, 204, 21);
    if (mask & 0x02) draw_bar(state, x + w + t, y + h / 2 + t, t, h / 2 - t, 250, 204, 21);
    if (mask & 0x01) draw_bar(state, x + t, y + h, w, t, 250, 204, 21);
}

static void render_surface_lessons(DemoState *state, Uint32 ticks)
{
    SDL_Rect dst;
    SDL_Surface *optimized;
    SDL_Surface *xpm;
    Uint32 fill = SDL_MapRGB(state->window_surface->format, 12, 18, 32);
    SDL_FillRect(state->window_surface, NULL, fill);

    if (LAZYFOO_LESSON == 1) {
        SDL_Rect rect = {120, 96, 400, 288};
        SDL_FillRect(state->window_surface, &rect,
                     SDL_MapRGB(state->window_surface->format, 56, 189, 248));
    } else if (LAZYFOO_LESSON == 2 || LAZYFOO_LESSON == 3) {
        dst.x = 192;
        dst.y = 144;
        SDL_BlitSurface(state->surface, NULL, state->window_surface, &dst);
    } else if (LAZYFOO_LESSON == 4) {
        dst.x = 192 + state->input_x * 80;
        dst.y = 144 + state->input_y * 60;
        SDL_BlitSurface(state->surface, NULL, state->window_surface, &dst);
    } else if (LAZYFOO_LESSON == 5) {
        optimized = SDL_ConvertSurface(state->surface, state->window_surface->format, 0);
        if (!optimized) fail("SDL_ConvertSurface");
        dst.x = 80;
        dst.y = 60;
        dst.w = 480;
        dst.h = 360;
        SDL_BlitScaled(optimized, NULL, state->window_surface, &dst);
        SDL_FreeSurface(optimized);
    } else if (LAZYFOO_LESSON == 6) {
        xpm = IMG_ReadXPMFromArray((char **)k_xpm_image);
        if (!xpm) fail("IMG_ReadXPMFromArray");
        dst.x = 160 + (int)(sin(ticks / 500.0) * 40.0);
        dst.y = 80;
        dst.w = 320;
        dst.h = 320;
        SDL_BlitScaled(xpm, NULL, state->window_surface, &dst);
        SDL_FreeSurface(xpm);
    }

    SDL_UpdateWindowSurface(state->window);
}

static void render_texture(DemoState *state, Uint32 ticks)
{
    SDL_Rect dst = {192, 144, 256, 192};
    Uint32 now = SDL_GetTicks();
    float dt = (now - state->last_ticks) / 1000.0f;
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    if (dt > 0.1f) dt = 0.1f;
    state->last_ticks = now;

    clear_renderer(state, 8, 13, 24);
    render_label_bars(state, ticks);

    switch (LAZYFOO_LESSON) {
    case 7:
        SDL_RenderCopy(state->renderer, state->texture, NULL, &dst);
        break;
    case 8:
        SDL_SetRenderDrawColor(state->renderer, 248, 113, 113, 255);
        SDL_RenderFillRect(state->renderer, &(SDL_Rect){80, 80, 160, 120});
        SDL_SetRenderDrawColor(state->renderer, 74, 222, 128, 255);
        SDL_RenderDrawRect(state->renderer, &(SDL_Rect){400, 80, 160, 120});
        SDL_SetRenderDrawColor(state->renderer, 96, 165, 250, 255);
        SDL_RenderDrawLine(state->renderer, 80, 360, 560, 260);
        SDL_SetRenderDrawColor(state->renderer, 250, 204, 21, 255);
        SDL_RenderDrawPoint(state->renderer, 320, 240);
        break;
    case 9:
        SDL_RenderSetViewport(state->renderer, &(SDL_Rect){0, 0, 320, 240});
        clear_renderer(state, 30, 41, 59);
        SDL_RenderCopy(state->renderer, state->texture, NULL, &(SDL_Rect){48, 48, 224, 144});
        SDL_RenderSetViewport(state->renderer, &(SDL_Rect){320, 0, 320, 240});
        clear_renderer(state, 69, 26, 3);
        SDL_RenderCopyEx(state->renderer, state->texture, NULL, &(SDL_Rect){48, 48, 224, 144}, 180.0, NULL, SDL_FLIP_NONE);
        SDL_RenderSetViewport(state->renderer, &(SDL_Rect){0, 240, 320, 240});
        clear_renderer(state, 6, 78, 59);
        SDL_RenderCopy(state->renderer, state->texture, NULL, &(SDL_Rect){48, 48, 224, 144});
        SDL_RenderSetViewport(state->renderer, &(SDL_Rect){320, 240, 320, 240});
        clear_renderer(state, 76, 29, 149);
        SDL_RenderCopyEx(state->renderer, state->texture, NULL, &(SDL_Rect){48, 48, 224, 144}, 90.0, NULL, SDL_FLIP_NONE);
        SDL_RenderSetViewport(state->renderer, NULL);
        break;
    case 10:
        SDL_RenderCopy(state->renderer, state->texture, NULL, &(SDL_Rect){80, 80, 256, 192});
        SDL_RenderCopy(state->renderer, state->color_key_texture, NULL, &(SDL_Rect){260, 180, 256, 192});
        break;
    case 11: {
        int i;
        for (i = 0; i < 4; ++i) {
            SDL_Rect out = {112 + (i % 2) * 240, 80 + (i / 2) * 180, 128, 128};
            SDL_RenderCopy(state->renderer, state->sprite_texture, &state->sprite_clips[i], &out);
        }
        break;
    }
    case 12:
        SDL_SetTextureColorMod(state->texture,
                               (Uint8)(128 + 127 * sin(ticks / 400.0)),
                               (Uint8)(128 + 127 * sin(ticks / 520.0 + 1.4)),
                               (Uint8)(128 + 127 * sin(ticks / 660.0 + 2.2)));
        SDL_RenderCopy(state->renderer, state->texture, NULL, &dst);
        SDL_SetTextureColorMod(state->texture, 255, 255, 255);
        break;
    case 13:
        SDL_RenderCopy(state->renderer, state->texture, NULL, &(SDL_Rect){96, 96, 320, 240});
        SDL_SetTextureAlphaMod(state->color_key_texture, (Uint8)(80 + 80 * (1.0 + sin(ticks / 380.0))));
        SDL_RenderCopy(state->renderer, state->color_key_texture, NULL, &(SDL_Rect){224, 128, 256, 192});
        SDL_SetTextureAlphaMod(state->color_key_texture, 255);
        break;
    case 14: {
        int frame = (ticks / 160) % 4;
        SDL_RenderCopy(state->renderer, state->sprite_texture, &state->sprite_clips[frame],
                       &(SDL_Rect){256, 176, 128, 128});
        break;
    }
    case 15: {
        double angle = (ticks / 18) % 360;
        SDL_RendererFlip flip = SDL_FLIP_NONE;
        if (state->input_x < 0) flip = SDL_FLIP_HORIZONTAL;
        if (state->input_y < 0) flip = SDL_FLIP_VERTICAL;
        SDL_RenderCopyEx(state->renderer, state->texture, NULL, &dst, angle, NULL, flip);
        break;
    }
    case 17:
        draw_circle(state, state->cursor_x, state->cursor_y, 36, 56, 189, 248, 255);
        SDL_SetRenderDrawColor(state->renderer, 255, 255, 255, 255);
        SDL_RenderDrawLine(state->renderer, state->cursor_x - 48, state->cursor_y, state->cursor_x + 48, state->cursor_y);
        SDL_RenderDrawLine(state->renderer, state->cursor_x, state->cursor_y - 48, state->cursor_x, state->cursor_y + 48);
        break;
    case 18:
        draw_bar(state, 110, 110, keys[SDL_SCANCODE_UP] || state->input_y < 0 ? 420 : 120, 36, 96, 165, 250);
        draw_bar(state, 110, 170, keys[SDL_SCANCODE_DOWN] || state->input_y > 0 ? 420 : 120, 36, 74, 222, 128);
        draw_bar(state, 110, 230, keys[SDL_SCANCODE_LEFT] || state->input_x < 0 ? 420 : 120, 36, 248, 113, 113);
        draw_bar(state, 110, 290, keys[SDL_SCANCODE_RIGHT] || state->input_x > 0 ? 420 : 120, 36, 250, 204, 21);
        break;
    case 19:
        draw_bar(state, 110, 180, 420, 36, 30, 41, 59);
        if (state->input_x < 0) {
            draw_bar(state, 140, 180, 180, 36, 56, 189, 248);
        } else if (state->input_x > 0) {
            draw_bar(state, 320, 180, 180, 36, 56, 189, 248);
        }
        draw_bar(state, 302, 90, 36, 300, 30, 41, 59);
        if (state->input_y < 0) {
            draw_bar(state, 302, 120, 36, 120, 249, 115, 22);
        } else if (state->input_y > 0) {
            draw_bar(state, 302, 240, 36, 120, 249, 115, 22);
        }
        draw_circle(state, state->cursor_x, state->cursor_y, 28, 250, 204, 21, 255);
        break;
    case 20:
        draw_circle(state, 320, 240, 64 + (int)(sin(ticks / 70.0) * 28.0), 248, 113, 113, 180);
        draw_bar(state, 160, 360, (ticks / 8) % 320, 20, 250, 204, 21);
        break;
    case 22:
        draw_bar(state, 90, 220, (ticks / 4) % 460, 40, 56, 189, 248);
        draw_digit(state, (ticks / 1000) % 10, 300, 120, 10);
        break;
    case 23:
        draw_bar(state, 100, 120, (ticks / 8) % 420, 28, 96, 165, 250);
        draw_bar(state, 100, 200, state->timer_count * 18 % 420, 28, 74, 222, 128);
        draw_bar(state, 100, 280, ((ticks / 1000) % 2) ? 420 : 80, 28, 249, 115, 22);
        break;
    case 24: {
        int fps = ticks > 0 ? (int)((ticks / FRAME_MS) * 1000 / ticks) : 0;
        draw_digit(state, (fps / 10) % 10, 260, 170, 14);
        draw_digit(state, fps % 10, 340, 170, 14);
        break;
    }
    case 25:
        draw_circle(state, 80 + (ticks / 12) % 480, 240, 40, 74, 222, 128, 255);
        draw_bar(state, 60, 400, 520, 8, 148, 163, 184);
        break;
    case 26:
    case 44:
        state->dot_x += 140.0f * dt;
        if (state->dot_x > 560.0f) state->dot_x = 80.0f;
        draw_circle(state, (int)state->dot_x, (int)state->dot_y, 32, 56, 189, 248, 255);
        break;
    case 27: {
        SDL_Rect a = {110 + (int)(sin(ticks / 420.0) * 80.0), 170, 160, 120};
        SDL_Rect b = {310, 180, 160, 120};
        SDL_bool hit = SDL_HasIntersection(&a, &b);
        draw_bar(state, a.x, a.y, a.w, a.h, hit ? 248 : 96, hit ? 113 : 165, hit ? 113 : 250);
        draw_bar(state, b.x, b.y, b.w, b.h, 250, 204, 21);
        break;
    }
    case 28:
        draw_circle(state, 250 + (int)(sin(ticks / 400.0) * 70.0), 240, 62, 96, 165, 250, 210);
        draw_circle(state, 360, 240, 62, 250, 204, 21, 210);
        break;
    case 29:
        draw_circle(state, 230 + (int)(sin(ticks / 360.0) * 80.0), 240, 58, 74, 222, 128, 210);
        draw_circle(state, 390, 240, 72, 249, 115, 22, 180);
        break;
    case 30:
    case 31: {
        int offset = (ticks / 12) % 128;
        int x, y;
        for (y = -64; y < SCREEN_HEIGHT; y += 64) {
            for (x = -offset; x < SCREEN_WIDTH; x += 128) {
                draw_bar(state, x, y, 64, 64, 30, 64, 91);
                draw_bar(state, x + 64, y, 64, 64, 15, 23, 42);
            }
        }
        draw_bar(state, 240, 160, 160, 160, 248, 113, 113);
        break;
    }
    case 32: {
        int count = 3 + (ticks / 500) % 12;
        int i;
        SDL_StartTextInput();
        for (i = 0; i < count; ++i) draw_digit(state, i % 10, 80 + i * 36, 210, 5);
        break;
    }
    case 33:
        draw_bar(state, 140, 170, 360, 120, state->file_ok ? 74 : 248, state->file_ok ? 222 : 113, state->file_ok ? 128 : 113);
        draw_digit(state, state->file_ok ? 1 : 0, 296, 190, 12);
        break;
    case 35:
        draw_bar(state, 80, 80, 480, 320, 30, 41, 59);
        draw_bar(state, 100, 104, 440, 28, 56, 189, 248);
        draw_bar(state, 100, 350, (ticks / 10) % 440, 24, 250, 204, 21);
        break;
    case 36:
        draw_bar(state, 72, 84, 220, 150, 30, 64, 175);
        draw_bar(state, 348, 84, 220, 150, 132, 27, 45);
        draw_bar(state, 72, 266, 220, 150, 6, 95, 70);
        draw_bar(state, 348, 266, 220, 150, 113, 63, 18);
        break;
    case 37: {
        int displays = SDL_GetNumVideoDisplays();
        draw_digit(state, displays % 10, 300, 170, 16);
        draw_bar(state, 140, 340, displays > 0 ? 360 : 80, 28, 56, 189, 248);
        break;
    }
    case 38: {
        int i;
        for (i = 0; i < 40; ++i) {
            int px = (i * 53 + ticks / 7) % SCREEN_WIDTH;
            int py = (i * 97 + ticks / 11) % SCREEN_HEIGHT;
            draw_circle(state, px, py, 5 + (i % 5), 250, 204, 21, 180);
        }
        break;
    }
    case 39: {
        int x, y;
        for (y = 0; y < SCREEN_HEIGHT; y += 40) {
            for (x = 0; x < SCREEN_WIDTH; x += 40) {
                draw_bar(state, x + 1, y + 1, 38, 38, ((x + y) / 40) % 2 ? 30 : 51, 41, 59);
            }
        }
        draw_circle(state, 320, 240, 34, 248, 113, 113, 255);
        break;
    }
    case 40: {
        int x, y;
        SDL_Texture *tex = SDL_CreateTexture(state->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 160, 120);
        if (tex) {
            Uint32 *pixels = NULL;
            int pitch = 0;
            if (SDL_LockTexture(tex, NULL, (void **)&pixels, &pitch) == 0) {
                for (y = 0; y < 120; ++y) {
                    for (x = 0; x < 160; ++x) {
                        pixels[y * (pitch / 4) + x] = 0xFF000000u | ((x + ticks / 8) & 255) << 16 | ((y * 2) & 255) << 8 | 0x60u;
                    }
                }
                SDL_UnlockTexture(tex);
            }
            SDL_RenderCopy(state->renderer, tex, NULL, &(SDL_Rect){160, 120, 320, 240});
            SDL_DestroyTexture(tex);
        }
        break;
    }
    case 41:
        draw_digit(state, 4, 160, 150, 14);
        draw_digit(state, 1, 250, 150, 14);
        draw_digit(state, (ticks / 500) % 10, 340, 150, 14);
        break;
    case 42:
        if (state->texture) {
            SDL_UpdateTexture(state->texture, NULL, state->surface->pixels, state->surface->pitch);
            SDL_RenderCopy(state->renderer, state->texture, NULL, &dst);
        }
        break;
    case 43: {
        SDL_Texture *target = SDL_CreateTexture(state->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, 256, 192);
        if (target && SDL_SetRenderTarget(state->renderer, target) == 0) {
            clear_renderer(state, 30, 41, 59);
            draw_circle(state, 128, 96, 54, 250, 204, 21, 255);
            SDL_SetRenderTarget(state->renderer, NULL);
            SDL_RenderCopyEx(state->renderer, target, NULL, &dst, ticks / 20.0, NULL, SDL_FLIP_NONE);
        } else {
            SDL_RenderCopyEx(state->renderer, state->texture, NULL, &dst, ticks / 20.0, NULL, SDL_FLIP_NONE);
        }
        if (target) SDL_DestroyTexture(target);
        break;
    }
    case 45:
        draw_digit(state, (state->timer_count / 10) % 10, 260, 170, 14);
        draw_digit(state, state->timer_count % 10, 340, 170, 14);
        break;
    case 46:
        draw_bar(state, 100, 210, (state->thread_count * 3) % 440, 52, 56, 189, 248);
        break;
    case 47: {
        Uint32 sem_value = state->sem ? SDL_SemValue(state->sem) : 0;
        draw_bar(state, 100, 210, (int)(sem_value * 3) % 440, 52, 74, 222, 128);
        break;
    }
    case 48: {
        int atom = SDL_AtomicGet(&state->atomic_count);
        draw_bar(state, 100, 210, (atom * 3) % 440, 52, 250, 204, 21);
        break;
    }
    case 49:
        draw_bar(state, 100, 210, (state->sync_count * 3) % 440, 52, 248, 113, 113);
        break;
    default:
        break;
    }

    SDL_RenderPresent(state->renderer);
}

static void run_demo(void)
{
    DemoState state;
    setup(&state);

    while (state.running) {
        Uint32 frame_start = SDL_GetTicks();
        Uint32 ticks;
        handle_input(&state);
        ticks = SDL_GetTicks() - state.start_ticks;

        if (LAZYFOO_LESSON <= 6) {
            render_surface_lessons(&state, ticks);
        } else {
            render_texture(&state, ticks);
        }

        if (SDL_GetTicks() - frame_start < FRAME_MS) {
            SDL_Delay(FRAME_MS - (SDL_GetTicks() - frame_start));
        }
    }

    cleanup(&state);
}

int main(void)
{
    XVideoSetMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, REFRESH_DEFAULT);
    debugPrint("Lazy Foo SDL lesson %02d starting\n", LAZYFOO_LESSON);
    run_demo();
    return 0;
}
