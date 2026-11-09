// SPDX-License-Identifier: GPL-3.0-or-later
#include "tools.h"

#include "types.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QStandardPaths>
#include <QDirIterator>

namespace wpe::tools {

static const char *kGnomeUuid = "wpe-linux@wpelinux.org";
static const char *kPlasmaId = "org.wpelinux.video";

QString find(const QString &name)
{
    const QString home = QDir::homePath();
    QString p = QStandardPaths::findExecutable(name); // PATH
    if (p.isEmpty())
        p = QStandardPaths::findExecutable(name, {home + "/.local/bin", "/usr/local/bin", "/usr/bin", "/opt/bin", "/usr/games"});
    return p;
}

QString hostBinary()
{
    if (const QString o = qEnvironmentVariable("WPE_HOST"); !o.isEmpty() && QFileInfo(o).isExecutable()) return o;
    const QString dir = QCoreApplication::applicationDirPath();
    for (const QString &c : {dir + "/wpe-wallpaper-host", dir + "/../libexec/wpe-wallpaper-host",
                             QStringLiteral("/usr/libexec/wpe-wallpaper-host"), QStringLiteral("/usr/lib/wpe-linux/wpe-wallpaper-host")})
        if (QFileInfo(c).isExecutable()) return QFileInfo(c).canonicalFilePath();
    return find(QStringLiteral("wpe-wallpaper-host"));
}

QString ownAssetsInstallDir()
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/wpe-linux/wallpaper_engine");
}

static QStringList steamLibraries()
{
    const QString h = QDir::homePath();
    QStringList roots = {h + "/.steam/steam", h + "/.local/share/Steam", h + "/.steam/debian-installation",
                         h + "/.var/app/com.valvesoftware.Steam/.local/share/Steam",
                         h + "/snap/steam/common/.local/share/Steam"};
    // Extra libraries listed in libraryfolders.vdf (cheap line scan, no full VDF parser).
    QStringList extra;
    for (const QString &r : roots) {
        QFile f(r + "/steamapps/libraryfolders.vdf");
        if (!f.open(QIODevice::ReadOnly)) continue;
        for (const QByteArray &line : f.readAll().split('\n')) {
            const QString l = QString::fromUtf8(line).trimmed();
            if (l.startsWith("\"path\"")) {
                const QStringList parts = l.split('"', Qt::SkipEmptyParts);
                if (parts.size() >= 3) extra << parts[2];
            }
        }
    }
    roots += extra;
    QStringList out;
    for (const QString &r : roots)
        if (QDir(r + "/steamapps").exists() && !out.contains(r)) out << r;
    return out;
}

QString wallpaperEngineAssets()
{
    QStringList c = {ownAssetsInstallDir() + "/assets"};
    for (const QString &r : steamLibraries()) c << r + "/steamapps/common/wallpaper_engine/assets";
    c << QStringLiteral("/opt/linux-wallpaperengine/assets");
    for (const QString &p : c)
        if (QDir(p).exists()) return p;
    return {};
}

QStringList steamWorkshopRoots()
{
    QStringList out;
    for (const QString &r : steamLibraries()) {
        const QString p = r + QStringLiteral("/steamapps/workshop/content/%1").arg(kWallpaperEngineAppId);
        if (QDir(p).exists()) out << p;
    }
    return out;
}

QString stateFilePath()
{
    QString base = qEnvironmentVariable("XDG_STATE_HOME");
    if (base.isEmpty()) base = QDir::homePath() + "/.local/state";
    return base + "/wpe-linux/state.json";
}

// ---- GNOME Shell extension -------------------------------------------------------
static QString gnomeExtDir() { return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/gnome-shell/extensions/" + kGnomeUuid; }
static QString plasmaPluginDir() { return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/plasma/wallpapers/" + kPlasmaId; }

bool gnomeExtensionInstalled() { return QFileInfo::exists(gnomeExtDir() + "/metadata.json"); }

bool gnomeExtensionEnabled()
{
    QProcess p;
    p.start("gsettings", {"get", "org.gnome.shell", "enabled-extensions"});
    if (!p.waitForFinished(1500)) return false;
    return QString::fromUtf8(p.readAllStandardOutput()).contains(kGnomeUuid);
}

bool plasmaPluginInstalled() { return QFileInfo::exists(plasmaPluginDir() + "/metadata.json"); }

static bool copyResourceTree(const QString &resRoot, const QString &dest, QString *error)
{
    QDirIterator it(resRoot, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        const QString src = it.next();
        const QString rel = src.mid(resRoot.size());
        const QString dst = dest + rel;
        QDir().mkpath(QFileInfo(dst).absolutePath());
        QFile::remove(dst);
        if (!QFile::copy(src, dst)) { if (error) *error = "Could not write " + dst; return false; }
        QFile::setPermissions(dst, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadGroup | QFileDevice::ReadOther);
    }
    return true;
}

bool installGnomeExtension(QString *error)
{
    if (!copyResourceTree(QStringLiteral(":/ext/gnome-shell"), gnomeExtDir(), error)) return false;
    QProcess::execute("gnome-extensions", {"enable", kGnomeUuid}); // fails harmlessly until the shell re-scans (next login on Wayland)
    return true;
}

bool installPlasmaPlugin(QString *error)
{
    return copyResourceTree(QStringLiteral(":/ext/plasma"), plasmaPluginDir(), error);
}

QVariantMap status()
{
    return {{"lwe", !find("linux-wallpaperengine").isEmpty()},
            {"lweAssets", !wallpaperEngineAssets().isEmpty()},
            {"steamcmd", !find("steamcmd").isEmpty()},
            {"host", !hostBinary().isEmpty()},
            {"gnomeExtInstalled", gnomeExtensionInstalled()},
            {"plasmaPlugin", plasmaPluginInstalled()}};
}

} // namespace wpe::tools
