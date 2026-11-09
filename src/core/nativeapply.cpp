// SPDX-License-Identifier: GPL-3.0-or-later
#include "nativeapply.h"

#include "tools.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QStandardPaths>
#include <QUrl>

namespace wpe::nativeapply {

static bool run(const QString &prog, const QStringList &args, QString *err, QString *out = nullptr)
{
    QProcess p;
    p.start(prog, args);
    if (!p.waitForStarted(2000)) { if (err) *err = QObject::tr("Could not run %1").arg(prog); return false; }
    if (!p.waitForFinished(8000)) { p.kill(); if (err) *err = QObject::tr("%1 timed out").arg(prog); return false; }
    if (out) *out = QString::fromUtf8(p.readAllStandardOutput());
    if (p.exitStatus() != QProcess::NormalExit || p.exitCode() != 0) {
        if (err) *err = QObject::tr("%1 failed: %2").arg(prog, QString::fromUtf8(p.readAllStandardError()).trimmed());
        return false;
    }
    return true;
}

static QString jsString(const QString &s)
{
    QString o = s;
    o.replace('\\', "\\\\").replace('"', "\\\"").replace('\n', "\\n");
    return '"' + o + '"';
}

bool plasmaSetWallpaper(const QString &plugin, const QString &key, const QString &value, const QString &output, QString *error)
{
    const QString js = QStringLiteral(R"JS(
var out = %1; var plugin = %2; var key = %3; var val = %4;
var ds = desktops();
for (var i = 0; i < ds.length; i++) {
  var d = ds[i];
  if (out !== "" && typeof screenForConnector === "function" && d.screen !== screenForConnector(out)) continue;
  d.wallpaperPlugin = plugin;
  d.currentConfigGroup = ["Wallpaper", plugin, "General"];
  d.writeConfig(key, val);
}
)JS").arg(jsString(output), jsString(plugin), jsString(key), jsString(value));

    QDBusMessage m = QDBusMessage::createMethodCall(QStringLiteral("org.kde.plasmashell"), QStringLiteral("/PlasmaShell"),
                                                    QStringLiteral("org.kde.PlasmaShell"), QStringLiteral("evaluateScript"));
    m << js;
    const QDBusMessage r = QDBusConnection::sessionBus().call(m, QDBus::Block, 5000);
    if (r.type() == QDBusMessage::ErrorMessage) {
        if (error) *error = QObject::tr("plasmashell scripting failed: %1").arg(r.errorMessage());
        return false;
    }
    return true;
}

static bool xfceStill(const QString &output, const QString &path, QString *err)
{
    QString list;
    if (!run("xfconf-query", {"-c", "xfce4-desktop", "-l"}, err, &list)) return false;
    bool any = false;
    for (const QString &prop : list.split('\n', Qt::SkipEmptyParts)) {
        if (!prop.endsWith("/last-image")) continue;
        if (!output.isEmpty() && !prop.contains("monitor" + output)) continue;
        QString e;
        if (run("xfconf-query", {"-c", "xfce4-desktop", "-p", prop, "-s", path}, &e)) {
            any = true;
            QString style = prop;
            style.replace("/last-image", "/image-style");
            QProcess::execute("xfconf-query", {"-c", "xfce4-desktop", "-p", style, "-n", "-t", "int", "-s", "5"}); // 5 = zoomed
        }
    }
    if (!any && err) *err = QObject::tr("No matching XFCE backdrop property for output \"%1\".").arg(output);
    return any;
}

bool applyStill(StaticMode mode, const QString &output, const QString &path, QString *error)
{
    const QString uri = QUrl::fromLocalFile(path).toString();
    switch (mode) {
    case StaticMode::GSettingsGnome:
        for (const char *key : {"picture-uri", "picture-uri-dark"})
            if (!run("gsettings", {"set", "org.gnome.desktop.background", key, uri}, error)) return false;
        run("gsettings", {"set", "org.gnome.desktop.background", "picture-options", "zoom"}, nullptr);
        return true;
    case StaticMode::GSettingsCinnamon:
        if (!run("gsettings", {"set", "org.cinnamon.desktop.background", "picture-uri", uri}, error)) return false;
        run("gsettings", {"set", "org.cinnamon.desktop.background", "picture-options", "zoom"}, nullptr);
        return true;
    case StaticMode::GSettingsMate:
        if (!run("gsettings", {"set", "org.mate.background", "picture-filename", path}, error)) return false;
        run("gsettings", {"set", "org.mate.background", "picture-options", "zoom"}, nullptr);
        return true;
    case StaticMode::Plasma:
        return plasmaSetWallpaper("org.kde.image", "Image", uri, output, error);
    case StaticMode::Xfce:
        return xfceStill(output, path, error);
    case StaticMode::HostWindow:
        break;
    }
    if (error) *error = QStringLiteral("internal: HostWindow is not a native applier");
    return false;
}

// ---- KWin ---------------------------------------------------------------------------
static QString scriptPath(const QString &name)
{
    QString base = qEnvironmentVariable("XDG_RUNTIME_DIR");
    if (base.isEmpty()) base = QDir::tempPath();
    QDir().mkpath(base + "/wpe-linux");
    return base + "/wpe-linux/kwin-" + name + ".js";
}

bool kwinPinWindow(const QString &name, qint64 pid, const QRect &g, QString *error)
{
    // Works with both the Plasma 6 (windowAdded/windowList) and Plasma 5 (clientAdded/clientList) scripting APIs.
    const QString js = QStringLiteral(R"JS((function () {
  var PID = %1, X = %2, Y = %3, W = %4, H = %5;
  function pin(w) {
    if (w.pid !== PID) return;
    w.keepBelow = true; w.noBorder = true; w.skipTaskbar = true; w.skipPager = true;
    w.skipSwitcher = true; w.onAllDesktops = true;
    w.frameGeometry = Qt.rect(X, Y, W, H);
  }
  var added = workspace.windowAdded || workspace.clientAdded;
  added.connect(pin);
  var list = workspace.windowList ? workspace.windowList() : workspace.clientList();
  for (var i = 0; i < list.length; i++) pin(list[i]);
})();
)JS").arg(pid).arg(g.x()).arg(g.y()).arg(g.width()).arg(g.height());

    const QString path = scriptPath(name);
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) { if (error) *error = QObject::tr("Cannot write %1").arg(path); return false; }
    f.write(js.toUtf8());
    f.close();

    kwinUnpin(name);
    auto bus = QDBusConnection::sessionBus();
    QDBusMessage load = QDBusMessage::createMethodCall("org.kde.KWin", "/Scripting", "org.kde.kwin.Scripting", "loadScript");
    load << path << name;
    const QDBusMessage r = bus.call(load, QDBus::Block, 3000);
    if (r.type() == QDBusMessage::ErrorMessage || r.arguments().isEmpty()) {
        if (error) *error = QObject::tr("KWin scripting unavailable: %1").arg(r.errorMessage());
        return false;
    }
    const int id = r.arguments().first().toInt();
    // loadScript only registers the script. It has to be started explicitly. Object path differs between Plasma versions.
    for (const QString &p : {QStringLiteral("/Scripting/Script%1").arg(id), QStringLiteral("/%1").arg(id)}) {
        QDBusMessage run = QDBusMessage::createMethodCall("org.kde.KWin", p, "org.kde.kwin.Script", "run");
        if (bus.call(run, QDBus::Block, 3000).type() != QDBusMessage::ErrorMessage) return true;
    }
    QDBusMessage start = QDBusMessage::createMethodCall("org.kde.KWin", "/Scripting", "org.kde.kwin.Scripting", "start");
    bus.call(start, QDBus::Block, 3000);
    return true;
}

void kwinUnpin(const QString &name)
{
    QDBusMessage m = QDBusMessage::createMethodCall("org.kde.KWin", "/Scripting", "org.kde.kwin.Scripting", "unloadScript");
    m << name;
    QDBusConnection::sessionBus().call(m, QDBus::Block, 2000);
}

} // namespace wpe::nativeapply
