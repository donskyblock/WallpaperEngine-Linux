// SPDX-License-Identifier: GPL-3.0-or-later
#include "types.h"

namespace wpe {

QJsonObject Display::toJson() const
{
    return {{"name", name},   {"description", description}, {"width", width}, {"height", height},
            {"refresh", refreshRate}, {"x", x}, {"y", y}, {"scale", scale}, {"primary", primary}};
}

QString kindName(Kind k)
{
    switch (k) {
    case Kind::Image: return QStringLiteral("image");
    case Kind::Video: return QStringLiteral("video");
    case Kind::Scene: return QStringLiteral("scene");
    case Kind::Web: return QStringLiteral("web");
    default: return QStringLiteral("unknown");
    }
}

Kind kindFromString(const QString &s)
{
    const QString l = s.toLower();
    if (l == "image") return Kind::Image;
    if (l == "video") return Kind::Video;
    if (l == "scene") return Kind::Scene;
    if (l == "web") return Kind::Web;
    return Kind::Unknown;
}

QJsonObject LibraryItem::toJson() const
{
    return {{"id", id}, {"title", title}, {"path", path}, {"preview", previewPath},
            {"type", contentType}, {"workshop_id", workshopId}};
}

} // namespace wpe
