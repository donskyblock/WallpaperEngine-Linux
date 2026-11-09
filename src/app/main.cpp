// SPDX-License-Identifier: GPL-3.0-or-later
#include "appcontroller.h"
#include "dbusapi.h"
#include "environment.h"
#include "settings.h"
#include "wallpapermanager.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDir>
#include <QFileInfo>
#include <QIcon>
#include <QMenu>
#include <QNetworkDiskCache>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlNetworkAccessManagerFactory>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QStandardPaths>
#include <QSystemTrayIcon>
#include <QTimer>

using namespace wpe;

namespace {
constexpr auto kService = "org.wpelinux.WallpaperEngine";
constexpr auto kPath = "/org/wpelinux/WallpaperEngine";

// Thumbnails come straight from Steam's CDN: cache them on disk so browsing is instant the second time.
class CachingNam : public QQmlNetworkAccessManagerFactory {
public:
    QNetworkAccessManager *create(QObject *parent) override
    {
        auto *nam = new QNetworkAccessManager(parent);
        auto *cache = new QNetworkDiskCache(nam);
        cache->setCacheDirectory(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/thumbs");
        cache->setMaximumCacheSize(256LL * 1024 * 1024);
        nam->setCache(cache);
        return nam;
    }
};

// Native look: KDE gets qqc2-desktop-style when it's installed, everyone else gets Fusion driven by the
// platform palette (Qt reads the portal/theme colours). We never hard-code colours in QML.
void chooseStyle(const QQmlEngine &engine)
{
    if (!qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) return;
    const bool kde = qEnvironmentVariable("XDG_CURRENT_DESKTOP").contains("KDE", Qt::CaseInsensitive);
    if (kde)
        for (const QString &p : engine.importPathList())
            if (QFileInfo::exists(p + "/org/kde/desktop/qmldir")) { QQuickStyle::setStyle("org.kde.desktop"); return; }
    QQuickStyle::setStyle("Fusion");
}
} // namespace

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("wpe-linux");
    QApplication::setOrganizationName("wpelinux");
    QApplication::setApplicationDisplayName("Wallpaper Engine");
    QApplication::setApplicationVersion(PROJECT_VERSION);
    QApplication::setDesktopFileName("wpe-linux");
    QApplication::setWindowIcon(QIcon::fromTheme("wpe-linux", QIcon(":/icons/wpe-linux.png")));
    QApplication::setQuitOnLastWindowClosed(false);

    QCommandLineParser p;
    p.setApplicationDescription("Wallpaper Engine for Linux");
    p.addHelpOption();
    p.addVersionOption();
    QCommandLineOption oBg("background", "Start hidden (restores wallpapers, lives in the tray).");
    QCommandLineOption oOnb("onboarding", "Force the first-run wizard.");
    QCommandLineOption oShot("screenshot", "(dev) Save a screenshot of the window and quit.", "file");
    QCommandLineOption oPage("page", "(dev) Open page: 0=Workshop 1=Library 2=Settings", "n", "0");
    QCommandLineOption oDemo("demo-data", "(dev) Fill Workshop with fake items using previews from <dir>.", "dir");
    p.addOptions({oBg, oOnb, oShot, oPage, oDemo});
    p.process(app);

    // ---- single instance + D-Bus API ------------------------------------------------------
    auto bus = QDBusConnection::sessionBus();
    bool haveBus = bus.isConnected();
    if (haveBus && !bus.registerService(kService)) {
        if (!p.isSet(oBg)) {
            QDBusMessage m = QDBusMessage::createMethodCall(kService, kPath, "org.wpelinux.WallpaperEngine1", "Show");
            bus.call(m, QDBus::NoBlock);
        }
        return 0; // already running
    }
    if (!haveBus) qWarning("No D-Bus session bus: wpectl and single-instance handling are disabled.");

    Settings settings;
    WallpaperManager manager(&settings);
    AppController controller(&settings, &manager);
    controller.setWindowVisible(!p.isSet(oBg));
    if (p.isSet(oOnb)) controller.setOnboarding(true);

    new DBusApi(&controller);
    if (haveBus) bus.registerObject(kPath, &controller, QDBusConnection::ExportAdaptors);

