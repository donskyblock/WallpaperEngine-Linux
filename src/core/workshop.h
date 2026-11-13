// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "types.h"

#include <QNetworkAccessManager>
#include <QObject>

namespace wpe {

class WorkshopClient : public QObject {
    Q_OBJECT
public:
    explicit WorkshopClient(QObject *parent = nullptr);

    // sort: vote | new | trend | subscriptions    typeFilter: "" | Video | Scene | Web
    void search(const QString &apiKey, const QString &query, int page, const QString &sort, const QString &typeFilter);
    void details(const QString &apiKey, const QString &itemId);

signals:
    void searchFinished(int page, QList<wpe::WorkshopItem> items, quint64 total);
    void detailsFinished(wpe::WorkshopItem item);
    void failed(QString message);

private:
    QNetworkAccessManager m_nam;
    quint64 m_generation = 0; // drop answers to superseded searches
};

} // namespace wpe
Q_DECLARE_METATYPE(wpe::WorkshopItem)
Q_DECLARE_METATYPE(QList<wpe::WorkshopItem>)
