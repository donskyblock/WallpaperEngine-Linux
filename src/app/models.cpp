// SPDX-License-Identifier: GPL-3.0-or-later
#include "models.h"

#include <QUrl>

namespace wpe {

// ---- WorkshopModel ---------------------------------------------------------------------
QHash<int, QByteArray> WorkshopModel::roleNames() const
{
    return {{IdRole, "itemId"}, {TitleRole, "title"}, {PreviewRole, "previewUrl"}, {TypeRole, "contentType"},
            {ScoreRole, "score"}, {SubsRole, "subscriptions"}, {UpRole, "votesUp"}, {DownRole, "votesDown"},
            {SizeRole, "fileSize"}, {DescRole, "description"}, {TagsRole, "tags"}, {UpdatedRole, "timeUpdated"},
            {ViewsRole, "views"}, {FavRole, "favorited"}, {DownloadedRole, "downloaded"}};
}

QVariant WorkshopModel::data(const QModelIndex &i, int role) const
{
    if (!i.isValid() || i.row() >= m_items.size()) return {};
    const WorkshopItem &it = m_items[i.row()];
    switch (role) {
    case IdRole: return it.id;
    case TitleRole: case Qt::DisplayRole: return it.title;
    case PreviewRole: return it.previewUrl;
    case TypeRole: return it.contentType;
    case ScoreRole: return it.score;
    case SubsRole: return QVariant::fromValue<qulonglong>(it.subscriptions);
    case UpRole: return QVariant::fromValue<qulonglong>(it.votesUp);
    case DownRole: return QVariant::fromValue<qulonglong>(it.votesDown);
    case SizeRole: return QVariant::fromValue<qulonglong>(it.fileSize);
    case DescRole: return it.description;
    case TagsRole: return it.tags;
    case UpdatedRole: return QVariant::fromValue<qulonglong>(it.timeUpdated);
    case ViewsRole: return QVariant::fromValue<qulonglong>(it.views);
    case FavRole: return QVariant::fromValue<qulonglong>(it.favorited);
    case DownloadedRole: return m_downloaded.contains(it.id);
    }
    return {};
}

void WorkshopModel::reset(const QList<WorkshopItem> &items)
{
    beginResetModel();
    m_items = items;
    endResetModel();
}

void WorkshopModel::append(const QList<WorkshopItem> &items)
{
    if (items.isEmpty()) return;
    beginInsertRows({}, m_items.size(), m_items.size() + items.size() - 1);
    m_items += items;
    endInsertRows();
}

void WorkshopModel::setDownloaded(const QSet<QString> &ids)
{
    m_downloaded = ids;
    if (!m_items.isEmpty()) emit dataChanged(index(0), index(m_items.size() - 1), {DownloadedRole});
}

QVariantMap WorkshopModel::get(int row) const
{
    QVariantMap m;
    if (row < 0 || row >= m_items.size()) return m;
    const auto names = roleNames();
    for (auto it = names.begin(); it != names.end(); ++it) m[QString::fromUtf8(it.value())] = data(index(row), it.key());
    return m;
}

// ---- LibraryModel ----------------------------------------------------------------------
QHash<int, QByteArray> LibraryModel::roleNames() const
{
    return {{IdRole, "itemId"}, {TitleRole, "title"}, {PathRole, "path"}, {PreviewRole, "previewUrl"},
            {TypeRole, "contentType"}, {WorkshopIdRole, "workshopId"}, {ActiveRole, "active"}};
}

QVariant LibraryModel::data(const QModelIndex &i, int role) const
{
    if (!i.isValid() || i.row() >= m_visible.size()) return {};
    const LibraryItem &it = m_visible[i.row()];
    switch (role) {
    case IdRole: return it.id;
    case TitleRole: case Qt::DisplayRole: return it.title;
    case PathRole: return it.path;
    case PreviewRole: return it.previewPath.isEmpty() ? QUrl() : QUrl::fromLocalFile(it.previewPath);
    case TypeRole: return it.contentType;
    case WorkshopIdRole: return it.workshopId;
    case ActiveRole: return m_active.contains(it.path);
    }
    return {};
}

void LibraryModel::setItems(const QList<LibraryItem> &items)
{
    m_all = items;
    refilter();
}

void LibraryModel::setFilter(const QString &text, const QString &type)
{
    m_text = text;
    m_type = type;
    refilter();
}

void LibraryModel::refilter()
{
    beginResetModel();
    m_visible.clear();
    for (const LibraryItem &it : std::as_const(m_all)) {
        if (!m_type.isEmpty() && m_type != "all" && it.contentType != m_type.toLower()) continue;
        if (!m_text.isEmpty() && !it.title.contains(m_text, Qt::CaseInsensitive)) continue;
        m_visible << it;
    }
    endResetModel();
}

void LibraryModel::setActivePaths(const QSet<QString> &paths)
{
    m_active = paths;
    if (!m_visible.isEmpty()) emit dataChanged(index(0), index(m_visible.size() - 1), {ActiveRole});
}

QSet<QString> LibraryModel::workshopIds() const
{
    QSet<QString> s;
    for (const LibraryItem &it : m_all)
        if (!it.workshopId.isEmpty()) s.insert(it.workshopId);
    return s;
}

QVariantMap LibraryModel::get(int row) const
{
    QVariantMap m;
    if (row < 0 || row >= m_visible.size()) return m;
    const auto names = roleNames();
    for (auto it = names.begin(); it != names.end(); ++it) m[QString::fromUtf8(it.value())] = data(index(row), it.key());
    return m;
}

} // namespace wpe
