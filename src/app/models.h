// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "types.h"
#include "workshop.h"

#include <QAbstractListModel>
#include <QSet>

namespace wpe {

class WorkshopModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles { IdRole = Qt::UserRole + 1, TitleRole, PreviewRole, TypeRole, ScoreRole, SubsRole, UpRole, DownRole,
                 SizeRole, DescRole, TagsRole, UpdatedRole, ViewsRole, FavRole, DownloadedRole };
    explicit WorkshopModel(QObject *parent = nullptr) : QAbstractListModel(parent) {}

    int rowCount(const QModelIndex &p = {}) const override { return p.isValid() ? 0 : m_items.size(); }
    QVariant data(const QModelIndex &i, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void reset(const QList<WorkshopItem> &items);
    void append(const QList<WorkshopItem> &items);
    void setDownloaded(const QSet<QString> &ids);
    Q_INVOKABLE QVariantMap get(int row) const;
    int count() const { return m_items.size(); }

private:
    QList<WorkshopItem> m_items;
    QSet<QString> m_downloaded;
};

class LibraryModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles { IdRole = Qt::UserRole + 1, TitleRole, PathRole, PreviewRole, TypeRole, WorkshopIdRole, ActiveRole };
    explicit LibraryModel(QObject *parent = nullptr) : QAbstractListModel(parent) {}

    int rowCount(const QModelIndex &p = {}) const override { return p.isValid() ? 0 : m_visible.size(); }
    QVariant data(const QModelIndex &i, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setItems(const QList<LibraryItem> &items);
    void setFilter(const QString &text, const QString &type);
    void setActivePaths(const QSet<QString> &paths);
    QSet<QString> workshopIds() const;
    Q_INVOKABLE QVariantMap get(int row) const;

private:
    void refilter();
    QList<LibraryItem> m_all, m_visible;
    QString m_text, m_type;
    QSet<QString> m_active;
};

} // namespace wpe
