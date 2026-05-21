#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <hal/debug.h>
#include <hal/video.h>
#include <hal/xbox.h>
#include <windows.h>

#include <math.h>
#include <stdio.h>
#include <string.h>

#define LAZYFOO_LESSON 33

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define FPS 60
#define FRAME_MS (1000 / FPS)

#define LAZYFOO_LESSON_SLUG "file_reading_writing"
#define LAZYFOO_LESSON_TITLE "File Reading and Writing"

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
    SDL_Texture *font_texture;
    TTF_Font *font;
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
    int ttf_initialized;
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

static void fail(const char *stage)
{
    debugPrint("LazyFoo %02d failed at %s\nSDL: %s\nIMG: %s\n",
               LAZYFOO_LESSON, stage, SDL_GetError(), IMG_GetError());
    Sleep(5000);
    XReboot();
}

static void fail_ttf(const char *stage)
{
    debugPrint("LazyFoo %02d failed at %s\nSDL: %s\nTTF: %s\n",
               LAZYFOO_LESSON, stage, SDL_GetError(), TTF_GetError());
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

    snprintf(title, sizeof(title), "Lazy Foo %02d - %s", LAZYFOO_LESSON, LAZYFOO_LESSON_TITLE);
    state->window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                     SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!state->window) {
        fail("SDL_CreateWindow");
    }

    state->window_surface = SDL_GetWindowSurface(state->window);
    if (!state->window_surface) {
        fail("SDL_GetWindowSurface");
    }

    if (LAZYFOO_LESSON == 16 || LAZYFOO_LESSON >= 7) {
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

    if (LAZYFOO_LESSON == 16) {
        SDL_Color title_color = {125, 255, 125, 255};
        SDL_Surface *title_surface;
        if (TTF_Init() != 0) {
            fail_ttf("TTF_Init");
        }
        state->ttf_initialized = 1;
        state->font = TTF_OpenFont("D:\\vegur-regular.ttf", 40);
        if (!state->font) {
            fail_ttf("TTF_OpenFont");
        }
        title_surface = TTF_RenderText_Blended(state->font, "Lazy Foo SDL_ttf", title_color);
        if (!title_surface) {
            fail_ttf("TTF_RenderText_Blended");
        }
        state->font_texture = texture_from_surface(state, title_surface);
        SDL_FreeSurface(title_surface);
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
    if (state->font_texture) SDL_DestroyTexture(state->font_texture);
    if (state->font) TTF_CloseFont(state->font);
    if (state->texture) SDL_DestroyTexture(state->texture);
    if (state->sprite_surface) SDL_FreeSurface(state->sprite_surface);
    if (state->surface) SDL_FreeSurface(state->surface);
    if (state->renderer) SDL_DestroyRenderer(state->renderer);
    if (state->window) SDL_DestroyWindow(state->window);
    if (state->ttf_initialized) TTF_Quit();
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
        0x77, 0x12, 0x5D, 0x5B, 0x3A, 0x6B, 0x6F, 0x52, 0x7F, 0x7B
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

static void draw_number(DemoState *state, int value, int digits, int x, int y, int scale)
{
    int divisor = 1;
    int i;
    if (value < 0) value = 0;
    for (i = 1; i < digits; ++i) {
        divisor *= 10;
    }
    for (i = 0; i < digits; ++i) {
        draw_digit(state, (value / divisor) % 10, x + i * scale * 7, y, scale);
        divisor /= 10;
    }
}

static void draw_gauge(DemoState *state, int x, int y, int w, int h, int value, int max_value,
                       Uint8 r, Uint8 g, Uint8 b)
{
    int filled;
    SDL_Rect outline = {x, y, w, h};
    if (max_value <= 0) max_value = 1;
    if (value < 0) value = 0;
    if (value > max_value) value = max_value;
    filled = (w - 8) * value / max_value;
    draw_bar(state, x + 4, y + 4, filled, h - 8, r, g, b);
    SDL_SetRenderDrawColor(state->renderer, 148, 163, 184, 255);
    SDL_RenderDrawRect(state->renderer, &outline);
}

static void render_surface_lessons(DemoState *state, Uint32 ticks)
{
    SDL_Rect dst;
    SDL_Surface *optimized;
    SDL_Surface *xpm;
    Uint32 fill = SDL_MapRGB(state->window_surface->format, 12, 18, 32);
    SDL_FillRect(state->window_surface, NULL, fill);

    (void)ticks;

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

        draw_bar(state, 140, 170, 360, 120, state->file_ok ? 74 : 248, state->file_ok ? 222 : 113, state->file_ok ? 128 : 113);
        draw_number(state, state->file_ok ? 1 : 0, 3, 236, 190, 12);
        draw_gauge(state, 180, 330, 280, 28, state->file_ok ? 280 : 70, 280, 56, 189, 248);

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
