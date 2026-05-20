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
- 17 Mouse Events
- 18 Key States
- 19 Gamepads and Joysticks
- 20 Force Feedback
- 22 Timing
- 23 Advanced Timers
- 24 Calculating Frame Rate
- 25 Capping Frame Rate
- 26 Motion
- 27 Collision Detection
- 28 Per-Pixel Collision Detection
- 29 Circular Collision Detection
- 30 Scrolling
- 31 Scrolling Backgrounds
- 32 Text Input and Clipboard Handling
- 33 File Reading and Writing
- 35 Window Events
- 36 Multiple Windows
- 37 Multiple Displays
- 38 Particle Engines
- 39 Tiling
- 40 Texture Manipulation
- 41 Bitmap Fonts
- 42 Texture Streaming
- 43 Render to Texture
- 44 Frame Independent Movement
- 45 Timer Callbacks
- 46 Multithreading
- 47 Semaphores
- 48 Atomic Operations
- 49 Mutexes and Conditions

Skipped for now:

- SDL_ttf lesson: 16
- SDL_mixer lesson: 21
- Audio recording lesson: 34
- SDL/OpenGL lessons: 50-51
- Platform/mobile/touch lessons: 52-55

That leaves 46 buildable SDL/SDL_image tutorial ports in this first sweep.

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
