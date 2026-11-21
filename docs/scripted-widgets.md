# Scripted widgets and responsive wallpapers

In **Creator**, add **Scripted widget**, or select any image, video, text or clock
layer and choose **Edit layer script**. JavaScript is provided by Qt's existing
QJSEngine; Lua is not required. Apply the script to preview it, then save to library.
The script editor enables the data capabilities that the project uses and accepts
JSON declarations for custom D-Bus property sources. Runtime errors appear below
the preview. Scripts travel inside `wallpaper.json`, including in catalog installs.

Open [`examples/widgets/wallpaper.json`](../examples/widgets/wallpaper.json) in Creator
for a working media card, playback progress, pointer animation, and UPower battery
readout. The file has no external assets. Try the host directly with:

```sh
wpe-wallpaper-host --kind native --path /absolute/path/to/examples/widgets/wallpaper.json --placement x11-desktop
```

Choose the placement appropriate to your compositor, as for other native wallpapers.

## Author API

Scripted projects use `"version": 2`; existing version 1 data-only wallpapers still
work. A layer's optional `script` string is a **function expression**:

```js
function(ctx) {
    return {
        text: ctx.media.title || 'Nothing playing',
        x: ctx.mouse.x * 0.5,
        rotation: Math.sin(ctx.time * 2) * 8,
        background: '#243449',
        radius: 24
    };
}
```

The function is called at approximately 30 frames per second while the wallpaper
or preview is visible. Any layer can animate its transforms and opacity. Widget
layers additionally render backgrounds, text, and `elements` containing `rect` or
`text` primitives. No custom QML is loaded.

| Context field | Meaning |
|---|---|
| `time`, `delta` | Seconds since document load and previous update (delta capped at 0.25 s) |
| `width`, `height` | Current canvas size in logical pixels |
| `mouse` | `{x, y, available, inside, pressed}`; x/y are normalized canvas coordinates |
| `media` | Selected MPRIS player, or `{}` when none is available |
| `dbus` | Values from declared read-only sources, keyed by their `id` |

`media` has `service`, `title`, `artist`, `album`, `status`, `playing`, `position`
and `duration`. Times are seconds. A playing player takes priority over paused or
stopped players; ties are resolved by service name. MPRIS services are discovered
and polled asynchronously about once a second. No playerctl process is required.
Position is the player's latest reported value, not a sample-accurate audio clock.
The bridge uses the standard [MPRIS Player interface](https://specifications.freedesktop.org/mpris/latest/Player_Interface.html).
Album-art URLs are not fetched by this API.

Scripts retain their own globals and closures until the document changes. A closure
can keep animation state:

```js
(function() {
    let angle = 0;
    return function(ctx) {
        angle += ctx.delta * 20;
        return {rotation: angle, text: 'Rotating'};
    };
})()
```

The context is a fresh snapshot each call; changing it does not change the host.
Each scripted layer has a separate engine. Hiding a preview stops its frame and
D-Bus polling timers. Document edits reset script state and data declarations.

## Visual output

Return an object with any of these fields; omitted fields use the original layer
values. Output is filtered and numeric values are clamped. Static layer geometry
in the manifest retains its original 0..1 validation.

| Field | Units / accepted range |
|---|---|
| `x`, `y` | Relative to canvas, -2..2 (offscreen animation allowed) |
| `width`, `height` | Relative to canvas, 0.001..4 |
| `opacity` | 0..1 |
| `rotation` | Degrees, -36000..36000 |
| `scale` | 0.001..10 |
| `text` | Plain text, up to 8192 characters |
| `color`, `background` | Valid Qt colour strings |
| `fontSize`, `radius` | 1080-pixel reference display units, 1..512 / 0..512 |
| `elements` | Up to 128 rect/text objects, for `widget` layers |

Elements have `type: 'rect'` or `type: 'text'` and the same primitive visual fields.
Their x/y/width/height are relative to the widget bounds. They are drawn in array
order above the widget's own text/background. Element scale and nested elements
are not supported. To animate an image or video, add a script to that media layer;
its bundled asset stays fixed. Scripts cannot substitute asset paths or fetch URLs.

## Capabilities and D-Bus

Declare needed inputs at the document's top level:

```json
"capabilities": ["media", "pointer", "dbus"],
"dbusSources": [{
  "id": "battery",
  "bus": "system",
  "service": "org.freedesktop.UPower",
  "path": "/org/freedesktop/UPower/devices/DisplayDevice",
  "interface": "org.freedesktop.UPower.Device",
  "property": "Percentage"
}]
```

Then read `ctx.dbus.battery`. `bus` is `session` (default) or `system`. Up to 16
sources are polled asynchronously, once per second, using **Properties.Get** with
a one-second timeout. These declarations grant no method calls, property writes,
arbitrary bus addresses or shell commands. Bus policy still applies. Scalars,
string arrays and variant dictionaries are supported; unsupported values or failed
reads become missing values. Handle unavailable services in your script.

Without `media`, no media discovery occurs. Without `dbus`, no custom source is
read. Without `pointer`, mouse input remains unavailable with centered coordinates.
These declarations control the bridge; they are not an OS-level permission sandbox.

## Mouse behaviour

The editor preview supports local hover and left-button state. On **X11**, the
host queries the global cursor and left-button state while its wallpaper window
remains click-through. Animations can react even when another application is above
the wallpaper.

On **Wayland**, pointer-enabled wallpapers receive local pointer events over exposed
background areas and do not take keyboard focus. Compositor policy determines which
background areas are available. This implementation does **not** read the pointer
through other applications on Wayland; that requires compositor-specific APIs.
Projects without pointer capability keep the original input-transparent host window.

## Execution limits

Only JavaScript values are exposed. Scripts receive no QObject, Qt namespace,
filesystem, process execution, network request or direct D-Bus API. Allowed visual
fields are copied explicitly; arbitrary returned objects never enter the QML engine.
A separate watchdog uses [QJSEngine interruption](https://doc.qt.io/qt-6/qjsengine.html#setInterrupted)
to disable a script exceeding 12 ms during initialization or one update, including
property accessors. Other layers continue to render and errors appear in Creator.
Script source is limited to 64 KiB per layer; the complete manifest to 1 MiB.

This is a constrained scripting API, **not a hardened process/memory sandbox**.
QJSEngine does not enforce a per-script heap limit here. A script that allocates
excessive memory can still exhaust the player or editor process. Catalog integrity
hashes do not establish that a publisher's code is trustworthy.

## Verification

Tests exercise animation/pointer context, closure state, engine isolation, unavailable
capabilities, syntax/errors/infinite loops (including getters), output filtering,
script save/reopen, and rendered rect primitives. Isolated D-Bus tests provide fake
MPRIS players and a custom property, checking priority, metadata, updates, lost
services, and stale replies. Real desktop pointer routing and individual media
players still need testing on their target compositors.
