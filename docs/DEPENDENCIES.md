# Dependency Matrix

This repository keeps each Lazy Foo tutorial as a standalone Xbox build. The
top-level Makefile includes optional lessons only when their dependencies are
present in the selected workspace.

## Lesson Groups

| Lessons | Dependency set | Notes |
| --- | --- | --- |
| 001-005, 008-015, 017-020, 022-033, 035-049 | nxdk SDL2 | Core SDL surfaces, events, textures, input, timers, collision, scrolling, file I/O, threading, and render-target lessons. |
| 006-007 | nxdk SDL2 + SDL2_image | Image loading and texture loading lessons. |
| 016 | nxdk SDL2 + SDL_ttf | TrueType font rendering. |
| 021 | nxdk SDL2 + SDL_mixer | Included when `NXDK_DIR/lib/sdl/SDL_mixer/SDL_mixer.h` exists. Lesson Makefile sets `NXDK_SDL_MIXER = y`. |
| 050_sdl_opengl_2_pbkit, 051_sdl_modern_opengl_pbkit | nxdk + PBKit | Always included with the base nxdk checkout. |
| 050_sdl_opengl_2_nxgl, 051_sdl_modern_opengl_nxgl | nxdk + NXGL | Included when `NXGL_DIR/nxgl.mk` exists. |

## Deferred

| Lessons | Status | Reason |
| --- | --- | --- |
| 034 | Deferred | SDL audio capture is not available in nxdk's Xbox audio backend. |
| 052-055 | Deferred | Mobile/touch/orientation APIs are not applicable to original Xbox as written. |

## Expected Counts

| Workspace dependencies | Built lesson apps |
| --- | ---: |
| Base nxdk only | 49 |
| Base nxdk + SDL_mixer | 50 |
| Base nxdk + NXGL | 51 |
| Base nxdk + SDL_mixer + NXGL | 52 |

Use `make print-apps` to see the exact evaluated list for the current
`NXDK_DIR` and `NXGL_DIR`.
