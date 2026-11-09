// SPDX-License-Identifier: GPL-3.0-or-later
#include "environment.h"

#include <QDir>
#include <QFile>

namespace wpe {

bool defaultProcessRunning(const QString &comm)
{
    const auto entries = QDir(QStringLiteral("/proc")).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &e : entries) {
        bool ok = false;
        e.toInt(&ok);
        if (!ok) continue;
        QFile f(QStringLiteral("/proc/%1/comm").arg(e));
        if (f.open(QIODevice::ReadOnly) && QString::fromUtf8(f.readAll()).trimmed() == comm)
            return true;
    }
    return false;
}

static bool hasToken(const QStringList &tokens, std::initializer_list<const char *> names)
{
    for (const char *n : names)
        if (tokens.contains(QLatin1String(n))) return true;
    return false;
}

Environment detectEnvironment(const QProcessEnvironment &env, const ProcessProbe &running)
{
    Environment out;
    out.rawDesktop = env.value(QStringLiteral("XDG_CURRENT_DESKTOP"));

    // --- session type -------------------------------------------------------
    const QString st = env.value(QStringLiteral("XDG_SESSION_TYPE")).toLower();
    if (st == "wayland") out.session = SessionType::Wayland;
    else if (st == "x11") out.session = SessionType::X11;
    else if (!env.value(QStringLiteral("WAYLAND_DISPLAY")).isEmpty()) out.session = SessionType::Wayland;
    else if (!env.value(QStringLiteral("DISPLAY")).isEmpty()) out.session = SessionType::X11;

    // --- desktop / compositor ----------------------------------------------
    QStringList tokens = out.rawDesktop.toLower().split(QLatin1Char(':'), Qt::SkipEmptyParts);
    tokens += env.value(QStringLiteral("DESKTOP_SESSION")).toLower();

    // Compositor-specific variables are the most reliable signal, check them first.
    if (!env.value(QStringLiteral("HYPRLAND_INSTANCE_SIGNATURE")).isEmpty()) out.desktop = Desktop::Hyprland;
    else if (!env.value(QStringLiteral("NIRI_SOCKET")).isEmpty()) out.desktop = Desktop::Niri;
    else if (!env.value(QStringLiteral("SWAYSOCK")).isEmpty()) out.desktop = Desktop::Sway;
    else if (hasToken(tokens, {"hyprland"})) out.desktop = Desktop::Hyprland;
    else if (hasToken(tokens, {"niri"})) out.desktop = Desktop::Niri;
    else if (hasToken(tokens, {"sway"})) out.desktop = Desktop::Sway;
    else if (hasToken(tokens, {"mango", "mangowc", "maomao", "maomaowm"})) out.desktop = Desktop::Mango;
    else if (hasToken(tokens, {"river"})) out.desktop = Desktop::River;
    else if (hasToken(tokens, {"labwc"})) out.desktop = Desktop::Labwc;
    else if (hasToken(tokens, {"wayfire"})) out.desktop = Desktop::Wayfire;
    else if (hasToken(tokens, {"cosmic"})) out.desktop = Desktop::Cosmic;
    else if (hasToken(tokens, {"kde", "plasma", "plasmawayland", "plasmax11"})) out.desktop = Desktop::Plasma;
    else if (hasToken(tokens, {"gnome", "gnome-classic", "gnome-xorg", "ubuntu", "unity"})) out.desktop = Desktop::Gnome;
    else if (hasToken(tokens, {"pantheon"})) out.desktop = Desktop::Pantheon;
    else if (hasToken(tokens, {"xfce", "xfce4"})) out.desktop = Desktop::Xfce;
    else if (hasToken(tokens, {"x-cinnamon", "cinnamon"})) out.desktop = Desktop::Cinnamon;
    else if (hasToken(tokens, {"mate"})) out.desktop = Desktop::Mate;
    else if (hasToken(tokens, {"budgie", "budgie-desktop"})) out.desktop = Desktop::Budgie;
    else if (hasToken(tokens, {"lxqt"})) out.desktop = Desktop::LXQt;
    else if (hasToken(tokens, {"deepin", "dde"})) out.desktop = Desktop::Deepin;

    // Bare compositors often don't set XDG_CURRENT_DESKTOP at all (mango, river, dwl forks...).
    if (out.desktop == Desktop::Unknown && out.session == SessionType::Wayland && running) {
        if (running(QStringLiteral("mango")) || running(QStringLiteral("mangowc"))) out.desktop = Desktop::Mango;
        else if (running(QStringLiteral("river"))) out.desktop = Desktop::River;
        else if (running(QStringLiteral("labwc"))) out.desktop = Desktop::Labwc;
        else if (running(QStringLiteral("wayfire"))) out.desktop = Desktop::Wayfire;
    }

    if (out.desktop == Desktop::Unknown)
        out.desktop = out.session == SessionType::Wayland ? Desktop::GenericWayland
                    : out.session == SessionType::X11     ? Desktop::GenericX11
                                                          : Desktop::Unknown;
    return out;
}

