// SPDX-License-Identifier: GPL-3.0-or-later
#include "library.h"

#include "tools.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSet>

namespace wpe {

static const QSet<QString> kVideoExts = {"mp4", "webm", "mkv", "avi", "mov", "m4v"};
static const QSet<QString> kImageExts = {"jpg", "jpeg", "png", "gif", "webp", "bmp", "avif"};

static QString firstWithExt(const QDir &d, const QSet<QString> &exts)
{
    const auto files = d.entryInfoList(QDir::Files, QDir::Name);
    for (const QFileInfo &fi : files)
        if (exts.contains(fi.suffix().toLower())) return fi.absoluteFilePath();
    return {};
}

static QJsonObject readProject(const QDir &d)
{
    QFile f(d.filePath("project.json"));
    if (!f.open(QIODevice::ReadOnly)) return {};
    return QJsonDocument::fromJson(f.readAll()).object();
}

Classified classify(const QString &path, const QString &typeHint)
{
    const QFileInfo fi(path);
    if (fi.isFile()) {
        const QString ext = fi.suffix().toLower();
        if (kVideoExts.contains(ext)) return {Kind::Video, fi.absoluteFilePath()};
        if (kImageExts.contains(ext)) return {Kind::Image, fi.absoluteFilePath()};
        if (ext == "html" || ext == "htm") return {Kind::Web, fi.absoluteFilePath()};
        return {kindFromString(typeHint), fi.absoluteFilePath()};
    }
    if (!fi.isDir()) return {};

    const QDir d(path);
    const QJsonObject proj = readProject(d);
    const Kind declared = kindFromString(proj.value("type").toString(typeHint));
    const QString file = proj.value("file").toString();
    const QString declaredFile = file.isEmpty() ? QString() : d.filePath(file);

    // A "scene" folder with no scene.pkg that just holds a video is a video (renderer crashes otherwise).
    if (d.exists("scene.pkg") || declared == Kind::Scene) {
        if (d.exists("scene.pkg")) return {Kind::Scene, d.absolutePath()};
        const QString v = firstWithExt(d, kVideoExts);
        if (!v.isEmpty()) return {Kind::Video, v};
        return {Kind::Scene, d.absolutePath()};
    }
    if (declared == Kind::Web || d.exists("index.html")) {
        const QString idx = !declaredFile.isEmpty() && QFileInfo::exists(declaredFile) ? declaredFile : d.filePath("index.html");
        return {Kind::Web, idx};
    }
    if (!declaredFile.isEmpty() && QFileInfo::exists(declaredFile)) {
        const QString ext = QFileInfo(declaredFile).suffix().toLower();
        if (kVideoExts.contains(ext)) return {Kind::Video, declaredFile};
        if (kImageExts.contains(ext)) return {Kind::Image, declaredFile};
    }
    if (const QString v = firstWithExt(d, kVideoExts); !v.isEmpty()) return {Kind::Video, v};
    if (const QString i = firstWithExt(d, kImageExts); !i.isEmpty()) return {Kind::Image, i};
    return {};
}

QString workshopContentDir(const QString &downloadDir)
{
    return QDir(downloadDir).filePath(QStringLiteral("steamapps/workshop/content/%1").arg(kWallpaperEngineAppId));
}

static QString stableId(const QString &path)
{
    return QString::fromLatin1(QCryptographicHash::hash(path.toUtf8(), QCryptographicHash::Sha1).toHex().left(16));
}

static void scanWorkshopRoot(const QString &root, QList<LibraryItem> &out, QSet<QString> &seen)
{
    const auto dirs = QDir(root).entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for (const QFileInfo &di : dirs) {
        if (seen.contains(di.fileName())) continue;
        const QDir d(di.absoluteFilePath());
        const QJsonObject proj = readProject(d);
        const Classified c = classify(d.absolutePath(), proj.value("type").toString());
        if (c.kind == Kind::Unknown) continue;

        LibraryItem it;
        it.workshopId = di.fileName();
        it.id = stableId(d.absolutePath());
        it.title = proj.value("title").toString(di.fileName());
        it.path = c.mainPath;
        it.contentType = kindName(c.kind);
        QString prev = proj.value("preview").toString();
        if (prev.isEmpty() || !d.exists(prev))
            for (const char *n : {"preview.jpg", "preview.png", "preview.gif", "preview.webp"})
                if (d.exists(QLatin1String(n))) { prev = QLatin1String(n); break; }
        if (!prev.isEmpty() && d.exists(prev)) it.previewPath = d.filePath(prev);
        else if (c.kind == Kind::Image) it.previewPath = c.mainPath;
        out << it;
        seen.insert(di.fileName());
    }
}

QList<LibraryItem> scanLibrary(const QString &downloadDir)
{
    QList<LibraryItem> out;
    QSet<QString> seen;
    scanWorkshopRoot(workshopContentDir(downloadDir), out, seen);
    // Wallpapers the user already subscribed to through the real Steam client.
    for (const QString &root : tools::steamWorkshopRoots()) scanWorkshopRoot(root, out, seen);

    // Loose image/video files dropped straight into the download dir.
    const auto files = QDir(downloadDir).entryInfoList(QDir::Files, QDir::Name);
    for (const QFileInfo &fi : files) {
        const Classified c = classify(fi.absoluteFilePath());
        if (c.kind != Kind::Image && c.kind != Kind::Video) continue;
        LibraryItem it;
        it.id = stableId(fi.absoluteFilePath());
        it.title = fi.completeBaseName();
        it.path = fi.absoluteFilePath();
        it.contentType = kindName(c.kind);
        if (c.kind == Kind::Image) it.previewPath = it.path;
        out << it;
    }
    return out;
}

} // namespace wpe
