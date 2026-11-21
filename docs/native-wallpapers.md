# Independent wallpapers

Steam, a Steam API key, Wallpaper Engine assets and linux-wallpaperengine are **optional**.
Use **My Library → Import media** for images/videos, **Creator** for layered native
wallpapers, or **Community** for an HTTPS catalog. Media is copied into
`<download_dir>/local/<uuid>/`, so moving the original file will not break playback.
Existing Workshop content remains supported through its existing renderer.

## Creator and player

Add image, video, text and clock layers, select a layer, drag to position, and adjust
its relative position, size, opacity, font and colour. Layers are drawn bottom to top;
Raise moves a selected layer toward the front. Clock text uses Qt date/time formats
(e.g. `hh:mm`, `dddd, d MMMM`). Save to library, then Apply to the selected output.
Open `wallpaper.json` to edit an existing project; saving makes a new library copy.
The folder button opens the saved project for sharing. Share the entire folder.

The bundled Qt host plays native projects and uses the **same NativeCanvas component**
as the editor preview. Existing video playback is reused for plain videos. Native
projects do not go through linux-wallpaperengine and do not load Steam assets.
Version 2 adds custom JavaScript widgets, read-only D-Bus/MPRIS data, and pointer
animations. See [scripted widgets](scripted-widgets.md) for the API and examples.
Wallpaper Engine scene compatibility and third-party QML plugins are not implemented.
Clock widgets update once per second.

`wallpaper.json` is a data-only document:

```json
{
  "format": "wpe-native",
  "version": 1,
  "title": "Mountain clock",
  "background": "#101820",
  "layers": [
    {"type": "image", "source": "mountain.jpg", "x": 0, "y": 0, "width": 1, "height": 1},
    {"type": "clock", "text": "hh:mm", "x": 0.2, "y": 0.2, "width": 0.6, "height": 0.2,
     "fontSize": 96, "color": "#ffffff", "opacity": 0.9}
  ]
}
```

Coordinates, dimensions and opacity are fractions from 0 to 1. Font sizes use a
1080-pixel reference display height. Assets must be local relative paths inside the
project; missing assets, traversal and symlinks outside the project are rejected.
Both versions permit up to 64 layers and a 1 MiB manifest. Version 1 remains
data-only; version 2 supports the constrained JavaScript widget API. No custom QML
is loaded.

## Metadata-only community repository

`catalog/catalog.json` is a ready-to-publish empty catalog. It deliberately contains
no wallpaper payloads. Put it in a dedicated GitHub repository and use its raw HTTPS
URL in Community. Until a repository URL is chosen there is no default remote catalog.
Only catalog metadata belongs in that repository; creators host each project file
on their own CDN/object storage. This application does not upload or host media.
Publishers must provide stable direct HTTPS file URLs, byte counts and SHA-256 hashes.

A catalog is versioned independently:

```json
{
  "version": 1,
  "wallpapers": [{
    "id": "creator.mountain-clock",
    "title": "Mountain clock",
    "author": "Creator",
    "license": "CC-BY-4.0",
    "description": "A mountain photo and clock.",
    "type": "native",
    "files": [
      {"path": "wallpaper.json", "url": "https://cdn.example/mountain/wallpaper.json",
       "bytes": 123, "sha256": "REPLACE_WITH_64_HEX_DIGITS"},
      {"path": "mountain.jpg", "url": "https://cdn.example/mountain/mountain.jpg",
       "bytes": 456, "sha256": "REPLACE_WITH_64_HEX_DIGITS"}
    ]
  }]
}
```

The example sizes and hashes are placeholders. For an image/video entry set `type`
to `image`/`video` and `entry` to its relative media filename. Include that file in
`files`. Native entries must include `wallpaper.json` plus all referenced assets.
Generate a real entry using `python3 tools/catalog-entry.py PROJECT_DIR HTTPS_BASE_URL
--id creator.name --author NAME --license LICENSE` and add its output to `wallpapers`.

Downloads require HTTPS, limit catalogs to 4 MiB / 2000 entries, projects to 128
files / 1 GiB and each file to 256 MiB. Files download sequentially; at most one
file is buffered in memory. Every file must match its declared size and SHA-256.
Projects are validated in a temporary directory and atomically moved into the
library only on success. A failed install removes its staging files. Hashes check
integrity against the catalog; they do not authenticate an untrusted catalog owner.
Changing the catalog URL is explicit; no Steam account is used for this feature.

## Validation status

Core tests cover catalog installation and hash-mismatch cleanup using a simulated
network, portable imports, native save/open/resave, classification,
path traversal/symlink rejection, failed-save cleanup and Steam-free onboarding.
An offscreen UI test adds image/clock layers, checks rendered image pixels, saves
and reopens the portable project.
Real compositor placement, audio/video backend behaviour and live CDN installations
must also be checked on the target desktop; headless checks cannot prove those.
