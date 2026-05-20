# Contributing

This project ports the Lazy Foo SDL2 tutorials to original Xbox using nxdk.

## Scope

- Keep each tutorial as a standalone `NNN_lesson_name` build.
- Prefer nxdk-shipped libraries before adding new external dependencies.
- Keep generated outputs out of git; `bin/`, `dist/`, object files, XBEs, and
  ISOs are ignored.
- When a tutorial cannot map cleanly to original Xbox hardware or nxdk's SDL
  backend, document the gap in `docs/ROADMAP.md` and `docs/SCREENSHOT_AUDIT.md`.

## Expected Checks

Before committing a port or tooling change:

```sh
make release
```

Then from PowerShell:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\verify_lazyfoo.ps1 -CheckReleaseArtifacts
```

For visual changes, capture the affected lesson in xemu and update
`docs/SCREENSHOT_AUDIT.md` when the status changes.

## Style

The shared implementation lives in `common/lazyfoo_demo.c`. Keep tutorial
rendering deterministic enough for screenshot comparison, and avoid depending
on keyboard input for the default visual state.
