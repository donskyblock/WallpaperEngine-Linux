# Wallpaper Engine for Linux

A native Linux app for animated desktop wallpapers. Use your own images and videos, build layered wallpapers with clocks and widgets, or bring in compatible Steam Workshop wallpapers.

[Installation](#installation) · [Quick start](#quick-start) · [Desktop compatibility](#desktop-compatibility) · [Catalogue](#catalogue) · [Contributing](#contributing)

- **My Library** imports local media and applies wallpapers to the selected display where your desktop supports it.
- **Creator** combines image, video, text and clock layers with a live preview. Native projects can also use [scripted widgets](docs/scripted-widgets.md).
- **Community** installs wallpapers from independent HTTPS catalogues. Local media, Creator and Community all work without Steam.
- **Workshop** browses and downloads compatible Steam Workshop items. **`wpectl`** lets you switch wallpapers from a terminal, script or keyboard shortcut.

## Installation

Build with a C++20 compiler, CMake 3.21+, Ninja, pkg-config and Qt 6.4+ development packages, including Qt Base, Declarative and Multimedia. Install the Qt Wayland runtime for Wayland sessions and the XCB development package for X11 wallpaper placement.

For Wayland compositors that use layer-shell, install the **Qt 6** build of layer-shell-qt.

| Distribution | Layer-shell development package |
| --- | --- |
| Arch Linux | `layer-shell-qt` |
| Fedora and Void Linux | `layer-shell-qt-devel` |
| Debian and Ubuntu, where packaged | `liblayershellqtinterface-dev` |

```sh
git clone https://github.com/donskyblock/WallpaperEngine-Linux.git
cd WallpaperEngine-Linux
./build.sh
ctest --test-dir build --output-on-failure
sudo ./install.sh
```

The default install prefix is `/usr/local`. Pass `-DCMAKE_INSTALL_PREFIX=/path` to `./build.sh` to use a different prefix. Set `WPE_BUILD_DIR` for both scripts to use a different build directory. If your desktop uses another placement backend and layer-shell-qt is unavailable, configure with `./build.sh -DWPE_REQUIRE_LAYER_SHELL=OFF`.

Workshop browsing uses a Steam Web API key, downloads use `steamcmd`, and Scene and Web wallpapers use [linux-wallpaperengine](https://github.com/Almamu/linux-wallpaperengine). Web wallpapers require a renderer build with CEF support.

## Quick start

1. Launch **Wallpaper Engine** from your application menu or run `wpe-linux`.
2. Open **My Library → Import media** and choose an image or video.
3. Select the wallpaper and a display, then click **Apply**. To build your own wallpaper, open **Creator** and save your project to the library.

Closing the window hides the app to the tray. **Quit** stops the app's wallpaper players. To restore your last wallpapers at login, add `wpe-linux --background` to your desktop or compositor's autostart.

```sh
wpectl list
wpectl set "$HOME/Videos/loop.mp4" -o DP-1
wpectl random
wpectl status
wpectl stop
```

Use a connector name from `wpectl list` in place of `DP-1`. Run `wpectl env` to see which desktop and placement strategy the app detected.

## Desktop compatibility

| Session | Wallpaper placement |
| --- | --- |
| Wayland with layer-shell, including Hyprland, Niri, Sway, River and COSMIC | The bundled host places images, videos and native projects on the desktop. Scene and Web wallpapers use the external renderer. |
| KDE Plasma | Uses Plasma's native image settings, the bundled wallpaper plugin for video and KWin window placement for Scene and Web wallpapers. |
| GNOME on Wayland | Uses the desktop background setting for images and the bundled Shell extension for live wallpaper placement. The image setting applies across displays. |
| X11, including XFCE, Cinnamon, MATE and EWMH window managers | Uses desktop settings where available and X11 desktop windows for live wallpapers. |

On Plasma, install the wallpaper plugin from **Settings → Components**. On GNOME Wayland, install the Shell extension from the same panel and sign out and back in.

Layer-shell placement has been exercised with headless Sway and X11 placement with Openbox on Xvfb. Video playback depends on the Qt Multimedia backend. GNOME Shell and Plasma integrations still need verification in real desktop sessions.

## Catalogue

The **Community** tab reads an HTTPS catalogue with entries for images, videos and native wallpaper projects. Each entry lists the creator, licence and files to download. The catalogue holds metadata, and creators host the actual wallpaper files separately.

### Browse and install

1. Open **Community**, paste the direct HTTPS URL of a catalogue's JSON file and click **Load catalog**.
2. Choose a wallpaper and click **Add to library**. The app checks each file's byte count and SHA-256 hash during installation.
3. Open **My Library** to apply the wallpaper.

The app ships without a default catalogue URL. [`catalog/catalog.json`](catalog/catalog.json) is an empty template you can publish and populate.

### Publish a catalogue

1. Host the wallpaper files on HTTPS storage you control. Use direct file URLs and keep the project's relative paths. A native project needs its `wallpaper.json` and every referenced media file.
2. From the repository root, use Python 3.9+ to generate an entry. Replace the example path, URL, author and licence with your project's details.

   ```sh
   python3 tools/catalog-entry.py ./mountain-clock https://cdn.example.com/mountain-clock/ \
     --id alex.mountain-clock --author "Alex" --license CC-BY-4.0
   ```

3. Add the generated JSON object to the `wallpapers` array in a copy of the template. Publish the JSON over HTTPS and share its direct URL. Load it in **Community** and install the entry to check that the hosted files match the metadata.

Use a unique ID per wallpaper and a licence that covers the files you distribute. See the [catalogue schema, limits and publishing notes](docs/native-wallpapers.md#metadata-only-community-repository).

## Contributing

Bug reports, compositor testing, fixes and documentation improvements are welcome. For a wallpaper you want to share, use the publishing workflow in the [Catalogue](#catalogue) section.

- [Report a bug or propose a change](https://github.com/donskyblock/WallpaperEngine-Linux/issues).
- [Read the contribution guide](CONTRIBUTING.md) for the development workflow and what to include in a pull request.
- [Explore the native wallpaper format](docs/native-wallpapers.md) and the [widget API](docs/scripted-widgets.md).

## Licence

Wallpaper Engine for Linux is licensed under [GPL-3.0-or-later](LICENSE). See [NOTICE](NOTICE) for copyright information.
