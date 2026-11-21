// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QDBusAbstractAdaptor>

namespace wpe {
class AppController;

// org.wpelinux.WallpaperEngine1 on the session bus. This is what `wpectl` talks to, and what you bind
// to keys / exec-once in Hyprland, Niri, mango, Sway, i3 ...
class DBusApi : public QDBusAbstractAdaptor {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.wpelinux.WallpaperEngine1")
public:
    explicit DBusApi(AppController *c);

public slots:
    void SetWallpaper(const QString &path, const QString &type, const QString &output);
    void Random(const QString &output);
    void Stop(const QString &output);
    void Restore();
    QString Outputs();
    QString Status();
    QString Environment();
    void Show();
    void Quit();

private:
    AppController *m_c;
};
} // namespace wpe
