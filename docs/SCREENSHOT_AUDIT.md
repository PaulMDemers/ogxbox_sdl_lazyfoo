# Lazy Foo SDL Screenshot Audit

Capture command:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\capture_lazyfoo_xemu.ps1
```

The capture script boots each lesson ISO in a fresh xemu process and writes PNGs
plus a manifest under `dist/captures/xemu/lazyfoo`. The default delay is 15
seconds because several renderer-backed demos can show the early debug framebuffer
if captured too soon.

## Current Coverage

Status legend:

- OK: visible, lesson-appropriate output captured in xemu.
- Watch: works, but needs deeper parity or dependency follow-up later.
- Deferred: not ported in this SDL/SDL_image/SDL_ttf pass.

| Lesson | Port | Capture | Notes |
| --- | --- | --- | --- |
| 01 | Hello SDL | OK | Solid window clear/fill path. |
| 02 | Image on Screen | OK | Procedural surface blit. |
| 03 | Event Driven Programming | OK | Same render with event loop active. |
| 04 | Key Presses | OK | D-pad/left-stick backed movement path. |
| 05 | Optimized Surface Loading | OK | Converted surface and scaled blit. |
| 06 | Extension Libraries | OK | SDL_image path plus visible procedural fallback overlay. |
| 07 | Texture Loading and Rendering | OK | SDL texture creation/render path. |
| 08 | Geometry Rendering | OK | Rect, outline, line, and point primitives. |
| 09 | Viewport | OK | Four viewport quadrants render independently. |
| 10 | Color Keying | OK | Color-keyed overlay visible. |
| 11 | Clip Rendering and Sprite Sheets | OK | Four sprite clips visible. |
| 12 | Color Modulation | OK | Modulated texture visible. |
| 13 | Alpha Blending | OK | Blended overlay visible. |
| 14 | Animated Sprites and VSync | OK | Animated sprite frame visible. |
| 15 | Rotation and Flipping | OK | Rotated texture visible after 15s settle. |
| 16 | True Type Fonts | Watch | SDL_ttf text render using nxdk's bundled sample font. |
| 17 | Mouse Events | OK | Cursor visual visible after 15s settle. |
| 18 | Key States | OK | Four input-state bars visible. |
| 19 | Gamepads and Joysticks | OK | Cross-axis and center marker visible. |
| 20 | Force Feedback | OK | Haptic pulse visualization visible. |
| 21 | Sound Effects and Music | Deferred | SDL_mixer batch. |
| 22 | Timing | OK | Timer bar and digit visible. |
| 23 | Advanced Timers | OK | SDL timer callback bar visible. |
| 24 | Calculating Frame Rate | OK | Bitmap FPS digits visible. |
| 25 | Capping Frame Rate | OK | Motion plus cap baseline visible. |
| 26 | Motion | OK | Moving dot visible. |
| 27 | Collision Detection | OK | Rect collision pair visible. |
| 28 | Per-Pixel Collision Detection | OK | Overlapping circle proxy visible. |
| 29 | Circular Collision Detection | OK | Circle pair visible. |
| 30 | Scrolling | OK | Scrolling tile field visible. |
| 31 | Scrolling Backgrounds | OK | Repeating background visible. |
| 32 | Text Input and Clipboard Handling | Watch | Uses bitmap input field until SDL_ttf text UI is available. |
| 33 | File Reading and Writing | Watch | File result indicator renders; verify real hardware storage later. |
| 34 | Audio Recording | Deferred | nxdk Xbox audio capture support is disabled. |
| 35 | Window Events | OK | Window-event proxy panel visible. |
| 36 | Multiple Windows | OK | Multiple panel proxy visible. |
| 37 | Multiple Displays | OK | Display count visualization visible. |
| 38 | Particle Engines | OK | Particles visible. |
| 39 | Tiling | OK | Tile grid visible. |
| 40 | Texture Manipulation | OK | Streaming color field visible. |
| 41 | Bitmap Fonts | OK | Bitmap digits and progress visible. |
| 42 | Texture Streaming | OK | Updated texture visible. |
| 43 | Render to Texture | OK | Render-target texture visible. |
| 44 | Frame Independent Movement | OK | Moving dot visible. |
| 45 | Timer Callbacks | OK | Timer counter and gauge visible. |
| 46 | Multithreading | OK | Worker counter and gauge visible. |
| 47 | Semaphores | OK | Semaphore counter and gauge visible. |
| 48 | Atomic Operations | OK | Atomic counter and gauge visible. |
| 49 | Mutexes and Conditions | OK | Mutex/condition counter and gauge visible. |
| 50 | SDL and OpenGL 2 | Deferred | nxdk SDL has no Xbox GL context path. |
| 51 | SDL and Modern OpenGL | Deferred | nxdk SDL has no Xbox GL context path. |
| 52-55 | Mobile/touch/orientation | Deferred | Not applicable as written. |

## Follow-Up Items

- Replace the lesson 6 procedural fallback with a real bundled PNG/JPG asset
  once the asset packaging convention is settled.
- Revisit lesson 32 after deciding whether to render real text input with
  SDL_ttf or keep the bitmap proxy for controller-friendly captures.
- Add visual comparison snapshots once a desktop SDL reference runner exists.
