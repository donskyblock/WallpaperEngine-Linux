# Wallpaper Engine for Linux: native C++/Qt rewrite

A ground-up rewrite of the Tauri/React app in C++20 and Qt 6. No webview, no Node, no Rust toolchain.
The UI is Qt Quick Controls driven by your system palette and icon theme, and wallpaper placement is chosen
per compositor instead of assuming Hyprland.

Scene and Web wallpapers are still rendered by [linux-wallpaperengine](https://github.com/Almamu/linux-wallpaperengine)
(already C++). This app orchestrates it; a from-scratch scene renderer is a separate multi-year project.

## Independent wallpapers (no Steam required)

Import your own images/videos in **My Library**, build layered wallpapers in
**Creator**, or load an HTTPS metadata catalog in **Community**. Creator supports
image/video layers, text/clock widgets, and custom JavaScript widgets with D-Bus
media data and pointer animations, using a shared Qt preview and native player. Steam Workshop remains an optional source for compatible wallpapers.

See [native wallpaper format, creator, catalog publishing and limits](docs/native-wallpapers.md).
See [scripted widget API and examples](docs/scripted-widgets.md).
The metadata-only catalog starter is [catalog/catalog.json](catalog/catalog.json).

## What runs where

| Session | Still images | Video | Scene / Web (via linux-wallpaperengine) |
|---|---|---|---|
| **Hyprland, Niri, Sway, mango, River, labwc, Wayfire, COSMIC** | host window, layer-shell | host window, layer-shell | `--screen-root` (layer-shell) |
| **KDE Plasma** | native, per-output via plasmashell | bundled Plasma wallpaper plugin | KWin script pins the window |
| **GNOME / Budgie / Pantheon (Wayland)** | native `gsettings` | host window + bundled Shell extension | same, via extension |
| **GNOME (X11)** | native `gsettings` | X11 desktop window | X11 `--screen-root` |
| **XFCE** | native `xfconf` | X11 desktop window | X11 `--screen-root` |
| **Cinnamon, MATE** | native `gsettings` | X11 desktop window | X11 `--screen-root` |
| **i3, bspwm, openbox, awesome, dwm, LXQt, any EWMH WM** | host window, X11 | X11 desktop window | X11 `--screen-root` |

The mapping lives in one place, `planFor()` in `src/core/environment.cpp`, and is unit-tested.

## Verification status

| Piece | Status |
|---|---|
| Environment detection and placement plan (Hyprland, Niri, Sway, mango, Plasma, GNOME, XFCE, Cinnamon, MATE, generic) | Unit tests pass |
| Library scanning and classification | Unit tests pass |
| QML UI (Workshop, Library, Settings) | Built, run, screenshotted headless |
| Single instance + D-Bus API + `wpectl` + state persistence | Run end to end under `dbus-run-session` |
| Children die with the app (`PR_SET_PDEATHSIG`) | Verified |
| **X11 desktop window** (type DESKTOP, below, skip taskbar/pager, all desktops, monitor-sized) | Verified on Xvfb + Openbox |
| **Wayland layer-shell**, two outputs with different sizes | Verified on headless Sway |
| Video playback | **Not tested**: no Qt Multimedia backend in the build sandbox |
| Hyprland / Niri / mango specifically | Not run; they implement the same protocol as Sway |
| **GNOME Shell extension** | **Written, never run on real GNOME** |
| **Plasma wallpaper plugin + KWin pinning script + `evaluateScript` still-image path** | **Written, never run on real Plasma** |
| Native still-image setters (`gsettings`, `xfconf`) | Written, not run (no GNOME/XFCE session available) |
| steamcmd in-app login (password and Steam Guard prompt matching) | **Written, never run against real steamcmd** |
| `linux-wallpaperengine` flags (`--window`, `--fps`, `--screen-root`, ...) | Based on the old app plus upstream docs; verify with `linux-wallpaperengine --help` |

## Build

Dependencies: Qt 6.4+ (Base, Declarative, Multimedia, Wayland, Svg), CMake 3.21+, a C++20 compiler,
`libxcb`, Ninja, and **layer-shell-qt built for Qt 6** (Arch `layer-shell-qt`, Fedora/Void `layer-shell-qt-devel`,
Debian 13+/Ubuntu 25.04+ `liblayershellqtinterface-dev`).
For a build that does not use Wayland layer-shell placement, pass `-DWPE_REQUIRE_LAYER_SHELL=OFF` to `build.sh`.
For KDE-native styling also install `qqc2-desktop-style`.
Qt 6.5+ is recommended: it follows the GNOME dark/light setting and accent colour through the portal.

```sh
./build.sh
ctest --test-dir build --output-on-failure
sudo ./install.sh
```

The default installation prefix is `/usr/local` for a new build. Set
`-DCMAKE_INSTALL_PREFIX=/path` when running `build.sh` to change it.
Both scripts accept `WPE_BUILD_DIR` for an alternate build directory. Use
`DESTDIR=/path ./install.sh` to stage the installation.

Runtime tools, all optional: `linux-wallpaperengine` (scenes), `steamcmd` (downloads).

## Release

Run `./release-maker.sh` to build and test a fresh release and create a source
tarball, a Linux binary tarball for the build machine's architecture, and a SHA-256
checksum file in `release/`. Pass a directory as the first argument to write the
artifacts elsewhere. The binary tarball uses the `/usr` layout and needs compatible
Qt 6, layer-shell-qt, and XCB runtime libraries on the target machine.

## Using it

```sh
wpe-linux                       # window
wpe-linux --background          # tray only; restores your last wallpapers
wpectl set ~/Videos/loop.mp4 -o DP-1
wpectl random                   # random wallpaper from the library
wpectl stop
wpectl env                      # what was detected and which strategy will be used
```

Autostart and key binds:

```
# Hyprland           exec-once = wpe-linux --background        bind = SUPER, W, exec, wpectl random
# Niri (config.kdl)  spawn-at-startup "wpe-linux" "--background"    Mod+W { spawn "wpectl" "random"; }
# Sway / i3          exec wpe-linux --background               bindsym $mod+w exec wpectl random
# mango              exec-once=wpe-linux --background          bind=SUPER,w,spawn,wpectl random
# Anything else      systemctl --user enable --now wpe-linux.service
```

GNOME: Settings → Components → install the Shell extension, then log out and in once.
Plasma: Settings → Components → install the Plasma plugin.

## Migrating from the Tauri version

`~/.config/wpe-linux/settings.json` and the download directory layout are unchanged, so existing installs and
downloaded wallpapers carry over. Wallpapers you subscribed to in the real Steam client are picked up too.
Behaviour differences:

- **Closing the window hides to the tray**, as before. **Quit** now stops the wallpapers (they are children of the app and die with it), and they are restored on next start.
- No `awww`, `mpvpaper`, `hyprctl`, `xdotool` or `wmctrl` dependency. Images and video are drawn by the bundled host.
- The terminal-emulator hack for Steam login is gone; sign-in happens in a dialog.
- Scripts in `scripts/` and the npm/Cargo files are obsolete.

## Layout

```
src/core   environment detection, placement plan, settings, library, Workshop API, steamcmd, WallpaperManager
src/host   wpe-wallpaper-host: one window per output (layer-shell / X11 desktop / plain window for GNOME+KWin)
src/app    wpe-linux: Qt Quick UI, D-Bus API (org.wpelinux.WallpaperEngine1), tray
src/cli    wpectl
extensions gnome-shell/  kwin (pinning is generated at runtime)  plasma/org.wpelinux.video
tests      environment + library unit tests
```

## Known gaps and next steps

1. Pause when a window is fullscreen/maximised, or on battery (lwe has its own fullscreen pause; the host has none).
2. Store the Steam API key in the Secret Service / KWallet instead of plaintext JSON (file is chmod 600 for now).
3. Multiple lwe scenes under Plasma on several monitors: pinning is per-PID and untested.
4. Web wallpapers rely on an lwe build with CEF.
5. Per-monitor still images on GNOME are impossible through `gsettings`; they apply to all monitors.
6. Packaging: PKGBUILD / RPM spec / Flatpak (Flatpak needs `--talk-name` rules for D-Bus and host spawning of lwe).
