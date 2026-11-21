// SPDX-License-Identifier: GPL-3.0-or-later
#include "appcontroller.h"
#include "widgetruntime.h"
#include "nativeproject.h"
#include <QApplication>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickView>
#include <QImage>
#include <QJSValue>
#include <QJsonArray>
#include <QtTest>
using namespace wpe;
class TestCreator : public QObject {
    Q_OBJECT
private slots:
    void drawsScriptedWidgetAndMouseAnimation() {
        QTemporaryDir dir; Settings settings; auto cfg = settings.data(); cfg.downloadDir = dir.path(); settings.set(cfg);
        WallpaperManager manager(&settings); AppController controller(&settings, &manager);
        QQuickView view; view.engine()->rootContext()->setContextProperty("App", &controller);
        QSignalSpy warnings(view.engine(), &QQmlEngine::warnings);
        view.resize(1100, 700); view.setResizeMode(QQuickView::SizeRootObjectToView);
        view.setSource(QUrl::fromLocalFile(QStringLiteral(SOURCE_ROOT "/src/app/qml/CreatorPage.qml")));
        QVERIFY(view.status() == QQuickView::Ready); auto *page = view.rootObject();
        QVERIFY(QMetaObject::invokeMethod(page, "add", Q_ARG(QVariant, "widget"), Q_ARG(QVariant, "")));
        QVERIFY(QMetaObject::invokeMethod(page, "setCapability", Q_ARG(QVariant, "pointer"), Q_ARG(QVariant, true)));
        QVERIFY(QMetaObject::invokeMethod(page, "update", Q_ARG(QVariant, "script"), Q_ARG(QVariant,
            "function(c){return {x:c.mouse.x/2,y:0,width:0.3,height:1,background:'#ff0000',elements:[{type:'rect',x:0,y:0,width:0.5,height:0.5,color:'#00ff00'}]}}")));
        view.show(); QTest::qWait(100);
        auto *canvas = page->findChild<QQuickItem *>("nativePreview"); QVERIFY(canvas);
        auto *runtime = canvas->findChild<WidgetRuntime *>("widgetRuntime"); QVERIFY(runtime);
        runtime->setPointer(0.6, 0.3, true, false); runtime->tick(); QTest::qWait(40);
        QCOMPARE(runtime->frames()[0].toMap().value("x").toDouble(), 0.3);
        const auto shot = view.grabWindow(); QVERIFY(!shot.isNull());
        const auto red = canvas->mapToScene(QPointF(canvas->width()*0.55, canvas->height()*0.8)).toPoint();
        const auto green = canvas->mapToScene(QPointF(canvas->width()*0.35, canvas->height()*0.2)).toPoint();
        QCOMPARE(shot.pixelColor(red), QColor(Qt::red)); QCOMPARE(shot.pixelColor(green), QColor(Qt::green));
        QTest::mouseMove(&view, canvas->mapToScene(QPointF(canvas->width()*0.8, canvas->height()*0.6)).toPoint());
        QTest::qWait(50); runtime->tick();
        QVERIFY(qAbs(runtime->frames()[0].toMap().value("x").toDouble() - 0.4) < 0.01);
        const auto project = page->property("project").value<QJSValue>().toVariant().toMap();
        const auto saved = controller.studio()->saveProject(project); QVERIFY2(!saved.isEmpty(), qPrintable(controller.studio()->error()));
        const auto reopened = controller.studio()->loadProject(saved); QCOMPARE(reopened.value("version").toInt(), 2);
        QCOMPARE(reopened.value("layers").toList()[0].toMap().value("script"), project.value("layers").toList()[0].toMap().value("script"));
        QVERIFY(runtime->errors().isEmpty());
        view.hide(); QTRY_VERIFY(!runtime->active());
        view.show(); QTRY_VERIFY(runtime->active());
        for (const auto &batch : warnings) for (const auto &error : qvariant_cast<QList<QQmlError>>(batch[0])) QVERIFY2(false, qPrintable(error.toString()));
    }
    void createsPreviewsAndReopensPortableWallpaper() {
        QTemporaryDir dir;
        Settings settings; auto cfg = settings.data(); cfg.downloadDir = dir.filePath("library"); cfg.restoreOnStart = false; cfg.onboardingCompleted = true; settings.set(cfg);
        WallpaperManager manager(&settings); AppController controller(&settings, &manager);
        QQuickView view;
        view.engine()->rootContext()->setContextProperty("App", &controller);
        QSignalSpy warnings(view.engine(), &QQmlEngine::warnings);
        view.setResizeMode(QQuickView::SizeRootObjectToView);
        view.resize(1100, 700);
        view.setSource(QUrl::fromLocalFile(QStringLiteral(SOURCE_ROOT "/src/app/qml/CreatorPage.qml")));
        QVERIFY2(view.status() == QQuickView::Ready, qPrintable(view.errors().isEmpty() ? "" : view.errors().first().toString()));
        auto *page = view.rootObject(); QVERIFY(page);
        QImage image(64, 64, QImage::Format_RGB32); image.fill(Qt::blue);
        const auto asset = dir.filePath("blue.png"); QVERIFY(image.save(asset));
        QVERIFY(QMetaObject::invokeMethod(page, "add", Q_ARG(QVariant, "image"), Q_ARG(QVariant, QUrl::fromLocalFile(asset).toString())));
        QVERIFY(QMetaObject::invokeMethod(page, "add", Q_ARG(QVariant, "clock"), Q_ARG(QVariant, "")));
        view.show(); QTest::qWait(150);
        auto *canvas = page->findChild<QObject *>("nativePreview"); QVERIFY(canvas);
        QSignalSpy failures(canvas, SIGNAL(playbackFailed(QString)));
        const auto project = page->property("project").value<QJSValue>().toVariant().toMap();
        QCOMPARE(project.value("layers").toList().size(), 2);
        const auto saved = controller.studio()->saveProject(project);
        QVERIFY2(!saved.isEmpty(), qPrintable(controller.studio()->error()));
        QVERIFY(QFile::remove(asset));
        const auto reopened = controller.studio()->loadProject(saved);
        QVERIFY(!reopened.isEmpty());
        const auto native = readNativeProject(saved + "/wallpaper.json");
        QCOMPARE(native.value("layers").toArray().size(), 2);
        canvas->setProperty("baseUrl", QUrl::fromLocalFile(saved + '/'));
        canvas->setProperty("document", native.toVariantMap());
        QTest::qWait(100);
        auto *item = qobject_cast<QQuickItem *>(canvas); QVERIFY(item);
        const auto shot = view.grabWindow(); QVERIFY(!shot.isNull());
        const auto point = item->mapToScene(QPointF(item->width()/2, item->height()*0.8)).toPoint();
        QCOMPARE(shot.pixelColor(point), QColor(Qt::blue));
        QCOMPARE(failures.size(), 0);
        for (const auto &batch : warnings) {
            const auto errors = qvariant_cast<QList<QQmlError>>(batch[0]);
            for (const auto &error : errors) QVERIFY2(false, qPrintable(error.toString()));
        }
    }
};
int main(int argc, char **argv) {
    qputenv("QT_QPA_PLATFORM", "offscreen"); qputenv("QT_QUICK_BACKEND", "software");
    QTemporaryDir config; qputenv("XDG_CONFIG_HOME", config.path().toUtf8());
    QApplication app(argc, argv); registerNativeWidgets(); TestCreator test; return QTest::qExec(&test, argc, argv);
}
#include "test_creator.moc"
