# Original Xbox Lazy Foo SDL Ports

Standalone original Xbox ports of the Lazy Foo SDL2 tutorial sequence.

The first pass focuses on tutorials that can be expressed with nxdk's built-in
SDL2, SDL2_image, SDL_ttf, PBKit, and optional NXGL support. SDL_mixer lesson
21 is included when the selected nxdk tree ships SDL_mixer. Mobile/touch APIs
remain tracked but skipped.

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
- 16 True Type Fonts
- 17 Mouse Events
- 18 Key States
- 19 Gamepads and Joysticks
- 20 Force Feedback
- 21 Sound Effects and Music (when `NXDK_DIR` includes SDL_mixer)
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
- 50 SDL and OpenGL 2 (PBKit, plus NXGL when `NXGL_DIR` is available)
- 51 SDL and Modern OpenGL (PBKit, plus NXGL when `NXGL_DIR` is available)

Skipped for now:

- Audio recording lesson: 34
- Platform/mobile/touch lessons: 52-55

That leaves 49 buildable stock nxdk tutorial ports in this sweep, 51 when NXGL
is available, and 52 when both NXGL and SDL_mixer are available. See
[docs/DEPENDENCIES.md](docs/DEPENDENCIES.md) for the dependency matrix and
expected build counts.

## Build

From MSYS2 MinGW64 with `NXDK_DIR` configured, or from this workspace layout
with `.nxdk` beside the repository:

```sh
make
```

To build one lesson:

```sh
make -C 001_hello_sdl
```

To stage release artifacts:

```sh
make release
```

This writes ISOs and XBE folders under `dist/release/lazyfoo`.

See [docs/BUILDING.md](docs/BUILDING.md) for the full build, release, and xemu
capture workflow.

## Verification

The release artifact checker follows the evaluated Makefile app list, including
optional SDL_mixer and NXGL lessons when their dependencies are available:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\verify_lazyfoo.ps1 -CheckReleaseArtifacts
```

To capture the completed lessons in xemu:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\capture_lazyfoo_xemu.ps1
```

Screenshots are written to `dist/captures/xemu/lazyfoo` with a JSON manifest.

To turn a capture folder into a reviewable contact sheet:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\make_capture_contact_sheet.ps1
```

Lesson 21 is enabled automatically when `NXDK_DIR` points at an nxdk tree that
ships SDL_mixer:

```powershell
$env:NXDK_DIR="C:\path\to\nxdk-with-sdl-mixer"
make release
powershell -ExecutionPolicy Bypass -File .\tools\verify_lazyfoo.ps1 -CheckReleaseArtifacts
```

The NXGL variants of lessons 50 and 51 are enabled automatically when
`NXGL_DIR` points at a checkout with `nxgl.mk`; otherwise the PBKit variants are
still built.

## Controls

The demos run without a keyboard. When a tutorial expects keyboard input, use
the D-pad or left stick. Press Back to quit when running in an environment that
handles `SDL_QUIT`.

## Layout

- `NNN_*` directories are standalone Xbox builds with lesson-local `main.c`
  files.
- `docs/DEPENDENCIES.md` lists optional dependency gates and expected lesson
  counts.
- `docs/ROADMAP.md` tracks completed and deferred tutorials.
- `docs/SCREENSHOT_AUDIT.md` tracks xemu capture status.

## License

This porting scaffold is released under the MIT License. Lazy Foo tutorial
material remains credited to Lazy Foo' Productions.
