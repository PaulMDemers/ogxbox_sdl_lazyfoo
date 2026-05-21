# Lazy Foo SDL Tutorial Roadmap

The current Lazy Foo SDL2 tutorial index lists lessons 1-55. This repository is
starting with the lessons that can be built on original Xbox with nxdk's SDL2,
SDL2_image, SDL_ttf, PBKit, optional SDL_mixer, and optional NXGL support.

## Implemented SDL/SDL_image/SDL_ttf/SDL_mixer Set

Lessons 1-33 except 34, 35-49, and PBKit variants of 50-51 are standalone ports
with one lesson-local `main.c` per tutorial. Lesson 21 is enabled when
`NXDK_DIR` points at an nxdk tree with SDL_mixer. NXGL variants of 50-51 are
enabled when `NXGL_DIR` points at a checkout with `nxgl.mk`. They cover SDL
initialization, surfaces, events,
controller-backed key input, optimized surface conversion, SDL_image, textures,
geometry, viewports, color keying, sprite clips, modulation, alpha blending,
animation timing, rotation/flipping, TTF text, input state, gamepad
visualization, sound effects/music, timing, collision, scrolling, file I/O,
windows, displays, particles, tiles, pixel/streaming texture paths, render
targets, threading, semaphores, atomics, mutex/condition primitives, and the
OpenGL lessons as explicit PBKit/NXGL rendering flavors.

## Deferred Dependencies

- 34 Audio Recording: nxdk's Xbox SDL audio backend currently has capture
  support disabled.
- 52-55 Mobile/touch/orientation: not applicable as written.

## Next Batch

The practical SDL2 desktop tutorial set is complete except for audio capture
and mobile/touch/orientation lessons. The next useful work is deeper visual
parity capture for the 50-51 PBKit/NXGL variants and revisiting lesson 34 if an
Xbox SDL audio capture backend lands. See `docs/DEPENDENCIES.md` for the
current optional dependency gates and expected build counts.
