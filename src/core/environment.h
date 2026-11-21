// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QProcessEnvironment>
#include <QString>
#include <QStringList>
#include <functional>

namespace wpe {

enum class SessionType { Wayland, X11, Unknown };

enum class Desktop {
    Hyprland, Niri, Sway, Mango, River, Labwc, Wayfire, Cosmic, // wlroots-style compositors
    Plasma, Gnome, Pantheon,                                    // compositors without layer-shell semantics we can use
    Xfce, Cinnamon, Mate, Budgie, LXQt, Deepin,                 // classic X11 desktops
    GenericWayland, GenericX11, Unknown
};

// How an *animated* wallpaper window (our host, or linux-wallpaperengine) gets
// underneath everything else on the desktop.
enum class Placement {
    LayerShell,         // wlr-layer-shell BACKGROUND layer
    X11Desktop,         // _NET_WM_WINDOW_TYPE_DESKTOP + below/sticky
    GnomeShellExtension,// our extension reparents the window into Shell's background group
    KWinScript          // KWin script forces below/borderless/geometry (Plasma)
};

// How a *still image* gets applied.
enum class StaticMode {
    HostWindow,         // our host process draws it (layer-shell / X11 desktop window)
    GSettingsGnome,     // org.gnome.desktop.background (GNOME, Budgie, Pantheon)
    GSettingsCinnamon,
    GSettingsMate,
    Plasma,             // plasmashell scripting over D-Bus (per-output)
    Xfce                // xfconf /backdrop/...
};

struct Environment {
    SessionType session = SessionType::Unknown;
    Desktop desktop = Desktop::Unknown;
    QString rawDesktop; // XDG_CURRENT_DESKTOP as given
};

struct Plan {
    StaticMode still = StaticMode::HostWindow;
    Placement window = Placement::LayerShell;
    bool plasmaVideoPlugin = false; // video via our Plasma wallpaper plugin instead of a window
    QString summary;                // one-liner for the UI
    QStringList caveats;            // things the user should know
};

using ProcessProbe = std::function<bool(const QString &comm)>;

bool defaultProcessRunning(const QString &comm);
Environment detectEnvironment(const QProcessEnvironment &env = QProcessEnvironment::systemEnvironment(),
                              const ProcessProbe &processRunning = defaultProcessRunning);
Plan planFor(const Environment &env);

QString desktopName(Desktop d);
QString sessionName(SessionType s);
QString placementName(Placement p);
bool parsePlacement(const QString &s, Placement *out);

} // namespace wpe
