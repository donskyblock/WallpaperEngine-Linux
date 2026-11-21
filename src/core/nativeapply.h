// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "environment.h"

#include <QRect>
#include <QString>

namespace wpe::nativeapply {

// Still images through the desktop's own settings, so they show up in the DE's wallpaper dialog and
// persist without us running. Return false + error on failure.
bool applyStill(StaticMode mode, const QString &output, const QString &imagePath, QString *error);

// Plasma: set a wallpaper plugin + a single config key on matching desktops (per output via screenForConnector).
bool plasmaSetWallpaper(const QString &plugin, const QString &configKey, const QString &value, const QString &output, QString *error);

// Plasma/KWin: pin the window owned by `pid` below everything, borderless, on all desktops, at `geometry`.
bool kwinPinWindow(const QString &scriptName, qint64 pid, const QRect &geometry, QString *error);
void kwinUnpin(const QString &scriptName);

} // namespace wpe::nativeapply
