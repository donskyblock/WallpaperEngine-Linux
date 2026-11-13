// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QJsonObject>
#include <QList>
#include <QString>
#include <QStringList>

namespace wpe {

constexpr int kWallpaperEngineAppId = 431960;

struct Display {
    QString name;
    QString description;
    int width = 0;
    int height = 0;
    double refreshRate = 0;
    int x = 0;
    int y = 0;
    qreal scale = 1.0;
    bool primary = false;
    QJsonObject toJson() const;
};

// What a wallpaper actually *is*, after looking at the files — not just what the
// Steam tag claims.
enum class Kind { Image, Video, Scene, Web, Unknown };
QString kindName(Kind k);
Kind kindFromString(const QString &s);

struct LibraryItem {
    QString id;
    QString title;
    QString path;        // file for image/video, directory for scene/web
    QString previewPath; // may be empty
    QString contentType; // "video" | "scene" | "web" | "image" | "unknown"
    QString workshopId;  // empty for local files
    QJsonObject toJson() const;
};

struct WorkshopItem {
    QString id;
    QString title;
    QString previewUrl;
    QString description;
    QStringList tags;
    quint64 subscriptions = 0;
    quint64 favorited = 0;
    quint64 votesUp = 0;
    quint64 votesDown = 0;
    double score = 0;
    QString contentType;
    quint64 timeUpdated = 0;
    quint64 timeCreated = 0;
    quint64 fileSize = 0;
    quint64 views = 0;
};

} // namespace wpe
