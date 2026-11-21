// SPDX-License-Identifier: GPL-3.0-or-later
#include "widgetruntime.h"
#include "nativeproject.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QtTest>
using namespace wpe;
class TestWidgets : public QObject {
    Q_OBJECT
    static QVariantMap document(const QString &code, const QStringList &caps = {}) {
        return {{"format", "wpe-native"}, {"version", 2}, {"title", "Widget"}, {"capabilities", caps},
            {"layers", QVariantList{QVariantMap{{"type", "widget"}, {"script", code}}}}};
    }
private slots:
    void pointerAndAnimationContext() {
        WidgetRuntime runtime;
        runtime.setDocument(document("function(c) { return {text: c.mouse.available ? 'mouse' : 'none', x:c.mouse.x, y:c.mouse.y, rotation:c.time*90}; }", {"pointer"}));
        runtime.setPointer(0.2, 0.7, true, false); runtime.tick();
        const auto frame = runtime.frames()[0].toMap();
        QCOMPARE(frame.value("text").toString(), QString("mouse"));
        QCOMPARE(frame.value("x").toDouble(), 0.2); QCOMPARE(frame.value("y").toDouble(), 0.7);
        QVERIFY(frame.contains("rotation")); QVERIFY(runtime.errors().isEmpty());
    }
    void capabilitiesAndVersionAreRespected() {
        WidgetRuntime runtime;
        auto doc = document("function(c) { return {text: String(c.mouse.available) + ':' + typeof c.dbus.secret}; }");
        runtime.setDocument(doc); runtime.setPointer(0.2, 0.3, true, true); runtime.tick();
        QCOMPARE(runtime.frames()[0].toMap().value("text").toString(), QString("false:undefined"));
        doc["version"] = 1; runtime.setDocument(doc); runtime.tick();
        QVERIFY(runtime.frames()[0].toMap().isEmpty());
    }
    void stateAndEngineIsolation() {
        WidgetRuntime runtime;
        auto doc = document("(function() { let n = 0; return function(c) { return {text: String(++n)}; }; })()");
        doc["layers"] = QVariantList{
            QVariantMap{{"type", "widget"}, {"script", "(function(){ globalThis.secret = 42; let n = 0; return function(){return {text:String(++n)}}; })()"}},
            QVariantMap{{"type", "widget"}, {"script", "function(){return {text:typeof secret + ':' + typeof Qt + ':' + typeof XMLHttpRequest};}"}}};
        runtime.setDocument(doc); runtime.tick();
        QCOMPARE(runtime.frames()[0].toMap().value("text").toString(), QString("2"));
        QCOMPARE(runtime.frames()[1].toMap().value("text").toString(), QString("undefined:undefined:undefined"));
        runtime.setActive(false); const auto frames = runtime.frames(); runtime.tick(); QCOMPARE(runtime.frames(), frames);
    }
    void badScriptsAreIsolated_data() {
        QTest::addColumn<QString>("code");
        QTest::newRow("syntax") << "function( {";
        QTest::newRow("throw") << "function(){throw new Error('bad')}";
        QTest::newRow("loop") << "function(){while(true){}}";
        QTest::newRow("init-loop") << "(function(){while(true){}})()";
        QTest::newRow("getter-loop") << "function(){return {get text(){while(true){}}}}";
        QTest::newRow("invalid-return") << "function(){return 42}";
    }
    void badScriptsAreIsolated() {
        QFETCH(QString, code); WidgetRuntime runtime;
        auto doc = document(code);
        auto layers = doc.value("layers").toList(); layers << QVariantMap{{"type", "widget"}, {"script", "function(){return {text:'healthy'}}"}}; doc["layers"] = layers;
        QElapsedTimer time; time.start(); runtime.setDocument(doc);
        QVERIFY(time.elapsed() < 2000);
        QVERIFY(!runtime.errors().isEmpty()); QCOMPARE(runtime.frames()[1].toMap().value("text").toString(), QString("healthy"));
        const auto errors = runtime.errors(); runtime.tick(); QCOMPARE(runtime.errors(), errors);
    }
    void sanitizesVisualOutputWithoutRecursiveConversion() {
        WidgetRuntime runtime;
        runtime.setDocument(document("function(){let a={}; a.cycle=a; return {x:Infinity,y:-99,opacity:4,color:'invalid-color',text:'ok',evil:a,elements:[{type:'rect',width:2,color:'#ffffff'}]}}"));
        const auto frame = runtime.frames()[0].toMap();
        QVERIFY(!frame.contains("x")); QVERIFY(!frame.contains("color")); QVERIFY(!frame.contains("evil"));
        QCOMPARE(frame.value("y").toDouble(), -2.0); QCOMPARE(frame.value("opacity").toDouble(), 1.0);
        QCOMPARE(frame.value("elements").toList().size(), 1); QVERIFY(runtime.errors().isEmpty());
    }
    void validatesV2AndReadOnlySources() {
        auto doc = QJsonObject::fromVariantMap(document("function(){return {text:'ok'}}", {"dbus"}));
        QJsonObject source{{"id", "battery"}, {"bus", "system"}, {"service", "org.freedesktop.UPower"}, {"path", "/org/freedesktop/UPower/devices/DisplayDevice"}, {"interface", "org.freedesktop.UPower.Device"}, {"property", "Percentage"}};
        doc["dbusSources"] = QJsonArray{source}; QString error;
        QVERIFY2(validateNativeProject(doc, {}, &error, false), qPrintable(error));
        source["bus"] = "arbitrary-address"; doc["dbusSources"] = QJsonArray{source}; QVERIFY(!validateNativeProject(doc, {}, &error, false));
        doc["dbusSources"] = QJsonArray(); doc["version"] = 1; QVERIFY(!validateNativeProject(doc, {}, &error, false));
    }
};
QTEST_GUILESS_MAIN(TestWidgets)
#include "test_widgets.moc"
