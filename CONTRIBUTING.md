# Contributing to Wallpaper Engine for Linux

Bug fixes, documentation, and testing on Linux desktops and compositors are welcome. For a change that affects several desktops, the wallpaper format or the catalogue schema, open an issue to discuss the approach before starting a large pull request.

## Report an issue

Search the [issue tracker](https://github.com/donskyblock/WallpaperEngine-Linux/issues) for an existing report. Include enough detail for someone else to reproduce the problem.

- App version or Git commit, Linux distribution, desktop or compositor version and whether the session is Wayland or X11.
- Wallpaper type and source, steps to reproduce, what you expected and what happened.
- Output from `wpectl env` for wallpaper placement issues. If displays are involved, include `wpectl list` and the affected connector or display arrangement.
- Relevant logs or screenshots where they clarify the issue. Remove credentials, Steam API keys and private catalogue URLs.

## Build and test

Fork the repository and create a branch from `main`. Install the [build dependencies](README.md#installation), then run from the repository root.

```sh
./build.sh
ctest --test-dir build --output-on-failure
./build/bin/wpe-linux
```

The test suite covers desktop detection, library handling, native projects, Creator, widgets and catalogue installation. For changes to wallpaper placement, playback or desktop integrations, try the affected path in a real session when possible. Record the desktop, session type, wallpaper type and what you verified.

| Path | What lives there |
| --- | --- |
| `src/app/` | Qt Quick UI, application controller and D-Bus API |
| `src/core/` | Library, settings, Workshop, catalogue and placement logic |
| `src/host/` and `src/shared/` | Wallpaper player and shared Creator canvas |
| `src/cli/` | The `wpectl` command |
| `extensions/` and `tests/` | Desktop integrations and automated tests |

For release packaging, run `./release-maker.sh`. It builds and tests in a fresh directory and writes a source archive, a binary archive for the build machine's architecture and SHA-256 checksums to `release/` by default. Pass an output directory to override it. The binary archive uses a `/usr` layout and requires compatible runtime libraries.

## Open a pull request

- Keep the change focused and follow the surrounding C++, QML or documentation style.
- Describe the user-visible change and link any related issue. Include a screenshot or short recording for visual changes when it helps explain the result.
- List the checks you ran and any affected desktop or workflow you could not verify.
- Update the relevant documentation when commands, settings, formats or workflows change.

## Share a wallpaper

Follow the [catalogue publishing guide](README.md#catalogue) to host the media and generate its entry. Use files you have permission to redistribute and identify their licence in the metadata. Keep wallpaper media on your hosting provider so the catalogue can stay lightweight.
