# Building and Release Artifacts

This repository builds standalone original Xbox XBE and ISO outputs for each
ported Lazy Foo SDL2 tutorial.

## Requirements

- Windows with MSYS2 MinGW64.
- nxdk available at `../.nxdk` relative to this repository, or `NXDK_DIR` set
  to an nxdk checkout.
- xemu plus Xbox firmware files for the optional capture workflow.

The current lesson set uses the SDL2, SDL2_image, and SDL_ttf libraries that
ship with nxdk.

## Build Everything

From an MSYS2 MinGW64 shell:

```sh
make
```

To build one lesson:

```sh
make -C 016_true_type_fonts
```

Lesson 16 copies `vegur-regular.ttf` from nxdk's bundled `samples/sdl_ttf`
folder into its build output; the font is not vendored in this repository.

## Stage Release Files

```sh
make release
```

Release files are written under `dist/release/lazyfoo`:

- `isos/`: one bootable ISO per lesson.
- `xbes/`: one folder per lesson containing `default.xbe`.

## Verify Release Layout

From PowerShell:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\verify_lazyfoo.ps1 -CheckReleaseArtifacts
```

The verifier checks the implemented lesson count, required source files, and
release ISO/XBE outputs.

## Capture Screenshots in xemu

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\capture_lazyfoo_xemu.ps1
```

The capture script boots every lesson ISO in a fresh xemu process and writes a
PNG plus `manifest.json` under `dist/captures/xemu/lazyfoo`.

Use `-ResumeExisting` when only new or missing lessons need captures:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\capture_lazyfoo_xemu.ps1 -ResumeExisting
```

The capture manifest is merged by lesson name, so recapturing a single lesson
into an existing full-pass folder updates that entry without dropping the rest
of the manifest.

## Build a Capture Contact Sheet

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\make_capture_contact_sheet.ps1
```

By default this reads `dist/captures/xemu/lazyfoo` and writes
`contact_sheet.png` plus a JSON summary in the same folder. Use `-CaptureRoot`
and `-OutputPath` to point it at a named capture pass:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\make_capture_contact_sheet.ps1 `
  -CaptureRoot .\dist\captures\xemu\lazyfoo_full_52_post_optin `
  -OutputPath .\dist\captures\xemu\lazyfoo_full_52_post_optin\contact_sheet.png
```

When desktop or reference captures are available, pass `-CompareRoot` to create
side-by-side cells and a sampled mean absolute color delta for each lesson.
