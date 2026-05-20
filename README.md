# Original Xbox Lazy Foo SDL Ports

Standalone original Xbox ports of the Lazy Foo SDL2 tutorial sequence.

The first pass focuses on tutorials that can be expressed with nxdk's built-in
SDL2 and SDL2_image support. Lessons that require SDL_ttf, SDL_mixer, mobile
APIs, or OpenGL are tracked but skipped until those dependencies get their own
pass.

Source tutorial index:

- https://lazyfoo.net/tutorials/SDL/

## Current Coverage

Implemented:

- 01 Hello SDL
- 02 Getting an Image on the Screen
- 03 Event Driven Programming
- 04 Key Presses
- 05 Optimized Surface Loading and Soft Stretching
- 06 Extension Libraries and Loading Other Image Formats
- 07 Texture Loading and Rendering
- 08 Geometry Rendering
- 09 The Viewport
- 10 Color Keying
- 11 Clip Rendering and Sprite Sheets
- 12 Color Modulation
- 13 Alpha Blending
- 14 Animated Sprites and VSync
- 15 Rotation and Flipping

Skipped for now:

- SDL_ttf lessons: 16, 47, 56
- SDL_mixer lessons: 21
- Platform/mobile lessons: 67-70
- SDL/OpenGL lessons: 65-66

Later SDL-only lessons are still pending and should be added in numbered
standalone folders following the same layout.

## Build

From MSYS2 with `NXDK_DIR` configured, or from this workspace layout with
`.nxdk` beside the repository:

```sh
make
```

To build one lesson:

```sh
make -C 001_hello_sdl
```

## Controls

The demos run without a keyboard. When a tutorial expects keyboard input, use
the D-pad or left stick. Press Back to quit when running in an environment that
handles `SDL_QUIT`.

## Layout

- `common/lazyfoo_demo.c` contains the shared tutorial implementations.
- `common/lazyfoo_demo.h` contains lesson metadata.
- `NNN_*` directories are standalone Xbox builds.

## License

This porting scaffold is released under the MIT License. Lazy Foo tutorial
material remains credited to Lazy Foo' Productions.
