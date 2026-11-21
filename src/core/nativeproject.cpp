// SPDX-License-Identifier: GPL-3.0-or-later
#include "nativeproject.h"
#include <QColor>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QRegularExpression>
#include <QSet>
#include <cmath>
namespace wpe {
bool safeAssetPath(const QString &p) {
    return !p.isEmpty() && QDir::cleanPath(p) == p && !QDir::isAbsolutePath(p) && !p.contains(':') && !p.contains('\\')
        && !p.split('/').contains("..") && !p.split('/').contains(".") && !p.contains(QChar::Null);
}
bool validateNativeProject(const QJsonObject &p, const QString &dir, QString *error, bool files) {
    auto fail = [&](const QString &s) { if (error) *error = s; return false; };
    if (QJsonDocument(p).toJson(QJsonDocument::Compact).size() > 1024 * 1024) return fail("Wallpaper document exceeds 1 MiB.");
    const int version = p.value("version").toInt();
    if (p.value("format").toString() != "wpe-native" || (version != 1 && version != 2))
        return fail("Unsupported native wallpaper format or version.");
    if (version == 1 && (p.contains("capabilities") || p.contains("dbusSources"))) return fail("Widget capabilities require format version 2.");
    QSet<QString> capabilities;
    if (p.contains("capabilities")) {
        if (!p.value("capabilities").isArray()) return fail("Capabilities must be an array.");
        for (const auto &value : p.value("capabilities").toArray()) {
            const auto cap = value.toString();
            if ((cap != "media" && cap != "pointer" && cap != "dbus") || capabilities.contains(cap)) return fail("Invalid or duplicate widget capability.");
            capabilities.insert(cap);
        }
    }
    if (p.contains("dbusSources")) {
        if (!capabilities.contains("dbus") || !p.value("dbusSources").isArray() || p.value("dbusSources").toArray().size() > 16) return fail("D-Bus sources require the dbus capability and permit at most 16 properties.");
        QSet<QString> ids;
        const QRegularExpression name("^[A-Za-z_][A-Za-z0-9_]*(\\.[A-Za-z_][A-Za-z0-9_]*)+$");
        const QRegularExpression service("^[A-Za-z_][A-Za-z0-9_-]*(\\.[A-Za-z_][A-Za-z0-9_-]*)+$");
        const QRegularExpression member("^[A-Za-z_][A-Za-z0-9_]{0,63}$");
        const QRegularExpression objectPath("^/([A-Za-z0-9_]+(/[A-Za-z0-9_]+)*)?$");
        for (const auto &value : p.value("dbusSources").toArray()) {
            const auto source = value.toObject(); const auto id = source.value("id").toString();
            const auto bus = source.value("bus").toString("session");
            if (!member.match(id).hasMatch() || ids.contains(id) || (bus != "session" && bus != "system")
                || !service.match(source.value("service").toString()).hasMatch() || source.value("service").toString().size() > 255
                || !name.match(source.value("interface").toString()).hasMatch() || source.value("interface").toString().size() > 255
                || !objectPath.match(source.value("path").toString()).hasMatch() || source.value("path").toString().size() > 1024
                || !member.match(source.value("property").toString()).hasMatch()) return fail("Invalid read-only D-Bus property source.");
            ids.insert(id);
        }
    }
    if (p.value("title").toString().trimmed().isEmpty()) return fail("A title is required.");
    if (!QColor(p.value("background").toString("#101820")).isValid()) return fail("Invalid background colour.");
    if (!p.value("layers").isArray() || p.value("layers").toArray().size() > 64) return fail("Expected up to 64 layers.");
    for (const auto &v : p.value("layers").toArray()) {
        if (!v.isObject()) return fail("Invalid layer.");
        const auto l = v.toObject();
        const auto type = l.value("type").toString();
        if (type != "image" && type != "video" && type != "text" && type != "clock" && type != "widget") return fail("Unsupported layer type.");
        if (type == "widget" && version != 2) return fail("Custom widgets require format version 2.");
        if (l.contains("script")) {
            if (version != 2 || !l.value("script").isString() || l.value("script").toString().toUtf8().size() > 65536)
                return fail("Scripts require format version 2 and at most 64 KiB of JavaScript.");
        }
        for (const auto &key : {"x", "y", "width", "height", "opacity"}) {
            if (!l.contains(key)) continue;
            const double n = l.value(key).toDouble(-1);
            if (!std::isfinite(n) || n < 0 || n > 1 || ((QString(key) == "width" || QString(key) == "height") && n == 0))
                return fail("Layer positions, sizes and opacity must be between 0 and 1.");
        }
        if (l.contains("fontSize") && (l.value("fontSize").toDouble() < 1 || l.value("fontSize").toDouble() > 512)) return fail("Invalid font size.");
        if (l.contains("color") && !QColor(l.value("color").toString()).isValid()) return fail("Invalid text colour.");
        if (type == "image" || type == "video") {
            const auto asset = l.value("source").toString();
            if (!safeAssetPath(asset)) return fail("Assets must use relative paths inside the project.");
            if (files) {
                const QFileInfo fi(QDir(dir).filePath(asset));
                const auto root = QFileInfo(dir).canonicalFilePath() + '/';
                if (!fi.isFile() || !fi.canonicalFilePath().startsWith(root)) return fail("Missing asset or asset outside project: " + asset);
            }
        }
    }
    return true;
}
QJsonObject readNativeProject(const QString &path, QString *error) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly) || f.size() > 1024 * 1024) { if (error) *error = "Cannot read wallpaper document (maximum 1 MiB)."; return {}; }
    QJsonParseError e;
    auto doc = QJsonDocument::fromJson(f.readAll(), &e);
    if (e.error != QJsonParseError::NoError || !doc.isObject()) { if (error) *error = "Invalid wallpaper JSON."; return {}; }
    auto p = doc.object();
    if (!validateNativeProject(p, QFileInfo(path).absolutePath(), error)) return {};
    return p;
}
}
