// SPDX-License-Identifier: GPL-3.0-or-later
#include "workshop.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QUrlQuery>

namespace wpe {

static const QString kApi = QStringLiteral("https://api.steampowered.com");

static quint64 asU64(const QJsonValue &v) // Steam sends numbers as either strings or numbers
{
    return v.isString() ? v.toString().toULongLong() : static_cast<quint64>(v.toDouble());
}

static WorkshopItem parseFile(const QJsonObject &f)
{
    WorkshopItem it;
    it.id = f.value("publishedfileid").toString();
    it.title = f.value("title").toString(QStringLiteral("Untitled"));
    it.previewUrl = f.value("preview_url").toString();
    it.description = f.value("short_description").toString();
    for (const QJsonValue &t : f.value("tags").toArray()) it.tags << t.toObject().value("tag").toString();
    for (const QString &t : std::as_const(it.tags)) {
        const QString l = t.toLower();
        if (l == "video" || l == "scene" || l == "web" || l == "application") { it.contentType = t; break; }
    }
    if (it.contentType.isEmpty()) it.contentType = QStringLiteral("unknown");
    it.subscriptions = asU64(f.value("subscriptions"));
    it.favorited = asU64(f.value("favorited"));
    const QJsonObject vote = f.value("vote_data").toObject();
    it.votesUp = asU64(vote.value("votes_up"));
    it.votesDown = asU64(vote.value("votes_down"));
    it.score = vote.value("score").toDouble();
    it.timeUpdated = asU64(f.value("time_updated"));
    it.timeCreated = asU64(f.value("time_created"));
    it.fileSize = asU64(f.value("file_size"));
    it.views = asU64(f.value("views"));
    return it;
}

WorkshopClient::WorkshopClient(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<wpe::WorkshopItem>();
    qRegisterMetaType<QList<wpe::WorkshopItem>>();
}

void WorkshopClient::search(const QString &apiKey, const QString &query, int page, const QString &sort, const QString &typeFilter)
{
    if (apiKey.isEmpty()) { emit failed(tr("Steam API key not set. Add it in Settings.")); return; }

    const QString queryType = sort == "new" ? "1" : sort == "trend" ? "9" : sort == "subscriptions" ? "12" : "0";
    QUrl url(kApi + "/IPublishedFileService/QueryFiles/v1/");
    QUrlQuery q;
    q.addQueryItem("key", apiKey);
    q.addQueryItem("query_type", queryType);
    q.addQueryItem("page", QString::number(page));
    q.addQueryItem("numperpage", "24");
    q.addQueryItem("appid", QString::number(kWallpaperEngineAppId));
    for (const char *k : {"return_vote_data", "return_tags", "return_previews", "return_metadata"})
        q.addQueryItem(k, "1");
    q.addQueryItem("search_text", query);
    if (!typeFilter.isEmpty() && typeFilter != "all") q.addQueryItem("required_tags", typeFilter);
    url.setQuery(q);

    const quint64 gen = ++m_generation;
    QNetworkReply *r = m_nam.get(QNetworkRequest(url));
    connect(r, &QNetworkReply::finished, this, [=, this] {
        r->deleteLater();
        if (gen != m_generation) return;
        if (r->error() != QNetworkReply::NoError) {
            const int code = r->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            emit failed(code == 403 ? tr("Steam rejected the API key (HTTP 403).") : tr("Steam API request failed: %1").arg(r->errorString()));
            return;
        }
        const QJsonObject resp = QJsonDocument::fromJson(r->readAll()).object().value("response").toObject();
        QList<WorkshopItem> items;
        for (const QJsonValue &v : resp.value("publishedfiledetails").toArray()) {
            WorkshopItem it = parseFile(v.toObject());
            if (!it.previewUrl.isEmpty()) items << it;
        }
        emit searchFinished(page, items, asU64(resp.value("total")));
    });
}

void WorkshopClient::details(const QString &apiKey, const QString &itemId)
{
    if (apiKey.isEmpty()) { emit failed(tr("Steam API key not set.")); return; }
    QNetworkRequest req(QUrl(kApi + "/ISteamRemoteStorage/GetPublishedFileDetails/v1/"));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QUrlQuery body;
    body.addQueryItem("key", apiKey);
    body.addQueryItem("itemcount", "1");
    body.addQueryItem("publishedfileids[0]", itemId);
    QNetworkReply *r = m_nam.post(req, body.toString(QUrl::FullyEncoded).toUtf8());
    connect(r, &QNetworkReply::finished, this, [=, this] {
        r->deleteLater();
        if (r->error() != QNetworkReply::NoError) { emit failed(tr("Steam API request failed: %1").arg(r->errorString())); return; }
        const QJsonArray a = QJsonDocument::fromJson(r->readAll()).object().value("response").toObject().value("publishedfiledetails").toArray();
        if (a.isEmpty()) { emit failed(tr("Item not found")); return; }
        emit detailsFinished(parseFile(a.first().toObject()));
    });
}

} // namespace wpe
