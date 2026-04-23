# MiniSnip (clipboard-only fork)

A stripped-down fork of [deminimis/MiniSnip](https://github.com/deminimis/MiniSnip).

One hotkey. Drag a box. The image goes straight to your clipboard. That's it.

## What was removed vs. upstream

- OCR (WinRT / `Windows.Media.Ocr`)
- Save-as-PNG (GDI+ encoder, save dialog)
- Post-snip action toolbar
- The large yellow "Select area to Snip" overlay label
- "Interactive" / "Save" / "OCR" hotkeys and their settings rows

## What was kept

- Native C++ Win32, zero runtime dependencies
- System tray icon with `Start Snip` / `Settings` / `Exit`
- Lightweight toast notification (`Copied to Clipboard`)
- Multi-monitor screen capture
- Configurable hotkey via `MiniSnip.ini` or tray -> Settings
- `Esc` / right-click to cancel during selection
- GPL-3.0, same as upstream

## Default hotkey

`F7`. Change it from the tray icon -> **Settings**.

`MiniSnip.ini` lives next to `MiniSnip.exe` (or in `%LOCALAPPDATA%\MiniSnip\` if the exe folder isn't writable).

## How to use

1. Run `MiniSnip.exe`. A scissor icon appears in the system tray.
2. Press your hotkey (default `F7`).
3. Click and drag to select.
4. Release. The image is on your clipboard. Paste with `Ctrl+V`.

## Building

Visual Studio 2022 with the "Desktop development with C++" workload.

```
msbuild MiniSnip\MiniSnip.vcxproj /p:Configuration=Release /p:Platform=x64
```

Output: `MiniSnip\x64\Release\MiniSnip.exe`.

## Upstream

Forked from [deminimis/MiniSnip](https://github.com/deminimis/MiniSnip).
All credit to the original author. This fork exists only to remove features I don't use.

## License

GPL-3.0, inherited from upstream. See `LICENSE`.
