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
    int running;
    Uint32 start_ticks;
} DemoState;

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
}

static void cleanup(DemoState *state)
{
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
        }
    }
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
