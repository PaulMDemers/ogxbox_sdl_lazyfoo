# Lazy Foo SDL Tutorial Roadmap

The current Lazy Foo SDL2 tutorial index lists lessons 1-55. This repository is
starting with the lessons that can be built on original Xbox with nxdk's SDL2,
SDL2_image, and SDL_ttf libraries.

## Implemented SDL/SDL_image/SDL_ttf Set

Lessons 1-20 except 21, 22-33, and 35-49 are standalone ports. They cover SDL
initialization, surfaces, events, controller-backed key input, optimized surface
conversion, SDL_image, textures, geometry, viewports, color keying, sprite
clips, modulation, alpha blending, animation timing, rotation/flipping, TTF text,
input state, gamepad visualization, timing, collision, scrolling, file I/O,
windows, displays, particles, tiles, pixel/streaming texture paths, render
targets, threading, semaphores, atomics, and mutex/condition primitives.

## Deferred Dependencies

- 21 Sound Effects and Music: requires SDL_mixer.
- 34 Audio Recording: nxdk's Xbox SDL audio backend currently has capture
  support disabled.
- 50-51 SDL and OpenGL: nxdk's Xbox SDL video backend does not expose an SDL GL
  context path; these should be handled in a separate NXGL-oriented pass.
- 52-55 Mobile/touch/orientation: not applicable as written.

## Next Batch

The next practical batch is dependency-focused: SDL_mixer, then a separate
decision on whether the SDL/OpenGL lessons should route through NXGL.
