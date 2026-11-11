# Mickey & Donald Collection - Nintendo Switch Port

A proof-of-concept port: the same emulation core, game modules, launcher, menus, saves and achievements as the
Windows and Android builds, as a homebrew `.nro` (devkitPro / libnx). Like the other builds it contains no game
data: the ROMs are supplied by the user.

## Build

1. Install devkitPro with the Switch toolchain and the portlibs this build links:
   ```
   dkp-pacman -S switch-dev switch-sdl2 switch-mesa switch-freetype switch-harfbuzz switch-libpng switch-bzip2 switch-zlib
   ```
   and make sure `DEVKITPRO` is set (`export DEVKITPRO=/opt/devkitpro`).
2. ```
   cd switch
   make              # -> MickeyDonald_nx.nro
   make NXLINK=1     # same, plus log output over nxlink (printf / stderr and an fps line every 5 s)
   ```

## Install and run

Copy `MickeyDonald_nx.nro` to `sdmc:/switch/MickeyDonald_nx/` on the SD card and start it from the homebrew menu. ROMs go in
`sdmc:/switch/MickeyDonald_nx/roms` (or next to the .nro): they are found by CRC32 (the same dumps as the PC build - see the
main README), raw or zipped; any file name works. Settings, save states, screenshots and achievements are stored
in `sdmc:/switch/MickeyDonald_nx/`; MD+ soundtrack packs go in `sdmc:/switch/MickeyDonald_nx/soundtracks/`.

For logs: `make NXLINK=1`, then run `nxlink -s MickeyDonald_nx.nro` on the PC (or start the app with the homebrew menu's
nxlink mode) and watch the console.

## Controls

Buttons are labelled as on the controller. Menus: **A** confirm, **B** back, D-pad / left stick move, **L / R**
switch pages. In game (all rebindable under Options > Controls): Genesis **A / B / C = Y / B / A**, **+** =
Start, **-** = pause menu, **ZR** (hold) = fast forward, **ZL** (hold) = rewind. Player 1 is the handheld controls
or controller 1; player 2 (World of Illusion) is controller 2. Quick save / load are in the pause menu.

## What differs from the other builds

| Part | Windows | Android | Switch |
|---|---|---|---|
| Window / main loop | `app_win.cpp` | `app_android.cpp` | `app_switch.cpp` (SDL2) |
| Video | Direct3D 11 | `platform/android/video_gles.cpp` | the same OpenGL ES 3 file (Mesa) |
| UI text | DirectWrite | Android fonts via JNI | FreeType over the console's system fonts (`platform/switch/glyphs_ft.cpp`) |
| Audio | XAudio2 | `platform/android/audio_sdl.cpp` | the same SDL queue file |
| Input | XInput / DirectInput / keyboard | SDL game controllers, touch | libnx HID (`platform/switch/input_switch.cpp`, `nx_platform.cpp`); no keyboard or touch yet |
| Files | `sys_win.cpp` | `sys_android.cpp` | `sys_switch.cpp` (`sdmc:/switch/MDC`) |

The shared code gained only small `__SWITCH__` branches (launcher wording, default pad layout in
`settings.cpp`); the Windows and Android paths are unchanged.

## Status

Written against the libnx / SDL2 / Mesa APIs:

- **Frame rate.** Run with `NXLINK=1` and watch the fps line (and Options > Video > Show FPS). The core was tuned
  on PCs and phones; the Switch's CPU is slower than most phones'. If it falls short at 60, try the widescreen
  modes off first, then profile `Session`'s frame step. (A CPU clock boost through libnx's clkrst is the usual
  next lever; it isn't included.)
- **Docked / handheld switch.** The window should follow the 1280x720 <-> 1920x1080 change through the
  size-change event; if the picture doesn't, recreate the GL surface on that event in `app_switch.cpp`.
- **OpenGL ES 3.0 context.** Requested through SDL; Mesa on Switch normally offers ES 3.x. If `video_.init`
  fails, the error goes to stderr (nxlink).
- **Glyphs.** UI symbols that the system fonts lack (arrows, play triangles, check mark) are drawn by hand; a
  missing letter shows as `?`.
- **Joy-Cons held sideways** (a single Joy-Con as one controller) aren't remapped; use a pair, a Pro Controller
  or handheld mode.
- **Memory.** Rewind keeps up to 64 MB, which fits an applet-mode launch; launching with title takeover gives
  more headroom.
