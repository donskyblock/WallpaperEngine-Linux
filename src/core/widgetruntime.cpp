// SPDX-License-Identifier: GPL-3.0-or-later
#include "widgetruntime.h"
#include "nativeproject.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusVariant>
#include <QJSEngine>
#include <QJSValue>
#include <QColor>
#include <QtQml/qqml.h>
#include <algorithm>
#include <cmath>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

namespace wpe {
namespace {
QVariant unpack(QVariant value) {
    if (value.metaType() == QMetaType::fromType<QDBusVariant>()) return unpack(qvariant_cast<QDBusVariant>(value).variant());
    if (value.metaType() == QMetaType::fromType<QDBusArgument>()) {
        const auto arg = qvariant_cast<QDBusArgument>(value);
        if (arg.currentSignature() == "a{sv}") {
            auto map = qdbus_cast<QVariantMap>(arg);
            for (auto it = map.begin(); it != map.end(); ++it) it.value() = unpack(it.value());
            return map;
        }
        if (arg.currentSignature() == "as") return qdbus_cast<QStringList>(arg);
        return QVariant(); // unsupported signatures are never exposed as QObject wrappers
    }
    if (value.metaType() == QMetaType::fromType<QVariantMap>()) {
        auto map = value.toMap();
        for (auto it = map.begin(); it != map.end(); ++it) it.value() = unpack(it.value());
        return map;
    }
    switch (value.metaType().id()) {
    case QMetaType::QString: return value.toString().left(8192);
    case QMetaType::QStringList: return value.toStringList().mid(0, 128);
    case QMetaType::Bool: case QMetaType::Int: case QMetaType::UInt:
    case QMetaType::LongLong: case QMetaType::ULongLong: case QMetaType::Double: return value;
    default: return QVariant();
    }
}
// A watchdog can interrupt JavaScript even while the GUI thread is executing it.
// It is disarmed under the same lock before any engine is destroyed.
class ScriptDeadline {
    std::mutex mutex;
    std::condition_variable changed;
    QJSEngine *engine = nullptr;
    std::chrono::steady_clock::time_point deadline;
    bool stopping = false;
    std::thread thread;
public:
    ScriptDeadline() : thread([this] {
        std::unique_lock lock(mutex);
        while (!stopping) {
            if (!engine) { changed.wait(lock); continue; }
            if (changed.wait_until(lock, deadline) == std::cv_status::timeout && engine) {
                engine->setInterrupted(true); engine = nullptr;
            }
        }
    }) {}
    ~ScriptDeadline() {
        { std::lock_guard lock(mutex); stopping = true; engine = nullptr; }
        changed.notify_one(); thread.join();
    }
    template<class F> void run(QJSEngine &js, F operation) {
        js.setInterrupted(false);
        { std::lock_guard lock(mutex); engine = &js; deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(12); }
        changed.notify_one();
        operation();
        { std::lock_guard lock(mutex); engine = nullptr; }
        changed.notify_one();
    }
};
QVariantMap sanitize(const QJSValue &value, bool elements = true) {
    QVariantMap result;
    if (!value.isObject() || value.isArray()) return result;
    // Never convert arbitrary JS objects recursively: cyclic objects/getters must
    // not escape the execution deadline. Only these primitive visual fields exist.
    const std::pair<const char *, std::pair<double,double>> numbers[] = {
        {"x", {-2, 2}}, {"y", {-2, 2}}, {"width", {0.001, 4}}, {"height", {0.001, 4}},
        {"opacity", {0, 1}}, {"rotation", {-36000, 36000}}, {"scale", {0.001, 10}},
        {"fontSize", {1, 512}}, {"radius", {0, 512}}
    };
    for (const auto &field : numbers) {
        auto v = value.property(QString::fromLatin1(field.first));
        if (v.isNumber() && std::isfinite(v.toNumber())) result[QString::fromLatin1(field.first)] = std::clamp(v.toNumber(), field.second.first, field.second.second);
    }
    for (const auto &key : {"text", "color", "background", "type"}) {
        auto v = value.property(QLatin1String(key));
        if (!v.isString()) continue;
        const auto text = v.toString().left(QString(key) == "text" ? 8192 : 128);
        if ((QString(key) == "color" || QString(key) == "background") && !QColor(text).isValid()) continue;
        if (QString(key) == "type" && text != "rect" && text != "text") continue;
        result[QLatin1String(key)] = text;
    }
    if (elements) {
        auto array = value.property("elements");
        if (array.isArray()) {
            QVariantList list;
            const auto count = qMin(array.property("length").toUInt(), 128u);
            for (quint32 i = 0; i < count; ++i) list << sanitize(array.property(i), false);
            result["elements"] = list;
        }
    }
    return result;
}
}
WidgetDataSources::WidgetDataSources(QObject *parent) : QObject(parent) {}
void WidgetDataSources::configure(const QVariantMap &doc) {
    ++m_generation; m_mediaPending = false; m_pending.clear(); m_media.clear(); m_values.clear();
    const auto caps = doc.value("capabilities").toStringList();
    m_mediaEnabled = caps.contains("media");
    const auto candidates = caps.contains("dbus") ? doc.value("dbusSources").toList().mid(0, 16) : QVariantList();
    m_sources.clear();
    // The creator can preview documents before saving, so validate declarations
    // here as well as at the on-disk format boundary.
    for (const auto &candidate : candidates) {
        const QJsonObject probe{{"format", "wpe-native"}, {"version", 2}, {"title", "Source"}, {"layers", QJsonArray()},
            {"capabilities", QJsonArray{"dbus"}}, {"dbusSources", QJsonArray{QJsonObject::fromVariantMap(candidate.toMap())}}};
        if (validateNativeProject(probe, {}, nullptr, false)) m_sources << candidate;
    }
    emit changed();
}
void WidgetDataSources::refresh() {
    const auto generation = m_generation;
    for (const auto &v : m_sources) {
        const auto source = v.toMap(); const auto id = source.value("id").toString();
        if (m_pending.contains(id)) continue;
        const auto bus = source.value("bus").toString() == "system" ? QDBusConnection::systemBus() : QDBusConnection::sessionBus();
        if (!bus.isConnected()) continue;
        auto message = QDBusMessage::createMethodCall(source.value("service").toString(), source.value("path").toString(), "org.freedesktop.DBus.Properties", "Get");
        message << source.value("interface").toString() << source.value("property").toString();
        m_pending.insert(id);
        auto *watcher = new QDBusPendingCallWatcher(bus.asyncCall(message, 1000), this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher, id, generation] {
            QDBusPendingReply<QDBusVariant> reply = *watcher; watcher->deleteLater();
            if (generation != m_generation) return;
            m_pending.remove(id);
            if (reply.isError()) m_values.remove(id); else m_values[id] = unpack(reply.value().variant());
            emit changed();
        });
    }
    const auto bus = QDBusConnection::sessionBus();
    if (!m_mediaEnabled || m_mediaPending || !bus.isConnected()) return;
    m_mediaPending = true;
    auto list = QDBusMessage::createMethodCall("org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus", "ListNames");
    auto *watcher = new QDBusPendingCallWatcher(bus.asyncCall(list, 1000), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher, bus, generation] {
        QDBusPendingReply<QStringList> reply = *watcher; watcher->deleteLater();
        if (generation != m_generation) return;
        if (reply.isError()) { m_mediaPending = false; m_media.clear(); emit changed(); return; }
        QStringList services;
        for (const auto &name : reply.value()) if (name.startsWith("org.mpris.MediaPlayer2.")) services << name;
        services.sort(); services = services.mid(0, 16);
        if (services.isEmpty()) { m_mediaPending = false; m_media.clear(); emit changed(); return; }
        auto remaining = std::make_shared<int>(services.size());
        auto players = std::make_shared<QMap<QString, QVariantMap>>();
        for (const auto &name : services) {
            auto message = QDBusMessage::createMethodCall(name, "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "GetAll");
            message << "org.mpris.MediaPlayer2.Player";
            auto *player = new QDBusPendingCallWatcher(bus.asyncCall(message, 1000), this);
            connect(player, &QDBusPendingCallWatcher::finished, this, [this, player, name, remaining, players, generation] {
                QDBusPendingReply<QVariantMap> response = *player; player->deleteLater();
                if (generation != m_generation) return;
                if (!response.isError()) {
                    const auto props = unpack(response.value()).toMap();
                    const auto metadata = props.value("Metadata").toMap();
                    (*players)[name] = {{"service", name}, {"title", metadata.value("xesam:title")}, {"artist", metadata.value("xesam:artist").toStringList().join(", ")},
                        {"album", metadata.value("xesam:album")}, {"status", props.value("PlaybackStatus")}, {"playing", props.value("PlaybackStatus").toString() == "Playing"},
                        {"position", props.value("Position").toDouble() / 1000000.0}, {"duration", metadata.value("mpris:length").toDouble() / 1000000.0}};
                }
                if (--*remaining != 0) return;
                QVariantMap selected;
                for (const auto &p : std::as_const(*players)) {
                    if (selected.isEmpty() || (selected.value("status") == "Stopped" && p.value("status") == "Paused")) selected = p;
                    if (p.value("playing").toBool()) { selected = p; break; }
                }
                m_media = selected; m_mediaPending = false; emit changed();
            });
        }
    });
}
struct WidgetRuntime::Private {
    struct Script { std::unique_ptr<QJSEngine> engine; QJSValue update; bool disabled = false; };
    std::vector<Script> scripts;
    ScriptDeadline deadline;
};
WidgetRuntime::WidgetRuntime(QObject *parent) : QObject(parent), d(std::make_unique<Private>()), m_data(this) {
    m_elapsed.start(); m_timer.setInterval(33); m_dataTimer.setInterval(1000);
    connect(&m_timer, &QTimer::timeout, this, &WidgetRuntime::tick);
    connect(&m_dataTimer, &QTimer::timeout, &m_data, &WidgetDataSources::refresh);
}
WidgetRuntime::~WidgetRuntime() = default;
void WidgetRuntime::setDocument(const QVariantMap &doc) {
    if (m_document == doc) return;
    m_document = doc; m_errors.clear(); m_frames.clear(); d->scripts.clear();
    m_elapsed.restart(); m_lastFrame = 0;
    const bool enabled = doc.value("version").toInt() == 2;
    m_pointerEnabled = enabled && doc.value("capabilities").toStringList().contains("pointer");
    m_pointer = {{"x", 0.5}, {"y", 0.5}, {"inside", false}, {"pressed", false}, {"available", false}};
    const auto layers = doc.value("layers").toList().mid(0, 64);
    bool scripted = false;
    for (int index = 0; index < layers.size(); ++index) {
        Private::Script script;
        const auto code = layers[index].toMap().value("script").toString();
        if (enabled && !code.isEmpty() && code.toUtf8().size() <= 65536) {
            scripted = true; script.engine = std::make_unique<QJSEngine>();
            script.engine->globalObject().setProperty("globalThis", script.engine->globalObject());
            // No QObject, Qt, file, network, process or D-Bus object is injected.
            // Authors supply a function expression, not code evaluated by QML.
            QJSValue result;
            QString detail;
            d->deadline.run(*script.engine, [&] {
                result = script.engine->evaluate("(\n" + code + "\n)", QString("widget-%1.js").arg(index));
                if (result.isError()) detail = result.toString().left(512) + " (line " + QString::number(result.property("lineNumber").toInt() - 1) + ')';
            });
            if (script.engine->isInterrupted() || script.engine->hasError() || result.isError() || !result.isCallable()) {
                m_errors << QString("Layer %1: %2").arg(index + 1).arg(script.engine->isInterrupted() ? "Script initialization exceeded 12 ms." : !detail.isEmpty() ? detail : "Expected function(ctx) { return {...}; }.");
                script.disabled = true;
            } else script.update = result;
        }
        d->scripts.push_back(std::move(script)); m_frames << QVariantMap();
    }
    m_data.configure(enabled ? doc : QVariantMap());
    if (m_active && scripted) { m_timer.start(); m_dataTimer.start(); m_data.refresh(); } else { m_timer.stop(); m_dataTimer.stop(); }
    emit documentChanged(); emit errorsChanged(); tick();
}
void WidgetRuntime::setActive(bool active) {
    if (m_active == active) return;
    m_active = active;
    const bool scripts = std::any_of(d->scripts.begin(), d->scripts.end(), [](const auto &s) { return s.engine && !s.disabled; });
    if (active && scripts) { m_lastFrame = m_elapsed.elapsed(); m_timer.start(); m_dataTimer.start(); m_data.refresh(); }
    else { m_timer.stop(); m_dataTimer.stop(); }
    emit activeChanged();
}
void WidgetRuntime::setPointer(qreal x, qreal y, bool inside, bool pressed) {
    if (!m_pointerEnabled || !std::isfinite(x) || !std::isfinite(y)) return;
    m_pointer = {{"x", std::clamp(x, 0.0, 1.0)}, {"y", std::clamp(y, 0.0, 1.0)}, {"inside", inside}, {"pressed", pressed}, {"available", true}};
}
void WidgetRuntime::tick() {
    if (!m_active) return;
    const auto now = m_elapsed.elapsed();
    QVariantMap context{{"time", now / 1000.0}, {"delta", std::clamp((now - m_lastFrame) / 1000.0, 0.0, 0.25)},
        {"width", m_width}, {"height", m_height}, {"mouse", m_pointer}, {"media", m_data.media()}, {"dbus", m_data.values()}};
    m_lastFrame = now;
    for (int i = 0; i < int(d->scripts.size()); ++i) {
        auto &script = d->scripts[i];
        if (!script.engine || script.disabled) continue;
        QVariantMap frame; bool error = false; QString detail;
        d->deadline.run(*script.engine, [&] {
            const auto result = script.update.call({script.engine->toScriptValue(context)});
            error = script.engine->hasError() || result.isError() || !result.isObject() || result.isArray();
            if (result.isError()) detail = result.toString().left(512) + " (line " + QString::number(result.property("lineNumber").toInt() - 1) + ')';
            if (!error) frame = sanitize(result);
            error = error || script.engine->hasError();
        });
        if (error || script.engine->isInterrupted()) {
            script.disabled = true; m_frames[i] = QVariantMap();
            m_errors << QString("Layer %1: %2").arg(i + 1).arg(script.engine->isInterrupted() ? "Script exceeded 12 ms and was disabled." : !detail.isEmpty() ? detail : "Script must return a visual property object; execution failed.");
            emit errorsChanged();
        } else m_frames[i] = frame;
    }
    emit framesChanged();
}
void registerNativeWidgets() { qmlRegisterType<WidgetRuntime>("Wpe.Native", 1, 0, "WidgetRuntime"); }
}
