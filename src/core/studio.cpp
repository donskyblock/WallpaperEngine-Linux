// SPDX-License-Identifier: GPL-3.0-or-later
#include "studio.h"
#include "library.h"
#include "nativeproject.h"
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QSaveFile>
#include <QTemporaryDir>
#include <QTimer>
#include <QUuid>
#include <memory>
namespace wpe {
namespace {
QString localPath(const QString &s) { QUrl u(s); return u.isLocalFile() ? u.toLocalFile() : s; }
bool writeJson(const QString &path, const QJsonObject &o) {
    QSaveFile f(path);
    return f.open(QIODevice::WriteOnly) && f.write(QJsonDocument(o).toJson()) != -1 && f.commit();
}
QString publish(QTemporaryDir &stage, const QString &root) {
    const QString dest = QDir(root).filePath(QUuid::createUuid().toString(QUuid::WithoutBraces));
    if (!QDir().rename(stage.path(), dest)) return {};
    stage.setAutoRemove(false);
    return dest;
}
}
Studio::Studio(Settings *settings, QObject *parent, QNetworkAccessManager *network)
    : QObject(parent), m_settings(settings), m_network(network ? network : &m_ownedNetwork) {}
QString Studio::catalogUrl() const { return m_settings->data().catalogUrl; }
QString Studio::root() const { return QDir(m_settings->data().downloadDir).filePath("local"); }
void Studio::fail(const QString &error) { m_busy = false; m_error = error; emit changed(); }
void Studio::fetch(const QUrl &url, qint64 limit, std::function<void(QByteArray, QString)> done) {
    if (url.scheme() != "https" || url.host().isEmpty() || !url.userInfo().isEmpty()) { done({}, "Downloads require an HTTPS URL without credentials."); return; }
    QNetworkRequest req(url);
    req.setTransferTimeout(30000);
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    auto *reply = m_network->get(req);
    auto data = std::make_shared<QByteArray>();
    auto oversized = std::make_shared<bool>(false);
    connect(reply, &QNetworkReply::readyRead, this, [reply, data, oversized, limit] {
        if (reply->bytesAvailable() > limit - data->size()) { *oversized = true; reply->abort(); return; }
        data->append(reply->readAll());
    });
    auto *deadline = new QTimer(reply);
    deadline->setSingleShot(true);
    connect(deadline, &QTimer::timeout, reply, &QNetworkReply::abort);
    deadline->start(300000);
    connect(reply, &QNetworkReply::finished, this, [reply, data, oversized, limit, done] {
        data->append(reply->readAll());
        QString error;
        if (*oversized || data->size() > limit) error = "Download exceeds size limit.";
        else if (reply->error() != QNetworkReply::NoError) error = reply->errorString();
        else if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 200) error = "Server did not return HTTP 200.";
        reply->deleteLater();
        done(error.isEmpty() ? *data : QByteArray(), error);
    });
}
void Studio::refreshCatalog(const QString &url) {
    if (m_busy) return;
    m_busy = true; m_error.clear(); m_entries.clear(); emit changed();
    fetch(QUrl(url.trimmed()), 4 * 1024 * 1024, [this, url](QByteArray bytes, QString error) {
        if (!error.isEmpty()) { fail(error); return; }
        const auto doc = QJsonDocument::fromJson(bytes);
        const auto o = doc.object();
        if (o.value("version").toInt() != 1 || !o.value("wallpapers").isArray() || o.value("wallpapers").toArray().size() > 2000) { fail("Unsupported catalog format."); return; }
        QSet<QString> ids;
        QVariantList entries;
        for (const auto &v : o.value("wallpapers").toArray()) {
            const auto entry = v.toObject();
            const auto id = entry.value("id").toString();
            const auto kind = entry.value("type").toString();
            const auto files = entry.value("files").toArray();
            if (id.isEmpty() || ids.contains(id) || entry.value("title").toString().isEmpty() || (kind != "native" && kind != "image" && kind != "video") || files.isEmpty() || files.size() > 128) { fail("Invalid catalog entry."); return; }
            QSet<QString> paths;
            qint64 total = 0;
            for (const auto &f : files) {
                const auto file = f.toObject();
                const auto path = file.value("path").toString();
                const QUrl link(file.value("url").toString());
                const double size = file.value("bytes").toDouble(-1);
                if (!safeAssetPath(path) || paths.contains(path) || link.scheme() != "https" || link.host().isEmpty() || !link.userInfo().isEmpty()
                    || !QRegularExpression("^[a-fA-F0-9]{64}$").match(file.value("sha256").toString()).hasMatch()
                    || size < 1 || size > 256 * 1024 * 1024 || size != qint64(size)) { fail("Invalid catalog file or file exceeds 256 MiB."); return; }
                paths.insert(path); total += qint64(size);
            }
            if (total > 1024LL * 1024 * 1024 || (kind == "native" && !paths.contains("wallpaper.json"))
                || (kind != "native" && (!paths.contains(entry.value("entry").toString()) || paths.contains("wallpaper.json") || paths.contains("project.json")))) { fail("Invalid catalog entry point or project exceeds 1 GiB."); return; }
            ids.insert(id); entries << entry.toVariantMap();
        }
        m_entries = entries;
        auto settings = m_settings->data(); settings.catalogUrl = url.trimmed(); m_settings->set(settings);
        m_busy = false; emit changed();
    });
}
void Studio::install(int index) {
    if (m_busy || index < 0 || index >= m_entries.size()) return;
    const auto entry = QJsonObject::fromVariantMap(m_entries[index].toMap());
    QDir().mkpath(root());
    auto stage = std::make_shared<QTemporaryDir>(root() + "/.install-XXXXXX");
    if (!stage->isValid()) { fail("Cannot create installation directory."); return; }
    m_busy = true; m_error.clear(); emit changed();
    // Sequential downloads bound memory to one asset. No archive extraction; scripts run only through the widget runtime.
    auto next = std::make_shared<std::function<void(int)>>();
    std::weak_ptr<std::function<void(int)>> weak = next;
    const auto installRoot = root();
    *next = [this, entry, stage, weak, installRoot](int i) {
        const auto files = entry.value("files").toArray();
        if (i == files.size()) {
            QString error;
            const auto kind = entry.value("type").toString();
            if (kind == "native") {
                if (readNativeProject(stage->filePath("wallpaper.json"), &error).isEmpty()) { fail(error); return; }
            } else {
                const auto c = classify(stage->filePath(entry.value("entry").toString()));
                if (kindName(c.kind) != kind) { fail("Entry point does not match wallpaper type."); return; }
                if (!writeJson(stage->filePath("project.json"), {{"title", entry.value("title")}, {"type", kind}, {"file", entry.value("entry")}})) { fail("Cannot save project metadata."); return; }
            }
            const auto dest = publish(*stage, installRoot);
            if (dest.isEmpty()) { fail("Cannot finish installation."); return; }
            m_busy = false; emit changed(); emit installed(dest); return;
        }
        const auto f = files[i].toObject();
        auto keep = weak.lock();
        fetch(QUrl(f.value("url").toString()), qint64(f.value("bytes").toDouble()), [this, stage, f, i, keep](QByteArray data, QString error) {
            if (!error.isEmpty()) { fail(error); return; }
            if (data.size() != qint64(f.value("bytes").toDouble()) || QString::fromLatin1(QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex()) != f.value("sha256").toString().toLower()) { fail("Download size or SHA-256 mismatch."); return; }
            const auto path = stage->filePath(f.value("path").toString());
            QDir().mkpath(QFileInfo(path).absolutePath());
            QSaveFile out(path);
            if (!out.open(QIODevice::WriteOnly) || out.write(data) != data.size() || !out.commit()) { fail("Cannot save download."); return; }
            if (keep) (*keep)(i + 1);
        });
    };
    (*next)(0);
}
QString Studio::importMedia(const QString &url) {
    if (m_busy) return {};
    const auto path = localPath(url);
    const auto c = classify(path);
    if (c.kind != Kind::Image && c.kind != Kind::Video) { fail("Choose an image or video file."); return {}; }
    QDir().mkpath(root()); QTemporaryDir stage(root() + "/.import-XXXXXX");
    const auto name = QFileInfo(path).fileName();
    if (!stage.isValid() || !QFile::copy(path, stage.filePath(name)) || !writeJson(stage.filePath("project.json"), {{"title", QFileInfo(path).completeBaseName()}, {"type", kindName(c.kind)}, {"file", name}})) { fail("Cannot import media."); return {}; }
    const auto dest = publish(stage, root());
    if (dest.isEmpty()) { fail("Cannot finish import."); return {}; }
    m_error.clear(); emit changed(); emit installed(dest); return dest;
}
QVariantMap Studio::loadProject(const QString &url) {
    if (m_busy) return {};
    QString path = localPath(url);
    if (QFileInfo(path).isDir()) path = QDir(path).filePath("wallpaper.json");
    QString error;
    auto p = readNativeProject(path, &error);
    if (p.isEmpty()) { fail(error); return {}; }
    auto layers = p.value("layers").toArray();
    for (int i = 0; i < layers.size(); ++i) {
        auto l = layers[i].toObject();
        if (l.value("type") == "image" || l.value("type") == "video") l["source"] = QUrl::fromLocalFile(QFileInfo(path).dir().filePath(l.value("source").toString())).toString();
        layers[i] = l;
    }
    p["layers"] = layers;
    return p.toVariantMap();
}
QString Studio::saveProject(const QVariantMap &project) {
    if (m_busy) return {};
    auto p = QJsonObject::fromVariantMap(project);
    QDir().mkpath(root()); QTemporaryDir stage(root() + "/.create-XXXXXX");
    if (!stage.isValid()) { fail("Cannot create project directory."); return {}; }
    auto layers = p.value("layers").toArray();
    for (int i = 0; i < layers.size(); ++i) {
        auto l = layers[i].toObject();
        if (l.value("type") == "image" || l.value("type") == "video") {
            const auto source = localPath(l.value("source").toString());
            const auto c = classify(source);
            if ((l.value("type") == "image" && c.kind != Kind::Image) || (l.value("type") == "video" && c.kind != Kind::Video)) { fail("Layer asset type does not match file."); return {}; }
            const auto name = QString("asset-%1.%2").arg(i).arg(QFileInfo(source).suffix().toLower());
            if (!QFile::copy(source, stage.filePath(name))) { fail("Cannot copy layer asset."); return {}; }
            l["source"] = name;
        }
        layers[i] = l;
    }
    bool scripted = p.value("version").toInt() == 2 || p.contains("capabilities") || p.contains("dbusSources");
    for (const auto &layer : layers) if (layer.toObject().contains("script") || layer.toObject().value("type") == "widget") scripted = true;
    p["format"] = "wpe-native"; p["version"] = scripted ? 2 : 1; p["layers"] = layers;
    QString error;
    if (!validateNativeProject(p, stage.path(), &error)) { fail(error); return {}; }
    if (!writeJson(stage.filePath("wallpaper.json"), p)) { fail("Cannot save wallpaper document."); return {}; }
    const auto dest = publish(stage, root());
    if (dest.isEmpty()) { fail("Cannot finish project."); return {}; }
    m_error.clear(); emit changed(); emit installed(dest); return dest;
}
}
