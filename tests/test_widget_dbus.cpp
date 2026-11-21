// SPDX-License-Identifier: GPL-3.0-or-later
#include "widgetruntime.h"
#include <QDBusConnection>
#include <QDBusVirtualObject>
#include <QDBusMessage>
#include <QDBusVariant>
#include <QtTest>
using namespace wpe;
class FakePlayer : public QDBusVirtualObject {
public:
    QString title = "Test song", status = "Playing";
    int reads = 0;
    QString introspect(const QString &) const override { return {}; }
    bool handleMessage(const QDBusMessage &m, const QDBusConnection &bus) override {
        if (m.interface() != "org.freedesktop.DBus.Properties") return false;
        ++reads;
        if (m.member() == "GetAll") {
            const QVariantMap metadata{{"xesam:title", title}, {"xesam:artist", QStringList{"Test artist"}}, {"xesam:album", "Test album"}, {"mpris:length", qlonglong(180000000)}};
            const QVariantMap properties{{"Metadata", QVariant::fromValue(QDBusVariant(metadata))}, {"PlaybackStatus", status}, {"Position", qlonglong(12000000)}};
            bus.send(m.createReply(QVariantList{properties})); return true;
        }
        if (m.member() == "Get") { bus.send(m.createReply(QVariantList{QVariant::fromValue(QDBusVariant(73.5))})); return true; }
        return false;
    }
};
class TestWidgetDbus : public QObject {
    Q_OBJECT
private slots:
    void readsMediaAndDeclaredPropertiesAndClearsStaleData() {
        const auto bus = QDBusConnection::sessionBus(); QVERIFY(bus.isConnected());
        auto playerBus = QDBusConnection::connectToBus(QDBusConnection::SessionBus, "widget-test-player");
        auto pausedBus = QDBusConnection::connectToBus(QDBusConnection::SessionBus, "widget-test-paused");
        FakePlayer player, paused; paused.title = "Paused song"; paused.status = "Paused";
        QVERIFY(playerBus.registerService("org.mpris.MediaPlayer2.zzz_widget_test"));
        QVERIFY(playerBus.registerVirtualObject("/org/mpris/MediaPlayer2", &player));
        QVERIFY(playerBus.registerVirtualObject("/fixture", &player));
        QVERIFY(pausedBus.registerService("org.mpris.MediaPlayer2.aaa_widget_test"));
        QVERIFY(pausedBus.registerVirtualObject("/org/mpris/MediaPlayer2", &paused));
        WidgetDataSources data;
        QVariantMap source{{"id", "battery"}, {"bus", "session"}, {"service", "org.mpris.MediaPlayer2.zzz_widget_test"}, {"path", "/fixture"}, {"interface", "org.example.Power"}, {"property", "Percentage"}};
        QVariantMap doc{{"capabilities", QStringList{"media", "dbus"}}, {"dbusSources", QVariantList{source}}};
        data.configure(doc); data.refresh();
        QTRY_COMPARE(data.media().value("title").toString(), QString("Test song"));
        QCOMPARE(data.media().value("artist").toString(), QString("Test artist"));
        QCOMPARE(data.media().value("position").toDouble(), 12.0); QCOMPARE(data.media().value("duration").toDouble(), 180.0);
        QTRY_COMPARE(data.values().value("battery").toDouble(), 73.5);
        // Data changes appear on the next asynchronous poll.
        player.title = "Next song"; data.refresh(); QTRY_COMPARE(data.media().value("title").toString(), QString("Next song"));
        // Reconfiguration invalidates already pending replies.
        data.refresh(); data.configure({}); QTest::qWait(50);
        QVERIFY(data.media().isEmpty()); QVERIFY(data.values().isEmpty());
        const int reads = player.reads; data.refresh(); QTest::qWait(20); QCOMPARE(player.reads, reads);
        playerBus.unregisterService("org.mpris.MediaPlayer2.zzz_widget_test"); pausedBus.unregisterService("org.mpris.MediaPlayer2.aaa_widget_test");
        data.configure(doc); data.refresh(); QTest::qWait(100); QVERIFY(data.media().isEmpty()); QVERIFY(data.values().isEmpty());
    }
};
QTEST_GUILESS_MAIN(TestWidgetDbus)
#include "test_widget_dbus.moc"