Plan planFor(const Environment &e)
{
    Plan p;
    const bool wayland = e.session == SessionType::Wayland;
    const QString name = desktopName(e.desktop);

    switch (e.desktop) {
    // ---- layer-shell compositors: one code path, fully native ------------------
    case Desktop::Hyprland: case Desktop::Niri: case Desktop::Sway: case Desktop::Mango:
    case Desktop::River: case Desktop::Labwc: case Desktop::Wayfire: case Desktop::Cosmic:
    case Desktop::GenericWayland:
        p.window = Placement::LayerShell;
        p.still = StaticMode::HostWindow;
        p.summary = QStringLiteral("%1 · wlr-layer-shell").arg(name);
        if (e.desktop == Desktop::GenericWayland)
            p.caveats << QStringLiteral("Unrecognised Wayland compositor: assuming it implements wlr-layer-shell. "
                                        "Override in Settings → Placement if wallpapers don't appear.");
        break;

    // ---- Plasma ------------------------------------------------------------------
    case Desktop::Plasma:
        p.still = StaticMode::Plasma;
        p.plasmaVideoPlugin = true;
        p.window = Placement::KWinScript;
        p.summary = QStringLiteral("KDE Plasma · native wallpaper plugin (video), KWin rules (scenes)");
        p.caveats << QStringLiteral("Video uses the bundled Plasma wallpaper plugin, so desktop icons and widgets keep working.")
                  << QStringLiteral("Scene wallpapers run as a KWin-managed window below everything else and can cover desktop icons.");
        break;

    // ---- GNOME-family (Mutter has no layer-shell) -----------------------------
    case Desktop::Gnome: case Desktop::Pantheon: case Desktop::Budgie:
        p.still = StaticMode::GSettingsGnome;
        if (wayland) {
            p.window = Placement::GnomeShellExtension;
            p.summary = QStringLiteral("%1 · Wayland · Shell extension").arg(name);
            p.caveats << QStringLiteral("Animated wallpapers need the bundled GNOME Shell extension (Settings → Install). "
                                        "Log out and back in once after installing it.")
                      << QStringLiteral("Still images are applied through GNOME's own background settings (all monitors share one image).");
        } else {
            p.window = Placement::X11Desktop;
            p.summary = QStringLiteral("%1 · X11 · desktop-type window").arg(name);
        }
        if (e.desktop != Desktop::Gnome && wayland)
            p.caveats << QStringLiteral("%1 on Wayland is only tested through its GNOME-compatible shell hooks; expect rough edges.").arg(name);
        break;

    // ---- X11 desktops -----------------------------------------------------------
    case Desktop::Xfce:
        p.still = StaticMode::Xfce; p.window = Placement::X11Desktop;
        p.summary = QStringLiteral("XFCE · X11 · desktop-type window");
        p.caveats << QStringLiteral("For animated wallpapers set xfdesktop's background to \"None\", otherwise it draws over/under ours; desktop icons may be covered.");
        break;
    case Desktop::Cinnamon:
        p.still = StaticMode::GSettingsCinnamon; p.window = Placement::X11Desktop;
        p.summary = QStringLiteral("Cinnamon · X11 · desktop-type window"); break;
    case Desktop::Mate:
        p.still = StaticMode::GSettingsMate; p.window = Placement::X11Desktop;
        p.summary = QStringLiteral("MATE · X11 · desktop-type window"); break;
    case Desktop::LXQt: case Desktop::Deepin: case Desktop::GenericX11:
        p.still = StaticMode::HostWindow; p.window = Placement::X11Desktop;
        p.summary = QStringLiteral("%1 · X11 · desktop-type window").arg(name);
        if (e.desktop == Desktop::GenericX11)
            p.caveats << QStringLiteral("Any EWMH-compliant window manager (i3, bspwm, openbox, awesome, dwm …) works. "
                                        "If you run a desktop-icon manager or pcmanfm --desktop, disable its wallpaper drawing.");
        break;

    case Desktop::Unknown:
        p.window = Placement::LayerShell; p.still = StaticMode::HostWindow;
        p.summary = QStringLiteral("Unknown session");
        p.caveats << QStringLiteral("Could not detect a graphical session (no XDG_SESSION_TYPE, WAYLAND_DISPLAY or DISPLAY).");
        break;
    }
    return p;
}

