# Lazy Foo SDL Tutorial Roadmap

The current Lazy Foo SDL2 tutorial index lists lessons 1-55. This repository is
starting with the lessons that can be built on original Xbox with nxdk's SDL2
and SDL2_image libraries.

## Implemented SDL/SDL_image Set

Lessons 1-15, 17-20, 22-33, and 35-49 are standalone ports. They cover SDL
initialization, surfaces, events, controller-backed key input, optimized surface
conversion, SDL_image, textures, geometry, viewports, color keying, sprite
clips, modulation, alpha blending, animation timing, rotation/flipping, input
state, gamepad visualization, timing, collision, scrolling, file I/O, windows,
displays, particles, tiles, pixel/streaming texture paths, render targets,
threading, semaphores, atomics, and mutex/condition primitives.

## Deferred Dependencies

- 16 True Type Fonts: requires SDL_ttf.
- 21 Sound Effects and Music: requires SDL_mixer.
- 34 Audio Recording: needs an Xbox audio capture decision.
- 50-51 SDL and OpenGL: should be evaluated after deciding whether this set
  should exercise NXGL through SDL or remain SDL-renderer only.
- 52-55 Mobile/touch/orientation: not applicable as written.

## Next Batch

The next practical batch is dependency-focused: SDL_ttf, SDL_mixer, and whether
the OpenGL SDL lessons should route through NXGL.
