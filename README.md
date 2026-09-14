# Sleapy Camera

Lightweight webcam-only desktop app for Windows.

## V1 target

- Webcam only — no audio initialization
- Camera selection
- 720p / 1080p
- 30 / 60 FPS
- Original background
- Background blur
- Custom image background
- Face tracking
- Face blur
- Camera / Settings modes
- System tray
- Native DirectX 11 rendering
- Small runtime footprint

OBS configuration is intentionally out of scope.

## Important

This repository is designed so GitHub Actions builds the Windows x64 executable.
The local ZIP does **not** contain a precompiled Windows EXE because this environment
cannot execute the Windows toolchain.

## Build on GitHub

1. Create a public GitHub repository.
2. Upload this repository.
3. Open Actions.
4. Run **Build Windows** manually.
5. Download the `SleapyCamera-Windows-x64` artifact.

A tagged release (`v0.1.0`, for example) can be added later.

## Local build

Requires Windows + Visual Studio C++ workload + Windows SDK.

The current V1 source is a native Win32/DX11 shell intended to be extended with
Media Foundation camera capture and face processing.
