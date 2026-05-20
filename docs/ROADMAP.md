# Lazy Foo SDL Tutorial Roadmap

The current Lazy Foo SDL2 tutorial index lists lessons 1-70. This repository is
starting with the lessons that can be built on original Xbox with nxdk's SDL2
and SDL2_image libraries.

## Implemented First Batch

Lessons 1-15 are standalone ports. They cover SDL initialization, surfaces,
events, controller-backed key input, optimized surface conversion, SDL_image,
textures, geometry, viewports, color keying, sprite clips, color modulation,
alpha blending, animation timing, and rotation/flipping.

## Deferred Dependencies

- 16 True Type Fonts: requires SDL_ttf.
- 21 Sound Effects and Music: requires SDL_mixer.
- 47 Text Input and Clipboard Handling: text input can be adapted, clipboard is
  not useful on Xbox; keep it deferred until the input plan is clearer.
- 56 Bitmap Fonts: implementable without SDL_ttf, but it belongs with the later
  text rendering pass.
- 65-66 SDL and OpenGL: should be evaluated after deciding whether this set
  should exercise NXGL through SDL or remain SDL-renderer only.
- 67-70 Mobile/touch/orientation: not applicable as written.

## Next SDL-Only Batch

The next practical batch is lessons 17-20 and 22-46, skipping SDL_mixer lesson
21. Most of these can be represented with synthetic assets and controller input.
