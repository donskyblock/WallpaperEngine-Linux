#include "widgetruntime.h"
// SPDX-License-Identifier: GPL-3.0-or-later
//
// wpe-wallpaper-host: draws one image/video wallpaper on one output and gets itself *underneath*
// everything, using whichever mechanism the compositor offers:
//   layer-shell      wlr-layer-shell BACKGROUND layer (Hyprland, Niri, Sway, mango, River, labwc, Wayfire, COSMIC, KWin)
//   x11-desktop      _NET_WM_WINDOW_TYPE_DESKTOP + below/sticky (XFCE, i3, bspwm, openbox, Cinnamon, MATE, ...)
//   gnome-extension  plain frameless window; the bundled GNOME Shell extension adopts it
//   kwin-script      plain frameless window; a KWin script pins it (pid-matched)
#include "nativeproject.h"
#include <QCommandLineParser>
#include <QFileInfo>
#include <QGuiApplication>
#include <QQmlEngine>
#include <QQuickView>
#include <QQuickItem>
#include <QCursor>
#include <QScreen>
#include <QTimer>

#ifdef WPE_HAVE_LAYERSHELL
#include <LayerShellQt/Shell>
#include <LayerShellQt/Window>
#endif
#ifdef WPE_HAVE_XCB
#include <xcb/xcb.h>
#endif

#include <cstring>

static QString argValue(int argc, char **argv, const char *name)
{
    for (int i = 1; i + 1 < argc; ++i)
        if (!std::strcmp(argv[i], name)) return QString::fromLocal8Bit(argv[i + 1]);
    return {};
}

#ifdef WPE_HAVE_XCB
static xcb_atom_t atom(xcb_connection_t *c, const char *name)
{
    auto *r = xcb_intern_atom_reply(c, xcb_intern_atom(c, 0, std::strlen(name), name), nullptr);
    const xcb_atom_t a = r ? r->atom : static_cast<xcb_atom_t>(XCB_ATOM_NONE);
    free(r);
    return a;
}

static xcb_connection_t *xcbConnection()
{
    auto *x11 = qGuiApp->nativeInterface<QNativeInterface::QX11Application>();
    return x11 ? x11->connection() : nullptr;
}

// Must run after QWindow::create() and before show(): Qt doesn't rewrite the window type afterwards.
static void markAsDesktopWindow(QWindow *w)
{
    xcb_connection_t *c = xcbConnection();
    if (!c) return;
    const xcb_atom_t type = atom(c, "_NET_WM_WINDOW_TYPE_DESKTOP");
    xcb_change_property(c, XCB_PROP_MODE_REPLACE, w->winId(), atom(c, "_NET_WM_WINDOW_TYPE"), XCB_ATOM_ATOM, 32, 1, &type);
    xcb_flush(c);
}

// Qt writes _NET_WM_STATE itself when mapping, so ask the WM for the extra states once we're mapped.
static void addWmStates(QWindow *w)
{
    xcb_connection_t *c = xcbConnection();
    if (!c) return;
    xcb_screen_t *screen = xcb_setup_roots_iterator(xcb_get_setup(c)).data;
    const xcb_atom_t state = atom(c, "_NET_WM_STATE");
    for (const char *name : {"_NET_WM_STATE_BELOW", "_NET_WM_STATE_STICKY", "_NET_WM_STATE_SKIP_TASKBAR", "_NET_WM_STATE_SKIP_PAGER"}) {
        xcb_client_message_event_t ev{};
        ev.response_type = XCB_CLIENT_MESSAGE;
        ev.format = 32;
        ev.window = w->winId();
        ev.type = state;
        ev.data.data32[0] = 1; // _NET_WM_STATE_ADD
        ev.data.data32[1] = atom(c, name);
        ev.data.data32[3] = 1; // source: normal application
        xcb_send_event(c, 0, screen->root, XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY, reinterpret_cast<const char *>(&ev));
    }
    xcb_flush(c);
}
#endif

