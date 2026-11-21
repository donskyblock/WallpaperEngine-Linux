// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QString>
#include <QStringList>
#include <QVariantMap>

namespace wpe::tools {

QString find(const QString &name);        // PATH + a few user/opt dirs; empty if missing
QString hostBinary();                     // wpe-wallpaper-host next to us, in libexec, or on PATH
QString wallpaperEngineAssets();          // Steam / Flatpak Steam / our own steamcmd install
QStringList steamWorkshopRoots();         // existing .../workshop/content/431960 dirs from Steam installs
QString ownAssetsInstallDir();
QString stateFilePath();                  // ~/.local/state/wpe-linux/state.json

bool gnomeExtensionInstalled();
bool gnomeExtensionEnabled();
bool plasmaPluginInstalled();
bool installGnomeExtension(QString *error);
bool installPlasmaPlugin(QString *error);

QVariantMap status(); // for Settings page

} // namespace wpe::tools