    if (settings.data().restoreOnStart)
        QTimer::singleShot(1200, &manager, &WallpaperManager::restore); // let the compositor finish announcing outputs

    // ---- tray -----------------------------------------------------------------------------
    QMenu trayMenu;
    QSystemTrayIcon tray(QIcon::fromTheme("wpe-linux", QIcon(":/icons/wpe-linux.png")));
    trayMenu.addAction(QObject::tr("Open Wallpaper Engine"), [&] { controller.setWindowVisible(true); });
    trayMenu.addAction(QObject::tr("Random wallpaper"), [&] { controller.applyRandom(QStringLiteral("*")); });
    trayMenu.addAction(QObject::tr("Stop all wallpapers"), [&] { manager.stop("*"); });
    trayMenu.addSeparator();
    trayMenu.addAction(QObject::tr("Quit"), [&] { controller.quitApp(); });
    tray.setContextMenu(&trayMenu);
    tray.setToolTip(QObject::tr("Wallpaper Engine"));
    QObject::connect(&tray, &QSystemTrayIcon::activated, [&](QSystemTrayIcon::ActivationReason r) {
        if (r == QSystemTrayIcon::Trigger) controller.setWindowVisible(!controller.windowVisible());
    });
    if (QSystemTrayIcon::isSystemTrayAvailable()) tray.show();

    // Quitting stops the wallpapers we own (the manager's destructor terminates them) but leaves the
    // saved state alone, so they come back on next login.
    QObject::connect(&controller, &AppController::quitRequested, &app, &QApplication::quit);

    // ---- UI -------------------------------------------------------------------------------
    QQmlApplicationEngine engine;
    CachingNam nam;
    engine.setNetworkAccessManagerFactory(&nam);
    chooseStyle(engine);
    engine.rootContext()->setContextProperty("App", &controller);
    engine.rootContext()->setContextProperty("startPage", p.value(oPage).toInt());
    engine.load(QUrl(QStringLiteral("qrc:/qt/qml/WpeLinux/qml/Main.qml")));
    if (engine.rootObjects().isEmpty()) return 1;

    if (p.isSet(oDemo)) {
        QList<WorkshopItem> items;
        const QDir d(p.value(oDemo));
        const QStringList imgs = d.entryList({"*.png", "*.jpg"}, QDir::Files, QDir::Name);
        const QStringList names = {"Neon Rain City", "Quiet Forest at Dawn", "Retro Synthwave Drive", "Anime Girl · Window Seat",
                                   "Abstract Fluid Waves", "Deep Space Nebula", "Cozy Cabin Snow", "Lo-fi Study Desk",
                                   "Mountain Lake Timelapse", "Cyberpunk Alley", "Sakura Wind", "Minimal Dark Gradient"};
        const QStringList types = {"Scene", "Video", "Scene", "Scene", "Video", "Video", "Scene", "Scene", "Video", "Scene", "Web", "Scene"};
        for (int i = 0; i < names.size() && !imgs.isEmpty(); ++i) {
            WorkshopItem it;
            it.id = QString::number(1000 + i);
            it.title = names[i];
            it.previewUrl = QUrl::fromLocalFile(d.filePath(imgs[i % imgs.size()])).toString();
            it.contentType = types[i];
            it.score = 0.7 + (i % 4) * 0.07;
            it.subscriptions = 120000 - i * 7300;
            it.votesUp = 4200 - i * 211;
            it.votesDown = 90 + i * 7;
            it.fileSize = quint64(14 + i * 9) * 1024 * 1024;
            it.views = 800000 - i * 31000;
            it.description = "A short description of this wallpaper as it would appear from the Steam Workshop API.";
            it.tags = {types[i], "Everyone", "Landscape"};
            items << it;
        }
        controller.workshopModel()->reset(items);
    }

    if (p.isSet(oShot)) {
        auto *w = qobject_cast<QQuickWindow *>(engine.rootObjects().first());
        const QString file = p.value(oShot);
        QTimer::singleShot(2500, &app, [&app, w, file] {
            w->grabWindow().save(file);
            app.quit();
        });
    }

    return app.exec();
}
