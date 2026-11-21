// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "settings.h"
#include <QNetworkAccessManager>
#include <QVariantList>
#include <functional>
namespace wpe {
class Studio : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList entries READ entries NOTIFY changed)
    Q_PROPERTY(bool busy READ busy NOTIFY changed)
    Q_PROPERTY(QString error READ error NOTIFY changed)
    Q_PROPERTY(QString catalogUrl READ catalogUrl NOTIFY changed)
public:
    explicit Studio(Settings *settings, QObject *parent = nullptr, QNetworkAccessManager *network = nullptr);
    QVariantList entries() const { return m_entries; }
    bool busy() const { return m_busy; }
    QString error() const { return m_error; }
    QString catalogUrl() const;
    Q_INVOKABLE void refreshCatalog(const QString &url);
    Q_INVOKABLE void install(int index);
    Q_INVOKABLE QString importMedia(const QString &url);
    Q_INVOKABLE QVariantMap loadProject(const QString &url);
    Q_INVOKABLE QString saveProject(const QVariantMap &project);
signals:
    void changed();
    void installed(QString path);
private:
    QString root() const;
    void fail(const QString &error);
    void fetch(const QUrl &url, qint64 limit, std::function<void(QByteArray, QString)> done);
    Settings *m_settings;
    QNetworkAccessManager m_ownedNetwork;
    QNetworkAccessManager *m_network;
    QVariantList m_entries;
    bool m_busy = false;
    QString m_error;
};
}