QString desktopName(Desktop d)
{
    switch (d) {
    case Desktop::Hyprland: return QStringLiteral("Hyprland");
    case Desktop::Niri: return QStringLiteral("Niri");
    case Desktop::Sway: return QStringLiteral("Sway");
    case Desktop::Mango: return QStringLiteral("mango");
    case Desktop::River: return QStringLiteral("River");
    case Desktop::Labwc: return QStringLiteral("labwc");
    case Desktop::Wayfire: return QStringLiteral("Wayfire");
    case Desktop::Cosmic: return QStringLiteral("COSMIC");
    case Desktop::Plasma: return QStringLiteral("KDE Plasma");
    case Desktop::Gnome: return QStringLiteral("GNOME");
    case Desktop::Pantheon: return QStringLiteral("Pantheon");
    case Desktop::Xfce: return QStringLiteral("XFCE");
    case Desktop::Cinnamon: return QStringLiteral("Cinnamon");
    case Desktop::Mate: return QStringLiteral("MATE");
    case Desktop::Budgie: return QStringLiteral("Budgie");
    case Desktop::LXQt: return QStringLiteral("LXQt");
    case Desktop::Deepin: return QStringLiteral("Deepin");
    case Desktop::GenericWayland: return QStringLiteral("Wayland compositor");
    case Desktop::GenericX11: return QStringLiteral("X11 window manager");
    default: return QStringLiteral("Unknown");
    }
}

QString sessionName(SessionType s)
{
    return s == SessionType::Wayland ? QStringLiteral("wayland") : s == SessionType::X11 ? QStringLiteral("x11") : QStringLiteral("unknown");
}

QString placementName(Placement p)
{
    switch (p) {
    case Placement::LayerShell: return QStringLiteral("layer-shell");
    case Placement::X11Desktop: return QStringLiteral("x11-desktop");
    case Placement::GnomeShellExtension: return QStringLiteral("gnome-extension");
    case Placement::KWinScript: return QStringLiteral("kwin-script");
    }
    return {};
}

bool parsePlacement(const QString &s, Placement *out)
{
    for (Placement p : {Placement::LayerShell, Placement::X11Desktop, Placement::GnomeShellExtension, Placement::KWinScript})
        if (placementName(p) == s) { *out = p; return true; }
    return false;
}

} // namespace wpe
