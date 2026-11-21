// SPDX-License-Identifier: GPL-3.0-or-later
//
// wpectl: control the running wpe-linux instance over D-Bus. Meant for key bindings and autostart lines:
//   Hyprland   exec-once = wpe-linux --background        bind = SUPER, W, exec, wpectl random
//   Niri       spawn-at-startup "wpe-linux" "--background"   Mod+W { spawn "wpectl" "random"; }
//   Sway/i3    exec wpe-linux --background               bindsym $mod+w exec wpectl random
//   mango      exec-once=wpe-linux --background          bind=SUPER,w,spawn,wpectl random
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QFileInfo>
#include <memory>
#include <QDBusInterface>
#include <QDBusReply>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <cstdio>

static int usage()
{
    puts("usage: wpectl <command> [options]\n\n"
         "  set <path> [-o OUTPUT] [-t TYPE]   apply an image, video, or wallpaper folder\n"
         "  random [-o OUTPUT]                 apply a random wallpaper from the library\n"
         "  stop [-o OUTPUT]                   stop (default: all outputs)\n"
         "  list                               list outputs\n"
         "  status                             what is playing where\n"
         "  env                                detected desktop and placement strategy\n"
         "  restore                            re-apply the last session\n"
         "  show | quit                        show the window / quit the app\n\n"
         "OUTPUT is a connector name (DP-1, HDMI-A-1 ...) or * for all; omitted = default monitor.");
    return 2;
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    const QStringList a = app.arguments().mid(1);
    if (a.isEmpty() || a[0] == "-h" || a[0] == "--help") return usage();

    QString cmd = a[0], output, type, path;
    for (int i = 1; i < a.size(); ++i) {
        if ((a[i] == "-o" || a[i] == "--output") && i + 1 < a.size()) output = a[++i];
        else if ((a[i] == "-t" || a[i] == "--type") && i + 1 < a.size()) type = a[++i];
        else path = a[i];
    }

    auto bus = QDBusConnection::sessionBus();
    const char *svc = "org.wpelinux.WallpaperEngine";
    auto connect = [&] { return std::make_unique<QDBusInterface>(svc, "/org/wpelinux/WallpaperEngine", "org.wpelinux.WallpaperEngine1", bus); };
    auto ifacePtr = connect();
    if (!ifacePtr->isValid()) {
        // Start the background instance and wait for it to claim its name.
        QProcess::startDetached("wpe-linux", {"--background"});
        for (int i = 0; i < 50 && !bus.interface()->isServiceRegistered(svc); ++i) QProcess::execute("sleep", {"0.1"});
        ifacePtr = connect();
        if (!ifacePtr->isValid()) { fprintf(stderr, "wpectl: could not reach wpe-linux (is it installed and is a session bus running?)\n"); return 1; }
    }
    QDBusInterface &iface = *ifacePtr;

    auto printJson = [](const QString &s) { puts(QJsonDocument::fromJson(s.toUtf8()).toJson(QJsonDocument::Indented).constData()); };

    if (cmd == "set") {
        if (path.isEmpty()) return usage();
        iface.call("SetWallpaper", QFileInfo(path).absoluteFilePath(), type, output);
    } else if (cmd == "random") iface.call("Random", output);
    else if (cmd == "stop") iface.call("Stop", output);
    else if (cmd == "restore") iface.call("Restore");
    else if (cmd == "show") iface.call("Show");
    else if (cmd == "quit") iface.call("Quit");
    else if (cmd == "list") printJson(QDBusReply<QString>(iface.call("Outputs")).value());
    else if (cmd == "status") printJson(QDBusReply<QString>(iface.call("Status")).value());
    else if (cmd == "env") printJson(QDBusReply<QString>(iface.call("Environment")).value());
    else return usage();
    return 0;
}