int main(int argc, char **argv)
{
    // Platform plugin / shell integration must be chosen before QGuiApplication exists.
    const QString placement = argValue(argc, argv, "--placement");
    if (placement == "layer-shell") {
#ifdef WPE_HAVE_LAYERSHELL
        qputenv("QT_WAYLAND_SHELL_INTEGRATION", "layer-shell");
        if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) qputenv("QT_QPA_PLATFORM", "wayland");
        LayerShellQt::Shell::useLayerShell();
#else
        fprintf(stderr, "wpe-wallpaper-host was built without layer-shell-qt support\n");
        return 64;
#endif
    } else if (placement == "x11-desktop") {
        if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) qputenv("QT_QPA_PLATFORM", "xcb");
    }

    QGuiApplication app(argc, argv);
    wpe::registerNativeWidgets();
    QGuiApplication::setApplicationName("wpe-wallpaper-host");
    QGuiApplication::setDesktopFileName("org.wpelinux.WallpaperHost");

    QCommandLineParser p;
    p.addHelpOption();
    QCommandLineOption oOutput("output", "Output (connector) name", "name");
    QCommandLineOption oKind("kind", "image | video | native", "kind", "image");
    QCommandLineOption oPath("path", "File to display", "path");
    QCommandLineOption oPlace("placement", "layer-shell | x11-desktop | gnome-extension | kwin-script", "mode", "layer-shell");
    QCommandLineOption oVol("volume", "0-100 (0 = muted)", "n", "0");
    p.addOptions({oOutput, oKind, oPath, oPlace, oVol});
    p.process(app);

    if (p.value(oPath).isEmpty()) { fprintf(stderr, "--path is required\n"); return 64; }

    QScreen *screen = nullptr;
    for (QScreen *s : QGuiApplication::screens())
        if (s->name() == p.value(oOutput)) screen = s;
    if (!screen) {
        if (!p.value(oOutput).isEmpty()) fprintf(stderr, "output \"%s\" not found, using primary\n", qPrintable(p.value(oOutput)));
        screen = QGuiApplication::primaryScreen();
    }
    if (!screen) { fprintf(stderr, "no screens available\n"); return 65; }

    QVariantMap nativeDocument;
    QUrl nativeBase;
    if (p.value(oKind) == "native") {
        QString error;
        const auto doc = wpe::readNativeProject(p.value(oPath), &error);
        if (doc.isEmpty()) { fprintf(stderr, "%s\n", qPrintable(error)); return 64; }
        nativeDocument = doc.toVariantMap();
        nativeBase = QUrl::fromLocalFile(QFileInfo(p.value(oPath)).absolutePath() + '/');
    }
    QQuickView view;
    view.setInitialProperties({{"kind", p.value(oKind)},
                               {"source", QUrl::fromLocalFile(p.value(oPath))},
                               {"nativeDocument", nativeDocument}, {"nativeBase", nativeBase},
                               {"volume", p.value(oVol).toInt()}});
    QObject::connect(view.engine(), &QQmlEngine::quit, &app, &QCoreApplication::quit);
    QObject::connect(view.engine(), &QQmlEngine::exit, &app, &QCoreApplication::exit);
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.setSource(QUrl(QStringLiteral("qrc:/Host.qml")));
    if (view.status() != QQuickView::Ready) return 66;

    // "wpe-wallpaper:<output>@x,y,w,h": the GNOME Shell extension and KWin helpers read this.
    const QRect g = screen->geometry();
    view.setTitle(QStringLiteral("wpe-wallpaper:%1@%2,%3,%4,%5").arg(screen->name()).arg(g.x()).arg(g.y()).arg(g.width()).arg(g.height()));
    view.setScreen(screen);
    view.setGeometry(screen->geometry());
    const bool pointer = nativeDocument.value("version").toInt() == 2 && nativeDocument.value("capabilities").toStringList().contains("pointer");
    bool globalPointer = false;
#ifdef WPE_HAVE_XCB
    globalPointer = QGuiApplication::platformName() == "xcb";
#endif
    Qt::WindowFlags flags = Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus | Qt::WindowStaysOnBottomHint;
    if (!pointer || globalPointer) flags |= Qt::WindowTransparentForInput;
    view.setFlags(flags);
    QTimer pointerTimer;
#ifdef WPE_HAVE_XCB
    if (pointer && globalPointer) {
        QObject::connect(&pointerTimer, &QTimer::timeout, &view, [&view] {
            auto *connection = xcbConnection();
            if (!connection) return;
            const auto root = xcb_setup_roots_iterator(xcb_get_setup(connection)).data->root;
            auto *reply = xcb_query_pointer_reply(connection, xcb_query_pointer(connection, root), nullptr);
            if (!reply) return;
            const auto point = view.mapFromGlobal(QPoint(reply->root_x, reply->root_y));
            const bool pressed = reply->mask & XCB_BUTTON_MASK_1;
            free(reply);
            view.rootObject()->setProperty("externalPointer", QVariantMap{{"available", true}, {"inside", QRect(QPoint(), view.size()).contains(point)},
                {"x", double(point.x()) / qMax(1, view.width())}, {"y", double(point.y()) / qMax(1, view.height())}, {"pressed", pressed}});
        });
        pointerTimer.start(33);
    }
#endif

    if (placement == "layer-shell") {
#ifdef WPE_HAVE_LAYERSHELL
        view.create();
        auto *lw = LayerShellQt::Window::get(&view);
        lw->setScope("wpe-wallpaper");
        lw->setLayer(LayerShellQt::Window::LayerBackground);
        lw->setAnchors(LayerShellQt::Window::Anchors(LayerShellQt::Window::AnchorTop | LayerShellQt::Window::AnchorBottom
                                                     | LayerShellQt::Window::AnchorLeft | LayerShellQt::Window::AnchorRight));
        lw->setExclusiveZone(-1); // ignore other surfaces' exclusive zones: cover the whole output
        lw->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityNone);
#endif
        view.show();
    } else if (placement == "x11-desktop") {
#ifdef WPE_HAVE_XCB
        view.create();
        markAsDesktopWindow(&view);
        view.show();
        QTimer::singleShot(150, &app, [&view] { addWmStates(&view); });
#else
        fprintf(stderr, "built without xcb support\n");
        return 64;
#endif
    } else {
        // gnome-extension / kwin-script: an ordinary frameless toplevel that something else pins.
        view.resize(screen->geometry().size());
        view.show();
    }
    return app.exec();
}
