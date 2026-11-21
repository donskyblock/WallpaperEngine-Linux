// SPDX-License-Identifier: GPL-3.0-or-later
#include "environment.h"

#include <QtTest>

using namespace wpe;

class TestEnvironment : public QObject {
    Q_OBJECT
    static QProcessEnvironment env(std::initializer_list<std::pair<const char *, const char *>> kv)
    {
        QProcessEnvironment e;
        for (auto &p : kv) e.insert(p.first, p.second);
        return e;
    }
    static Environment detect(QProcessEnvironment e, QStringList running = {})
    {
        return detectEnvironment(e, [running](const QString &c) { return running.contains(c); });
    }

private slots:
    void data_layerShell_data()
    {
        QTest::addColumn<QProcessEnvironment>("env");
        QTest::addColumn<QStringList>("running");
        QTest::addColumn<int>("desktop");
        QTest::newRow("hyprland") << env({{"XDG_SESSION_TYPE", "wayland"}, {"HYPRLAND_INSTANCE_SIGNATURE", "abc"}, {"XDG_CURRENT_DESKTOP", "Hyprland"}}) << QStringList{} << int(Desktop::Hyprland);
        QTest::newRow("niri") << env({{"XDG_SESSION_TYPE", "wayland"}, {"NIRI_SOCKET", "/run/x"}, {"XDG_CURRENT_DESKTOP", "niri"}}) << QStringList{} << int(Desktop::Niri);
        QTest::newRow("sway") << env({{"XDG_SESSION_TYPE", "wayland"}, {"SWAYSOCK", "/run/s"}}) << QStringList{} << int(Desktop::Sway);
        QTest::newRow("mango via XDG") << env({{"XDG_SESSION_TYPE", "wayland"}, {"XDG_CURRENT_DESKTOP", "mango"}}) << QStringList{} << int(Desktop::Mango);
        QTest::newRow("mango via process") << env({{"WAYLAND_DISPLAY", "wayland-0"}}) << QStringList{"mango"} << int(Desktop::Mango);
        QTest::newRow("cosmic") << env({{"XDG_SESSION_TYPE", "wayland"}, {"XDG_CURRENT_DESKTOP", "COSMIC"}}) << QStringList{} << int(Desktop::Cosmic);
    }
    void layerShell_data() { data_layerShell_data(); }
    void layerShell()
    {
        QFETCH(QProcessEnvironment, env);
        QFETCH(QStringList, running);
        QFETCH(int, desktop);
        const Environment e = detect(env, running);
        QCOMPARE(int(e.desktop), desktop);
        QCOMPARE(e.session, SessionType::Wayland);
        const Plan p = planFor(e);
        QCOMPARE(p.window, Placement::LayerShell);
        QCOMPARE(p.still, StaticMode::HostWindow);
    }

    void plasma()
    {
        const Plan p = planFor(detect(env({{"XDG_SESSION_TYPE", "wayland"}, {"XDG_CURRENT_DESKTOP", "KDE"}})));
        QCOMPARE(p.still, StaticMode::Plasma);
        QVERIFY(p.plasmaVideoPlugin);
        QCOMPARE(p.window, Placement::KWinScript);
    }

    void gnomeWaylandVsX11()
    {
        const Plan w = planFor(detect(env({{"XDG_SESSION_TYPE", "wayland"}, {"XDG_CURRENT_DESKTOP", "ubuntu:GNOME"}})));
        QCOMPARE(w.window, Placement::GnomeShellExtension);
        QCOMPARE(w.still, StaticMode::GSettingsGnome);
        const Plan x = planFor(detect(env({{"XDG_SESSION_TYPE", "x11"}, {"XDG_CURRENT_DESKTOP", "GNOME"}})));
        QCOMPARE(x.window, Placement::X11Desktop);
    }

    void x11Desktops()
    {
        const Plan xfce = planFor(detect(env({{"XDG_SESSION_TYPE", "x11"}, {"XDG_CURRENT_DESKTOP", "XFCE"}})));
        QCOMPARE(xfce.still, StaticMode::Xfce);
        QCOMPARE(xfce.window, Placement::X11Desktop);
        QVERIFY(!xfce.caveats.isEmpty());
        QCOMPARE(planFor(detect(env({{"XDG_SESSION_TYPE", "x11"}, {"XDG_CURRENT_DESKTOP", "X-Cinnamon"}}))).still, StaticMode::GSettingsCinnamon);
        QCOMPARE(planFor(detect(env({{"XDG_SESSION_TYPE", "x11"}, {"XDG_CURRENT_DESKTOP", "MATE"}}))).still, StaticMode::GSettingsMate);
        const Environment i3 = detect(env({{"DISPLAY", ":0"}}));
        QCOMPARE(i3.desktop, Desktop::GenericX11);
        QCOMPARE(planFor(i3).window, Placement::X11Desktop);
    }

    void unknownWaylandAssumesLayerShellWithCaveat()
    {
        const Plan p = planFor(detect(env({{"WAYLAND_DISPLAY", "wayland-1"}})));
        QCOMPARE(p.window, Placement::LayerShell);
        QVERIFY(!p.caveats.isEmpty());
    }

    void placementNamesRoundTrip()
    {
        for (Placement p : {Placement::LayerShell, Placement::X11Desktop, Placement::GnomeShellExtension, Placement::KWinScript}) {
            Placement out;
            QVERIFY(parsePlacement(placementName(p), &out));
            QCOMPARE(out, p);
        }
        Placement dummy;
        QVERIFY(!parsePlacement("nonsense", &dummy));
    }
};

QTEST_APPLESS_MAIN(TestEnvironment)
#include "test_environment.moc"
